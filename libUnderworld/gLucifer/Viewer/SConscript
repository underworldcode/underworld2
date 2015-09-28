import os
Import('env')

#
# Need to make a copy because SCons uses the environment
# at it's final state, so StGermain ends up depending on
# StgDomain, etc.
#

origenv = env
env = env.Clone()

#############################################
# Switch env to LavaVu base!
values = {}
execfile("output.cfg", globals(), values)
env._dict.update(values)
#############################################

env['CURR_PROJECT'] = 'LavaVu'

# Need the module name, which is just the directory.
d = "."
mod_name = env['ESCAPE']('"' + ''.join(d.split('/')) + '"')
# Binary install dir and rpath
if env['prefix'] == env.GetLaunchDir():
   bin_dir = os.path.join(env['prefix'], env['build_dir'], "bin");
   rpath = [os.path.join(env['prefix'], env['build_dir'], "lib")]
else:
   bin_dir = os.path.join(env['prefix'], "bin");
   rpath = [os.path.join(env['prefix'], "lib")]

cpp_defs = [('CURR_MODULE_NAME', mod_name)] + [('SHADER_PATH', '"\\"' + bin_dir + '/\\""')] + [('USE_FONTS')]
#cpp_defs = [('CURR_MODULE_NAME', mod_name)] + [('SHADER_PATH', '"\\"' + bin_dir + '/\\""')] + [('USE_FONTS')] + [('USE_ZLIB')]
env['CPPDEFINES'] += cpp_defs

# Setup where to look for files.
src_dir = 'src'
inc_dir = 'include/LavaVu/Viewer/'

# Install the shaders
env.Install('bin/', Glob(src_dir + '/shaders/*.vert'))
env.Install('bin/', Glob(src_dir + '/shaders/*.frag'))
# Install the html source
env.Install('bin/html/', Glob(src_dir + '/html/*.*'))

# Build our source files.
# C++ sources
srcs = Glob(src_dir + '/*.cpp')
srcs += [src_dir + '/mongoose/mongoose.c']
srcs += [src_dir + '/miniz/miniz.c']
srcs += [src_dir + '/jpeg/jpge.cpp']
srcs += [src_dir + '/jpeg/jpgd.cpp']
srcs += [src_dir + '/tiny_obj_loader.cc']
objs = env.SharedObject(srcs)
#build SQLite3 source (named object prevents clash)
sqlite3 = env.SharedObject('sqlite3-c', ['src/sqlite3/src/sqlite3.c'])

#Create renderer shared library
#Add pthreads & dl (required for sqlite3)
libs = ['pthread', 'dl'] + env.get('LIBS', [])
#libs = ['pthread', 'dl', 'z'] + env.get('LIBS', [])
l = env.SharedLibrary('lib/LavaVuRender', objs + sqlite3, LIBS=libs)

# Build LavaVu viewer (interactive version)
env = origenv.Clone()
values = {}
execfile("viewer.cfg", globals(), values)
env._dict.update(values)
env['CPPDEFINES'] += cpp_defs
srcs = Glob(src_dir + '/Main/main.cpp')
srcs += Glob(src_dir + '/Main/X11Viewer.cpp')
srcs += Glob(src_dir + '/Main/SDLViewer.cpp')
srcs += Glob(src_dir + '/Main/GlutViewer.cpp')
vobjs = env.SharedObject(srcs)
#Set search paths for libraries
env['RPATH'] += rpath
build_lib = os.path.join(env['build_dir'], "lib")
if build_lib[0] != "/":
   build_lib = "#" + build_lib
env['LIBPATH'] += [build_lib]
#Add the renderer library
libs = ['LavaVuRender'] + env.get('LIBS', [])
#Build the executable
env.Program('bin/LavaVu', vobjs, LIBS=libs)

if FindFile('offscreen.cfg', '.'):
   # Build LavaVu viewer (offscreen version)
   env = origenv.Clone()
   values = {}
   execfile("offscreen.cfg", globals(), values)
   env._dict.update(values)
   env['CPPDEFINES'] += cpp_defs
   srcs = Glob(src_dir + '/Main/AGLViewer.cpp')
   srcs += Glob(src_dir + '/Main/OSMesaViewer.cpp')
   vobjs = env.SharedObject(srcs)
   main = env.SharedObject('osmain', Glob(src_dir + '/Main/main.cpp'))
   #Set search paths for libraries
   env['RPATH'] += rpath
   env['LIBPATH'] += [build_lib]
   #Add the renderer library
   libs = ['LavaVuRender'] + env.get('LIBS', [])
   #Build the executable
   env.Program('bin/LavaVuOS', main + vobjs, LIBS=libs)
else:
   env.Program('bin/LavaVuOS', vobjs, LIBS=libs)

