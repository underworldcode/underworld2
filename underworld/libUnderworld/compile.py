#!/usr/bin/env python3
import os, sys, subprocess

# stuff for remote builds
build_success_file='.build_success'
# remove 'success' file
if os.path.exists(build_success_file):
    os.remove(build_success_file)

# call scons once
sconsBin = 'python3 `which scons`'
returncode = subprocess.call( sconsBin + ' ' + ' '.join(sys.argv[1:]), shell=True )
if returncode != 0:
    sys.exit(returncode)

# run swig generation - must go down a directory
os.chdir('libUnderworldPy')
returncode = subprocess.call( 'python3 ./swigall.py', shell=True )
os.chdir('..')
if returncode != 0:
    sys.exit(returncode)

# return to previous directory and call scons twice
returncode = subprocess.call( sconsBin + ' ' + ' '.join(sys.argv[1:]), shell=True )

# So we can easily check if build was successful remotely
if returncode == 0: # success
    open(build_success_file, 'w').close() 
sys.exit(returncode)
