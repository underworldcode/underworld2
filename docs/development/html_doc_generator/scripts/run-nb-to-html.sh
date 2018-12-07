#! /usr/bin/env bash


set -e
cd $(dirname "$0")/..
cd content/docs

cp -r notebooks nb_html

find nb_html -name \*.ipynb  -print0 | xargs -0 jupyter-nbconvert --to html
find nb_html -name \*.ipynb  -print0 | xargs -0 rm







# #! /usr/bin/env python
#
# import os
# import inspect
# from glob import glob
#
# this_script = os.path.abspath(inspect.stack()[0][1])
# script_dir  = os.path.dirname(os.path.abspath(this_script))
# root_dir    = os.path.dirname(os.path.abspath(script_dir))
# nb_dir      = os.path.join(root_dir, "content", "docs", "notebooks")
#
# #
# result = []
#
# for x in os.walk(nb_dir):
#     for y in glob(os.path.join(x[0], '*.ipynb')):
#         result.append(y)
#
# for nb in result:
#     print nb
