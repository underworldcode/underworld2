#!/usr/bin/env python3
import os
import runpy
#import imp
import ntpath
from inspect import getsourcefile
# change dir to where this test lives
ntpath.os.chdir(ntpath.dirname(getsourcefile(lambda:0)))

path = os.path.abspath('./image_tests/')

#Check if the viewer is working
import underworld.visualisation as vis
if not vis.lavavu:
    print("Image tests skipped, Viewer disabled")
    exit()
try:
    import nbconvert
except ModuleNotFoundError:
    print("Skipping image_test.py as nbconvert can't be found")
    exit()

for d in os.listdir(path):
    testd = os.path.join(path,d)
    if not os.path.isdir(testd): continue
    if str(d)[0] == '.': continue
    filepath = os.path.join(path, str(d) + '/runtest.py')
    if not os.path.isfile(filepath): continue;
    os.chdir(testd)
    print("Running tests in " + os.getcwd())
    print("===================================================")
    #print filepath
    runpy.run_path(filepath)
    #testmod = imp.load_source('runtest', filepath)
    os.chdir(path)
    print("===================================================")

