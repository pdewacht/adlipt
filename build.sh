#!/bin/sh
set -e

VERSION_MAJOR=0
VERSION_MINOR=3

CC='wcc -bt=dos -zq -oxhs'
AS='wasm -zq'
DEFS="-dVERSION_MAJOR=$VERSION_MAJOR -dVERSION_MINOR=$VERSION_MINOR"

set -x
$CC $DEFS main.c
$CC $DEFS res_opl2.c
$AS $DEFS res_glue.s
$AS $DEFS res_end.s
wlink @adlipt.wl
