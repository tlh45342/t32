; T32 BOOT 0.0.4
;
; Boot ABI v0.2 consumer and NEXT.BIN stage-3 loader.
;
; Entry contract:
;   r0 = pointer to 72-byte Bootinfo v0.2 record
;   current BIOS places the record at 0x00002000
;
; BOOT validates the handoff before using machine information.

.org 0x00010000

start:
    ; Boot ABI v0.2 promises only r0. Establish our own temporary stack.
    movi r15, 0x0000F000

    ; Save the handoff pointer before using r0 as scratch.
    mov  r8, r0

    ; Validate Bootinfo magic "T32B".
    ldw  r1, [r8]
    movi r2, 0x42323354
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; size == 72
    addi r3, r8, 4
    ldw  r1, [r3]
    movi r2, 72
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; version == 0.2
    addi r3, r3, 4
    ldw  r1, [r3]
    jnz  r1, bad_bootinfo
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 2
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; flags bit0 says boot disk is valid; bit1 says BIOS services exist.
    addi r3, r3, 4
    ldw  r7, [r3]
    mov  r1, r7
    movi r2, 1
    and  r1, r1, r2
    jz   r1, bad_bootinfo

    ; RAM base must be zero and RAM size must be nonzero.
    addi r3, r3, 4
    ldw  r1, [r3]
    jnz  r1, bad_bootinfo
    addi r3, r3, 4
    ldw  r1, [r3]
    jz   r1, bad_bootinfo

    ; boot disk must be disk0.
    addi r3, r3, 4
    ldw  r1, [r3]
    jnz  r1, bad_bootinfo

    ; sector size must match the current T32 disk ABI.
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 512
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; sector count must be nonzero.
    addi r3, r3, 4
    ldw  r1, [r3]
    jz   r1, bad_bootinfo

    ; Validate canonical text framebuffer geometry.
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 0x90000000
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 80
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 25
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; Validate MMIO bases and the entry address recorded by BIOS.
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 0x90001000
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 0x90004000
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 0x00010000
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; BIOS services are optional in Bootinfo v0.2. If flag bit1 is clear,
    ; the remaining service fields are informational zeros and BOOT continues.
    mov  r1, r7
    movi r2, 2
    and  r1, r1, r2
    jz   r1, bootinfo_valid

    ; BIOS service ABI version must be 0.1.
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 1
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; disk_read service is the fixed v0.1 entry at 0x00001008.
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 0x00001008
    xor  r1, r1, r2
    jnz  r1, bad_bootinfo

    ; Prove the firmware service boundary: read disk0 LBA 0 through BIOS into
    ; ordinary RAM, then validate the T32D fingerprint there.
    movi r0, 0
    movi r1, 0
    movi r2, 0x00003000
    call 0x00001008
    jnz  r0, bios_service_failed

    movi r3, 0x00003000
    ldw  r1, [r3]
    movi r2, 0x44323354
    xor  r1, r1, r2
    jnz  r1, bios_service_failed

    ; Make the stage-2 handoff visible before loading the next stage.
    call show_boot_status

    ; Use only the BIOS disk service from this point forward. Parse the T32D
    ; header copied into ordinary RAM and locate NEXT.BIN in the fixed v0.1
    ; directory. r8-r14 survive BIOS service calls by contract.
    movi r3, 0x00003010
    ldw  r10, [r3]            ; directory LBA
    addi r3, r3, 4
    ldw  r9, [r3]             ; directory entry count
    movi r2, 32
    xor  r1, r9, r2
    jnz  r1, next_bad_format
    addi r3, r3, 4
    ldw  r1, [r3]             ; directory entry size
    movi r2, 64
    xor  r1, r1, r2
    jnz  r1, next_bad_format

next_scan_sector:
    jz   r9, next_not_found
    movi r0, 0
    mov  r1, r10
    movi r2, 0x00003000
    call 0x00001008
    jnz  r0, bios_service_failed

    movi r5, 0x00003000
    movi r6, 8

