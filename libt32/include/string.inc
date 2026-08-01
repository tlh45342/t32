; libt32 string routine contracts
;
; strlen : r0=str -> r1=length
; strcpy : r0=dst r1=src -> r2=original dst
; strncpy: r0=dst r1=src r2=count -> r3=original dst
; strcmp : r0=left r1=right -> r2=byte difference
; strncmp: r0=left r1=right r2=count -> r3=byte difference
; strchr : r0=str r1=value -> r2=match pointer or zero
; strstr : r0=haystack r1=needle -> r2=match pointer or zero
; strrev : r0=mutable string -> r1=original pointer
