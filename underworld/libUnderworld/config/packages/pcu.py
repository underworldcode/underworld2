import os
from config import Package

class pcu(Package):

    def gen_locations(self):
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('pcu', 'pcu.h')]
            if self.find_libraries(loc[2], 'pcu'):
                env.PrependUnique(LIBS=['pcu'])
                yield env
