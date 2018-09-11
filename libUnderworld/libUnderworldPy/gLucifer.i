/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module gLucifer

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
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <gLucifer/gLucifer.h>
}
%}


%include "Base/types.h"
%include "Base/ColourMap.h"
%include "Base/DrawingObject.h"
%include "Base/Database.h"
%include "Base/Finalise.h"
%include "Base/Init.h"
%include "Base/ViewerTypes.h"
%include "DrawingObjects/CrossSection.h"
%include "DrawingObjects/SwarmViewer.h"
%include "DrawingObjects/VectorArrows.h"
%include "DrawingObjects/MeshViewer.h"
%include "DrawingObjects/DrawingObjects.h"
%include "DrawingObjects/Isosurface.h"
%include "DrawingObjects/ContourCrossSection.h"
%include "DrawingObjects/ScalarField.h"
%include "DrawingObjects/FieldSampler.h"
%include "DrawingObjects/Sampler.h"
%include "DrawingObjects/types.h"
%include "libgLucifer/Finalise.h"
%include "libgLucifer/gLucifer.h"
%include "libgLucifer/Init.h"




