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
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"
#include "types.h"
#include "Init.h"
#include "AdvectionDiffusion.h"

#include <stdio.h>

Stream* StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_Debug = NULL;

/** Initialises the Linear Algebra package, then any init for this package
such as streams etc */
Bool StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	/* initialise this level's streams */
	StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_Debug = Stream_RegisterChild( StgFEM_SLE_Debug,
		"ProvidedSystems_AdvectionDiffusion" );

	Stg_ComponentRegister_Add( componentRegister, AdvectionDiffusionSLE_Type, (Name)"0", _AdvectionDiffusionSLE_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, AdvDiffResidualForceTerm_Type, (Name)"0", _AdvDiffResidualForceTerm_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, LumpedMassMatrixForceTerm_Type, (Name)"0", _LumpedMassMatrixForceTerm_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, AdvDiffMulticorrector_Type, (Name)"0", _AdvDiffMulticorrector_DefaultNew  );

	RegisterParent( AdvectionDiffusionSLE_Type,     SystemLinearEquations_Type );
	RegisterParent( AdvDiffResidualForceTerm_Type,  ForceTerm_Type );
	RegisterParent( LumpedMassMatrixForceTerm_Type, ForceTerm_Type );
	RegisterParent( AdvDiffMulticorrector_Type,     SLE_Solver_Type );
	
	return True;
}


