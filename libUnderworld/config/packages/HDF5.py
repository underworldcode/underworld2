import os
from config import Package

class HDF5(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['hdf5.h']
            if self.find_libraries(loc[2], 'hdf5'):
                env.PrependUnique(LIBS=['hdf5'])
                yield env
