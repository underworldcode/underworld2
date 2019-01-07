#! /usr/bin/env bash

set -e
mkdir -p user_guide
cd user_guide
cp ../../../user_guide/*.ipynb .
#find . -name \*.md    -print0 | xargs -0 rm
find . -name \*.ipynb -print0 | xargs -0 jupyter-nbconvert --to markdown
find . -name \*.ipynb -print0 | xargs -0 rm
find . -name \*.md    -print0 | xargs -0 sed -i "s/\`\`\`python/\`\`\`TOREPLACE/g"
