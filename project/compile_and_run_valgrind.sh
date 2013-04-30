#!/bin/bash

# Argument should be something like test1, test2 etc
test=$1

echo "clang ${test}.bc -o ${test}.unopt"
clang ${test}.bc -o ${test}.unopt

echo "clang ${test}.opt.bc -o ${test}.opt"
clang ${test}.opt.bc -o ${test}.opt
echo "***************************************************************"

echo "valgrind --tool=lackey ./${test}.unopt"
valgrind --tool=lackey ./${test}.unopt
echo "***************************************************************"

echo "valgrind --tool=lackey ./${test}.opt"
valgrind --tool=lackey ./${test}.opt
