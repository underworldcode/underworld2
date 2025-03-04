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

#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>

#include "types.h"
#include "shortcuts.h"
#include "Finalise.h"

#include <stdio.h>

Bool StgDomainMesh_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	return True;
}