next_scan_entry:
    jz   r9, next_not_found

    ; Guest filename is exactly NEXT.BIN.
    ldw  r1, [r5]
    movi r2, 0x5458454E       ; "NEXT"
    xor  r1, r1, r2
    jnz  r1, next_entry
    addi r3, r5, 4
    ldw  r1, [r3]
    movi r2, 0x4E49422E       ; ".BIN"
    xor  r1, r1, r2
    jnz  r1, next_entry
    addi r3, r5, 8
    ldb  r1, [r3]
    jnz  r1, next_entry

    addi r3, r5, 32
    ldw  r11, [r3]            ; NEXT.BIN start LBA
    addi r3, r3, 4
    ldw  r12, [r3]            ; NEXT.BIN byte length
    jz   r12, next_not_found

    ; sectors = ceil(byte_length / 512)
    addi r12, r12, 511
    movi r13, 512
    divu r12, r12, r13
    movi r13, 0x00020000      ; stage-3 load address

next_load_sector:
    movi r0, 0
    mov  r1, r11
    mov  r2, r13
    call 0x00001008
    jnz  r0, bios_service_failed
    addi r11, r11, 1
    addi r13, r13, 512
    subi r12, r12, 1
    jnz  r12, next_load_sector

    ; Stage-3 ABI v0.1 mirrors the firmware handoff: r0 points to Bootinfo.
    movi r0, 0x00002000
    jmp  0x00020000

next_entry:
    addi r5, r5, 64
    subi r6, r6, 1
    subi r9, r9, 1
    jnz  r6, next_scan_entry
    addi r10, r10, 1
    jmp  next_scan_sector

bootinfo_valid:
    call show_boot_status
    halt

; show_boot_status
;   Displays the BOOT/Bootinfo/service checkpoint without changing the
;   stage-3 loader's callee-saved state.
show_boot_status:
    ; Clear display.
    movi r4, 0x90000000
    movi r0, 32
    movi r1, 2000
show_clear_loop:
    stb  r0, [r4]
    addi r4, r4, 1
    subi r1, r1, 1
    jnz  r1, show_clear_loop

    movi r4, 0x90000000
    movi r0, msg_banner
    call print_string

    movi r4, 0x900000A0
    movi r0, msg_bootinfo_ok
    call print_string

    mov  r1, r7
    movi r2, 2
    and  r1, r1, r2
    jz   r1, show_skip_service_banner
    movi r4, 0x90000140
    movi r0, msg_bios_service_ok
    call print_string

show_skip_service_banner:
    movi r4, 0x900001E0
    movi r0, msg_hello
    call print_string
    ret

next_bad_format:
    movi r4, 0x90000000
    movi r0, msg_next_bad_format
    call print_string
    halt

next_not_found:
    movi r4, 0x90000000
    movi r0, msg_next_not_found
    call print_string
    halt

bad_bootinfo:
    movi r4, 0x90000000
    movi r0, msg_bad_bootinfo
    call print_string
    halt

bios_service_failed:
    movi r4, 0x90000000
    movi r0, msg_bad_service
    call print_string
    halt

; print_string
;   r0 = NUL-terminated string address
;   r4 = display cursor
;   clobbers r1
print_string:
    ldb  r1, [r0]
    jz   r1, print_string_done
    stb  r1, [r4]
    addi r0, r0, 1
    addi r4, r4, 1
    jmp  print_string
print_string_done:
    ret

msg_banner:
    .ascii "T32 BOOT 0.0.4"
    .byte 0
msg_bootinfo_ok:
    .ascii "BOOTINFO v0.2 OK"
    .byte 0
msg_bios_service_ok:
    .ascii "BIOS DISK SERVICE v0.1 OK"
    .byte 0
msg_hello:
    .ascii "Hello from BOOT.BIN"
    .byte 0
msg_bad_bootinfo:
    .ascii "BOOTINFO v0.2 INVALID"
    .byte 0

msg_bad_service:
    .ascii "BIOS DISK SERVICE v0.1 FAILED"
    .byte 0

msg_next_bad_format:
    .ascii "T32D DIRECTORY INVALID"
    .byte 0
msg_next_not_found:
    .ascii "NEXT.BIN NOT FOUND"
    .byte 0
