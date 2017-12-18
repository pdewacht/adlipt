#!/bin/sh
set -e

VERSION=20171218

CC='wcc -bt=dos -zq -oxhs'
DEFS="-dVERSION=$VERSION"

set -x
python3 genpat.py > patterns.rl
ragel -F1 patch.rl
$CC $DEFS main.c
$CC $DEFS patch.c
wlink name adpatch system dos file main,patch option quiet
