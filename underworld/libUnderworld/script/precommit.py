#!/usr/bin/python

"""
 pre commit mercurial script
 to strip IPython Notebooks in the InputFiles dir

 requires 'nbstripout.py' to be located repoRoot/.hg/  

"""

import os, subprocess

script_path="./libUnderworld/script/nbstripout.py"

# parse dir structure under InputFiles
for path, dis, files in os.walk("./InputFiles"):
  for file in files:
    if file.endswith(".ipynb"):
      abp = os.path.join(path,file) # make absolute path

      # check if abp is tracked
      out = subprocess.check_output(["hg","st",abp]) 
      if out.startswith(("M", "A")): # if so we strip it
         print "Removing output from "+ abp
         subprocess.call( [script_path, abp] )

