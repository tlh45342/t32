# Archived Foundry Control-Plane Note

This note is preserved as non-normative project context.

Foundry should be authoritative for VM inventory. Nodes report and execute state; clients should not query every node individually.

```text
VM is owned by Foundry.
Foundry schedules the VM.
A node executes the VM.
```

VM records should carry ownership and placement metadata, for example:

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

Tenant and administrator views may use different command surfaces and authorization filters.
