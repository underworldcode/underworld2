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
#include "Mesh.h"


Stream* Mesh_VerboseConfig = NULL;
Stream* Mesh_Debug = NULL;
Stream* Mesh_Warning = NULL;
Stream* Mesh_Error = NULL;


Bool StgDomainMesh_Init( int* argc, char** argv[] ) {
	Mesh_VerboseConfig = Journal_Register( Info_Type, (Name)"Mesh_VerboseConfig"  );
	Mesh_Debug = Journal_Register( Debug_Type, (Name)"Mesh"  );
	Mesh_Warning = Journal_Register( Error_Type, (Name)"Mesh"  );
	Mesh_Error = Journal_Register( Error_Type, (Name)"Mesh"  );

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Mesh_Algorithms_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)Mesh_Algorithms_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Mesh_HexAlgorithms_Type, "0", (Stg_Component_DefaultConstructorFunction*)Mesh_HexAlgorithms_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Mesh_CentroidAlgorithms_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)Mesh_CentroidAlgorithms_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Mesh_RegularAlgorithms_Type, "0", (Stg_Component_DefaultConstructorFunction*)Mesh_RegularAlgorithms_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), CartesianGenerator_Type, "0", (Stg_Component_DefaultConstructorFunction*)_CartesianGenerator_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Mesh_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)Mesh_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), MeshVariable_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)MeshVariable_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Remesher_Type, "0", (Stg_Component_DefaultConstructorFunction*)_Remesher_DefaultNew );

	RegisterParent( Mesh_ElementType_Type, Stg_Class_Type );
	RegisterParent( Mesh_HexType_Type, Mesh_ElementType_Type );
	RegisterParent( Mesh_CentroidType_Type, Mesh_ElementType_Type );
	RegisterParent( Mesh_Algorithms_Type, Stg_Component_Type );
	RegisterParent( Mesh_HexAlgorithms_Type, Mesh_Algorithms_Type );
	RegisterParent( Mesh_CentroidAlgorithms_Type, Mesh_Algorithms_Type );
	RegisterParent( Mesh_RegularAlgorithms_Type, Mesh_Algorithms_Type );
	RegisterParent( MeshTopology_Type, Stg_Component_Type );
	RegisterParent( Mesh_Type, Stg_Component_Type );
	RegisterParent( MeshGenerator_Type, Stg_Component_Type );
	RegisterParent( CartesianGenerator_Type, MeshGenerator_Type );
	RegisterParent( MeshVariable_Type, StgVariable_Type );
	RegisterParent( Remesher_Type, Stg_Component_Type );

	return True;
}


