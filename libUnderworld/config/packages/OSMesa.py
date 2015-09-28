import os
from config import Package

class OSMesa(Package):

    def gen_locations(self):
        yield ('/usr', [], ['/usr/X11R6'])
        yield ('/usr/X11R6', [], [])

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield (e[0] + [os.path.join(h, 'GL') for h in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['gl.h', 'osmesa.h']
            if self.find_libraries(loc[2], 'OSMesa'):
                env.PrependUnique(LIBS=['OSMesa', 'GLU', 'GL'])
                #OK: Hack to force search for libGL in mesa dirs instead of system paths
                env.PrependUnique(RPATH=[loc[2][0] + '/mesa', loc[2][0]])
                env.PrependUnique(LIBPATH=[loc[2][0] + '/mesa', loc[2][0]])
                yield env
