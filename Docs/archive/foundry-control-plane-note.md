# Archived Foundry Control-Plane Note

This note was previously stored in the T32 documentation as `tre important.md`. It concerns Foundry tenancy and control-plane architecture rather than the T32 ISA, so it has been preserved here as non-normative project context.

## Two audiences

Foundry must distinguish:

1. Infrastructure administrators who can view and manage the full inventory.
2. Tenants/users who should see only resources they own or are permitted to access.

The same conceptual list operation may return different inventory based on authorization.

## Registry-centered architecture

Foundry should be the authority for VM inventory. Nodes execute VMs and report state; clients should not need to query every node individually.

```text
                Foundry
                   |
             VM Registry
                   |
    +--------------+--------------+
    |              |              |
 t32-node1     t32-node2     shim-node
```

The preferred model is:

```text
VM is owned by Foundry.
Foundry schedules the VM.
A node executes the VM.
```

The node is an implementation and scheduling detail rather than the tenant's primary object.

## Ownership metadata

A VM record should include ownership and placement information, for example:

```json
{
  "id": "vm-1234",
  "owner": "tom",
  "name": "hello",
  "kind": "t32",
  "node": "t32-node-01",
  "status": "stopped"
}
```

## Administrative and tenant CLIs

The note anticipates two command surfaces:

```text
vmctl       tenant/user operations
foundryctl  infrastructure operations
```

Examples:

```text
vmctl vm list
foundryctl node list
foundryctl node drain
foundryctl node evacuate
foundryctl storage list
foundryctl network list
```

This is a useful Foundry design decision, but it should eventually live in the Foundry repository rather than the T32 ISA documentation.
