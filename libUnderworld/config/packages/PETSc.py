import os, sys
from config import Package
from MPI import MPI

class PETSc(Package):

    def setup_dependencies(self):
        self.mpi = self.add_dependency(MPI)

    def setup_options(self):
        from SCons.Script.Main import AddOption
        Package.setup_options(self)
        AddOption('--petsc-arch', dest='petsc_arch', nargs=1, type='string',
                  action='store', help='PETSc architecture.')
        AddOption('--petsc-library', dest='petsc_library', nargs=1, type='string',
                  action='store', help='PETSc library name.')

    def gen_locations(self):
        yield ('/usr/lib/petsc', [], [])

    def gen_base_extensions(self):
        for e in Package.gen_base_extensions(self):
            yield e
            yield ([os.path.join(i, 'petsc') for i in e[0]], e[1])

    def gen_envs(self, loc):
        env = self.env.Clone()

        # Must have the architecture as well.
        self.arch = self.get_option('petsc_arch')
        self.library = self.get_option('petsc_library')

        # Try to find PETSc information.
        extra_libs = []
        petscconf = None
        if loc[0]:
            bmake_dir = os.path.join(loc[0], 'bmake')
            # If we don't already have an arch, try read it.
            if not self.arch:
                petscconf = os.path.join(bmake_dir, 'petscconf')
                try:
                    inf = open(petscconf)
                    self.arch = inf.readline().split('=')[1].strip()
                except:
                    petscconf = None
                # Try looking for it the PETSc 3.6 uninstalled way.
                try:
                    items = os.listdir(loc[0])
                    for i in items:
                        i = os.path.join(loc[0], i)
                        if os.path.isdir(i):
                            if os.path.exists(os.path.join(i, 'lib','petsc','conf', 'petscvariables')):
                                self.arch = os.path.basename(i)
                                break
                except:
                    petscconf = None
                # Try looking for it the PETSc 3 uninstalled way.
                try:
                    items = os.listdir(loc[0])
                    for i in items:
                        i = os.path.join(loc[0], i)
                        if os.path.isdir(i):
                            if os.path.exists(os.path.join(i, 'conf', 'petscvariables')):
                                self.arch = os.path.basename(i)
                                break
                except:
                    pass

            # If we were able to find a architecture.
            if self.arch is not None:
                # Try PETSc 2.x
                petscconf = os.path.join(bmake_dir, self.arch, 'petscconf')
                if os.path.exists(petscconf):
                    loc[1].append(os.path.dirname(petscconf))
                    loc[2].append(os.path.join(loc[2][0], self.arch))
                # Try PETSc 3 uninstalled.
                else:
                    petscconf = os.path.join(loc[0], self.arch, 'conf', 'petscvariables')
                    if os.path.exists(petscconf):
                        loc[1].append(os.path.join(loc[0], self.arch, 'include'))
                        loc[1].append(os.path.join(loc[0], 'include'))
                        loc[2].append(os.path.join(loc[0], self.arch, 'lib'))
                    else:
                        petscconf = None
                    # PETSc 3.6
                    if not petscconf:
                        petscconf = os.path.join(loc[0], self.arch, 'lib','petsc', 'conf', 'petscvariables')
                        if os.path.exists(petscconf):
                            loc[1].append(os.path.join(loc[0], self.arch, 'include'))
                            loc[1].append(os.path.join(loc[0], 'include'))
                            loc[2].append(os.path.join(loc[0], self.arch, 'lib'))
                        else:
                            petscconf = None                   

            # Try PETSc 3 installed information.
            if not petscconf:
                petscconf = os.path.join(loc[0], 'conf', 'petscvariables')
                if not os.path.exists(petscconf):
                    petscconf = None
            # Try PETSc 3.6 installed information.
            if not petscconf:
                petscconf = os.path.join(loc[0], 'lib','petsc', 'conf', 'petscvariables')
                if not os.path.exists(petscconf):
                    petscconf = None

        # Can we locate static or shared libs?
        if self.library is not None:
            libs = [ self.library, ]
        else:
            libs = ['petscsnes', 'petscksp', 'petscdm',
                    'petscmat', 'petscvec', 'petsc']
        lib_types = self.find_libraries(loc[2], libs)
        # If we couldn't find any libs, try PETSc 3 installed.
        if lib_types is None:
            libs = ['petsc']
            lib_types = self.find_libraries(loc[2], libs)
        if lib_types is not None:
            # Add basic environment.
            env.AppendUnique(CPPPATH=loc[1])
            env.AppendUnique(LIBPATH=loc[2])
            env.AppendUnique(RPATH=loc[2])
        
