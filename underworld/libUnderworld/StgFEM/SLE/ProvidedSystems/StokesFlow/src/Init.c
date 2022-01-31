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
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "StgFEM/Discretisation/src/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/src/SystemSetup.h"

#include "StokesFlow.h"

#include <stdio.h>

Stream* StgFEM_SLE_ProvidedSystems_StokesFlow_Debug = NULL;

/** Initialises the Linear Algebra package, then any init for this package
such as streams etc */
Bool StgFEM_SLE_ProvidedSystems_StokesFlow_Init( int* argc, char** argv[] ) {

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	/* initialise this level's streams */
	StgFEM_SLE_ProvidedSystems_StokesFlow_Debug = Stream_RegisterChild( StgFEM_SLE_Debug,
		"ProvidedSystems_StokesFlow" );

	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Stokes_SLE_Type , (Name)"0", _Stokes_SLE_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), Stokes_SLE_UzawaSolver_Type , "0", _Stokes_SLE_UzawaSolver_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Stokes_SLE_PenaltySolver_Type , (Name)"0", Stokes_SLE_PenaltySolver_DefaultNew );  
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), UzawaPreconditionerTerm_Type , "0", _UzawaPreconditionerTerm_DefaultNew );

	RegisterParent( Stokes_SLE_Type,               		SystemLinearEquations_Type );
	RegisterParent( Stokes_SLE_PenaltySolver_Type, 		SLE_Solver_Type );
	RegisterParent( Stokes_SLE_UzawaSolver_Type,   		SLE_Solver_Type );
	RegisterParent( UzawaPreconditionerTerm_Type,  		StiffnessMatrixTerm_Type );
	
	return True;
}


