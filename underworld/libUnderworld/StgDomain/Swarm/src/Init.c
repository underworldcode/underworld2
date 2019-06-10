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
#include <StgDomain/Utils/Utils.h>
#include "Swarm.h"
#include "ParticleCommHandler.h"
#include "ParticleMovementHandler.h"
#include "ParticleShadowSync.h"
/*#include "SwarmShapeVC.h"*/


Stream* Swarm_VerboseConfig = NULL;
Stream* Swarm_Debug = NULL;
Stream* Swarm_Warning = NULL;
Stream* Swarm_Error = NULL;


Bool StgDomainSwarm_Init( int* argc, char** argv[] ) {
	Swarm_VerboseConfig = Journal_Register( Info_Type, (Name)"Swarm_VerboseConfig"  );
	Swarm_Debug = Journal_Register( Debug_Type, (Name)"Swarm"  );
	Swarm_Warning = Journal_Register( Error_Type, (Name)"Swarm"  );
	Swarm_Error = Journal_Register( Error_Type, (Name)"Swarm"  );
	
	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), GaussParticleLayout_Type, (Name)"0", _GaussParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), TriGaussParticleLayout_Type, "0", _TriGaussParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), GaussBorderParticleLayout_Type, (Name)"0", _GaussBorderParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), RandomParticleLayout_Type, "0", _RandomParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), SpaceFillerParticleLayout_Type, "0", _SpaceFillerParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SingleCellLayout_Type, (Name)"0", _SingleCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), ElementCellLayout_Type, "0", _ElementCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TriSingleCellLayout_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)_TriSingleCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Swarm_Type, "0", (Stg_Component_DefaultConstructorFunction*)_Swarm_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SwarmVariable_Type, (Name)"0", _SwarmVariable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ParticleShadowSync_Type, (Name)"0", ParticleShadowSync_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), ParticleMovementHandler_Type, "0", ParticleMovementHandler_DefaultNew );

	RegisterParent( CellLayout_Type, Stg_Component_Type );
	RegisterParent( ElementCellLayout_Type, CellLayout_Type );
	RegisterParent( SingleCellLayout_Type, CellLayout_Type );
	RegisterParent( TriSingleCellLayout_Type, CellLayout_Type );

	RegisterParent( ParticleLayout_Type, Stg_Component_Type );

	RegisterParent( GlobalParticleLayout_Type, ParticleLayout_Type );
	RegisterParent( SpaceFillerParticleLayout_Type, GlobalParticleLayout_Type );

	RegisterParent( PerCellParticleLayout_Type, ParticleLayout_Type );
	RegisterParent( GaussParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( RandomParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( TriGaussParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( GaussBorderParticleLayout_Type,   GaussParticleLayout_Type );
		
	RegisterParent( SwarmVariable_Register_Type, NamedObject_Register_Type );
	RegisterParent( SwarmVariable_Type,         Stg_Component_Type );
	
	RegisterParent( ParticleCommHandler_Type, Stg_Component_Type );
	RegisterParent( ParticleMovementHandler_Type, ParticleCommHandler_Type);
	RegisterParent( ParticleShadowSync_Type, ParticleCommHandler_Type);
	RegisterParent( Swarm_Type, Stg_Component_Type );
	RegisterParent( Swarm_Register_Type, Stg_Class_Type );

	/* Create the singletons */
	stgSwarm_Register = Swarm_Register_New(); 

	return True;
}