####
#### Disabling this block. I'm not convinced it's best for us to always explicitly
#### link against the library petsc has linked against. I believe there are sometimes
#### instances where there can be conflicts due to libraries we both (uw & petsc) require,
#### but these instances seem pretty rare, and this block is breaking linking against homebrew
#### petsc builds (note that this is due to petscvariable having incorrect info).
####
#            # Add additional libraries.
#            if petscconf is not None:
#                from distutils import sysconfig
#                #import pdb; pdb.set_trace()
#                vars = {}
#                flags = None
#                sysconfig.parse_makefile(petscconf, vars)
#                if 'PACKAGES_LIBS' in vars:
#                   flags = sysconfig.expand_makefile_vars(vars['PACKAGES_LIBS'], vars)
#                elif 'PETSC_EXTERNAL_LIB_BASIC' in vars:
#                   flags = sysconfig.expand_makefile_vars(vars['PETSC_EXTERNAL_LIB_BASIC'], vars)
#            
#                # Static libs? i.e. no shared libs. Must also do this if we are
#                # linking static libraries.
#                if lib_types[1] is None or self.static:
#                    # Add a bunch of extra jazz.
#                    if 'X11_INCLUDE' in vars:
#                        flags += ' ' + sysconfig.expand_makefile_vars(str(vars['X11_INCLUDE']), vars)
#                    if 'MPI_INCLUDE' in vars:
#                        flags += ' ' + sysconfig.expand_makefile_vars(str(vars['MPI_INCLUDE']), vars)
#                    if 'BLASLAPACK_INCLUDE' in vars:
#                        flags += sysconfig.expand_makefile_vars(str(vars['BLASLAPACK_INCLUDE']), vars)
#                    if 'PCC_LINKER_FLAGS' in vars:
#                        flags += ' ' + sysconfig.expand_makefile_vars(str(vars['PCC_LINKER_FLAGS']), vars)
#                    if 'PCC_FLAGS' in vars:
#                        flags += ' ' + sysconfig.expand_makefile_vars(str(vars['PCC_FLAGS']), vars)
#                    if 'PCC_LINKER_LIBS' in vars:
#                        flags += ' ' + sysconfig.expand_makefile_vars(str(vars['PCC_LINKER_LIBS']), vars)
#
#                # Use SCons to parse the flags.
#                flag_dict = env.ParseFlags(flags)
#                # Store for later use.
#                env['PETSC_FLAGS'] = flag_dict
#                # Keep the libs for a bit later.
#                if 'LIBS' in flag_dict:
#                    extra_libs = flag_dict['LIBS']
#                    del flag_dict['LIBS']
#                env.MergeFlags(flag_dict)

        #env.PrependUnique(LIBS=libs)
        env.AppendUnique(LIBS=extra_libs)
        env.AppendUnique(LIBS=libs)
        yield env

    def check(self, conf, env):
        call = "PetscInitialize(NULL, NULL, NULL, NULL);\nPetscFinalize();"
        
        baselibs = ['mpi.h', 'petsc.h', 'petscvec.h', 'petscmat.h', 'petscksp.h', 'petscsnes.h']
        if   conf.CheckLibWithHeader(None, baselibs + ['petsc/private/kspimpl.h'], 'c', call=call, autoadd=0): #3.6 version
            pass
        elif conf.CheckLibWithHeader(None, baselibs + ['petsc-private/kspimpl.h'], 'c', call=call, autoadd=0): #3.3-3.5 version
            pass
        elif conf.CheckLibWithHeader(None, baselibs + ['private/kspimpl.h'],       'c', call=call, autoadd=0): #early version to try
            pass
        else:
            return False

        # Check if we can build PETSc into a shared library.
        if env['shared_libs'] and \
               not conf.CheckSharedLibWithHeader(None,
                                                 ['mpi.h', 'petsc.h', 'petscvec.h', 'petscmat.h',
                                                  'petscksp.h', 'petscsnes.h'], 'c',
                                                 call=call,
                                                 autoadd=0):
            print '\n\nThe PETSc located in:'
            print '  %s'%repr(self.location[0])
            print 'is usable, but could not be linked into a shared library.'
            print 'This is most likely due to PETSc not being compiled with'
            print 'position independant code enabled. Either rebuild PETSc'
            print 'with the configuration option \'--with-pic=1\' if you are'
            print 'sure you need shared libraries, or reconfigure StGermain'
            print 'with the \'--lib-type=static\' option.\n'
            sys.exit()
            return False

        return True
