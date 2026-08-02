# t32-cc

Native C shim for the future T32 C compiler.

This iteration only proves that `t32-cc` can be built, run, tested, and
installed as a native executable. It does not compile C yet.

## Linux

```bash
make
make test
sudo make install
```

Installs:

```text
/usr/local/bin/t32-cc
```

## Windows

```bat
make
make test
make install
```

Default install location:

```text
C:/Program Files/libvm/bin/t32-cc.exe
```

Override the prefix when needed:

```bat
make PREFIX="C:/Program Files/t32" install
```

## Current behavior

```bash
t32-cc
t32-cc --version
t32-cc --help
t32-cc hello.c
```

The final form is reserved for the future compiler pipeline.
