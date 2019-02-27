#!/bin/sh

# ensure we bail on errors
set -e

export PYTHONPATH=$PWD:$PYTHONPATH

./docs/development/run_tests.py ./docs/examples/*.ipynb ./docs/user_guide/*.ipynb ./docs/test/*
