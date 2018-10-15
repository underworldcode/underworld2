#!/usr/bin/env python
import sys, subprocess
subp = subprocess.Popen(
    'scons --config=force -f SConfigure ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
