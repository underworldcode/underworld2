import os
from config import Package
from Python import Python

class Mpi4py(Package):

    def setup_dependencies(self):
        self.python = self.add_dependency(Python, required=True)

    def gen_envs(self, loc):
        env = self.env.Clone()

        try:
            import mpi4py as m4
        except ImportError:
            print "Cannot import mpi4py. Perhaps it is not installed."
            raise
        yield env
