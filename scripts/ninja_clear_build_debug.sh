#!/bin/bash
rm -rf build/debug
mkdir -p build/debug
cmake -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -B build/debug -S .
cd build/debug
ninja
