; libt32 string contracts -- T32 ABI 0.1
; strlen : r0=str -> r0=length
; strcpy : r0=dst r1=src -> r0=dst
; strncpy: r0=dst r1=src r2=count -> r0=dst
; strcmp : r0=left r1=right -> r0=difference
; strncmp: r0=left r1=right r2=count -> r0=difference
; strchr : r0=str r1=value -> r0=match or zero
; strstr : r0=haystack r1=needle -> r0=match or zero
; strrev : r0=mutable string -> r0=original pointer
; All routines preserve r8-r15.
