;
; T32 C runtime startup
;
; ABI 0.1 entry point for standalone flat-binary programs.
;
; Entry:
;     Machine state is otherwise unspecified.
;
; Actions:
;     Establish r15 as the initial stack pointer.
;     Call the externally supplied main function.
;     Preserve main's return value in r0.
;     Halt the machine.
;
; Exit:
;     r0  = main return value
;     r15 = STACK_TOP
;

.section .text
.global _start
.extern main

.equ STACK_TOP, 0x0000F000

_start:
    movi r15, STACK_TOP
    call main
    halt
