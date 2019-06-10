import os
from config import Package
from .StgFEM import StgFEM

class PICellerator(Package):

    def setup_dependencies(self):
        self.stgfem = self.add_dependency(StgFEM, required=True)

    def gen_locations(self):
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('StGermain', 'StGermain.h'),
                            os.path.join('StgDomain', 'StgDomain.h'),
                            os.path.join('StgFEM', 'StgFEM.h'),
                            os.path.join('PICellerator', 'PICellerator.h')]
            if self.find_libraries(loc[2], 'PICellerator'):
                env.PrependUnique(LIBS=['PICellerator'])
                yield env
