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
#include "StgFEM/Discretisation/src/Discretisation.h"
#include "StgFEM/SLE/src/SLE.h"
#include "StgFEM/Assembly/src/Assembly.h"
#include "StgFEM/Utils/src/Utils.h"
#include "Finalise.h"

#include <stdio.h>

Bool StgFEM_Finalise( void ) {
	if( ToolboxesManager_IsInitialised( stgToolboxesManager, "StgFEM" ) ) {
		Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
		
		Stream_IndentBranch( StgFEM_Debug );
		StgFEM_Discretisation_Finalise();
		StgFEM_SLE_Finalise();
		StgFEM_Assembly_Finalise();
		StgFEM_Utils_Finalise();
		Stream_UnIndentBranch( StgFEM_Debug );
		return True;
	} else {
		return False;
	}
}


