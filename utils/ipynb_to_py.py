#!/bin/sh
#################################################################
# This script converts jupyter notebook files to standard python.
# Generated python files are created alongside their corresponding
# notebook file.
#
# Usage: `ipynb_to_py.py foo.ipynb [bar.ipynb [...]]`
#################################################################


jupyter nbconvert --to python --template=python.tpl $*
