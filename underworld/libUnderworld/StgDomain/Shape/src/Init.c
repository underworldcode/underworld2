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


#include "Shape.h"

#include <stdio.h>

Bool StgDomainShape_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( componentRegister, ConvexHull_Type, (Name)"0", _ConvexHull_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, PolygonShape_Type, (Name)"0", _PolygonShape_DefaultNew  );

	RegisterParent( Stg_Shape_Type,                 Stg_Component_Type );
	RegisterParent( ConvexHull_Type,                Stg_Shape_Type );
	RegisterParent( PolygonShape_Type,              Stg_Shape_Type );
	
	return True;
}


