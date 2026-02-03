#!/bin/bash
rm -rf build/release
mkdir -p build/release
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build/release -S .
cd build/release
ninja
