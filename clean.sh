#!/usr/bin/env bash

set -x  # show commands

rm -fr build
find . -name \*.so -exec rm {} +
find . -name __pycache__ -exec rm -r {} +
find . -name '*.egg-info' -exec rm -r {} +
rm -rf .pytest_cache
## To remove untracked git files
#git clean -dfX ; git clean -dfx
