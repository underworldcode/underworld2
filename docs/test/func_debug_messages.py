
def outside_domain():
    import underworld as uw
    mesh = uw.mesh.FeMesh_Cartesian()
    def create_var():
        return mesh.add_variable(1)
    var = create_var()
    var.evaluate((10.,10.))   # evaluate outside the domain
outside_domain_message  = b'ValueError: Issue utilising function of class \'MeshVariable\' constructed at:\n\n0- 0:func_debug_messages.py:7\n    var = create_var()\n0- 1:func_debug_messages.py:6\n    return mesh.add_variable(1)\n\nError message:\nFeVariable interpolation at location (10, 10) does not appear to be valid.\nLocation is probably outside local domain.\n' 
outside_domain_message2 = b'ValueError: Issue utilising function of class \'MeshVariable\'\n\nFull function debug info disabled due to UW_NO_FUNC_MESSAGES environment flag.\n\nError message:\nFeVariable interpolation at location (10, 10) does not appear to be valid.\nLocation is probably outside local domain.\n' 
outside_domain_message_jupyter= b'ValueError: Issue utilising function of class \'MeshVariable\' constructed at:\n\n    Line 5 of notebook cell 1:\n       var = create_var()\n    Line 4 of notebook cell 1:\n       return mesh.add_variable(1)\n\nError message:\nFeVariable interpolation at location (10, 10) does not appear to be valid.\nLocation is probably outside local domain.\n\n'

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        if   sys.argv[1] == "outside_domain":
            outside_domain()
    else:
        import subprocess
        def do_test_py(test,expected_message):
            command = "python3 func_debug_messages.py {}".format(test)
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            strmessage = expected_message.decode("utf-8")
            strenderr = result.stderr.decode("utf-8")[-len(strmessage)::]
            if not (strmessage==strenderr):
                raise RuntimeError("Incorrect error message encountered for {}. \n"
                                "Expected:\n{}\n\n"
                                "Encountered:\n{}\n\n".format(test,strmessage,strenderr))
        def do_test_jupyter(test,expected_message):
            command = "jupyter nbconvert --to python --execute {}".format(test)
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            strmessage = expected_message.decode("utf-8")
            strenderr = result.stderr.decode("utf-8")[-len(strmessage)::]
            if not (strmessage==strenderr):
                raise RuntimeError("Incorrect error message encountered for {}. \n"
                                "Expected:\n{}\n\n"
                                "Encountered:\n{}\n\n".format(test,strmessage,strenderr))


        # default messages
        do_test_py("outside_domain",outside_domain_message)
        do_test_jupyter("func_debug_messages_notebook.ipynb_do_not_run_direct", outside_domain_message_jupyter)

        # no func messages
        import os
        os.environ['UW_NO_FUNC_MESSAGES']='1'
        do_test_py("outside_domain",outside_domain_message2)

        # No coverage for parallel messages & UW_WORLD_FUNC_MESSAGES.
        # Unfortunately capturing and comparing parallel messages robustly 
        # is difficult due to different MPI implementations giving 
        # different error messages, and also due to arbitrary
        # ordering of messages. 