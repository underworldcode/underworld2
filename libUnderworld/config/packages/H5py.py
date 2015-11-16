import os
from config import Package
from Mpi4py import Mpi4py

class H5py(Package):

    def setup_dependencies(self):
        self.mpi4py = self.add_dependency(Mpi4py, required=True)

    def gen_envs(self, loc):
        env = self.env.Clone()

        # first check if parallel hdf5 is enabled
        import subprocess
        command = "h5cc -showconfig"
        proc = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if proc.wait() == 0:
          log = proc.communicate()[0]
          if not ("Parallel HDF5: yes" in log):
              raise OSError("h5py requires parallel hdf5. The given hdf5 doesn't appear to be parallel, see 'h5cc -showconfig'")
        else:
            raise OSError( "Error: can't run '{0}' to analyse the hdf5 installation".format(command))

        try:
            import h5py as h5
        except ImportError:
            print "Cannot import h5py. Perhaps it is not installed."
            raise
        yield env
