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
%module (package="underworld.libUnderworld.libUnderworldPy") PICellerator
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") PICellerator
#endif


%{
/* Includes the header in the wrapper code */
//#define SWIG_FILE_WITH_INIT
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
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

%include "PICellerator/MaterialPoints/src/IntegrationPointsSwarm.h"
%include "PICellerator/MaterialPoints/src/CoincidentMapper.h"
%include "PICellerator/MaterialPoints/src/SwarmMap.h"
%include "PICellerator/MaterialPoints/src/GeneralSwarm.h"
%include "PICellerator/MaterialPoints/src/SwarmAdvector.h"
%include "PICellerator/MaterialPoints/src/types.h"
%include "PICellerator/PopulationControl/src/types.h"
%include "PICellerator/PopulationControl/src/EscapedRoutine.h"
%include "PICellerator/Weights/src/WeightsCalculator.h"
%include "PICellerator/Weights/src/DVCWeights.h"
%include "PICellerator/Weights/src/types.h"
%include "PICellerator/Utils/src/PCDVC.h"
%include "PICellerator/Utils/src/types.h"

%extend GeneralSwarm
{
    PyObject* GeneralSwarm_AddParticlesFromCoordArray( int DIM1, int DIM2, double* IN_ARRAY2 ){
        return GeneralSwarm_AddParticlesFromCoordArray( $self, DIM1, DIM2, IN_ARRAY2 );
    }
}
