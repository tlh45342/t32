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
