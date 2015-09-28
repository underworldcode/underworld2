#!/bin/sh
rm -rf CMakeFiles
rm CMakeCache.txt
cmake .
make -j4
