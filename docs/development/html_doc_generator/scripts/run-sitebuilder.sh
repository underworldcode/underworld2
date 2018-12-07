#!/bin/sh
#

set -e
cd $(dirname "$0")/..

# One shot build in cwd

# find content/docs/notebooks -name \*.ipynb  -print0 | xargs -0 jupyter-nbconvert --to html 

mkdocs build --clean
