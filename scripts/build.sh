#!/usr/bin/env bash

echo "Start build solution..."

cd ..

mkdir -p build

cd build

cmake --build . --parallel $(($(nproc) - 1))