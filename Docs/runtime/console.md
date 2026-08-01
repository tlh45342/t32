# T32 Console Runtime

## Layering

```text
printf-like formatting
        ↓
puts / print_string
        ↓
putchar
        ↓
console MMIO or firmware service
        ↓
VCONSOLE host implementation
```

## Recommended progression

1. `putchar`
2. `puts`
3. unsigned decimal output
4. hexadecimal output
5. pointer output
6. tiny formatted output

## Tiny formatted output

The first formatter should support only:

```text
%c
%s
%u
%x
%%
```

Width, precision, floating point, locale, positional arguments, and full ISO C compatibility are explicitly out of scope for the first version.
