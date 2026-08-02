; libt32 memory contracts -- T32 ABI 0.1
; memset : r0=dst r1=value r2=count -> r0=dst
; memcpy : r0=dst r1=src r2=count -> r0=dst
; memmove: r0=dst r1=src r2=count -> r0=dst
; memcmp : r0=left r1=right r2=count -> r0=difference
; memchr : r0=ptr r1=value r2=count -> r0=match or zero
; All routines preserve r8-r15.
