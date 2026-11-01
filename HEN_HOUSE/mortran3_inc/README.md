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

Run the local reference regression tests with:

```sh
tests/run_mortran_regression.sh
```

Use `-h`/`--help` to show options or `-y`/`--yes` to run without the
confirmation prompt. By default, the script shows the reference projects and
asks before building or translating. It then reports progress and separate
comparison results for each project.
Generated Fortran is compared byte-for-byte; listing content is compared after
normalizing only date/time stamps. The default reference tree is
`~/egs/000/egs_home`; set `EGS_MORTRAN_GOLDEN_ROOT` to use another directory.
Set `EGS_MORTRAN_ASSUME_YES=1` to skip the prompt in automated runs. Additional
options and environment variables are listed by `tests/run_mortran_regression.sh --help`.
