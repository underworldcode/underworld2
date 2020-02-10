from SCons.Script import *

class SaveFile:

    def __init__(self, f):
        self.path = f.abspath

    def __repr__(self):
        return 'File(\'' + self.path + '\')'

def UsePackage(env, mod, **kw):
    name = mod.__module__[mod.__module__.rfind('.') + 1:]
    pkg = env['packages'].get(mod, None)
    if not pkg:
        pkg = mod(name, env, **kw)
        env['packages'][mod] = pkg
    return pkg

def ConfigurePackage(env, mod, **kw):
    # If we aren't aware of the package throw an error.
    if mod not in env['packages']:
        print('Error: UsePackage not called prior to ConfigurePackage for')
        print('       package %s.'%repr(mod.__module__))
        env.Exit()
    # Don't configure if we're cleaning or helping.
    pkg = env['packages'][mod]
    if not (GetOption('clean') or GetOption('help')):
        kw['env'] = env
        pkg(**kw)
    return pkg

#Function to merge all lists in environment from a second environment
def MergeEnv(env, merge, replaceDupes=False):
    for k in merge._dict:
        if k in env and type(merge[k]) == list:
            #Merge lists
            for v in merge[k]:
                if not v in env[k]:
                    env[k].insert(0, v)
        else:
            #Copy if no data or replacing duplicates
            if not (k in env and env[k]) or replaceDupes:
                env[k] = merge[k]

def SetFlags(env):
    #Set debug and optimise settings (call last to override any from PETSc)
    if env.get('with_debugging', None):
        env.MergeFlags('-g -DDEBUG -O0')
    else:
        env.MergeFlags('-O2 -g -DNDEBUG')

    # enable all warnings.
    if env.get('with_warnings', None):    
        env.MergeFlags('-Wall')
    if env.get('CC', None) == 'icc':
        env.MergeFlags('-wd869 -wd1419')

def SaveConfig(env, filename='config.cfg'):
    if (GetOption('help') or GetOption('clean')):
        return

    env.SetFlags()

    if 'LIBS' in env:
        # Need to make sure there are no SCons files in the
        # LIBS environment.
        for i in range(len(env['LIBS'])):
            if not isinstance(env['LIBS'][i], str):
                env['LIBS'][i] = SaveFile(env['LIBS'][i])
    else:
        env['LIBS'] = [];

    out = open(filename, 'w')
    opts = [o[1] for o in env['cfg_options']] + [
        'CPPPATH', 'LIBPATH', 'RPATH', 'LIBS', 'CPPDEFINES',
        'CFLAGS', 'CCFLAGS', 'FRAMEWORKS',
        ] + env.get('save_vars', [])
    for o in opts:
        v = env.get(o, None)
        if v is not None:
            out.write('%s = %s\n'%(o, repr(env[o])))
    out.close()

    env.PrintSummary(filename)

def PrintSummary(env, filename="config.cfg"):
    if not (GetOption('help') or GetOption('clean')):
        print('')
        print('Saved configuration: "' + filename + '"')
        if 'PETSC_FLAGS' in env:
            print('  Flags imported from PETSc:')
            for key, val in env['PETSC_FLAGS'].items():
                if val:
                    print('   ' + key + ': ' + repr(val))
        if (filename == "config.cfg"):
            print('  C compiler:     %s'%repr(env['CC']))
            print('  C flags:        %s'%env.subst('$CFLAGS $CCFLAGS'))
        print('  C preprocessor: %s'%repr(env.get('CPPDEFINES')))
        print('  Libraries: %s'%repr(env.get('LIBS')))

def generate(env, options=[]):
    import platform

    # Setup basic flags.
    env['save_vars'] = []

    # Print out an inital log file line with the options used.
    conf = env.Configure()
    if conf.logstream != None:
        conf.logstream.write('\nConfiguring using:\n  ' + ' '.join(sys.argv) + '\n')
    conf.Finish()

    # Add the options to SCons.
    env['cfg_options'] = options
    for o in options:
        if len(o) > 4:
            type = o[4]
        else:
            type = 'string'
        AddOption(o[0], dest=o[1], nargs=1, type=type,
                  action='store', help=o[2], default=o[3])

    # Store option values on the environment.
    for o in options:
        if GetOption(o[1]) is not None:
            env[o[1]] = GetOption(o[1])
        elif o[1].upper() in os.environ:
            env[o[1]] = os.environ[o[1].upper()]

    # Need to handle Darwin shared libraries.
    if platform.system() == 'Darwin':
        env['_RPATH'] = ''
        env.Append(SHLINKFLAGS=['-dynamiclib', '-flat_namespace',
                                '-single_module', '-undefined', 'suppress',
                                '-install_name', '${_abspath(TARGET)}'])
        env['_abspath']=lambda x: "'" + File(x).abspath.replace("'", "'\\''") + "'"
    env.AppendUnique(save_vars=['_RPATH', 'SHLINKFLAGS'])

    env.AddMethod(UsePackage)
    env.AddMethod(ConfigurePackage)
    env.AddMethod(MergeEnv)
    env.AddMethod(SetFlags)
    env.AddMethod(SaveConfig)
    env.AddMethod(PrintSummary)
    env['packages'] = {}

def exists(env):
    return True
