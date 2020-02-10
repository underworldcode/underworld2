#!/usr/bin/env python 
"""
This scipt will launch a seperate gdb (or cgdb) process against all found instances of the provided executable name

"""

import sys, os
import commands
import shlex,time, subprocess

if len(sys.argv) < 2:
   print "\n\nYou must at least provide the executable you wish to launch gdb against.\n\n"
   sys.exit()


outputString = commands.getoutput("ps -o %p --no-headers -C "+sys.argv[1])

processIDs = str.splitlines(outputString)

if len(processIDs) == 0:
   print "\n\nNo Processes with the name \'"+sys.argv[1]+"\' found.\n\n"
   sys.exit()

if len(sys.argv) == 3:
   debugger = sys.argv[2]
else:
   debugger = "gdb"
   
process = {}
for pid in processIDs:
   args = shlex.split("./gdbbt.sh "+pid)
   process[pid] = subprocess.Popen(args)
