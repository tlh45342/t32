; T32 BIOS 0.0.6
;
; First real disk bootstrap firmware.
;
; Boot contract:
;   - firmware executes at 0x00001000
;   - disk0 uses the synchronous 512-byte MMIO controller at 0x90001000
;   - disk0 must contain a T32D v0.1 image
;   - BOOT.BIN is located by scanning the fixed T32D directory
;   - BOOT.BIN is loaded at 0x00010000
;   - Bootinfo v0.2 is built at 0x00002000
;   - BIOS service ABI v0.1 exposes disk_read at fixed entry 0x00001008
;   - r0 points to Bootinfo when control transfers to 0x00010000
;
; T32D v0.1:
;   LBA 0      header
;   LBA 1..4   32 directory entries, 64 bytes each
;   LBA 8..    file data
;
; Disk MMIO:
;   +0x04 STATUS   bit0=attached, bit1=ready, bit2=error
;   +0x08 COMMAND  1=read
;   +0x0c LBA
;   +0x10 ERROR    0=none
;   +0x100 DATA    512-byte sector buffer

.org 0x00001000

; Reset entry is deliberately one fixed JMP instruction. This makes the first
; firmware service entry stable at 0x00001008 without requiring a linker.
start:
    jmp bios_start

; ---------------------------------------------------------------------------
; BIOS service ABI v0.1 -- disk_read
; Fixed entry: 0x00001008
;
;   r0 = disk number (only disk0 is currently valid)
;   r1 = LBA
;   r2 = destination RAM address
;
; Return:
;   r0 = 0 success, 1 error
;
; Caller may assume r8-r14 are preserved. r0-r7 are volatile.
; The service copies exactly one 512-byte sector into guest RAM.
; ---------------------------------------------------------------------------
bios_service_disk_read:
    jnz  r0, bios_service_disk_read_failed

    movi r3, 0x9000100C
    stw  r1, [r3]
    movi r3, 0x90001008
    movi r4, 1
    stw  r4, [r3]

    movi r3, 0x90001010
    ldw  r4, [r3]
    jnz  r4, bios_service_disk_read_failed

    movi r3, 0x90001100
    movi r5, 512
bios_service_disk_copy:
    ldb  r6, [r3]
    stb  r6, [r2]
    addi r3, r3, 1
    addi r2, r2, 1
    subi r5, r5, 1
    jnz  r5, bios_service_disk_copy

    movi r0, 0
    ret

bios_service_disk_read_failed:
    movi r0, 1
    ret

bios_start:
    ; CALL/RET use r15 as the stack pointer. Keep the small firmware stack
    ; below the BOOT.BIN load address.
    movi r15, 0x0000F000

    ; Clear the canonical 80x25 byte-per-cell text display.
    movi r4, 0x90000000
    movi r0, 32
    movi r1, 2000
clear_loop:
    stb  r0, [r4]
    addi r4, r4, 1
    subi r1, r1, 1
    jnz  r1, clear_loop

    ; BIOS banner. r4 is the display cursor used by print_string.
    movi r4, 0x90000000
    movi r0, msg_banner
    call print_string

    ; Require attached, ready disk0.
    movi r1, 0x90001004
    ldw  r2, [r1]
    movi r3, 3
    and  r2, r2, r3
    movi r3, 3
    xor  r2, r2, r3
    jnz  r2, no_disk

    ; READ LBA 0 (T32D header).
    movi r8, 0
    call disk_read_sector
    jnz  r0, disk_error

    ; Verify magic "T32D".  The guest is little-endian, so bytes
    ; 54 33 32 44 appear as word 0x44323354.
    movi r1, 0x90001100
    ldw  r2, [r1]
    movi r3, 0x44323354
    xor  r2, r2, r3
    jnz  r2, bad_format

    ; Require T32D version 0.1.
    addi r1, r1, 4
    ldb  r2, [r1]
    jnz  r2, bad_format
    addi r1, r1, 1
    ldb  r2, [r1]
    movi r3, 1
    xor  r2, r2, r3
    jnz  r2, bad_format

    ; Require the canonical 512-byte sector size.
    movi r1, 0x90001108
    ldw  r2, [r1]
    movi r3, 512
    xor  r2, r2, r3
    jnz  r2, bad_format

    ; Validate the fixed v0.1 directory geometry, then read its LBA.
    movi r1, 0x90001114
    ldw  r9, [r1]
    movi r3, 32
    xor  r2, r9, r3
    jnz  r2, bad_format

    addi r1, r1, 4
    ldw  r2, [r1]
    movi r3, 64
    xor  r2, r2, r3
    jnz  r2, bad_format

    movi r1, 0x90001110
    ldw  r8, [r1]             ; current directory LBA

scan_directory_sector:
    jz   r9, boot_not_found
    call disk_read_sector
    jnz  r0, disk_error

    movi r5, 0x90001100       ; current directory entry
    movi r6, 8                ; entries in this directory sector

scan_directory_entry:
    jz   r9, boot_not_found

    ; Compare first eight bytes with "BOOT.BIN".
    ldw  r1, [r5]
    movi r2, 0x544F4F42       ; "BOOT"
    xor  r1, r1, r2
    jnz  r1, next_directory_entry

    addi r7, r5, 4
    ldw  r1, [r7]
    movi r2, 0x4E49422E       ; ".BIN"
    xor  r1, r1, r2
    jnz  r1, next_directory_entry

    ; Require a NUL immediately after BOOT.BIN so BOOT.BIN.OLD cannot match.
    addi r7, r5, 8
    ldb  r1, [r7]
    jnz  r1, next_directory_entry

    ; Found it. Save start LBA and byte length before the next disk read
    ; replaces the controller's sector buffer.
    addi r7, r5, 32
    ldw  r10, [r7]            ; BOOT.BIN start LBA
    addi r7, r7, 4
    ldw  r11, [r7]            ; BOOT.BIN byte length
    call build_bootinfo
    jmp  load_boot_file

