#!/usr/bin/env bash

echo "Prepare library"

cd ../lib/ffmpeg-9.0

./configure --prefix=$(pwd)/dist --enable-shared --disable-static --disable-x86asm

make -j$(nproc)

make install