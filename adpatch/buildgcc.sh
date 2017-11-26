#!/bin/sh
set -e

eval $(grep ^VERSION= build.sh)
DEFS="-DVERSION=$VERSION"

python3 genpat.py > patterns.rl
ragel -F1 patch.rl
cc -Wall -O2 $DEFS -o adpatch main.c patch.c
