# t32-cc 0.4.0

## Added

- One binary `+` expression.
- Literal-plus-literal expressions.
- Local-plus-literal and literal-plus-local expressions.
- Local-plus-local expressions.
- Addition expressions in assignments.
- Repeated increment validation.
- Negative-literal addition validation.

## Preserved

- `-S`, `-c`, and full-link modes.
- ABI 0.1 stack restoration.
- Quiet success and opt-in `-v` output.
- Existing constant, local, and assignment behavior.
- Failure cleanup and missing-runtime diagnostics.

## Deferred

- Chained expressions and precedence.
- Parenthesized expressions.
- Operators other than `+`.
