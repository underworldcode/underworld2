#!/usr/bin/env python
"""
Simple script for running and testing notebooks.

Usage: `testNotebook.py foo.ipynb [bar.ipynb [...]]`

Notebooks are considered to complete the test successfully if they
simply run to completion without errors.

"""

# this global just increments as tests are run
testnumber = 0

import os, sys, subprocess, string

from runipy.notebook_runner import NotebookRunner, NotebookError

# out a stream to /dev/null
try:
    from subprocess import DEVNULL
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')


# for every ipynb
def run_file(fname, dir, logFile):
  """
  Runs ipython notebook using runipy. 
  
  Parameters
  ----------
      fname: input filename
      dir:   output directory
  Returns
  -------
      bool:  True = pass, False = fail.
  """
  global testnumber
  if fname.endswith(".ipynb"):

    # create the test directory if needed
    try:
       os.stat(dir)
    except:
       os.mkdir(dir)

    exe = ["runipy"]
    exe.append(fname)         # append filename

    testnumber += 1
    out = dir+"test_"+str(testnumber)+"__"+os.path.basename(fname)
    exe.append(out)       # append output notebook filename

    print("\nRunning "+fname);
    logFile.write("\nRunning "+fname);

    # try run runipy on given notebook
    try:
      subprocess.check_call( exe, stdout=DEVNULL, stderr=logFile )
    except subprocess.CalledProcessError:
      print(" .... ERROR (see "+out+" for details)")
      logFile.write(" .... ERROR (see "+out+" for details)\n"); logFile.flush()
      return False
    else:
      print(" .... PASS"); 
      logFile.write(" .... PASS"); logFile.flush()
      return True


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print __doc__ % {'scriptName' : sys.argv[0].split("/")[-1]}
        sys.exit(0)
    # test if we can execute runipy
    command = "runipy -h"
    try:
      subprocess.check_call(command.split(), stdout=DEVNULL, stderr=DEVNULL)
    except:
      print "\nCannot execute tests because I can't execute 'runipy'"
      print "Make sure 'runipy' is installed"
      print "$ pip install runipy\n\n"
      sys.exit(1)

    nfails=0
    list_fails=[]
    dir = "./testResults/"

    # create the test directory if needed
    try:
       os.stat(dir)
    except:
       os.mkdir(dir)

    # create test log file
    logFileName = "testing.log"
    logFile = open(dir+logFileName, "w")

    for ipynb in sys.argv[1:]:
        result = run_file( ipynb, dir, logFile )
        if result == False:
            nfails = nfails+1
            list_fails.append(ipynb)

    # Report in testing.log
    logFile.write("\nNumber of fails " + str(nfails) +":\n"); logFile.flush()
    logFile.write(str(list_fails)); logFile.flush()
    logFile.close()

    # Report to stdout
    print "\n\nTotal: Number of fails " + str(nfails)
    print list_fails

    # Return appropriate error code
    if nfails == 0:
        sys.exit(0)
    else:
        sys.exit(1)