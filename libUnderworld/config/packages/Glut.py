import os, platform
from config import Package

class Glut(Package):

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'glut') for i in e[0]], e[1])

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            # Add GL to the inclue dirs, but keep the original include
            # dirs in there too.
            yield (e[0] + [os.path.join(h, 'GL') for h in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('glut.h')]

            if platform.system() == "Darwin":
                env.AppendUnique(CPPPATH=['/System/Library/Frameworks/GLUT.framework/Headers'])
                env.AppendUnique(FRAMEWORKS=['GLUT'])
                env.AppendUnique(CPPPATH=['/System/Library/Frameworks/OpenGL.framework/Headers'])
                env.AppendUnique(FRAMEWORKS=['OpenGL'])
            else:
                env.PrependUnique(LIBS=['glut'])
            yield env
