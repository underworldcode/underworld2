from SCons.Script import *
#from SCons.Builder import Builder

def multiget(dicts, key, default=None):
    for d in dicts:
        if d.has_key(key):
            return d[key]
    else:
        return default

def build_suite_runner(env, target, hdrs, objs, **kw):
    hdr_txt = ""
    suite_txt = ""
    init = multiget([kw, env], "PCU_INIT", "")

    bld_tests_dir = os.path.join( env['build_dir'], "tests" )

    libheaders = multiget([kw, env], "PCU_LIBHEADERS", "")

    project_name = multiget([kw, env], "PROJECT", "")

    setup = multiget([kw, env], "PCU_SETUP", "")
    if setup:
        setup = '\n   ' + setup

    teardown = multiget([kw, env], "PCU_TEARDOWN", "")
    if teardown:
        teardown = '\n   ' + teardown

    for h in hdrs:
        name = os.path.splitext(os.path.basename(h.path))[0]
        moduleDir = os.path.split( os.path.dirname( h.path ) )[0]
        suite_txt += "   pcu_runner_addSuite( %s, %s, %s );\n"%(name, name + init, moduleDir )
        hdr_txt += "#include \"%s\"\n"%str(h.abspath)

    src = """#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <pcu/src/pcu.h>
%s
%s

int main( int argc, char* argv[] ) {
   pcu_listener_t*   textlsnr;
   pcu_listener_t*   xmllsnr;
   PCU_Runner_Status result;

   chdir( "%s" );

   MPI_Init( &argc, &argv );
   pcu_runner_init( argc, argv );%s

%s
   textlsnr = pcu_textoutput_create( "%s", PCU_PRINT_DOCS );
   xmllsnr = pcu_xmloutput_create( "%s", PCU_PRINT_DOCS, "test-results" );
   pcu_runner_associateListener( textlsnr );
   pcu_runner_associateListener( xmllsnr );
   result = pcu_runner_run();
   pcu_textoutput_destroy( textlsnr );
   pcu_xmloutput_destroy( xmllsnr );
%s
   pcu_runner_finalise();
   MPI_Finalize();
   if ( result == PCU_RUNNER_ALLPASS ) {
      return EXIT_SUCCESS;
   }
   else {
      return EXIT_FAILURE;
   }
}
"""%(libheaders, hdr_txt, bld_tests_dir, setup, suite_txt, project_name, project_name, teardown)

    dir_path = os.path.dirname(target.abspath)
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)
    f = open(target.abspath, "w")
    f.write(src)
    f.close()

    return File(target.abspath)

def generate(env, **kw):
    env.SetDefault(CHECK_DEFAULT_TARGET="check")
    env.SetDefault(CHECK_COMPLETE_TARGET="check-complete")
    env.SetDefault(CHECK_UNIT_TARGET="check-unit")
        # Make the 'all' target point to everything.

    def PCUSuite(env, target, source, **kw):
        """Create an object/header pair out of a
        *Suite.c/*Suite.h pair. The target should just
        be the name of the suite. So, if target were
        "Happy", the sources would be "HappySuite.c" and
        "HappySuite.h" """

        obj = env.Object(target[0], source[0])
        return [obj + [File(os.path.splitext(source[0].abspath)[0] + ".h")]]

    def PCUTest(env, target, source, **kw):
        # Generate a list of headers, one for each suite source.
        hdrs = []
        objs = []
        for s in source:
            objs.append(s)
            hdrs.append(File(os.path.splitext(s.srcnode().abspath)[0] + '.h'))

        # Generate the program source.
        prog_src = build_suite_runner(env, File(str(target[0]) + ".c"), hdrs, objs, **kw)

        # Build everything.
        exps = multiget([kw, env], 'PCU_EXP', [])
        inputs = multiget([kw, env], 'PCU_INPUT', [])
        objs = env.StaticObject(os.path.splitext(prog_src.abspath)[0], prog_src) + objs
        libs = multiget([kw, env], 'LIBS', []) + ["pcu"]
        test = env.Program(target[0], objs, LIBS=libs)
        runner = env.Action('-' + test[0].abspath)
        # Adds the current tests to the unit test target
        env.Alias(env["CHECK_UNIT_TARGET"], [exps, inputs, test], runner)
        env.AlwaysBuild(env["CHECK_UNIT_TARGET"])
        # Adds the unit tests to both the default and complete targets
        env.Alias(env['CHECK_COMPLETE_TARGET'], env['CHECK_UNIT_TARGET'])
        env.Alias(env['CHECK_DEFAULT_TARGET'], env['CHECK_UNIT_TARGET'])
        return test

    env.Append(BUILDERS={"PCUSuite": PCUSuite, "PCUTest": PCUTest})


def exists(env):
    # Should probably have this search for the pcu
    # libraries/source or something.
    return True
