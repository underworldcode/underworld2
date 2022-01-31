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
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>

#include "Solvers.h"

#include <stdio.h>

Bool Solvers_Finalise( void ) {
	if( ToolboxesManager_IsInitialised( stgToolboxesManager, "Solvers" ) ) {
		Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

		Solvers_SLE_Finalise();
		Solvers_Assembly_Finalise();
		Solvers_KSPSolvers_Finalise();

#ifdef HAVE_PETSCEXT
		PetscExtFinalize( );
#endif

		return True;
	} else {
		return False;
	}
}


