from config import Package
from distutils.sysconfig import get_python_inc, get_python_lib
import os

class Python(Package):

    def gen_locations(self):
        python_inc = get_python_inc()
        python_base = os.path.dirname(get_python_inc())
        python_lib = os.path.join(python_base,"lib")
        yield ( python_base, [python_inc,], [python_lib,] )
        yield ('/usr/local', ['/usr/local/include'], ['/usr/local/lib'])
        yield ('/opt/local', ['/opt/local/include'], ['/opt/local/lib'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = ['Python.h']
            if self.find_libraries(loc[2], 'python'):
                env.PrependUnique(LIBS=['python'])
                yield env
            if self.find_libraries(loc[2], 'python3.5m'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python3.5m')])
                env.PrependUnique(LIBS=['python3.5m'])
                yield env
            if self.find_libraries(loc[2], 'python3.6m'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python3.6m')])
                env.PrependUnique(LIBS=['python3.6m'])
                yield env
            if self.find_libraries(loc[2], 'python3.7m'):
                env.AppendUnique(CPPPATH=[os.path.join(self.location[1][0],'python3.7m')])
                env.PrependUnique(LIBS=['python3.7m'])
                yield env
