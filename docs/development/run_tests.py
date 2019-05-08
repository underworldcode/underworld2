#!/usr/bin/env python3
"""
This script tests multiple python scripts or Jupyter Notebooks. A test is
considered to have been completed successfully if it returns with the '0' exit
code, which should signify that no uncaught exceptions were encountered.

This script will return a non-zero exit code if any tests fail.

Usage: `run_tests.py --prepend="mpirun -np 2" foo.py [bar.ipynb [...]]`


"""
import os, sys, subprocess, string, argparse
import time

# this global just increments as tests are run
testnumber = 0

# dictionary for results
results = {}

# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"

# out a stream to /dev/null
try:
    from subprocess import DEVNULL
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')

def get_files(args,recursive):
    """
    Returns a list of files from a list of files/directories. 
    Where recursive is true, directories are recursed and any files
    within added to the returned files list.

    Parameters
    ----------
        args: list
            The files & directories
        recursive: bool
            if true, directories will be recursed for files

    Returns
    -------
        list:  The files
    """

    files=[]
    
    for arg in args:
        if not recursive:
            if os.path.isfile(arg):
                files.append(arg)
        else:
            for dirname, dirnames, filenames in os.walk(arg):
                # add files to list
                if '.ipynb_checkpoints' in dirname:
                    continue
                if 'development' in dirname:
                    print('Skipping {}'.format(os.path.join(dirname, '')))
                    continue
                for file in filenames:
                    files.append(os.path.join(dirname, file))

    return files



def convert_ipynb_to_py(fname):
    """
    Converts a given ipython notebook file into a python files
    using the script `ipynb_to_py.sh`
    """

    # create error files for reporting
    outName = "./convert_"+os.path.basename(fname)+".out"
    with open(outName, "w") as outFile:
        subprocess.check_call(['sh', 'ipynb_to_py.sh', fname], stdout=outFile, stderr=outFile )
    os.remove(outName)
    return fname.replace('.ipynb', '.py')   # return new the python file

def run_file(fname, dir, exe, job):
    """
    Runs a test and records stdout and stderr to givin directory.
    Accepted input file format is .ipynb or .py.


    Parameters
    ----------
        fname:
            input filename
        dir:
            output directory
        exe:
            the command to use as a per 1st argument of subprocess.Popen()

    Returns
    -------
        bool:  True = pass, False = fail.
    """
    # create the test directory if needed
    try:
        os.stat(dir)
    except:
        os.mkdir(dir)

    exe.append(os.path.basename(fname))  # append filename

    out = dir+"test_"+str(testnumber)+"__"+os.path.basename(fname)

    try:
        outFile = open(out+".out", "w")
        errFile = open(out+".err", "w")
        script_dir = os.path.dirname(fname)  # get script directory to run from
        if not script_dir:
            script_dir="."
        subprocess.check_call( exe, stdout=outFile, stderr=errFile, cwd=script_dir )
    except subprocess.CalledProcessError:
        results[job] = False
    else:
        results[job] = True
    finally:
        outFile.close()
        errFile.close()


if __name__ == '__main__':

    # use the argparse module to read cmd line
    parser = argparse.ArgumentParser()
    parser.add_argument("--prepend", help="Command to prepend before test executable. Useful for mpi tests (currently broken).", type=str, default=None)
    parser.add_argument("--recursive", help="Recurse directories for files.", type=bool, default=False)
    parser.add_argument("--jobs", help="Number of processes to use for concurrent test running.", type=int, default=1)
    parser.add_argument("files", nargs="+", help="the input file list")
    args = parser.parse_args()

    # error check input
    if len(args.files) == 0 :
        parser.print_help()
        sys.exit(1)

    # initialise test counters
    nfails=0
    list_fails=[]

    # create the test directory if needed
    dir = os.path.join(os.path.dirname(os.path.realpath(__file__)),"./testResults/")
    try:
       os.stat(dir)
    except:
       os.mkdir(dir)

    # create test log file
    logFileName = "testing.log"
    logFile = open(dir+logFileName, "w")

    maxjobs = args.jobs
    jobthreads = {}

    # loop though tests given as input, ie args.files
    for fname in get_files(args.files, recursive=args.recursive):

        # if file is not ipynb or py skip it
        is_ipynb = fname.endswith(".ipynb")
        if not (is_ipynb or fname.endswith(".py")):
            continue
        
        # build executable command
        exe = ['python3']

        if args.prepend:
            exe = args.prepend.split() + exe

        cleanup=False
        # if prefix args found and it's an ipynb, use 'jupyter nbconvert --execute'
        if is_ipynb and not args.prepend:
            exe = ['jupyter', 'nbconvert', '--ExecutePreprocessor.kernel_name="python3"',
                   '--ExecutePreprocessor.timeout=360','--execute', '--stdout']
        elif is_ipynb and args.prepend:
            # convert ipynb to py and run with python
            print("Converting test {} to .py".format(fname));
            logFile.write("\nconverting "+fname+" to a python script");

            try:
                fname = convert_ipynb_to_py(fname) # convert
            except subprocess.CalledProcessError:
                print("Error: unable to convert. Skipping.")
                continue

            cleanup=True

        # log and run test
        import threading
        import string
        import random
        def random_string():
            return ''.join(random.choice(string.ascii_letters) for m in range(8))

        testnumber += 1
        print("Running test {}: {}".format(testnumber," ".join(exe)+" "+fname));
        logFile.write("\nRunning "+ " ".join(exe)+" "+fname);
        import threading
        jobname = random_string()
        jobthreads[jobname] = (threading.Thread(target=run_file, args=( fname, dir, exe, jobname )), fname, testnumber, cleanup)
        jobthreads[jobname][0].start()

        def check():
            global nfails
            rmlist = []
            for key,value in jobthreads.items():
                if not value[0].is_alive():
                    rmlist.append(key)
                    result = results[key]
                    if result:
                        text = "PASS : {}".format(value[1])
                        print(text);
                        logFile.write(text); logFile.flush()
                    else:
                        out = dir+"test_"+str(value[2])+"__"+os.path.basename(value[1])+"*"
                        print("\nERROR (see "+out+" for details)\n")
                        logFile.write("\nERROR (see "+out+" for details)\n"); logFile.flush()
                        nfails = nfails+1
                        list_fails.append(value[1])

                    if value[3]:   # clean up if required
                        os.remove(value[1])
            # remove completed jobs
            for key in rmlist:
                del jobthreads[key]

        # halt here while at max jobs
        while( len(jobthreads)==maxjobs ):
            time.sleep(1)
            check()

    # check remaining jobs
    while( len(jobthreads)>0 ):
        time.sleep(1)
        check()


    # Report in testing.log
    logFile.write("\nNumber of fails " + str(nfails) +":\n"); logFile.flush()
    logFile.write(str(list_fails)); logFile.flush()
    logFile.close()

    # Report to stdout
    print("\n\nTotal: Number of fails " + str(nfails))
    print(list_fails)
    if len(list_fails) > 0:
        with open("fail_list.txt", 'w') as f:
            for failure in list_fails:
                f.write("{} ".format(failure))

    # Return appropriate error code
    if nfails == 0:
        sys.exit(0)
    else:
        sys.exit(1)


