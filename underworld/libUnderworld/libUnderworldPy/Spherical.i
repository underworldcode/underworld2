/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") Spherical

%import "StGermain.i"
%import "StgDomain.i"
%import "StgFEM.i"
%import "PICellerator.i"
%import "Underworld.i"

%{
/* Includes the header in the wrapper code */
#include <StGermain/libStGermain/StGermain.h>
#include <StgDomain/libStgDomain/StgDomain.h>
#include <StgFEM/libStgFEM/StgFEM.h>
#include <PICellerator/libPICellerator/PICellerator.h>
#include <Underworld/libUnderworld/Underworld.h>
#include <Spherical/Spherical.h>
%}

%include "Components/src/SphericalGenerator.h"
