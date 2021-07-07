#!/bin/sh

# ensure we bail on errors
set -e

export UW_LONGTEST=1
./docs/development/run_tests.py ./docs/examples/* ./docs/user_guide/* docs/test/*
./docs/development/run_tests.py --prepend="mpirun -np 2" ./docs/examples/*
./docs/development/run_tests.py --prepend="mpirun -np 4" ./docs/examples/*
