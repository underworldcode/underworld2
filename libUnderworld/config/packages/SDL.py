import platform
from config import Package
from .OpenGL import OpenGL

class SDL(Package):

    def gen_locations(self):
        yield ('/usr', ['/usr/include'], ['/usr/lib'])
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])
        yield ('/opt/local', ['/opt/local/include'], ['/opt/local/lib'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            #SDL conftest won't work on mac with SDL.h, use another header
            self.headers = ['SDL/SDL_video.h']
            if platform.system() == "Darwin":
                env.PrependUnique(LIBS=['SDL', 'SDLmain'])
                env.AppendUnique(CPPPATH=['/System/Library/Frameworks/OpenGL.framework/Headers'])
                env.AppendUnique(FRAMEWORKS=['OpenGL'])
                env.AppendUnique(FRAMEWORKS=['Cocoa'])
            else:
                env.PrependUnique(LIBS=['SDL'])
            yield env
