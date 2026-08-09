# T32 Supervisor and Firmware Services

## Status

T32 currently has `TRAP` and `IRET`, but no frozen supervisor-call ABI.

## Initial direction

Early software may expose a very small service table for diagnostics and bootstrapping. Candidate services include:

```text
putchar
puts
getchar
exit
read time
basic disk read
```

Service numbers, argument registers, return values, and error reporting must be versioned.

## Design requirements

A service contract must define:

- invocation instruction;
- service number location;
- argument registers;
- return registers;
- error convention;
- preserved registers;
- privilege transition, if any;
- exception frame;
- reentrancy and interrupt behavior.

## Boundary

Firmware services are not a substitute for a general operating-system ABI. They should remain small and stable.


## BIOS service ABI v0.1

The first concrete firmware service is now defined by Boot ABI v0.2.

`disk_read` is entered at `0x00001008` and reads one 512-byte sector from
disk0 into caller-supplied RAM. Its register convention is documented in
`docs/abi/T32-BOOT-ABI.md`.

This is a firmware bootstrap service, not the final application-facing SVC
ABI. `TRAP` semantics remain unfrozen and are not changed by this milestone.
