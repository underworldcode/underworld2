/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module (package="libUnderworld.libUnderworldPy") PICellerator

%{
/* Includes the header in the wrapper code */
//#define SWIG_FILE_WITH_INIT
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}
%}

// not sure why, but in this module we seem to need to comment out the numpy interface things
//%include "numpy.i"
//%init %{
//import_array();
//%}


%include "exception.i"

%import "StGermain.i"
%import "StgDomain.i"
%import "StgFEM.i"


%apply ( int DIM1, int DIM2, double* IN_ARRAY2 ) {(Index count, Index dim, double* array )};

%include "MaterialPoints/IntegrationPointsSwarm.h"
%include "MaterialPoints/CoincidentMapper.h"
%include "MaterialPoints/SwarmMap.h"
%include "MaterialPoints/GeneralSwarm.h"
%include "MaterialPoints/SwarmAdvector.h"
%include "MaterialPoints/types.h"
%include "PopulationControl/types.h"
%include "PopulationControl/EscapedRoutine.h"
%include "Weights/WeightsCalculator.h"
%include "Weights/DVCWeights.h"
%include "Weights/types.h"
%include "Utils/PCDVC.h"
%include "Utils/types.h"

%extend GeneralSwarm
{
    PyObject* GeneralSwarm_AddParticlesFromCoordArray( int DIM1, int DIM2, double* IN_ARRAY2 ){
        return GeneralSwarm_AddParticlesFromCoordArray( $self, DIM1, DIM2, IN_ARRAY2 );
    }
}
