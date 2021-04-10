import platform
from config import Package

class spatialdata(Package):

    def gen_locations(self):
        yield ('/usr/local', ['/usr/local'], ['/usr/local'])

    def gen_envs(self, loc):
        for env in Package.gen_envs(self, loc):
        	# lets default to clang where on Darwin. Also note the stdlib option, which is required for some business relating to std::strings & c11
            if platform.system() == "Darwin":
                env['CXX'] = 'clang++ -stdlib=libc++'
            self.headers = ['spatialdata/spatialdb/SimpleDB.hh', 'spatialdata/spatialdb/SimpleDBQuery.hh']
            if self.find_libraries(loc[2], 'spatialdata'):
                env.PrependUnique(LIBS=['spatialdata'] )
                yield env
