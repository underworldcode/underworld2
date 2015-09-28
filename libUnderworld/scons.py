#!/usr/bin/env python
import os, sys, subprocess

sconsBin = os.path.join('config', 'scons', 'scons.py')

subp = subprocess.Popen(
    sconsBin + ' ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
