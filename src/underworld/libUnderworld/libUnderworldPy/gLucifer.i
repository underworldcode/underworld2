/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

#if (SWIG_VERSION <= 0x040000)
%module (package="underworld.libUnderworld.libUnderworldPy") gLucifer
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") gLucifer
#endif


%{
/* Includes the header in the wrapper code */
#include <cmath>
%}


%include "exception.i"
%import "StGermain.i"
%import "StgDomain.i"
%import "StgFEM.i"
%import "PICellerator.i"

%include "carrays.i"
%include "cdata.i"
/* Create some functions for working with "float *" */
%array_functions(float, farray)

%apply (float* INPLACE_ARRAY1, int DIM1) {(float* vertices, int V), (float* values, int N)};

%{
/* Includes the header in the wrapper code */
#include <mpi.h>
#include <petsc.h>
#include <vector>

extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <gLucifer/libgLucifer/src/gLucifer.h>
}
%}


%include "gLucifer/Base/src/types.h"
%include "gLucifer/Base/src/ColourMap.h"
%include "gLucifer/Base/src/DrawingObject.h"
%include "gLucifer/Base/src/Database.h"
%include "gLucifer/Base/src/Finalise.h"
%include "gLucifer/Base/src/Init.h"
%include "gLucifer/Base/src/ViewerTypes.h"
%include "gLucifer/DrawingObjects/src/CrossSection.h"
%include "gLucifer/DrawingObjects/src/SwarmViewer.h"
%include "gLucifer/DrawingObjects/src/VectorArrows.h"
%include "gLucifer/DrawingObjects/src/MeshViewer.h"
%include "gLucifer/DrawingObjects/src/DrawingObjects.h"
%include "gLucifer/DrawingObjects/src/Isosurface.h"
%include "gLucifer/DrawingObjects/src/ContourCrossSection.h"
%include "gLucifer/DrawingObjects/src/ScalarField.h"
%include "gLucifer/DrawingObjects/src/FieldSampler.h"
%include "gLucifer/DrawingObjects/src/Sampler.h"
%include "gLucifer/DrawingObjects/src/types.h"
%include "gLucifer/libgLucifer/src/Finalise.h"
%include "gLucifer/libgLucifer/src/gLucifer.h"
%include "gLucifer/libgLucifer/src/Init.h"




