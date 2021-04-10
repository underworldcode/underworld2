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

#include "Weights.h"

#include <stdio.h>

Bool PICellerator_Weights_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentsRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( componentsRegister, ConstantWeights_Type, (Name)"0", _ConstantWeights_DefaultNew  );
	Stg_ComponentRegister_Add( componentsRegister, DVCWeights_Type, (Name)"0", _DVCWeights_DefaultNew  );
	
	RegisterParent( WeightsCalculator_Type,      Stg_Component_Type );
	RegisterParent( ConstantWeights_Type,        WeightsCalculator_Type );
	RegisterParent( DVCWeights_Type,             WeightsCalculator_Type );
	
	return True;
}


