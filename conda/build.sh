#!/bin/bash
#export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib"

find /usr/local -name Python.h
cd underworld/libUnderworld
#./configure.py --prefix=$CONDA_PREFIX || (cat config.log && ls /lib exit 1)
./configure.py || (cat config.log && ls /lib exit 1)
./compile.py
./scons.py install 
