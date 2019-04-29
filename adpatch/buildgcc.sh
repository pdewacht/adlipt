#!/bin/sh
set -e

eval $(grep ^VERSION= build.sh)
DEFS="-DVERSION=$VERSION"

python3 genpat.py
ragel -s -G2 scan.rl
cc -Wall $DEFS -o adpatch -fsanitize=undefined,address main.c patch.c scan.c
