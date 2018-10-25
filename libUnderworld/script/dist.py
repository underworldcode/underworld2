import os, shutil
from SCons.Script import *

class ToolInstWarning(SCons.Warnings.Warning):
    pass

SCons.Warnings.enableWarningClass(ToolInstWarning)

def to_list(var):
    if isinstance(var, str):
        return [var]
    elif var is None:
        return []
    elif isinstance(var, list):
        return var
    elif isinstance(var, tuple):
        return list(var)
    else:
        return [var]

def multiget(dicts, key, default=None):
    for d in dicts:
        if key in d:
            return d[key]
    else:
        return default

def copytree(src, dest, symlinks=False, ignore=None):
    def copyItems(src, dest):
        dir_list = os.listdir(src)
        if ignore:
            ignored = ignore(src, dir_list)
            dir_list = [n for n in dir_list if n not in ignored]
        for item in dir_list:
            srcPath = os.path.join(src, item)
            if os.path.isdir(srcPath):
                srcBasename = os.path.basename(srcPath)
                destDirPath = os.path.join(dest, srcBasename)
                if not os.path.exists(destDirPath):
                    os.makedirs(destDirPath)
                copyItems(srcPath, destDirPath)
            elif os.path.islink(srcPath):
                pass
#             elif os.path.islink(srcPath) and symlinks:
#                 linkto = os.readlink(srcPath)
#                 os.symlink(linkto, dest)
            else:
                shutil.copy2(srcPath, dest)
 
    # case 'cp -R src/ dest/' where dest/ already exists
    if os.path.exists(dest):
        if os.path.isdir(src):
            destPath = os.path.join(dest, os.path.basename(src))
            if not os.path.exists(destPath):
                os.makedirs(destPath)
        else:
            destPath = dest
    # case 'cp -R src/ dest/' where dest/ does not exist
    else:
        os.makedirs(dest)
        destPath = dest
    # actually copy the files
    if os.path.isdir(src):
        copyItems(src, destPath)
    else:
        shutil.copy2(src, destPath)

def check_target(env, target_name, **kw):
    target = multiget([kw, env], target_name)
    return target in COMMAND_LINE_TARGETS

def check_inst_target(env, **kw):
    return check_target(env, "INST_TARGET", **kw) or \
        check_target(env, "BIN_TARGET", **kw)

def check_bin_target(env, **kw):
    return check_target(env, "BIN_TARGET", **kw)

def check_dist_target(env, **kw):
    return check_target(env, "DIST_TARGET", **kw)

def get_prefix(env, **kw):
    if check_inst_target(env, **kw):
        prefix = multiget([kw, env], "INST_PREFIX", "")
    else:
        prefix = multiget([kw, env], "INST_BUILD_DIR", "")
    return prefix

def modify_args(env, **kw):
    new_kw = dict(kw)
    if check_inst_target(env, **kw):
        prefix = multiget([kw, env], "INST_PREFIX", "")
        build_dir = multiget([kw, env], "INST_BUILD_DIR", "")

        if build_dir:
            rpaths = multiget([kw, env], "RPATH", [])
            if not isinstance(rpaths, list):
                rpaths = [rpaths]
            rpaths = [p for p in rpaths if p != build_dir + "/lib"]
        rpaths.append(prefix + "/lib")

        if build_dir:
            lib_paths = multiget([kw, env], "LIBPATH", [])
            if not isinstance(lib_paths, list):
                lib_paths = [lib_paths]
            lib_paths = [p for p in lib_paths if p != build_dir + "/lib"]
        lib_paths.append(prefix + "/lib")

        new_kw["RPATH"] = rpaths
        new_kw["LIBPATH"] = lib_paths

    return new_kw

