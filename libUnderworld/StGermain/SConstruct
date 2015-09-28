import sys, os

sys.path.insert(0, os.path.abspath(os.path.join(os.getcwd(), 'config')))
import SConfig
SConscript('config/SConfig/SConscript')

#
# CUSTOMISE THE ENVIRONMENT HERE.
#

env = Environment(ENV=os.environ)
env['_abspath'] = lambda x: File(x).abspath # Needed by Darwin.

# Determine whether we are configuring, helping or building.
if 'config' in COMMAND_LINE_TARGETS or 'help' in COMMAND_LINE_TARGETS:

    #
    # INSERT CONFIGURATION HERE.
    #

    proj = env.Package(SConfig.Project)
    proj.dependency(SConfig.packages.cmath)
    proj.dependency(SConfig.packages.libXML2)
    proj.dependency(SConfig.packages.MPI)
    proj.dependency(SConfig.packages.SVNRevision)
    env.configure_packages()

    # Need to define the extension for shared libraries as well
    # as the library directory.
    ext = env['ESCAPE']('"' + env['SHLIBSUFFIX'][1:] + '"')
    lib_dir = env['ESCAPE']('"' + os.path.abspath(env['build_dir']) + '/lib' + '"')
    env.AppendUnique(CPPDEFINES=[('MODULE_EXT', ext), ('LIB_DIR', lib_dir)])

    # Save results.
    env.save_config()

else:
    # Load configuration.
    env.load_config()
    SConscript('pcu/script/scons.py', exports='env')
    SConscript('script/scons.py', exports='env')

    #
    # INSERT TARGETS HERE.
    #

    SConscript('SConscript', exports='env')
