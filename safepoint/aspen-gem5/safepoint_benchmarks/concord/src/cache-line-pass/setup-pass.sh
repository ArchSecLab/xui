#!/bin/bash
LLVM_VERSION=11

rm -rf build
mkdir -p build
cd build
# cmake -DLLVM_DIR=/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/ ..
cmake -DLLVM_DIR=/usr/lib/llvm-${LLVM_VERSION}/cmake/  ..
make
cd ..

