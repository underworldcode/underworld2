#!/usr/bin/env python
import sys, subprocess, shutil, os
subp = subprocess.Popen('git submodule update --init', shell=True, cwd=os.path.join(os.path.dirname(__file__), '../'))  #Checkout submodules (LavaVu)
subp.wait()
if subp.wait() != 0:
    raise RuntimeError("Unable to access submodule required for Underworld.\n\nNote that if you wish to "\
                       "compile Underworld, you must obtain the project via the 'git clone' command:\n"\
                       "     git clone https://github.com/underworldcode/underworld2.git")
subp = subprocess.Popen(
    'scons --config=force -f SConfigure ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
