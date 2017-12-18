from config import Package
from .Python import Python
import os

class Numpy(Package):
    def setup_dependencies(self):
        self.python = self.add_dependency(Python, required=True)

    def gen_locations(self):
        try:
            import numpy as np
            incpath = np.get_include()
            if incpath.endswith('/include'):
                nppath = incpath[:-8]
            else:
                nppath = ''
            yield (nppath, [incpath], [nppath + '/lib'])
        except ImportError:
            print("Cannot import numpy. Perhaps it is not installed.")
            raise
        yield ('/usr', [], [])
        yield ('/usr/local', [], [])

    def gen_envs(self, loc):
        try:
            import numpy as np
        except ImportError:
            print("Cannot import numpy. Perhaps it is not installed.")
            raise
        for env in Package.gen_envs(self, loc):
            self.headers = [self.python.headers[0], 'numpy/arrayobject.h']
            env.AppendUnique(CPPPATH=[self.location[1][0]])
            yield env
