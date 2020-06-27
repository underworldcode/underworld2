#!/bin/bash
#export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib"

find ${BUILD_PREFIX} -name Python.h
python -c "from distutils.sysconfig import get_python_inc, get_python_lib; print(get_python_inc()); print(get_python_lib())"
cd underworld/libUnderworld
#./configure.py --prefix=$CONDA_PREFIX || (cat config.log && ls /lib exit 1)
./configure.py || (cat config.log && ls /lib exit 1)
./compile.py
./scons.py install 
