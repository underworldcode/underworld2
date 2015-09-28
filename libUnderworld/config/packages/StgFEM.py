import os
from config import Package
from StgDomain import StgDomain
from PETSc import PETSc

class StgFEM(Package):

    def setup_dependencies(self):
        self.petsc = self.add_dependency(PETSc, required=True)
        self.stgdomain = self.add_dependency(StgDomain, required=True)

    def gen_locations(self):
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('StGermain', 'StGermain.h'),
                            os.path.join('StgDomain', 'StgDomain.h'),
                            os.path.join('StgFEM', 'StgFEM.h')]
            if self.find_libraries(loc[2], 'StgFEM'):
                env.PrependUnique(LIBS=['StgFEM'])
                yield env
