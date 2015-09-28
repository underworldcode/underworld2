import os
from config import Package

class MPI(Package):

    def __init__(self, name, env, **kw):
        Package.__init__(self, name, env, **kw)
        self.try_compilers = True

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'mpi') for i in e[0]], e[1])
            yield ([os.path.join(i, 'openmpi') for i in e[0]], e[1])

    def gen_envs(self, loc):
	# If we've been given an MPI compiler just try that.
	if os.path.basename(self.env['CC']) in ['mpicc', 'mpicxx']:
            yield self.env.Clone()
            return

        # If flagged to do so, try standard MPI compilers, unless
        # we were given other options to use.
        if self.try_compilers and \
                self.get_option('mpi_dir') is None and \
                self.get_option('mpi_inc_dir') is None and \
                self.get_option('mpi_lib_dir') is None and \
                self.get_option('mpi_libs') is None:
            env = self.env.Clone()
            env['CC'] = 'mpicc'
            env['CXX'] = 'mpicxx'
            yield env

        # attempt to use mpicc to find libs reqired
        import subprocess
        # test for open-mpi 
        try:
            proc = subprocess.Popen(['mpicc', '-showme:libs'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if proc.wait() == 0:
              env = self.env.Clone()
              libs=proc.communicate()[0].split()
              for env in Package.gen_envs(self, loc):
                lib_env = env.Clone()
                lib_env.PrependUnique(LIBS=libs)
                yield lib_env
        except OSError:
            # if error in 'mpicc -showme:libs'
            import sys
            #sys.stdout.write("no mpicc here, ")

        # attempt to find everything from mpicc.conf
        mpiconf = None
        if loc[0]:
           mpiconf = os.path.join( loc[0], 'etc/mpicc.conf' )
           if not os.path.exists( mpiconf ):
              mpiconf = None

        if mpiconf:
           # rape and pilage through the mpicc.conf file
           from distutils import sysconfig 
           vars = {} 
           flags = ''
           # get a list of all 'variable' names
           sysconfig.parse_makefile(mpiconf, vars) 
           if 'MPILIBNAME' in vars:
              flags += vars['MPILIBNAME']
           if 'LIBS' in vars:
              flags += vars['LIBS']
           if 'MPI_OTHERLIBS' in vars:
              flags += vars['MPI_OTHERLIBS']
           if flags != []:
              # format the flags string 
              flags_nice=flags.replace('"', '').replace('-l', ' ').split(' ')
              # remove empty elements
              for xxx in reversed(flags_nice):
                 if xxx=='': 
                    flags_nice.remove(xxx)

              for env in Package.gen_envs(self, loc):
                 lib_env = env.Clone()
                 lib_env.PrependUnique(LIBS=flags_nice)
                 yield lib_env


        for env in Package.gen_envs(self, loc):

            # Not sure which extra libraries to check for, so try them all.
            extra_libs = [[], ['rt'], ['pthread', 'rt'],
                          ['dl'], ['dl', 'rt'], ['dl', 'pthread'],
                          ['dl', 'pthread', 'rt']]
            for libs in extra_libs:

                # Check for OpenMPI with nsl and util libs.
                if self.find_libraries(loc[2], ['open-pal', 'nsl', 'util']):
                    lib_env = env.Clone()
                    lib_env.PrependUnique(LIBS=['mpi', 'open-rte', 'open-pal', 'nsl', 'util'] + libs)
                    yield lib_env

                # Check for OpenMPI.
                if self.find_libraries(loc[2], 'open-pal'):
                    lib_env = env.Clone()
                    lib_env.PrependUnique(LIBS=['mpi', 'open-rte', 'open-pal'] + libs)
                    yield lib_env

                # Check for general MPI.
                if self.find_libraries(loc[2], 'mpi'):
                    lib_env = env.Clone()
                    lib_env.PrependUnique(LIBS=['mpi'] + libs)
                    yield lib_env

                # Check for MPICH + pmpich.
                if self.find_libraries(loc[2], ['mpich', 'pmpich']):
                    lib_env = env.Clone()
                    lib_env.PrependUnique(LIBS=['pmpich', 'mpich'] + libs)
                    yield lib_env

                # Check for MPICH
                if self.find_libraries(loc[2], 'mpich'):
                    lib_env = env.Clone()
                    lib_env.PrependUnique(LIBS=['mpich'] + libs )
                    yield lib_env


    def check(self, conf, env):
        return conf.CheckLibWithHeader(None, 'mpi.h', 'c', call='MPI_Finalize();', autoadd=0)
