#!/bin/sh
set -e

(cd adlipt && ./build.sh)
(cd adpatch && ./build.sh)
(cd opl2test && ./build.sh)

DIR=$(mktemp -d)
cp adlipt/adlipt.exe "$DIR/"
cp adlipt/jadlipt.dll "$DIR/"
cp adlipt/README.md "$DIR/adlipt.txt"
cp adpatch/adpatch.exe "$DIR/"
cp adpatch/README.md "$DIR/adpatch.txt"
cp opl2test/opl2test.exe "$DIR/"
cp opl2test/opl3test.exe "$DIR/"
cp opl2test/opl2test.mp3 "$DIR/"
cp opl2test/oplreset.exe "$DIR/"
cp opl2test/README.md "$DIR/opl2test.txt"

rm -f oplutil.zip
zip -9j oplutil.zip "$DIR"/*

rm -rf "$DIR"
