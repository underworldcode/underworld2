from config import Package

class libavcodec(Package):

    def gen_locations(self):
        yield ('/usr', ['/usr/include'], ['/usr/lib'])
        yield ('/usr/local', ['/usr/local/include'], ['/usr/local/lib'])
        yield ('/opt/local', ['/opt/local/include/'], ['/opt/local/lib'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['libavcodec/avcodec.h', 'libavformat/avformat.h']
            lib_env = env.Clone()
            lib_env.PrependUnique(LIBS=['avcodec', 'avutil', 'avformat'])
            yield lib_env    

