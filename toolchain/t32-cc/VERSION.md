# t32-cc 0.16.0

Stage 17 matures the first T32 function-call model.

New capabilities:
- function calls as ordinary expression operands
- calls on either side of arithmetic/comparison expressions
- nested function calls
- function calls used as arguments to other calls
- early `return` from conditional and loop bodies
- multiple return paths with a shared function epilogue
- generated recursive calls (recursion groundwork)
- parameters and ordinary locals used together

Caller-saved argument/result registers are protected during nested expression
evaluation by compiler-owned scratch slots in the current stack frame. Calls
still use up to four ABI register arguments (`r0`-`r3`) and return through `r0`.

Still deferred:
- arguments beyond four / stack-passed compiler arguments
- function prototypes separate from definitions
- richer type system
- pointer parameters
