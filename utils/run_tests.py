#!/usr/bin/env python3
"""
This script tests multiple python scripts or Jupyter Notebooks. A test is
considered to have been completed successfully if it returns with the '0' exit
code, which should signify that no uncaught exceptions were encountered.

This script will return a non-zero exit code if any tests fail.

Usage: `run_tests.py --prepend="mpirun -np 2" --convert=True foo.py [bar.ipynb [...]]`


"""
import os, sys, subprocess, string, argparse, shutil

# this global just increments as tests are run
testnumber = 0

# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"

# out a stream to /dev/null
try:
    from subprocess import DEVNULL
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')

# test if we can execute runipy
can_runipy = True
try:
    subprocess.check_call("runipy -h".split(), stdout=DEVNULL, stderr=DEVNULL)
except:
    can_runipy = False
    print("'runipy' does not appear to be available. All jupyter notebooks will be skipped.")


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

def run_file(fname, dir, exe):
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
    global testnumber
    # create the test directory if needed
    try:
        os.stat(dir)
    except:
        os.mkdir(dir)

    exe.append(os.path.basename(fname))  # append filename

    testnumber += 1
    out = dir+"test_"+str(testnumber)+"__"+os.path.basename(fname)

    try:
        outFile = open(out+".out", "w")
        errFile = open(out+".err", "w")
        script_dir = os.path.dirname(fname)  # get script directory to run from
        subprocess.check_call( exe, stdout=outFile, stderr=errFile, cwd=script_dir )
    except subprocess.CalledProcessError:
        return False
    else:
        return True
    finally:
        outFile.close()
        errFile.close()


if __name__ == '__main__':

    # use the argparse module to read cmd line
    parser = argparse.ArgumentParser()
    parser.add_argument("--prepend", help="Command to prepend before test executable. Useful for mpi tests.", type=str, default=None)
    parser.add_argument("--recursive", help="Recurse directories for files.", type=bool, default=False)
    parser.add_argument("--replace_outputs", help="Runs ipynb models replacing output cells.", type=bool, default=False)
    parser.add_argument("--convert", help="If enabled, all ipynb files are converted to py before processing.", type=bool, default=False)
    parser.add_argument("files", nargs="+", help="the input file list")
    args = parser.parse_args()

    # error check input
    if len(args.files) == 0 :
        parser.print_help()
        sys.exit(1)

    # initialise test counters
    nfails=0
    list_fails=[]
    cleanup=False

    # create the test directory if needed
    dir = os.path.join(os.path.dirname(os.path.realpath(__file__)),"./testResults/")
    try:
       os.stat(dir)
    except:
       os.mkdir(dir)

    # create test log file
    logFileName = "testing.log"
    logFile = open(dir+logFileName, "w")

    # loop though tests given as input, ie args.files
    for fname in get_files(args.files, recursive=args.recursive):

        # if file is not ipynb or py skip it
        is_ipynb = fname.endswith(".ipynb")
        if not (is_ipynb or fname.endswith(".py")):
            continue
        
        # if replacing outputs, only run ipynb models
        if args.replace_outputs and fname.endswith(".py"):
            continue

        # build executable command
        exe = ['python']

        if is_ipynb and can_runipy and not args.convert:
            exe = ['runipy']      # use runipy instead
        elif is_ipynb:
            # convert ipynb to py and move to 'testResults'
            print("Converting test {} to .py".format(fname));
            logFile.write("\nconverting "+fname+" to a python script");

            try:
                fname = convert_ipynb_to_py(fname) # convert
            except subprocess.CalledProcessError:
                print("Error: unable to convert. Skipping.")
                continue

            cleanup=True

        if args.replace_outputs:
            exe = ['runipy','-o']   # use runipy with -o arg

        if args.prepend:
            exe = args.prepend.split() + exe
        # log and run test
        print("Running test {}: {}".format(testnumber+1," ".join(exe)+" "+fname));
        logFile.write("\nRunning "+ " ".join(exe)+" "+fname);
        result = run_file( fname, dir, exe )
        if result:
            print(" .... PASS\n");
            logFile.write(" .... PASS\n"); logFile.flush()
        else:
            out = dir+"test_"+str(testnumber)+"__"+os.path.basename(fname)+"*"
            print(" .... ERROR (see "+out+" for details)\n")
            logFile.write(" .... ERROR (see "+out+" for details)\n"); logFile.flush()
            nfails = nfails+1
            list_fails.append(fname)

        if cleanup:   # clean up if required
            os.remove(fname)
            cleanup=False

    # Report in testing.log
    logFile.write("\nNumber of fails " + str(nfails) +":\n"); logFile.flush()
    logFile.write(str(list_fails)); logFile.flush()
    logFile.close()

    # Report to stdout
    print("\n\nTotal: Number of fails " + str(nfails))
    print(list_fails)

    # Return appropriate error code
    if nfails == 0:
        sys.exit(0)
    else:
        sys.exit(1)


