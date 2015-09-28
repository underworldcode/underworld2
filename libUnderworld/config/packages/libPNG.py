from config import Package

class libPNG(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local/include'], ['/usr/local/lib'])
        yield ('/opt/local', ['/opt/local/include'], ['/opt/local/lib'])

    def gen_envs(self, loc):
        #env = self.env.Clone()
        for env in Package.gen_envs(self, loc):
            self.headers = ['png.h']
            if self.find_libraries(loc[2], 'png'):
                env.PrependUnique(LIBS=['png'])
                yield env
