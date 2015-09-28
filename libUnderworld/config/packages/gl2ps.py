import os
from config import Package

class gl2ps(Package):

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'gl2ps') for i in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('gl2ps.h')]
            if self.find_libraries(loc[2], 'gl2ps'):
                env.PrependUnique(LIBS=['gl2ps'])
                yield env
