import os
from config import Package
from libXML2 import libXML2
from MPI import MPI
from pcu import pcu

class StGermain(Package):

    def setup_dependencies(self):
        self.mpi = self.add_dependency(MPI, required=True)
        self.libxml2 = self.add_dependency(libXML2, required=True)
        self.pcu = self.add_dependency(pcu, required=True)

    def gen_locations(self):
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('StGermain', 'StGermain.h')]
            if self.find_libraries(loc[2], 'StGermain'):
                env.PrependUnique(LIBS=['StGermain'])
                yield env
