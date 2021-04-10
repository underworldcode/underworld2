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
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include <Underworld/Rheology/Rheology.h>
#include <Underworld/Utils/Utils.h>

#include "Finalise.h"

#include <stdio.h>

Bool Underworld_Finalise( void ) {
	if( ToolboxesManager_IsInitialised( stgToolboxesManager, "Underworld" ) ) {
		Underworld_Utils_Finalise();
		Underworld_Rheology_Finalise();
		
		Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

		return True;
	} else {
		return False;
	}
}


