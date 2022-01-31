/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") StGermain_Tools


%{
/* Includes the header in the wrapper code */
#include <string.h>
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include "libUnderworldPy/StGermain_Tools.h"
}

%}

%include "exception.i"
%import "StGermain.i"

/* Parse the header file to generate wrappers */
%include <argcargv.i>
%apply (int ARGC, char **ARGV) { (int argc, char *argv[]) }

%include "StGermain_Tools.h"
