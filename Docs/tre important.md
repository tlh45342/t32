Some of this is going to depend on how we want to view things as an administrator that has agency over the domain/hardware and as an end user to which that should be transparent.  Let me ask it this way if I was using a vsphere cli how do I list all VMs regardless of node.  And how as a versphere end user how would I list all vms I have?

I actually think you just stumbled onto one of the most important design decisions for Foundry.

There are **two audiences**:

1. **Infrastructure Administrator** (owns the hardware)
2. **Tenant / User** (owns the VMs)

VMware makes this distinction even if you don't always notice it.

------

## Infrastructure administrator

The administrator thinks:

> "Show me everything."

PowerCLI:

```
Get-VM
```

returns every VM the administrator is permitted to see across the entire vCenter inventory.

You can then do things like:

```
Get-VM | Select Name, PowerState, VMHost
```

Example:

```
Name        Host       State
------------------------------------
mail01      esx01      PoweredOn
db01        esx01      PoweredOff
build01     esx03      PoweredOn
test17      esx05      PoweredOn
```

Notice the important part:

**The VM already knows which node owns it.**

The administrator doesn't have to ask each ESXi host individually.

------

## End user

An ordinary user doesn't think in terms of ESXi hosts.

They think:

```
Show me my VMs.
```

PowerCLI still uses:

```
Get-VM
```

but RBAC filters the result.

If I own three machines, I see:

```
hello
ubuntu
compiler
```

I never see:

```
finance-db
exchange01
payroll
```

because permissions hide them.

------

# This suggests Foundry's architecture

I actually would **not** have:

```
Foundry
    |
    +-- ask node1
    +-- ask node2
    +-- ask node3
```

every time someone types `vmctl list`.

Instead:

```
                Foundry
                   |
          VM Registry (database)
                   |
    +--------------+--------------+
    |              |              |
 t32-node1     t32-node2     shim-node
```

Foundry is the authority.

Nodes simply report status.

------

## Administrator

```
vmctl vm list
```

returns

```
VM ID        NAME      OWNER   NODE        STATE
------------------------------------------------
1A4F....     hello     tom     node-01     running
9B22....     db        alice   node-02     stopped
...
```

------

## User

Exactly the same command:

```
vmctl vm list
```

returns

```
NAME
--------
hello
compiler
test
```

because Foundry filtered the results.

------

## The node shouldn't matter

This is the important conceptual leap.

Today we're thinking:

```
VM
    lives
        on node
```

I think the model should be

```
VM
    is owned by Foundry

Foundry
    schedules VM

Node
    executes VM
```

That's a Kubernetes mindset.

The node becomes almost an implementation detail.

------

# Which means...

I think we should actually start recording an **owner**.

Today we have:

```
{
    "id":"vm-1234",
    "name":"hello",
    "kind":"t32"
}
```

I'd already change it to

```
{
    "id":"vm-1234",
    "owner":"tom",
    "name":"hello",
    "kind":"t32",
    "node":"t32-node-01",
    "status":"stopped"
}
```

Notice the order.

The owner comes before the node.

------

## Even more interesting...

I think you're eventually going to have **two CLIs** without realizing it.

One:

```
vmctl
```

for tenants.

Another:

```
foundryctl
```

for infrastructure.

For example:

```
foundryctl node list
foundryctl node drain
foundryctl node evacuate
foundryctl storage list
foundryctl network list
```

versus

```
vmctl vm list
vmctl create
vmctl start
vmctl stop
vmctl console
```

Those are two very different viewpoints.

------

This is one place where I would consciously borrow from VMware and Kubernetes. Users shouldn't care *where* their VM runs. Administrators care very much *which* node is executing it, how healthy that node is, and what capacity remains. Keeping those perspectives separate from the beginning will make Foundry feel much more coherent as it grows. I think that's one of the architectural decisions that will pay dividends years from now.