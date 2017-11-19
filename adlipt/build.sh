#!/bin/sh
set -e

VERSION_MAJOR=0
VERSION_MINOR=4

CC='wcc -bt=dos -zq -oxhs'
CC32='wcc386 -mf -zl -zls -zq -oxhs'
AS='wasm -zq'
DEFS="-dVERSION_MAJOR=$VERSION_MAJOR -dVERSION_MINOR=$VERSION_MINOR"
#DEFS="$DEFS -dDEBUG"

set -x
$CC $DEFS adlipt.c
$CC $DEFS res_opl2.c
$AS $DEFS res_glue.s
$AS $DEFS res_end.s
wlink @adlipt.wl

$CC32 $DEFS jadlipt.c
$CC32 $DEFS -fo=jlm_opl2.o res_opl2.c
wlink @jadlipt.wl
python3 patchpe.py jadlipt.dll
