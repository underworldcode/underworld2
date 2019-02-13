import os, sys
from config import Package
import subprocess
from SCons.Script.Main import AddOption

class H5py(Package):

    def setup_dependencies(self):
        pass

    def _importtest(self):
        # next check for mpi compat
        self._logfile.write("\nChecking if h5py is available.")
        self._logfile.flush()

        subp = subprocess.Popen(self.launcher+'python3 -c \'import h5py\'', shell=True, stdout=self._logfile, stderr=self._logfile)
        if subp.wait() != 0:
            self._logfile.write("\nh5py does not appear to be available.\n")
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
            if self._importtest():
                return True

            return False

    def _gen_locations(self):
        # lets just return a dummy here.. this is a python package so it don't have
        # multiple locations
        yield ('', [''], [''])

    def setup_options(self):
        AddOption('--h5py-launcher', dest='launcher', nargs=1, type='string',
                  action='store', help='Launcher application within which to launch build and test executables.')
