# t32-cc 0.2.0 — First Local Variable

Stage 3 preserves the Stage 2 compiler driver and adds one initialized local `int`.

Supported forms:

```c
int main(void) { return 42; }

int main(void) {
    int x = 5;
    return x;
}
```

The local variable is deliberately stored in a four-byte stack slot. The compiler emits allocation, store, load, release, and `RET` rather than optimizing the variable away. This makes the first symbol/storage model directly inspectable.

Commands remain:

```text
t32-cc -S main.c
t32-cc -c main.c
t32-cc main.c
t32-cc -v main.c
```
