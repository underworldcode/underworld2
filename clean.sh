#!/usr/bin/env bash

set -x  # show commands

rm -fr build

find src -name '*.so' -exec rm {} +
find src -name '*.__pycache__' -exec rm -r {} +
find src -name '*.egg-info' -exec rm -r {} +
rm -rf .pytest_cache
## To remove untracked git files
#git clean -dfX ; git clean -dfx
