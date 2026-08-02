# Object-library status

`libt32 0.0.2` builds individual T32OBJ modules with:

```text
t32-as -f obj
```

It validates them with `t32-nm` and links them directly with `t32-ld`.

The next phase is `t32-ar`, after which this repository can activate:

```text
make archive
libt32.a
```
