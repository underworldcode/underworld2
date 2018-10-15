import os, sys
from config import Package
from .MPI import MPI
from .HDF5 import HDF5
import subprocess
from SCons.Script.Main import AddOption

class H5py(Package):

    _buildh5py = False
    _h5pysrc   = os.path.abspath('./h5py_ext')
    _h5pyp     = os.path.abspath('../h5py')

    def setup_dependencies(self):
        self.MPI = self.add_dependency(MPI, required=True)
        self.hdf5 = self.add_dependency(HDF5, required=False)

    def _buildh5py(self, cc):
        print("\nAttempting to build private h5py version at {} using cc={}. This may take a few minutes.".format(self._h5pyp,cc))
        os.chdir(self._h5pysrc)
        with open('h5py_build.out','w') as outfile:
            with open('h5py_build.err','w') as errfile:
                command = "python setup.py configure -rm --hdf5 " + self.hdf5.location[0]
                self._logfile.write("\n\nh5py configuration command:\n")
                self._logfile.write(self.launcher+command)
                subp = subprocess.Popen((self.launcher+command).split(), stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("\nFailed configuring h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext\n")
                    raise RuntimeError
                subp = subprocess.Popen(self.launcher+'python setup.py clean', shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("\nFailed cleaning h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext\n")
                    raise RuntimeError
                
                # The following doesn't work on raijin, one only requires 'setup.py build'
                # but as we use a system h5py it isn't critical JG May2017
                cmd = 'python setup.py build_ext --include-dirs=' 
                # use all header paths the config has thus far added.
                # we really just need the path to mpi.h
                for header_path in self.env["CPPPATH"]:
                    cmd += header_path +':'
                cmd2 = ' --build-lib ' + os.path.dirname(self._h5pyp)
                cmd += cmd2

                self._logfile.write("\nh5py build command:\n")
                self._logfile.write('CC='+cc+' '+self.launcher+' '+cmd )
                subp = subprocess.Popen( 'CC='+cc+' '+self.launcher+' '+cmd, shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("\nFailed building h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext\n")
                    raise RuntimeError
                # need to run this now to make it importable
                self._logfile.write("\nh5py install command:\n")
                self._logfile.write(self.launcher+'python setup.py build' + cmd2)
                subp = subprocess.Popen(self.launcher+'python setup.py build' + cmd2, shell=True, stdout=outfile, stderr=errfile)
                subp.wait()
                if subp.wait() != 0:
                    self._logfile.write("\nFailed installing h5py :(\nPlease check 'h5py_build.out' and 'h5py_build.err' in libUnderworld/h5py_ext\n")
                    raise RuntimeError
        os.chdir(self._h5pysrc+'/..')
        self._logfile.write("\nh5py build completed successfully.\n")

    def _importtest(self):
        '''
        We run test on h5py here. Note that we first check if a custom h5py is
        in the parent directory ie it is preferenced over system packages. 
	Also note that we do this via subprocess to keep our own python environment 
	clean and allow re-importing after (perhaps) we have built our own h5py.
        '''
        # next check for mpi compat
        self._logfile.write("\nChecking if h5py is importable and built against mpi.\n")
        self._logfile.flush()
        proj_folder = os.path.realpath(os.path.dirname("../.."))

        # try h5py in the parent directory
        subp = subprocess.Popen(self.launcher+'python -c \'import sys\nsys.path.insert(0, \"{}\")\nimport h5py\nif not h5py.get_config().mpi: raise RuntimeError(\"h5py imported, but not compiled against mpi.\")\''.format(proj_folder), shell=True, stdout=self._logfile, stderr=self._logfile)
        subp.wait()

        if subp.wait() != 0:
            self._logfile.write("\nh5py is not importable from {}, or does not appear to be built against mpi.\n".format(proj_folder))
            # try h5py in PYTHONPATH
            subp = subprocess.Popen(self.launcher+'python -c \'import h5py\nif not h5py.get_config().mpi: raise RuntimeError(\"h5py imported, but not compiled against mpi.\")\'', shell=True, stdout=self._logfile, stderr=self._logfile)
            if subp.wait() != 0:
                self._logfile.write("\nh5py is not importable from system, or does not appear to be built against mpi.\n")
                self._logfile.write("\nNote that you can install parallel h5py via pip using the following command:\n \
    CC=mpicc HDF5_MPI=\"ON\" HDF5_DIR=/path/to/your/hdf5/install/ pip install --no-binary=h5py h5py\n")

                self._logfile.flush()
                return False

        # if we made it this far, all is probably good
        self._logfile.write("\nh5py configuration succeeded.\n")
        return True

    def check(self, conf, env):
        self.launcher = self.get_option('launcher')
        if self.launcher is None:
            self.launcher = ""
        else:
            self.launcher = self.launcher + " "

        with open('config.log', 'a') as self._logfile:
            self._logfile.write("\n\nCONFIGURE H5PY\n")
            self._logfile.write("Checking if h5py in built in uw2 already, or provided by system (in that order of preference).\n")
            if self._importtest():
                return True
            
            self._logfile.write("\n")
            self._logfile.write("h5py not found. try build local version using uw2 CC.\n")
            if not self.hdf5.result:
                self._logfile.write("    libhdf5 not found by config. Please explicitly specify your libhdf5 install via the --hdf5-dir flags.\n")
                return False
            
            # let's get h5py
            self.pull_from_git( "https://github.com/h5py/h5py", "2.7.0", self._h5pysrc, logfile=self._logfile)
            
            try:
                self._buildh5py(self.env['CC'])
                if self._importtest():
                    return True
            except:
                pass
                
                
            self._logfile.write("\n")
            self._logfile.write("h5py not found, try build local version using h5pcc.\n")
            try:
                self._buildh5py('h5pcc')
                if self._importtest():
                    return True
            except:
                pass

            return False

    def _gen_locations(self):
        # lets just return a dummy here.. this is a python package so it don't have
        # multiple locations
        yield ('', [''], [''])

    def setup_options(self):
        AddOption('--h5py-launcher', dest='launcher', nargs=1, type='string',
                  action='store', help='Launcher application within which to launch build and test executables.')
