#!/usr/bin/env python
import sys, subprocess, shutil, os, glob	

dir = os.path.join( '..', 'build')

swigcommandbase = 'swig -v -Wextra -python -ignoremissing -O -c++ -DSWIG_DO_NOT_WRAP'
swigheaders  =  ' -I'+os.path.join( dir, 'include')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StGermain')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StgDomain')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StgFEM')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'Solvers')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'PICellerator')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'Underworld')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'gLucifer')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'petsc')
swigheaders +=  ' -I'+os.path.join( '..', 'ctools')

swigfiles = [ 
              "c_arrays.i",
              "c_pointers.i",
              "petsc.i",
              "StGermain.i",
              "StGermain_Tools.i",
              "StgDomain.i",
              "StgFEM.i",
              "PICellerator.i",
              "Function.i",
              "Underworld.i",
              "Solvers.i",
              "gLucifer.i",
             ]
#
for swigfile in swigfiles:
    print ""
    print "executing command:"
    print swigcommandbase + swigheaders + ' ' + swigfile
    print ""
    subp = subprocess.Popen(swigcommandbase + swigheaders + ' ' + swigfile, shell=True )
    if subp.wait() != 0 :
        raise RuntimeError("Error generating swig bindings.")