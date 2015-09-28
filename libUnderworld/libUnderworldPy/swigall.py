#!/usr/bin/env python
import sys, subprocess, shutil, os, glob	

dir = os.path.join( '..', 'build')

swigcommandbase = 'swig -v -Wextra -python -importall -ignoremissing -O -c++ -DSWIG_DO_NOT_WRAP'
swigheaders  =  ' -I'+os.path.join( dir, 'include')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StGermain')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StgDomain')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'StgFEM')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'Solvers')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'PICellerator')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'Underworld')
swigheaders +=  ' -I'+os.path.join( dir, 'include', 'gLucifer')
#swigheaders +=  ' -I'+os.path.join( dir, 'include', 'ImportersToolbox')
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
              "Solvers.i",
              "PICellerator.i",
              "Underworld.i",
              "gLucifer.i",
             ]
#
for swigfile in swigfiles:
	print ""
	print "executing command:"
	print swigcommandbase + swigheaders + ' ' + swigfile
	print ""
	subp = subprocess.Popen(swigcommandbase + swigheaders + ' ' + swigfile, shell=True )
	subp.wait()

swigfilescpp = [ "Function.i",]
swigcommandbase = 'swig -Wextra -python -ignoremissing -O -c++ -DSWIG_DO_NOT_WRAP'
#swigcommandbase = 'swig -Wextra -python -importall -ignoremissing -O -c++'

for swigfile in swigfilescpp:
	print ""
	print "executing command:"
	print swigcommandbase + swigheaders + ' ' + swigfile
	print ""
	subp = subprocess.Popen(swigcommandbase + swigheaders + ' ' + swigfile, shell=True )
	subp.wait()
