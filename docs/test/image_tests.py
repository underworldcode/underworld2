#!/usr/bin/env python
import os
import importlib
import imp
path = os.path.abspath('./image_tests/')

for d in os.listdir(path):
    testd = os.path.join(path,d)
    if not os.path.isdir(testd): continue
    if str(d)[0] == '.': continue
    modfile = os.path.join(path, str(d) + '/runtest.py')
    if not os.path.isfile(modfile): continue;
    os.chdir(testd)
    print "Running tests in " + os.getcwd()
    print "==================================================="
    #print modfile
    testmod = imp.load_source('runtest', modfile)
    os.chdir(path)
    print "==================================================="

