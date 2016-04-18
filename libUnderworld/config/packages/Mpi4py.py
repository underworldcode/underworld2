import os
from config import Package

class Mpi4py(Package):

    def check(self, conf, env):
        try:
            import mpi4py as m4
        except ImportError:
            return False
        return True

    def _gen_locations(self):
        # lets just return a dummy here.. this is a python package so it don't have
        # multiple locations
        yield ('', [''], [''])
