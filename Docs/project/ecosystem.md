# T32 in the Foundry Ecosystem

T32 is a processor architecture and software target.

Foundry is the control plane that owns virtual-machine inventory, scheduling, lifecycle, and access policy.

A node executes a VM; it does not define ownership of that VM.

```text
Foundry owns VM records
        ↓
Foundry schedules placement
        ↓
t32-node executes the machine
        ↓
VCONSOLE and other services expose devices
```

Tenant and infrastructure administration should remain distinct:

```text
vmctl       tenant-facing VM lifecycle
foundryctl  infrastructure and node administration
```

This separation keeps T32 architecture documentation independent from control-plane policy.
