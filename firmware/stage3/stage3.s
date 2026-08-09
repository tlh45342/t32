; T32 C STAGE3 0.0.4 startup
;
; Stage-3 ABI v0.1:
;   load/entry address 0x00020000
;   r0 = pointer to Bootinfo v0.2
;   all other general-purpose registers unspecified
;
; This shim establishes the C execution environment, validates the handoff,
; calls compiler-generated main(), verifies its r0 return value, and reports
; success on the text console.
;
; It deliberately keeps machine-specific setup outside C for this first
; mixed-language milestone.

.section .text
.global _start
.extern main

_start:
    ; Stage 3 owns its stack; never inherit BOOT's stack state.
    movi r15, 0x0000E000
    mov  r8, r0

    ; Validate Bootinfo magic "T32B".
    ldw  r1, [r8]
    movi r2, 0x42323354
    xor  r1, r1, r2
    jnz  r1, bad_handoff

    ; Bootinfo v0.2 is 72 bytes.
    addi r3, r8, 4
    ldw  r1, [r3]
    movi r2, 72
    xor  r1, r1, r2
    jnz  r1, bad_handoff

    ; Validate version 0.2.
    addi r3, r3, 4
    ldw  r1, [r3]
    jnz  r1, bad_handoff
    addi r3, r3, 4
    ldw  r1, [r3]
    movi r2, 2
    xor  r1, r1, r2
    jnz  r1, bad_handoff

    ; Show that the mixed-language stage has actually started.
    movi r4, 0x90000280
    movi r0, msg_banner
    call print_string

    movi r4, 0x90000320
    movi r0, msg_loaded
    call print_string

    ; The existing t32-cc ABI returns int values in r0.
    call main
    mov  r9, r0

    movi r1, 42
    xor  r1, r9, r1
    jnz  r1, bad_c_return

    movi r4, 0x900003C0
    movi r0, msg_return
    call print_string

    movi r4, 0x90000460
    movi r0, msg_handoff
    call print_string

    ; Preserve main()'s return code as the final visible register result.
    mov  r0, r9
    halt

bad_handoff:
    movi r4, 0x90000280
    movi r0, msg_bad_handoff
    call print_string
    movi r0, 1
    halt

bad_c_return:
    movi r4, 0x900003C0
    movi r0, msg_bad_return
    call print_string
    mov  r0, r9
    halt

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
    .ascii "T32 C STAGE3 0.0.4"
    .byte 0
msg_loaded:
    .ascii "NEXT.BIN C strings + libt32 puts"
    .byte 0
msg_return:
    .ascii "C main() returned 42"
    .byte 0
msg_handoff:
    .ascii "Bootinfo v0.2 handoff OK"
    .byte 0
msg_bad_handoff:
    .ascii "C STAGE3 HANDOFF INVALID"
    .byte 0
msg_bad_return:
    .ascii "C main() RETURN FAILED"
    .byte 0
