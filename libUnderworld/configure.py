#!/usr/bin/env python
import sys, subprocess
subp = subprocess.Popen(
'config/scons/scons.py --config=force -f SConfigure ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
