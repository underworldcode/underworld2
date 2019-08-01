
def fail_collective():
    import underworld as uw
    raise RuntimeError("Collective error.")
collective_msg = b'An uncaught exception appears to have been raised by all processes. Set the \'UW_ALL_MESSAGES\' environment variable to see all messages. Rank 0 message is:\nTraceback (most recent call last):\n  File "collective_fail.py", line 18, in <module>\n    fail_collective()\n  File "collective_fail.py", line 4, in fail_collective\n    raise RuntimeError("Collective error.")\nRuntimeError: Collective error.\n'

def fail_single():
    import underworld as uw
    if uw.mpi.rank==1:
        raise RuntimeError("Rank 1 error.")
single_msg = b'An uncaught exception was encountered on processor 1.\nTraceback (most recent call last):\n  File "collective_fail.py", line 20, in <module>\n    fail_single()\n  File "collective_fail.py", line 10, in fail_single\n    raise RuntimeError("Rank 1 error.")\nRuntimeError: Rank 1 error.\n'


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        if   sys.argv[1] == "collective":
            fail_collective()
        elif sys.argv[1] == "single":
            fail_single()
    else:
        import subprocess
        command = "mpirun -np 2 python collective_fail.py"
        result = subprocess.run(command + " collective", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        if not result.stderr.startswith(collective_msg):
            raise RuntimeError("Incorrect collective error message encountered. \n"
                               "Expected:\n{}\n\n"
                               "Encountered:\n{}\n\n".format(collective_msg,result.stderr[0:len(collective_msg)+1]))
        result = subprocess.run(command + " single", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        if not result.stderr.startswith(single_msg):
            raise RuntimeError("Incorrect collective error message encountered. \n"
                               "Expected:\n{}\n\n"
                               "Encountered:\n{}\n\n".format(single_msg,result.stderr[0:len(single_msg)+1]))
