#!/usr/bin/env python
"""
This script tests multiple python scripts or Jupyter Notebooks. A test is 
considered to have been completed successfully if it returns with the '0' exit 
code, which should signify that no uncaught exceptions were encountered.

This script will return a non-zero exit code if any tests fail.

Usage: `run_tests.py foo.py [bar.ipynb [...]]`


"""

# this global just increments as tests are run
testnumber = 0

import os, sys, subprocess, string
# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"

from runipy.notebook_runner import NotebookRunner, NotebookError

# out a stream to /dev/null
try:
    from subprocess import DEVNULL
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')


def run_file(fname, dir):
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
    # create the test directory if needed
    try:
        os.stat(dir)
    except:
        os.mkdir(dir)

    if fname.endswith(".ipynb"):
        exe = ["runipy"]
    elif fname.endswith(".py"):
        exe = ["python"]
    else:
        raise ValueError("Filename must have extension 'py' or 'ipynb'.")
    exe.append(fname)         # append filename

    testnumber += 1
    out = dir+"test_"+str(testnumber)+"__"+os.path.basename(fname)
    if fname.endswith(".ipynb"):
        exe.append(out)       # append output notebook filename

    # try run runipy on given notebook
    try:
        outFile = open(out+".out", "w")
        errFile = open(out+".err", "w")
        subprocess.check_call( exe, stdout=outFile, stderr=errFile )
    except subprocess.CalledProcessError:
        return False
    else:
        return True
    finally:
        outFile.close()
        errFile.close()


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

    for arg in sys.argv[1:]:
        if arg.endswith(".ipynb") or arg.endswith(".py") :
            print("\nRunning "+arg);
            logFile.write("\nRunning "+arg);
            result = run_file( arg, dir )
            if result:
                print(" .... PASS");
                logFile.write(" .... PASS"); logFile.flush()
            else:
                out = dir+"test_"+str(testnumber)+"__"+os.path.basename(arg)+"*"
                print(" .... ERROR (see "+out+" for details)")
                logFile.write(" .... ERROR (see "+out+" for details)\n"); logFile.flush()
                nfails = nfails+1
                list_fails.append(arg)

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