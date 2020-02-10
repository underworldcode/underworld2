import os
from config import Package
from .StGermain import StGermain

class StgDomain(Package):

    def setup_dependencies(self):
        self.stgermain = self.add_dependency(StGermain, required=True)

    def gen_locations(self):
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('StGermain', 'StGermain.h'),
                            os.path.join('StgDomain', 'StgDomain.h')]
            if self.find_libraries(loc[2], 'StgDomain'):
                env.PrependUnique(LIBS=['StgDomain'])
                yield env
