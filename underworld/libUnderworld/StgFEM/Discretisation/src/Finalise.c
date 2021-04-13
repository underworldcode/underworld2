/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "types.h"

#include "ElementType_Register.h"
#include "Finalise.h"

#include "FeVariable.h"

#include <stdio.h>

Bool StgFEM_Discretisation_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	Stream_IndentBranch( StgFEM_Debug );

	Stg_Class_Delete( elementType_Register );
	elementType_Register=NULL;

	PetscFinalize();
	
	Stream_UnIndentBranch( StgFEM_Debug );
	return True;
}


