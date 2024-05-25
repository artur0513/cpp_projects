#!/bin/sh
cd "$(dirname "$0")" || exit
cmake -B build -S src
make -C ./build
