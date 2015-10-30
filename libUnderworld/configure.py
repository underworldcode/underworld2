#!/usr/bin/env python
import sys, subprocess, shutil, os
subp = subprocess.Popen('git submodule update --init', shell=True)  #Checkout submodules (LavaVu)
subp.wait()
subp = subprocess.Popen(
    'config/scons/scons.py --config=force -f SConfigure ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
