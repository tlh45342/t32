; libt32 memory routine contracts
;
; memset : r0=dst r1=value r2=count -> r3=original dst
; memcpy : r0=dst r1=src   r2=count -> r3=original dst
; memmove: r0=dst r1=src   r2=count -> r3=original dst
; memcmp : r0=left r1=right r2=count -> r3=byte difference
; memchr : r0=ptr r1=value r2=count -> r3=match pointer or zero
