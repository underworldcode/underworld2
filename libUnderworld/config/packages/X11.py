from config import Package

class X11(Package):

    def gen_locations(self):
        yield ('/usr', ['/usr/include'], ['/usr/lib'])
        yield ('/usr/X11R6', ['/usr/X11R6/include'], ['/usr/X11R6/lib'])
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])
        yield ('/usr/local', ['/usr/local/X11'], ['/usr/local'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['X11/Xlib.h', 'GL/glx.h']
            lib_env = env.Clone()
            lib_env.PrependUnique(LIBS=['X11'])
            yield lib_env
            lib_env = env.Clone()
            lib_env.PrependUnique(LIBS=['X11', 'Xau', 'Xdmcp'])
            yield lib_env
            lib_env = env.Clone()
            lib_env.PrependUnique(LIBS=['X11', 'xcb', 'xcb-xlib', 'Xau', 'Xdmcp'])
            yield lib_env
