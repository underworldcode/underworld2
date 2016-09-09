#!/bin/sh

# ensure we bail on errors
set -e

export PYTHONPATH=$PWD:$PYTHONPATH

cd utils
./run_tests.py ../docs/examples/1_0* ../docs/test/* ../docs/user_guide/*