#!/usr/bin/env python
import os, sys, subprocess
build_success_file='.build_success'
# remove 'success' file
if os.path.exists(build_success_file):
    os.remove(build_success_file)

sconsBin = os.path.join('config', 'scons', 'scons.py')

subp = subprocess.Popen(
    sconsBin + ' ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# So we can easily check if build was successful remotely
if subp.returncode == 0: # success
    open(build_success_file, 'w').close() 
# return the return code
sys.exit(subp.returncode)
