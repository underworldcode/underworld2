/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "Utils.h"


Bool StgDomainUtils_Init( int* argc, char** argv[] ) {

    Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ );
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), DomainContext_Type, (Name)"0", (void* (*)(Name))_DomainContext_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), DofLayout_Type, "0", (void* (*)(Name))_DofLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), FieldVariable_Type, (Name)"0", (void* (*)(Name))_FieldVariable_DefaultNew );

	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TimeIntegrator_Type, (Name)"0", (void*  (*)(Name))_TimeIntegrator_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), TimeIntegrand_Type, "0", (void*  (*)(Name))_TimeIntegrand_DefaultNew );

	RegisterParent( DomainContext_Type, AbstractContext_Type );
	RegisterParent( DofLayout_Type, Stg_Component_Type );
	RegisterParent( FieldVariable_Type, Stg_Component_Type );
	RegisterParent( FieldVariable_Register_Type, NamedObject_Register_Type );
	RegisterParent( TimeIntegrand_Type, Stg_Component_Type );
	RegisterParent( TimeIntegrator_Type, Stg_Component_Type );
	
	return True;
}


