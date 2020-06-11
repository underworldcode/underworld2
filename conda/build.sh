#!/bin/bash

cd underworld/libUnderworld
./configure.py --prefix=$PREFIX || (cat config.log && exit 1)
./compile.py
./scons.py install 