#!/usr/bin/env sh
set -e
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ -z "$PREFIX" ]; then PREFIX="$HOME/.local"; fi
if [ -z "$CC" ]; then CC=cc; fi
mkdir -p "$PREFIX/bin"
"$CC" -I"$HERE/include" -O2 -std=c11 -Wall -Wextra -Wpedantic -Werror   "$HERE/src/qnova.c" "$HERE/src/main.c" -lm -o "$PREFIX/bin/qnova"
printf 'installed %s
' "$PREFIX/bin/qnova"
"$PREFIX/bin/qnova" limits
