import os, sys, subprocess, string

from runipy.notebook_runner import NotebookRunner, NotebookError
#from IPython.nbformat.current import read

# create the test directory if needed
dir = "./testResults/"
try:
   os.stat(dir)
except:
   os.mkdir(dir)

# out a stream to /dev/null
try:
    from subprocess import DEVNULL
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')

# test if we can execute runipy
command = "runipy -h"
try:
  subprocess.check_call(command.split(), stdout=DEVNULL, stderr=DEVNULL)
except:
  print "\nCannot execute tests because I can't execute 'runipy'"
  print "Make sure 'runipy' is installed"
  print "$ pip install runipy\n\n"
  sys.exit(1)


#global vars
command = "runipy "
nfails=0
list_fails=[]

# create test log file
logFileName = "testing.log"
logFile = open(dir+logFileName, "w") 

# for every ipynb
for f in os.listdir('.'):
  if f.endswith(".ipynb"):

    fname = f             # file name
    exe = command.split() # make a list
    exe.append(f)         # append filename

    out = dir+"res"+f
    exe.append(out)       # append output notebook filename

    print("\nRunning "+fname);
    logFile.write("\nRunning "+fname);

    # try run runipy on given notebook
    try:
      subprocess.check_call( exe, stdout=DEVNULL, stderr=logFile )
    except subprocess.CalledProcessError:
      print(" .... ERROR (see "+out+" for details)")
      logFile.write(" .... ERROR (see "+out+" for details)\n"); logFile.flush()
      nfails = nfails+1
      list_fails.append(fname)
    else:
      print(" .... PASS"); 
      logFile.write(" .... PASS"); logFile.flush()


# Report in testing.log
logFile.write("\nNumber of fails " + str(nfails) +":\n"); logFile.flush()
logFile.write(str(list_fails)); logFile.flush()
logFile.close()

# Report to stdout
print "\n\nTotal: Number of fails " + str(nfails)  
print list_fails

'''
if nfails > 0:
   sys.exit( nfails )
else:
   sys.exit(0)
'''
