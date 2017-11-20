#!/bin/bash
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
cp opl2test/README.md "$DIR/opl2test.txt"

rm -f opl2util.zip
zip -9j opl2util.zip "$DIR"/*

rm -rf "$DIR"
