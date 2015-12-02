import os
from config import Package
from Mpi4py import Mpi4py

class H5py(Package):

    def setup_dependencies(self):
        self.mpi4py = self.add_dependency(Mpi4py, required=True)

    def gen_envs(self, loc):
        env = self.env.Clone()

        try:
            import h5py as h5
        except ImportError:
            print "Cannot import h5py. Perhaps it is not installed."
            raise
        yield env