def generate(env, **kw):

    env.SetDefault(INST_TARGET="install",
                   DIST_TARGET="dist",
                   DIST_EXCLUDE_PATTERNS=[".*", "*.pyc"],
                   BIN_TARGET="binary")

    def Install(env, target, source, **kw):
        prefix = get_prefix(env, **kw)
        nodes = env.SConsInstall(os.path.join(prefix, target[0]), source, **kw)
        return nodes

    def Library(env, target, source, **kw):
        prefix = get_prefix(env, **kw)
        nodes = env.SConsLibrary(os.path.join(prefix, target[0]), source, **kw)
        return nodes

    def SharedLibrary(env, target, source, **kw):
        prefix = get_prefix(env, **kw)
        new_kw = modify_args(env, **kw)
        nodes = env.SConsSharedLibrary(os.path.join(prefix, target[0]), source, **new_kw)
        return nodes

    def Program(env, target, source, **kw):
        prefix = get_prefix(env, **kw)
        new_kw = modify_args(env, **kw)
        nodes = env.SConsProgram(os.path.join(prefix, target[0]), source, **new_kw)
        return nodes

    def Dist(env, target, source, **kw):
        if check_dist_target(env, **kw):
            n = env.DistTar(target, source, **kw)
            env.Alias(multiget([kw, env], "DIST_TARGET"), n)
            env.AlwaysBuild(n)
        else:
            n = []
        return n

    def DistTarAction(target, source, env):
        import shutil, fnmatch, tarfile

        def ignore(cur_dir, cur_names):
            ptrns = to_list(env.get("DIST_EXCLUDE_PATTERNS", []))
            excludes = []
            for n in cur_names:
                if target[0].abspath == os.path.join(cur_dir, n):
                    excludes.append(n)
                    continue
                for p in ptrns:
                    if fnmatch.fnmatch(n, p):
                        excludes.append(n)
            return excludes

        import tempfile
        tmp_dir = tempfile.mkdtemp()
        src_dir = os.path.basename(str(target[0]))[:-len(".tar.gz")]

        dir = os.path.join(tmp_dir, src_dir)
        for s in source:
            copytree(str(s), dir, symlinks=True, ignore=ignore)

        f = tarfile.open(str(target[0]), "w:gz")
        old_dir = os.getcwd()
        os.chdir(tmp_dir)
        f.add(src_dir)
        f.close()
        os.chdir(old_dir)

    def DistTarPrint(target, source, env):
        return "Creating distribution '%s'"%(target[0])

#     def Binary(env, target, source, **kw):
#         dir = multiget([kw, env], "DIST_TMP_DIR")
#         env.Command([Mkdir(dir), Copy(dir
#         target = source
#         prefix = multiget([kw, env], "INST_PREFIX")
#         dist_target = multiget([kw, env], "DIST_TARGET")
#         if dist_target is None or prefix is None or \
#                 dist_target not in COMMAND_LINE_TARGETS:
#             return []
#         env.Command(target[0], prefix, Mkdir(target[0]))
#         return env.Alias(dist_target, target[0],
#                          env.Tar(env.CopyAs(Dir(target[0]), Dir(prefix)),
#                                  TARFLAGS="-cz",
#                                  TARSUFFIX=".tar.gz"),
#                          Mkdir(Dir(prefix)))

    env.SConsInstall = env.Install
    env["BUILDERS"]["Install"] = Install
    env["BUILDERS"]["SConsLibrary"] = env["BUILDERS"]["Library"]
    env["BUILDERS"]["Library"] = Library
    env["BUILDERS"]["SConsSharedLibrary"] = env["BUILDERS"]["SharedLibrary"]
    env["BUILDERS"]["SharedLibrary"] = SharedLibrary
    env["BUILDERS"]["SConsProgram"] = env["BUILDERS"]["Program"]
    env["BUILDERS"]["Program"] = Program
    env["BUILDERS"]["Dist"] = Dist
    env["BUILDERS"]["DistTar"] = env.Builder(action=env.Action(DistTarAction, DistTarPrint),
                                             suffix="tar.gz",
                                             target_factory=env.File,
                                             source_factory=env.fs.Entry)

    AddOption("--dist-version", dest="dist_version", nargs=1, type="string",
              action="store", help="Distribution version",
              default="unknown")

def exists(env):
    return True
