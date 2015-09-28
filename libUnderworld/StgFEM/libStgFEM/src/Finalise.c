/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SLE.h"
#include "StgFEM/Assembly/Assembly.h"
#include "StgFEM/Utils/Utils.h"
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


