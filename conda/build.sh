#!/bin/bash
#export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib"

cd underworld/libUnderworld
#./configure.py --prefix=$CONDA_PREFIX || (cat config.log && ls /lib exit 1)
./configure.py || (cat config.log && ls /lib exit 1)
./compile.py
./scons.py install 
