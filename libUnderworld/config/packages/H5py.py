import os, sys
from config import Package
from Mpi4py import Mpi4py
from MPI import MPI
import subprocess

class H5py(Package):

    _buildh5py = False
    _h5pysrc   = os.path.abspath('./h5py_ext')
    _h5pyp     = os.path.abspath('../h5py')

    def setup_dependencies(self):
        self.mpi4py = self.add_dependency(Mpi4py, required=True)
        self.MPI = self.add_dependency(MPI, required=True)

    def _buildh5py(self, cc):
        print("\nAttempting to build private h5py version at {} using cc={}. This may take a few minutes.".format(self._h5pyp,cc))
        os.chdir(self._h5pysrc)
        with open('h5py_build.out','w') as outfile:
            with open('h5py_build.err','w') as errfile:
                subp = subprocess.Popen('python setup.py configure -m', shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("Failed configuring h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext")
                    raise RuntimeError
                subp = subprocess.Popen('python setup.py clean', shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("Failed cleaning h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext")
                    raise RuntimeError
                cmd = 'python setup.py build_ext --include-dirs='
                # use all header paths the config has thus far added.
                # we really just need the path to mpi.h
                for header_path in self.env["CPPPATH"]:
                    cmd += header_path +':'
                cmd2 = ' --build-lib ' + os.path.dirname(self._h5pyp)
                cmd += cmd2

                subp = subprocess.Popen( 'CC='+cc+' '+cmd, shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("Failed building h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext")
                    raise RuntimeError
                # need to run this now to make it importable
                subp = subprocess.Popen('python setup.py build' + cmd2, shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("Failed building h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext")
                    raise RuntimeError
        os.chdir(self._h5pysrc+'/..')
        self._logfile.write("h5py build completed successfully.\n")


    def _importtest(self):
        '''
        We run test on h5py here. Not that we first make sure we have project 
        parent directory in sys.path and that it is in the first place so that 
        it is preferenced over system packages. Also note that we do this 
        via subprocess to keep our own python environment clean and allow
        re-importing after (perhaps) we have built our own h5py.
        '''
        self._logfile.write("Attempting to import h5py.\n")
        proj_folder = os.path.realpath(os.path.dirname("../.."))
        subp = subprocess.Popen('python -c \'import sys\nsys.path.insert(0, \"{}\")\nimport h5py\''.format(proj_folder), shell=True, stdout=self._logfile, stderr=self._logfile)
        subp.wait()
        if subp.wait() != 0:
            self._logfile.write("h5py import failed.\n")
            return False
        self._logfile.write("h5py import succeeded.\n")

        # next check for mpi compat
        self._logfile.write("Checking if h5py is built against mpi.\n")
        subp = subprocess.Popen('python -c \'import sys\nsys.path.insert(0, \"{}\")\nimport h5py\nif not h5py.get_config().mpi: raise RuntimeError\''.format(proj_folder), shell=True, stdout=self._logfile, stderr=self._logfile)
        subp.wait()
        if subp.wait() != 0:
            self._logfile.write("h5py does not appear to be built against mpi.\n")
            return False
        self._logfile.write("h5py does appear to be built against mpi.\n")
        self._logfile.write("h5py configuration succeeded.\n")

        # if we made it this far, all is probably good
        return True

    def check(self, conf, env):
        with open('config.log', 'a') as self._logfile:
            self._logfile.write("\n\nCONFIGURE H5PY\n")
            self._logfile.write("Checking if h5py in built in uw2 already, or provided by system (in that order of preference).\n")
            if self._importtest():
                return True
            
            self._logfile.write("\n")
            self._logfile.write("No functional h5py found, lets try build our own using h5pcc.\n")
            try:
                self._buildh5py('h5pcc')
                if self._importtest():
                    return True
            except:
                pass

            self._logfile.write("\n")
            self._logfile.write("Still no functional h5py found, lets try build our own using uw2 CC.\n")
            try:
                self._buildh5py(self.env['CC'])
                if self._importtest():
                    return True
            except:
                pass
                
            return False

    def _gen_locations(self):
        # lets just return a dummy here.. this is a python package so it don't have
        # multiple locations
        yield ('', [''], [''])

