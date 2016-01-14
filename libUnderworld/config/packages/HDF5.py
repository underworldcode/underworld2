import os
from config import Package

class HDF5(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])

    def gen_envs(self, loc):
	import pdb
        pdb.set_trace()
        # attempt to use h5pcc to find lib
        import subprocess
        try:
            proc = subprocess.Popen(['h5pcc', '--print-file'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if proc.wait() == 0:
              env = self.env.Clone()
              libdir = proc.communicate()[0].split()[0]
              libdir = libdir[2:] #Strip -L
              for env in Package.gen_envs(self, loc):
                self.headers = ['hdf5.h']
                lib_env = env.Clone()
                lib_env.PrependUnique(LIBS=['hdf5'])
                lib_env.PrependUnique(LIBPATH=[libdir])
                lib_env.PrependUnique(RPATH=[libdir])
                #Get include dir
                lib_env.PrependUnique(CPPPATH=libdir + '/include')
                yield lib_env
            else:
               raise RuntimeError("Error trying to use 'h5pcc'")
        except:
            # if error in 'h5pcc --print-file'
            for env in Package.gen_envs(self, loc):
              self.headers = ['hdf5.h']
              if self.find_libraries(loc[2], 'hdf5'):
                env.PrependUnique(LIBS=['hdf5'])
                yield env

