#! /usr/bin/env bash

set -e
mkdir -p build/user_guide
cd build/user_guide
cp ../../../../user_guide/*.ipynb .
find . -name \*.ipynb -print0 | xargs -0 jupyter-nbconvert --to rst
find . -name \*.ipynb -print0 | xargs -0 rm
find . -name \*.rst   -print0 | xargs -0 python ../../switch_code_blocks.py
