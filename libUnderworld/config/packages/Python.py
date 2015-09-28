from config import Package
import os

class Python(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local/include'], ['/usr/local/lib'])
        yield ('/opt/local', ['/opt/local/include'], ['/opt/local/lib'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['Python.h']
            if self.find_libraries(loc[2], 'python'):
                env.PrependUnique(LIBS=['python'])
                yield env
            if self.find_libraries(loc[2], 'python2.7'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python2.7')])
                env.PrependUnique(LIBS=['python2.7'])
                yield env
            if self.find_libraries(loc[2], 'python2.6'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python2.6')])
                env.PrependUnique(LIBS=['python2.6'])
                yield env
            if self.find_libraries(loc[2], 'python2.5'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python2.5')])
                env.PrependUnique(LIBS=['python2.5'])
                yield env
