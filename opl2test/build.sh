#!/bin/sh
set -e

VERSION=20171119

CXX='wpp -bt=dos -zq -oxhs'
DEFS="-dVERSION=$VERSION"

set -x
$CXX $DEFS main.cpp
$CXX $DEFS OPL2.cpp
$CXX $DEFS demotune.cpp
$CXX $DEFS timer.cpp
wlink name opl2test system dos file main,OPL2,demotune,timer option quiet,map
