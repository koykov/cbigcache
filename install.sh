#!/usr/bin/env bash

if (( $EUID != 0 )); then
    echo "Please run as root or sudo." >&2
    exit 1
fi

echo "Prepare workspace ..."
rm -rf build
mkdir build
cd build
echo "done"

echo "Build library ..."
cmake ..
make
echo "done"

echo "Clean up LD cache ..."
ldconfig
echo "done"

echo "Deploy ..."
cp -f libcbigcache.so /usr/lib64
echo "done"
