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
#include <PICellerator/PopulationControl/PopulationControl.h>
#include <PICellerator/Weights/Weights.h>

#include "MaterialPoints.h"

#include <stdio.h>

Bool PICellerator_MaterialPoints_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentsRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( componentsRegister, PICelleratorContext_Type, (Name)"0", _PICelleratorContext_DefaultNew  );
	
	Stg_ComponentRegister_Add( componentsRegister, IntegrationPointsSwarm_Type, (Name)"0", _IntegrationPointsSwarm_DefaultNew  );
		
	Stg_ComponentRegister_Add( componentsRegister, CoincidentMapper_Type, (Name)"0", _CoincidentMapper_DefaultNew  );
	
	Stg_ComponentRegister_Add( componentsRegister, SwarmAdvector_Type, (Name)"0", _SwarmAdvector_DefaultNew  );

	Stg_ComponentRegister_Add( componentsRegister, PeriodicBoundariesManager_Type, (Name)"0", _PeriodicBoundariesManager_DefaultNew  );
	
	/* Doing this in alphabetical order to match ls output */
	RegisterParent( CoincidentMapper_Type,          Stg_Component_Type );
	RegisterParent( PICelleratorContext_Type,       FiniteElementContext_Type );
	RegisterParent( IntegrationPointsSwarm_Type,    Swarm_Type );
	RegisterParent( SwarmAdvector_Type,             TimeIntegrand_Type );
	
	RegisterParent( SwarmMap_Type,                  Stg_Class_Type );

    Stg_ComponentRegister_Add( componentsRegister, GeneralSwarm_Type, (Name)"0", _GeneralSwarm_DefaultNew  );
    RegisterParent( GeneralSwarm_Type, Swarm_Type );

	return True;
}


