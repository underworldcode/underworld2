#!/bin/bash
#export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib"

# test
cd underworld/libUnderworld
export PETSC_DIR=$PREFIX
#./configure.py --prefix=$CONDA_PREFIX || (cat config.log && ls /lib exit 1)
./configure.py || (cat config.log && ls /lib exit 1)
./compile.py
./scons.py install 
