import os
from config import Package

class libXML2(Package):

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'libxml2') for i in e[0]], e[1])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
            # the following is required to force libxml2 to not use the cpp-api
            env.AppendUnique(CPPDEFINES=["U_SHOW_CPLUSPLUS_API=0",])
            self.headers = [os.path.join('libxml', 'parser.h')]
            if self.find_libraries(loc[2], 'xml2'):
                env.PrependUnique(LIBS=['xml2'])
                yield env
