import os
from config import Package

class OpenGL(Package):

    def gen_locations(self):
        yield ('/usr', [], ['/usr/X11R6'])
        yield ('/usr/X11R6', [], [])

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            # Add GL to the inclue dirs, but keep the original include
            # dirs in there too.
            yield (e[0] + [os.path.join(h, 'GL') for h in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['gl.h']

            env.PrependUnique(LIBS=['GL'])

            yield env
