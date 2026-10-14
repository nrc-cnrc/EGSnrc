# Incremental Mortran 3

This directory contains an incremental development copy of the Mortran 3
translator. It is kept separate from `../mortran3`, which contains the
translator used by the existing EGSnrc build, so changes here can be developed
and tested without replacing that version. The Fortran source includes named
indices for the translator's O-table buffers.

Build the standalone executable with GNU Fortran and `make`:

```sh
make
```

By default, the executable is written to `../bin/linux/mortran3_inc.exe`. The
Makefile uses `../lib/linux/machine.f`; override `MACHINE_SOURCE` or
`MORTRAN_EXE` to use different paths.
