import os
import sys
import py_compile
import subprocess, platform

from SCons.Script import *

##This is a documentation generation toolbox for scons.
# It allows the user to pick which documentation they are interested in based on
# a set of options - see Help information on this page for details on how to execute.


#Notes for developers:
# If you want to create a new document-style to add to the list of options, you will need to:
# 1. Create a def addMyOptionSuite command in this file
# 2. Set up any information to Import / Export from other Sconscript commands here and
#    in the Sconscript files of interest.
# 3. Add an alias command for scons to execute ie. "doc-MyOption". This should be in the addMyOptionSuite
#     function. It will also need an AlwaysBuild("doc-myOption") command as well. 
# 4. Add an env.AddMethod(addMyOptionSuite, "AddMyOptionSuite") command to the bottom of the generate 
#    script - after the def commands in this file.
# 5. Add a description in the Help section below for "doc-MyOption"
# 6. Add a env.Alias("docMyOption", Action(env.AddMyOptionSuite())) command followed by a
#    AlwaysBuild("docMyOption") to the stgUnderworld SConstruct file 
#    (near the bottom after the SConscript calls)
# Then you will need to run 'scons'. And then you should be able to run 'scons doc-MyOption'

class ToolDocWarning(SCons.Warnings.Warning):
    pass

SCons.Warnings.enableWarningClass(ToolDocWarning)

def generate(env, **kw):
    # Use the generate script to create a suite of executable
    # scons commands to generate the different documents
    stgBaseDir = os.path.abspath(os.curdir)
    docPath = os.path.abspath('StGermain'+ os.sep +'script')
    sys.path.insert(0, docPath)
    #import uwa.systest.systestrunner
    os.environ['STG_BASEDIR'] = stgBaseDir
    # Set up the environment for sub-scripts
    env['ENV']['STG_BASEDIR'] = stgBaseDir
    env.PrependENVPath('PATH', docPath)
    env.PrependENVPath('PYTHONPATH', "%s" % docPath)

    #These dictionaries will be filled by the Sconscript file for each toolbox. 
    DOXYGEN_SUITES = {}
    DOXYGEN_DIRS = {}
    DOXYGEN_PLUGINS = {}

    # Need to use Export rather than saving on env object, since we clone
    #  the env for each sub-project

    Export('DOXYGEN_SUITES')
    Export('DOXYGEN_DIRS')
    Export('DOXYGEN_PLUGINS')


    # This will append to the standard help with testing help.
    Help("""
SCons-Doc Options:
    Type: './scons.py doc' to run all document generation scripts for stgUnderworld,
          './scons.py doc-doxygen' to run the doxygen document generation script for stgUnderworld,
          './scons.py doc-doxygenlite' to run the doxygen lite document generation script for stgUnderworld (No graphs, ONLY plugins),
          './scons.py doc-codex' to run the codex generation script for stgUnderworld,
""" )

    #Sets up Doxygen creation script Alias and passes some variables through.
    def addDoxygenSuite(env):
        Import('DOXYGEN_SUITES')

        Import('DOXYGEN_PLUGINS')
        Import('DOXYGEN_DIRS')
        
        DOXYGEN_SUITES['Plugins'] = DOXYGEN_PLUGINS
        DOXYGEN_SUITES['Dirs'] = DOXYGEN_DIRS
        ## Pass in values from scons to the executions script now!
        #Add in command to run doxygen code.
        #TODO: Why are these python scripts executed this way? Because I couldn't
        #figure out how to make it work when I executed the functions directly.
        # --KathleenHumble 27/08/2010
        env.Alias("doc-doxygen",None, Action("StGermain/script/createDoxygen.py . ./doc ./StGermain/doc/ Yes '' "+ '"'+ str(DOXYGEN_DIRS)+'"'))
        env.AlwaysBuild("doc-doxygen")
        Export('DOXYGEN_SUITES')
        Export('DOXYGEN_PLUGINS')
        Export('DOXYGEN_DIRS')

    #Sets up Doxygen creation script Alias and passes some variables through.
    def addDoxygenLiteSuite(env):
        Import('DOXYGEN_SUITES')

        Import('DOXYGEN_PLUGINS')
        Import('DOXYGEN_DIRS')
        
        DOXYGEN_SUITES['Plugins'] = DOXYGEN_PLUGINS
        DOXYGEN_SUITES['Dirs'] = DOXYGEN_DIRS
        ##Pass in values from scons to the executions script now!
        #Add in command to run doxygen code.
        #TODO: Why are these python scripts executed this way? Because I couldn't
        #figure out how to make it work when I executed the functions directly.
        # --KathleenHumble 27/08/2010
        env.Alias("doc-doxygenlite",None, Action("StGermain/script/createDoxygen.py . ./doc ./StGermain/doc/ No '' "+ '"'+ str(DOXYGEN_DIRS)+'"'))
        env.AlwaysBuild("doc-doxygenlite")
        Export('DOXYGEN_SUITES')
        Export('DOXYGEN_PLUGINS')
        Export('DOXYGEN_DIRS')   
     
    #Sets up codex creation script Alias
    def addCodexSuite(env):

        env.Alias("doc-codex", None, Action("StGermain/script/createCodex.py '' '' '' '.' './doc' "))
        env.AlwaysBuild("doc-codex")        

    #Sets up Doxygen and Codex creation script Alias and passes some variables through.
    def addAllSuite(env):
        Import('DOXYGEN_SUITES')
        Import('DOXYGEN_PLUGINS')
        Import('DOXYGEN_DIRS')
        DOXYGEN_SUITES['Plugins'] = DOXYGEN_PLUGINS
        DOXYGEN_SUITES['Dirs'] = DOXYGEN_DIRS

        env.Alias("doc", None, Action("StGermain/script/createDocs.py '' '' '' '' '' " + '"'+ str(DOXYGEN_DIRS)+'"'))
        env.AlwaysBuild("doc")
        Export('DOXYGEN_SUITES')
        Export('DOXYGEN_PLUGINS')
        Export('DOXYGEN_DIRS')

    #Exporting these functions as methods so they can be executed later  in the SConstruct phase ...
    env.AddMethod(addDoxygenSuite, "AddDoxygenSuite")
    env.AddMethod(addCodexSuite, "AddCodexSuite")
    env.AddMethod(addAllSuite, "AddAllSuite")
    env.AddMethod(addDoxygenLiteSuite, "AddDoxygenLiteSuite")
    
def exists(env):
    # Should probably have this search for the 
    # libraries/source or something.
    return True        
