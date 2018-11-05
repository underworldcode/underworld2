#!/usr/bin/env python3
import sys, subprocess
subp = subprocess.Popen(
    'python3 `which scons` --config=force -f SConfigure ' + ' '.join(sys.argv[1:]), shell=True
)
subp.wait()

# return the return code
sys.exit(subp.returncode)
