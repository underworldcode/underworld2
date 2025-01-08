#!/usr/bin/env bash -x

rm -fr build
find . -name \*.so -exec rm {} +
find . -name __pycache__ -exec rm -r {} +
find . -name '*.egg-info' -exec rm -r {} +
rm -rf .pytest_cache
## To remove git 
#git clean -dfX ; git clean -dfx
