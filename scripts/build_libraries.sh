#!/bin/bash
set -e

echo "=========================================="
echo "  Building and installing libraries"
echo "  (one time, or when libraries change)"
echo "=========================================="

INSTALL_DIR="${PWD}/build/libs"
rm -rf "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# Сборка и установка
mkdir -p build/libs_build
cd build/libs_build

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebugDLL -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -B . -S ../../external

ninja
ninja install

echo ""
echo "=========================================="
echo "  Libraries installed: $INSTALL_DIR"
echo "=========================================="
echo ""
