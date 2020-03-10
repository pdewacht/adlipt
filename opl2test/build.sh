#!/bin/sh
set -e

VERSION=20181028

CXX='wpp -bt=dos -zq -oxhs'
DEFS="-dVERSION=$VERSION"

set -x
$CXX $DEFS -fo=opl2main main.cpp
$CXX $DEFS -fo=opl3main -dOPL3 main.cpp
$CXX $DEFS -zm OPL2.cpp
$CXX $DEFS demotune.cpp
$CXX $DEFS timer.cpp
wlink name opl2test system dos file opl2main,OPL2,demotune,timer option eliminate,quiet,map
wlink name opl3test system dos file opl3main,OPL2,demotune,timer option eliminate,quiet,map

$CXX $DEFS oplreset.cpp
wlink name oplreset system dos file oplreset,OPL2 option eliminate,quiet,map
