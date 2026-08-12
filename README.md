# t32-cc 0.16.0 Division and Remainder Patch

Changed-files-only update for `toolchain/t32-cc`.

## New language forms

`t32-cc` now supports one binary `+`, `-`, `*`, `/`, or `%` expression. Each
operand may be an integer literal (including a negative literal) or the one
declared local integer.

Examples:

```c
return 42 / 6;
return x / 2;
return 42 / x;
x = x / 2;

return 20 % 6;
return x % 6;
x = x % 6;
```

Addition, subtraction, and multiplication support from earlier stages is
preserved unchanged.

## Division model

`/` lowers directly to T32 signed `DIV`:

```asm
div rd, ra, rb
```

The current T32 VM defines signed division as truncating toward zero and faults
on division by zero and on `INT32_MIN / -1`.

## Remainder model

T32 currently has no dedicated remainder instruction. `%` is therefore lowered
using the C identity that follows truncation-toward-zero division:

```text
a % b = a - (a / b) * b
```

The generated T32 sequence uses a scratch register for the quotient/product and
leaves the remainder in the requested destination register.

## Expression model

Stage 8 still intentionally permits exactly one binary arithmetic operator per
expression:

```text
operand
operand + operand
operand - operand
operand * operand
operand / operand
operand % operand
```

This completes the initial integer arithmetic family before the compiler moves
to a real precedence-aware expression parser.

## Deliberately deferred

- chained expressions
- mixed arithmetic expressions
- parentheses
- operator precedence and associativity
- multiple locals
- compile-time diagnosis of division by zero

## Test

From `O:\Foundry\t32\toolchain\t32-cc`:

```bat
make clean
make
make install
make test
```

The Stage 8 suite preserves all earlier compiler tests and adds signed division,
synthesized remainder, assignment, negative-operand, and chain-rejection
coverage.


## Stage 9: structured expressions

Version 0.8.0 replaces the one-binary-operator recognizer with a
precedence-aware recursive expression parser.

```c
return 2 + 3 * 4;      /* 14 */
return (2 + 3) * 4;    /* 20 */
x = x * 2 + 1;
```

`*`, `/`, and `%` bind more tightly than `+` and `-`. Operators at the same
precedence associate left-to-right. Parentheses create nested subexpressions
and override the normal precedence rules.

Code generation remains deliberately transparent: the expression tree is
evaluated with a small temporary-register stack. Register spilling/allocation
is a later compiler milestone.


## Stage 10: comparisons

Version 0.9.0 adds comparison expressions that produce ordinary integer
Boolean values (`0` or `1`):

```c
return 2 + 3 * 4 == 14;
return -5 < 3;
x = x >= 5;
```

The parser now adds C-style relational and equality precedence levels above
the arithmetic expression grammar. Comparisons are values, so they may be
returned, assigned, parenthesized, or used inside a larger arithmetic
expression.

Signed `<`, `<=`, `>`, and `>=` are lowered with explicit arithmetic and
bitwise operations that account for subtraction overflow. This intentionally
does not require a new conditional branch instruction or a decision about
exposing T32 condition flags to generated code.


## Stage 11: if / else

Version 0.10.0 introduces the first source-level control flow:

```c
int main(void)
{
    int x = 5;

    if (x < 10) {
        x = x + 1;
    } else {
        x = x - 1;
    }

    return x;
}
```

The condition is an ordinary integer expression. As in C, zero is false and
any nonzero value is true. The compiler evaluates the condition and emits T32's register-based `JZ`
to branch when the result is zero, with `JMP` used where needed to skip an
`else` arm.

Single-statement bodies, braced bodies, nested `if`, and `else if` all share
the same internal statement representation. `return` is deliberately still
restricted to the final top-level statement in `main`; general early returns
are a later control-flow milestone.


## Stage 12: while loops

Version 0.11.0 adds the first repetition construct:

```c
int main(void)
{
    int x = 0;

    while (x < 5)
        x = x + 1;

    return x;
}
```

A `while` condition is an ordinary integer expression. Zero exits the loop;
any nonzero value executes the body. The compiler emits a loop-head label,
evaluates the condition, uses T32 `JZ` to branch to the exit label when false,
executes the body, and uses `JMP` to return to the loop head.

Single-statement and braced bodies are supported, as are nested loops and
existing `if` / `else` statements inside a loop. `break`, `continue`,
`do/while`, and `for` remain later milestones.


## Stage 13: multiple locals

Version 0.12.0 replaces the single-local shortcut with a small local-symbol
table and a fixed stack-frame layout.

```c
int main(void)
{
    int x = 0;
    int sum = 0;

    while (x < 10) {
        sum = sum + x;
        x = x + 1;
    }

    return sum;
}
```

