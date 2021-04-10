from config import Package

class libm(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['math.h']
            if self.find_libraries(loc[2], 'm'):
                env.PrependUnique(LIBS=['m'])
                yield env
