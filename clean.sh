#!/usr/bin/env bash

set -x  # show commands

rm -fr build

rm -rf .pytest_cache

# remove install distinct-id
rm -f src/underworld/_uwid.py

find . -name '__pycache__' -exec rm -r {} +

find docs -name '.ipynb_checkpoints' -exec rm -r {} +

find src -name '*.egg-info' -exec rm -r {} +
find src -name '*.so' -exec rm {} +
## To remove untracked git files
#git clean -dfX ; git clean -dfx
