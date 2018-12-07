#! /usr/bin/env bash

set -e
mkdir -p ../content/docs/UserGuide
cd ../content/docs/UserGuide
cp ../../../../../user_guide/*.ipynb .
find . -name \*.ipynb  -print0 | xargs -0 jupyter-nbconvert --to markdown
find . -name \*.ipynb  -print0 | xargs -0 rm