next_directory_entry:
    addi r5, r5, 64
    subi r6, r6, 1
    subi r9, r9, 1
    jnz  r6, scan_directory_entry

    addi r8, r8, 1
    jmp  scan_directory_sector

load_boot_file:
    ; Empty BOOT.BIN is not bootable.
    jz   r11, boot_not_found

    ; sectors = ceil(byte_length / 512)
    addi r12, r11, 511
    movi r13, 512
    divu r12, r12, r13

    movi r14, 0x00010000      ; destination in guest RAM

load_boot_sector:
    mov  r8, r10
    call disk_read_sector
    jnz  r0, disk_error

    movi r5, 0x90001100       ; MMIO sector source
    movi r6, 512
copy_sector:
    ldb  r1, [r5]
    stb  r1, [r14]
    addi r5, r5, 1
    addi r14, r14, 1
    subi r6, r6, 1
    jnz  r6, copy_sector

    addi r10, r10, 1
    subi r12, r12, 1
    jnz  r12, load_boot_sector

    ; BOOT.BIN is a flat binary assembled for this fixed load address.
    ; Boot ABI v0.2 passes r0 = pointer to the Bootinfo record.
    movi r0, 0x00002000
    jmp  0x00010000

; ---------------------------------------------------------------------------
; build_bootinfo
;   writes the fixed 72-byte Bootinfo v0.2 record at 0x00002000
;   clobbers r0-r3, r5
; ---------------------------------------------------------------------------
build_bootinfo:
    movi r5, 0x00002000

    ; +0x00 magic = "T32B"
    movi r1, 0x42323354
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x04 structure size = 72 bytes
    movi r1, 72
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x08/+0x0c version = 0.2
    movi r1, 0
    stw  r1, [r5]
    addi r5, r5, 4
    movi r1, 2
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x10 flags: bit0 = boot disk valid, bit1 = BIOS services available
    movi r1, 3
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x14 RAM base = 0
    movi r1, 0
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x18 RAM size discovered from platform MMIO.
    movi r2, 0x9000400C
    ldw  r1, [r2]
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x1c boot disk = disk0
    movi r1, 0
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x20 sector size and +0x24 sector count from disk0 MMIO.
    movi r2, 0x90001014
    ldw  r1, [r2]
    stw  r1, [r5]
    addi r5, r5, 4
    addi r2, r2, 4
    ldw  r1, [r2]
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x28 text framebuffer base, +0x2c columns, +0x30 rows.
    movi r1, 0x90000000
    stw  r1, [r5]
    addi r5, r5, 4
    movi r1, 80
    stw  r1, [r5]
    addi r5, r5, 4
    movi r1, 25
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x34 disk MMIO, +0x38 platform MMIO, +0x3c boot entry.
    movi r1, 0x90001000
    stw  r1, [r5]
    addi r5, r5, 4
    movi r1, 0x90004000
    stw  r1, [r5]
    addi r5, r5, 4
    movi r1, 0x00010000
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x40 BIOS service ABI version = 0.1 encoded as 0x00000001.
    movi r1, 1
    stw  r1, [r5]
    addi r5, r5, 4

    ; +0x44 fixed BIOS disk_read service entry.
    movi r1, 0x00001008
    stw  r1, [r5]

    ret

; ---------------------------------------------------------------------------
; disk_read_sector
;   input:  r8 = LBA
;   output: r0 = 0 success, 1 error
;   clobbers r1-r3
; ---------------------------------------------------------------------------
disk_read_sector:
    movi r1, 0x9000100C
    stw  r8, [r1]

    movi r1, 0x90001008
    movi r2, 1
    stw  r2, [r1]

    ; The 0.0.6 controller is synchronous. Validate ERROR after command.
    movi r1, 0x90001010
    ldw  r2, [r1]
    jnz  r2, disk_read_failed

    movi r0, 0
    ret

disk_read_failed:
    movi r0, 1
    ret

; ---------------------------------------------------------------------------
; print_string
;   input: r0 = NUL-terminated string address
;          r4 = display cursor
;   output: r4 advanced past string
;   clobbers r1
; ---------------------------------------------------------------------------
print_string:
    ldb  r1, [r0]
    jz   r1, print_string_done
    stb  r1, [r4]
    addi r0, r0, 1
    addi r4, r4, 1
    jmp  print_string
print_string_done:
    ret

no_disk:
    movi r4, 0x900000A0
    movi r0, msg_no_disk
    call print_string
    halt

bad_format:
    movi r4, 0x900000A0
    movi r0, msg_bad_format
    call print_string
    halt

boot_not_found:
    movi r4, 0x900000A0
    movi r0, msg_no_boot
    call print_string
    halt

disk_error:
    movi r4, 0x900000A0
    movi r0, msg_disk_error
    call print_string
    halt

msg_banner:
    .ascii "T32 BIOS 0.0.6"
    .byte 0
msg_no_disk:
    .ascii "Disk0 not present"
    .byte 0
msg_bad_format:
    .ascii "Disk0 not T32D v0.1"
    .byte 0
msg_no_boot:
    .ascii "BOOT.BIN not found"
    .byte 0
msg_disk_error:
    .ascii "Disk0 read error"
    .byte 0
