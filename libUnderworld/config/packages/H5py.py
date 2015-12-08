import os
from config import Package
from Mpi4py import Mpi4py

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
            print "Can't find a build version of the h5py module ... building now"
            return False
        except:
            raise


    def h5pybuild(self, mpath):
        import subprocess
        print "Building private h5py version at " + mpath
        os.chdir(self._h5pysrc)
        cmd = "python setup.py configure -m"
        subp = subprocess.Popen(cmd, shell=True); subp.wait()
        cmd = 'python setup.py build --build-lib '+ os.path.dirname(mpath)
        subp = subprocess.Popen( cmd , shell=True); subp.wait()
        os.chdir(self._h5pysrc+'/..')


    def setup_dependencies(self):
        self.mpi4py = self.add_dependency(Mpi4py, required=True)

    def gen_envs(self, loc):
        env = self.env.Clone()



        if not self.testh5py(self._h5pyp):
            self.h5pybuild(self._h5pyp)

        if self.testh5py(self._h5pyp):
            import h5py
        else:
            import sys
            sys.exit(1)

        yield env


        '''
        mpath = os.path.abspath('../h5py')
        try:
            import imp
            fp, pathname, desc = imp.find_module( os.path.basename(mpath), 
                                                  os.path.dirname(mpath).split() )
            mod = imp.load_module( 'h5py', fp, pathname, desc )
            import h5py
        except:
            print "Cannot import h5py. Perhaps it is not installed."
            raise

        yield env
        '''
