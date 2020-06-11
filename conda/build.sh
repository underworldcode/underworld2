#!/bin/bash
export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$PREFIX/lib"

cd underworld/libUnderworld
./configure.py --prefix=$PREFIX || (cat config.log && ls /lib exit 1)
./compile.py
./scons.py install 