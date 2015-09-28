import os, tempfile, glob
import conv

_temp_counter = 0

def replext(path, ext):
    """Replace the extension of a path."""
    return os.path.splitext(path)[0] + ext


def remext(path):
    """Remove the extension of a path."""
    return replext(path, "")


def make_file_names(name, prefixes=[], suffixes=[]):
    """Combine prefixes and suffixes with a name. Returns a list
    with all combinations."""

    # If we weren't given any prefixes or suffixes add an empty
    # string to each. This way we will at least check for the name
    # which should be the default behavior.
    if not prefixes:
        prefixes = ['']
    else:
        prefixes = conv.to_list(prefixes)
    if not suffixes:
        suffixes = ['']
    else:
        suffixes = conv.to_list(suffixes)
        
    for p in prefixes:
        for s in suffixes:
            yield p + name + s


def gen_dirs(dirs, patterns=[], exists=True):
    """Generate directories from a list of base directories and an
    optional list of glob patterns. Optionally force the directories
    to exist"""
    ptrn_paths = [os.path.join(d, p) for p in patterns for d in dirs]
    all_dirs = dirs + reduce(lambda x,y: x+y, [glob.glob(p) for p in ptrn_paths], [])
    for d in all_dirs:
        if os.path.exists(d) and os.path.isdir(d):
            yield d


def find(name, dirs=[], prefixes=[], suffixes=[], max=0):
    """Try to locate a file system entry by combining prefixes
    and suffixes with a name, then searching a set of provided
    directories. By default a list of all matches is returned,
    but this can be limited by 'max'."""

    # If we weren't given any directories to search try the current
    # working directory.
    if not dirs:
        dirs = [os.getcwd()]
    else:
        dirs = conv.to_list(dirs)

    names = [n for n in make_file_names(name, prefixes, suffixes)]
    cur = 0
    for d in dirs:
        for n in names:
            p = os.path.join(d, n)
            if os.path.exists(p):
                yield p
                if max:
                    cur += 1
                    if cur == max:
                        return


def find_all(names, dirs=[], prefixes=[], suffixes=[]):
    """Search directories in 'dirs' for all the names contiained in
    'names'. Apply prefixes and suffixes. Will accept the first
    instance of a match."""

    paths = []
    for n in names:
        p = list(find(n, dirs, prefixes, suffixes))
        if not len(p):
            return None
        paths.append(p[0])
    return paths


def make_temp_name(prefix=None, suffix=None, ext=None, len=8):
    # Check that the prefix and suffix don't leave us no room for
    # inserting some temporary/random characters.
    if prefix:
        len -= __builtins__["len"](prefix)
    if suffix:
        len -= __builtins__["len"](suffix)
    if len <= 0:
        raise "Prefix and suffix leave no room for temporary name."

    name = ("%%0%dd"%len)%globals()["_temp_counter"]
    globals()["_temp_counter"] += 1

    if prefix:
        name = prefix + name
    if suffix:
        name += suffix
    if ext:
        name += ext
    return name


def make_temp_file(contents=None, dir=None, open=False, *args, **kw):
    # If we weren't given a directory, create the file in the
    # current working directory.
    if not dir:
        dir = os.getcwd()

    name = make_temp_name(*args, **kw)
    f = __builtins__["open"](name, "w")
    if contents:
        f.write(contents)
    if not open:
        f.close()
        return name
    return (name, f)


def make_temp_dir(dir=None):
    if not dir:
        dir = os.getcwd()
    return tempfile.mkdtemp(dir=dir)