Each local receives a stable four-byte slot. The complete frame is allocated
once on entry and released once before `return`. Expressions resolve an
identifier to its slot index, and assignments store back to that same slot.

This stage deliberately keeps declarations at the beginning of `main` and
keeps initializers literal-only. General scope, shadowing, and expression
initializers are separate milestones.


## Stage 14: expression initializers

Version 0.13.0 allows a local initializer to be a normal expression:

```c
int x = 5;
int y = x + 3;
int z = (x * y) + 2;
```

Locals become visible after their initializer has been parsed. This gives a
simple declaration-order rule: later declarations can depend on earlier ones,
while self-reference and forward-reference are rejected.

The stack frame is allocated once, then each initializer is evaluated and
stored into its fixed local slot in declaration order.


## Stage 15: for / break / continue

Version 0.14.0 adds the conventional counted-loop form:

```c
for (x = 0; x < 10; x = x + 1) {
    sum = sum + x;
}
```

The initializer and update clauses are assignments to previously declared
locals, while the condition uses the normal expression grammar.

`break` and `continue` are resolved against the innermost active loop. A
`continue` inside `while` returns directly to the condition. A `continue`
inside `for` jumps to the update clause first, preserving normal C loop
semantics.


## 0.14.1 compatibility cleanup

Automatic locals no longer require an initializer, so ordinary C such as
`int x; x = 5;` is accepted. Storage is reserved for the local, but no
initialization value is invented.

`main` also follows the C rule that reaching its closing brace returns zero.
These are deliberately patch-level compatibility changes before 0.15.0
introduces general functions and parameters.


## Stage 16: functions and parameters

Version 0.15.0 introduces multiple integer functions and the first compiler-driven use of the T32 calling convention. The first four integer arguments are passed in `r0-r3`; integer results return in `r0`. Parameters are copied into each callee's fixed stack frame so the existing local/expression machinery can use them naturally. Function calls are intentionally complete expressions in this stage; more compositional/nested calls are reserved for the function-maturity stage.


## Stage 17: function maturity

Version 0.16.0 removes the Stage 16 restriction that a call had to occupy an
entire expression. Calls can now participate naturally in expressions:

```c
return add(1, 2) + 3;
return 10 + add(3, 4);
return add(twice(5), twice(7));
```

Argument values are staged in compiler-owned stack scratch slots before the
ABI registers are loaded, so nested calls do not destroy arguments that were
already evaluated.

Early returns now lower to a shared per-function epilogue:

```c
int choose(int x)
{
    if (x < 10)
        return 7;
    return 9;
}
```

This also provides the control-flow foundation needed for straightforward
recursive functions.

## 0.16.1 runtime external: putchar

The compiler now recognizes `putchar(int)` as the first known libt32 external
function. Calls use the existing r0-r3 argument ABI and return in r0. This is a
narrow runtime-linkage extension; string literals and pointers remain deferred.

## 0.17.0 string literal / puts milestone

`t32-cc` now accepts a string literal as an expression value, primarily to
support runtime calls such as:

```c
int rc;
rc = puts("Hello from T32");
```

The compiler emits the literal as zero-terminated bytes in `.data`, loads its
relocatable address into the normal argument register, and emits `call puts`.
`puts(int)` and `putchar(int)` are known libt32 externals.

This is intentionally not a complete C pointer model yet. Pointer
declarations, dereference, arrays, and general `char *` syntax remain separate
compiler milestones.

## 0.18.0 expression statements and explicit externals

Stage 18 removes the compiler's special knowledge of `puts` and `putchar`.

External target functions are now declared explicitly:

```c
extern int puts(int s);
```

The `int` parameter is a temporary ABI-level placeholder until the pointer/type
milestone introduces `char *`. The important change is that the function name
and arity now come from the source declaration rather than a hard-coded runtime
table inside `t32-cc`.

Stage 18 also adds expression statements, so this is legal:

```c
int main(void)
{
    puts("Hello");
    return 0;
}
```

The expression is evaluated using the ordinary expression/call machinery and
its result is discarded.

A referenced external declaration emits an unresolved `.extern` symbol for the
assembler/linker. Declared-but-unused externals do not create unnecessary
linker dependencies.

The next type-system milestone will replace the transitional integer-shaped
string parameter with genuine `char` and pointer types.


## 0.19.0 int-pointer / lvalue groundwork

Stage 19 introduces the first genuine pointer operations for local `int`
objects:

```c
int x = 42;
int *p = &x;
*p = 73;
return *p;
```

