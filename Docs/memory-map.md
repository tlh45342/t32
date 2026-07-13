Right now this can be intentionally short.

```
Current Test Environment

Load Address

    0x1000

Entry Point

    0x1000

Future

ROM
RAM
video memory
I/O
exception vectors
```

Notice it explicitly says that **0x1000 is currently a testing convention**, not an architectural requirement. That gives us room for `t32-ld` later.