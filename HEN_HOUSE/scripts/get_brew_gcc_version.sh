#!/bin/sh
###############################################################################
#
#  EGSnrc helper: highest Homebrew GCC version with a full compiler triplet
#  Copyright (C) 2026 National Research Council Canada
#
#  Prints the version suffix N (e.g. 14) when gcc-N, g++-N, and gfortran-N
#  are all executable under a Homebrew bin directory. Exits 1 if none found.
#
###############################################################################

best_ver=
vers=

for brew_bin in /opt/homebrew/bin /usr/local/bin; do
    test -d "$brew_bin" || continue
    for gcc_exe in "$brew_bin"/gcc-[0-9]*; do
        test -x "$gcc_exe" || continue
        ver=$(basename "$gcc_exe" | sed 's/^gcc-//')
        case $ver in
            ''|*[!0-9]*) continue ;;
        esac
        test -x "$brew_bin/g++-$ver" || continue
        test -x "$brew_bin/gfortran-$ver" || continue
        vers="$vers $ver"
    done
done

if test "x$vers" = x; then
    exit 1
fi

best_ver=$(echo "$vers" | tr ' ' '\n' | sort -n | tail -1)
echo "$best_ver"