The compiler now distinguishes the address of a local object from the value
stored there. `&x` materializes the stack address of `x`; dereference loads
through a pointer with `LDW`; and `*p = value` stores through the pointer with
`STW`. This is intentionally the first narrow lvalue/rvalue milestone: only
local `int *` pointers are supported here. General pointer expressions,
`char *`, pointer arithmetic, arrays, and pointer parameters remain subsequent
stages.

## 0.20.0 char and byte-pointer milestone

`t32-cc 0.20.0` adds the first 8-bit C object type and typed byte pointers.

Supported examples now include:

```c
char c = 65;
char *p = &c;
*p = 66;
return *p;
```

A scalar `char` keeps a four-byte stack slot in the current simple stack-frame
layout, but stores and loads its object value with `STB`/`LDB`; assignment is
truncated to the low eight bits. This keeps stack layout intentionally simple
while giving `char` real byte-width object semantics.

Dereferencing `int *` continues to use `LDW`/`STW`. Dereferencing `char *`
uses `LDB`/`STB`, so pointee type now directly controls generated T32 memory
operations.

External declarations can describe byte pointers:

```c
extern int puts(char *s);
```

and a string literal can initialize a `char *` local:

```c
char *message = "Hello Thomas";
puts(message);
```

Pointer arithmetic, arrays, pointer parameters for user-defined functions, and
general type conversions remain later milestones.

## 0.22.0 typed pointer arithmetic

`t32-cc 0.22.0` adds pointer-plus-integer and pointer-minus-integer arithmetic
for the pointer types introduced in 0.19.0 and 0.20.0.

```c
char *cp;
int *ip;

cp = cp + 1;    /* address + 1 */
ip = ip + 1;    /* address + 4 */
```

The pointee type controls scaling:

```text
char * +/- n  -> address +/- n
int  * +/- n  -> address +/- (n * 4)
```

The resulting expression remains a pointer of the original pointee type, so a
later dereference continues to select the correct T32 memory operation:
`LDB/STB` for `char *`, and `LDW/STW` for `int *`.

Stage 21 deliberately supports the pointer on the left side only. Pointer to
pointer arithmetic and pointer differences remain unsupported; arrays are the
next natural layer above this groundwork.


## Stage 22: fixed-size local arrays

Stage 22 adds fixed-size local `int` and `char` arrays, array-to-pointer decay in expressions, and indexed loads/stores. `int` subscripts scale by four bytes; `char` subscripts scale by one byte. Array lengths must currently be positive integer constants and array initializers are intentionally deferred.

## 0.23.0 named structs and member access

`t32-cc 0.23.0` introduces the first aggregate C object type.

```c
struct point {
    int x;
    int y;
};

int main(void)
{
    struct point p;

    p.x = 10;
    p.y = 32;

    return p.x + p.y;
}
```

Struct layout is computed at compile time. `char` members occupy one byte;
`int` members are aligned to four-byte boundaries; and the total struct size is
rounded to four bytes. Member access therefore becomes a typed fixed offset
from the containing object's address.

For example:

```c
struct record {
    char tag;   /* offset 0 */
    int value;  /* offset 4 */
};
```

uses `LDB/STB` for `tag` and `LDW/STW` for `value`.

Stage 23 deliberately supports named struct definitions, local struct objects,
and the `.` operator only. Struct pointers / `->`, arrays of structs, nested
structs, whole-struct assignment, struct initializers, `typedef`, and `sizeof`
remain later milestones.

## 0.24.0 struct pointers and `->`

Stage 24 connects aggregate layout to the pointer machinery:

```c
struct point {
    int x;
    int y;
};

int main(void)
{
    struct point p;
    struct point *q = &p;

    q->x = 40;
    q->y = 2;

    return q->x + q->y;
}
```

A `struct T *` local occupies one machine word. `->member` loads the pointer,
adds the member's compile-time offset, then performs the member's typed memory
operation (`LDB/STB` for `char`, `LDW/STW` for `int`).

Stage 24 keeps the boundary deliberately small: `.` is for struct objects and
`->` is for struct pointers. Arrays of structs, nested structs, whole-struct
assignment, struct initializers, `typedef`, and `sizeof` remain later work.

## 0.24.1 struct-member regression fix

0.24.1 is a surgical correction to the 0.24.0 `struct *` / `->` milestone.

The 0.24.0 implementation temporarily reused the statement `pointer_local`
field as a Boolean marker for `->`. That field uses `-1` as its normal
"not-a-pointer-local" sentinel, and nonzero values are true in C. As a result,
ordinary `object.member = value` stores could incorrectly enter the
pointer-member code-generation path.

0.24.1 gives struct-member expressions and stores a dedicated
`member_base_is_pointer` flag. `.` and `->` now share member layout/type
metadata without sharing unrelated parser sentinel fields.
