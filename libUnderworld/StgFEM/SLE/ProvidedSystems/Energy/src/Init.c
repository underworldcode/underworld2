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

#include "Energy_SLE_Solver.h"
#include "Energy_SLE.h"
#include <stdio.h>

Stream* StgFEM_SLE_ProvidedSystems_Energy_Debug = NULL;

/** Initialises the Linear Algebra package, then any init for this package
such as streams etc */
Bool StgFEM_SLE_ProvidedSystems_Energy_Init( int* argc, char** argv[] ) {

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	/* initialise this level's streams */
	StgFEM_SLE_ProvidedSystems_Energy_Debug = Stream_RegisterChild( StgFEM_SLE_Debug, "ProvidedSystems_Energy" );
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Energy_SLE_Solver_Type , (Name)"0", Energy_SLE_Solver_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Energy_SLE_Type , "0", _Energy_SLE_DefaultNew );

	RegisterParent( Energy_SLE_Type,        SystemLinearEquations_Type );
	RegisterParent( Energy_SLE_Solver_Type, SLE_Solver_Type );

	return True;
}


