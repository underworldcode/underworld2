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

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>
#include <StgDomain/Utils/Utils.h>
#include <StgDomain/Swarm/Swarm.h>

#include "Finalise.h"

#include <stdio.h>

Bool StgDomain_Finalise( void ) {
	if( ToolboxesManager_IsInitialised( stgToolboxesManager, "StgDomain" ) ) {
		Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
		
		StgDomainSwarm_Finalise();
		StgDomainUtils_Finalise();
		StgDomainMesh_Finalise();
		StgDomainShape_Finalise();
		StgDomainGeometry_Finalise();
		return True;
	} else { 
		return False;
	}
}


