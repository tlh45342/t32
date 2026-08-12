; T32 RTC MMIO v0.1 platform integration test
; r4 = RTC ID
; r5 = RTC STATUS
; r6 = first EPOCH
; r7 = second EPOCH

.org 0x00001000
_start:
    movi r0, 0x90003000
    ldw  r4, [r0]

    mov  r1, r0
    addi r1, r1, 4
    ldw  r5, [r1]

    mov  r1, r0
    addi r1, r1, 8
    ldw  r6, [r1]
    ldw  r7, [r1]

    halt
