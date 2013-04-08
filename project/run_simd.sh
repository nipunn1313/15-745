#!/bin/bash

## Optimizes a simd program with ready to use passes:

echo "Compiling and optimizing for SIMD"

# Compile the test to generate llvm bitcode
clang -O0 -emit-llvm -o simd.bc -c simd.c

# Compile the bit code into unoptimized x86 binary :
clang simd.bc -o unoptimized_simd
#chmod +x unoptimized_simd

# Optimize llvm bit code into optimized bitcode using llvm passes :
opt -force-vector-width=4 -debug -mem2reg  -loop-simplify -loop-idiom -lcssa -sink -simplifycfg -loop-vectorize -o simd.opt.bc simd.bc

# Compile optimized bitcode into optimized x86 binary :
clang simd.opt.bc -o optimized_simd
#chmod +x optimized_simd

echo "********************************************************************************"
echo "Running unoptimized trace"

# Run the tests on valgrind :
valgrind --tool=lackey ./unoptimized_simd

echo "********************************************************************************"
echo "Running optimized trace"
valgrind --tool=lackey ./optimized_simd
