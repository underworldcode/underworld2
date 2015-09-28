import os, SCons.SConf, SCons.Conftest
import string

def CheckLibs(context, library = None, symbol = 'main',
              header = None, language = None, autoadd = 1, extra_libs=[]):
    """
    Copied from SConf.py but modified to check all libraries at once
    instead of one at a time.
    """

    if library == []:
        library = [None]

    if not SCons.Util.is_List(library):
        library = [library]

    # ToDo: accept path for the library
    res = SCons.Conftest.CheckLib(context, [library[0]], symbol, header = header,
                                  language = language, autoadd = autoadd,
                                  extra_libs=library[1:] + extra_libs)
    context.did_show_result = 1
    return not res

def CheckLibsWithHeader(context, libs, header, language,
                       call = None, autoadd = 1, extra_libs=[]):
    # ToDo: accept path for library. Support system header files.
    """
    Copied from SConf.py, but modified to check all libraries at once
    instead of one at a time.
    """
    prog_prefix, dummy = \
        SCons.SConf.createIncludesFromHeaders(header, 0)
    if libs == []:
        libs = [None]

    if not SCons.Util.is_List(libs):
        libs = [libs]

    res = SCons.Conftest.CheckLib(context, [libs[0]], None, prog_prefix,
                                  call = call, language = language, autoadd = autoadd,
                                  extra_libs=libs[1:] + extra_libs)
    context.did_show_result = 1
    return not res

def CheckCCFixed(context):
    res = SCons.Conftest.CheckCC(context)
    context.did_show_result = 1
    return not res

def Conftest_CheckSharedLib(context, libs, func_name = None, header = None,
                            extra_libs = None, call = None, language = None, autoadd = 1):
    """
    Configure check for a C or C++ libraries "libs".  Searches through
    the list of libraries, until one is found where the test succeeds.
    Tests if "func_name" or "call" exists in the library.  Note: if it exists
    in another library the test succeeds anyway!
    Optional "header" can be defined to include a header file.  If not given a
    default prototype for "func_name" is added.
    Optional "extra_libs" is a list of library names to be added after
    "lib_name" in the build command.  To be used for libraries that "lib_name"
    depends on.
    Optional "call" replaces the call to "func_name" in the test code.  It must
    consist of complete C statements, including a trailing ";".
    Both "func_name" and "call" arguments are optional, and in that case, just
    linking against the libs is tested.
    "language" should be "C" or "C++" and is used to select the compiler.
    Default is "C".
    Note that this uses the current value of compiler and linker flags, make
    sure $CFLAGS, $CPPFLAGS and $LIBS are set correctly.
    Returns an empty string for success, an error message for failure.
    """    
    # Include "confdefs.h" first, so that the header can use HAVE_HEADER_H.
    if context.headerfilename:
        includetext = '#include "%s"' % context.headerfilename
    else:
        includetext = ''
    if not header:
        header = ""

    text = """
%s
%s""" % (includetext, header)

    # Add a function declaration if needed.
    if func_name and func_name != "main":
        if not header:
            text = text + """
#ifdef __cplusplus
extern "C"
#endif
char %s();
""" % func_name

        # The actual test code.
        if not call:
            call = "%s();" % func_name

    # if no function to test, leave main() blank
    text = text + """
int
dummy_func() {
  %s
return 0;
}
""" % (call or "")

    if call:
        i = string.find(call, "\n")
        if i > 0:
            calltext = call[:i] + ".."
        elif call[-1] == ';':
            calltext = call[:-1]
        else:
            calltext = call

    for lib_name in libs:

        lang, suffix, msg = SCons.Conftest._lang2suffix(language)
        if msg:
            context.Display("Cannot check for library %s: %s\n" % (lib_name, msg))
            return msg

        # if a function was specified to run in main(), say it
        if call:
                context.Display("Checking for %s in %s library %s... "
                                % (calltext, lang, lib_name))
        # otherwise, just say the name of library and language
        else:
                context.Display("Checking for %s library %s... "
                                % (lang, lib_name))

        if lib_name:
            l = [ lib_name ]
            if extra_libs:
                l.extend(extra_libs)
            oldLIBS = context.AppendLIBS(l)
            sym = "HAVE_LIB" + lib_name
        else:
            oldLIBS = -1
            sym = None

        context.sconf.cached = 1
        ret = not context.TryBuild(context.env.SharedLibrary, text, suffix)

        SCons.Conftest._YesNoResult(context, ret, sym, text,
                                    "Define to 1 if you have the `%s' library." % lib_name)
        if oldLIBS != -1 and (ret or not autoadd):
            context.SetLIBS(oldLIBS)
            
        if not ret:
            return ret

    return ret

def CheckSharedLibWithHeader(context, libs, header, language,
                             call = None, autoadd = 1):
    prog_prefix, dummy = \
                 SCons.SConf.createIncludesFromHeaders(header, 0)
    if libs == []:
        libs = [None]

    if not SCons.Util.is_List(libs):
        libs = [libs]

    res = Conftest_CheckSharedLib(context, libs, None, prog_prefix,
                                  call = call, language = language, autoadd = autoadd)
    context.did_show_result = 1
    return not res
