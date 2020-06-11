#!/bin/bash

cd underworld/libUnderworld
./configure.py --prefix=$PREFIX
./compile.py
./scons.py install