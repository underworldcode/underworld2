import os
from config import Package

class SQLite3(Package):

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'sqlite3') for i in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            self.headers = [os.path.join('sqlite3.h')]
            if self.find_libraries(loc[2], 'sqlite3'):
                env.PrependUnique(LIBS=['sqlite3'])
                yield env
