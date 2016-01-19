import os
from config import Package
from Mpi4py import Mpi4py
from MPI import MPI

class H5py(Package):

    _buildh5py = False
    _h5pysrc   = os.path.abspath('./h5py_ext')
    _h5pyp     = os.path.abspath('../h5py')

    #### TEST/BUILD FUNCTIONS FOR H5PY ####
    def testh5py(self, mpath):
        try:
            import imp
            fp, pathname, desc = imp.find_module( os.path.basename(mpath), 
                                                  os.path.dirname(mpath).split() )
            mod = imp.load_module( 'h5py', fp, pathname, desc )
            return True
        except ImportError:
            return False
        except:
            raise

    def h5pybuild(self, mpath):
        import subprocess
        print "Building private h5py version at " + mpath
        os.chdir(self._h5pysrc)
        subp = subprocess.Popen('python setup.py configure -m', shell=True); subp.wait()
        cmd = 'python setup.py build_ext --include-dirs='
        # use all header paths the config has thus far added.
        # we really just need the path to mpi.h
        for header_path in self.env["CPPPATH"]:
            cmd += header_path +':' 
        cmd2 = ' --build-lib ' + os.path.dirname(mpath)
        cmd += cmd2

        subp = subprocess.Popen( 'CC='+self.env['CC']+' '+cmd, shell=True);
        if subp.wait() != 0:
            import sys
            print "Failed building h5py :(\n"
            sys.exit(1)
        # need to run this now to make it importable
        subp = subprocess.Popen('python setup.py build' + cmd2, shell=True); subp.wait()
        if subp.wait() != 0:
            import sys
            print "Failed building h5py :(\n"
            sys.exit(1)
        os.chdir(self._h5pysrc+'/..')
    ##########################################

    def setup_dependencies(self):
        self.mpi4py = self.add_dependency(Mpi4py, required=True)
        self.MPI = self.add_dependency(MPI, required=True)

    def gen_envs(self, loc):
        env = self.env.Clone()
        
        # check if an existing location was given
        passedinPath=os.path.abspath(loc[0])

        if self.testh5py(passedinPath):
            print "passed in h5py at "+passedinPath+" works ",
            yield env

        if not self.testh5py(self._h5pyp):
            print "Can't find a built version of the h5py module ... building h5py ",
            self.h5pybuild(self._h5pyp)
            # lets run the test again.. this will also ensure that the module is importable below
            if not self.testh5py(self._h5pyp):
                raise RuntimeError("Unable to import h5py module. It was possibly not build successfully.")
            print "h5py installed at "+self._h5pyp + " ",

        try:
            import h5py
        except:
            import sys
            print "Can't import python module h5py" 
            sys.exit(1)

        yield env
