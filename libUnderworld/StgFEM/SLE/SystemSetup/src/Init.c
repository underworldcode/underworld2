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
#include <StgDomain/StgDomain.h>

#include "StgFEM/Discretisation/Discretisation.h"
#include "SystemSetup.h"


Stream* StgFEM_SLE_Debug = NULL;
Stream* StgFEM_SLE_SystemSetup_Debug = NULL;

/** Initialises the Linear Algebra package, then any init for this package
such as streams etc */
Bool StgFEM_SLE_SystemSetup_Init( int* argc, char** argv[] ) {
	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
	
	/* initialise this level's streams */
	StgFEM_SLE_Debug = Stream_RegisterChild( StgFEM_Debug, "SLE" );
	StgFEM_SLE_SystemSetup_Debug = Stream_RegisterChild( StgFEM_SLE_Debug, "SystemSetup" );
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), FiniteElementContext_Type, (Name)"0", FiniteElementContext_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), ForceVector_Type, "0", _ForceVector_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SolutionVector_Type, (Name)"0", _SolutionVector_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), StiffnessMatrix_Type, "0", StiffnessMatrix_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), StiffnessMatrixTerm_Type, (Name)"0", _StiffnessMatrixTerm_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), SystemLinearEquations_Type, "0", _SystemLinearEquations_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ForceTerm_Type, (Name)"0", _ForceTerm_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SROpGenerator_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)SROpGenerator_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister( ), 
				   PETScMGSolver_Type, "0", 
				   (Stg_Component_DefaultConstructorFunction*)PETScMGSolver_New );

	RegisterParent( SystemLinearEquations_Type,    Stg_Component_Type );
	RegisterParent( SLE_Solver_Type,               Stg_Component_Type );
	RegisterParent( StiffnessMatrix_Type,          Stg_Component_Type );
	RegisterParent( StiffnessMatrixTerm_Type,      Stg_Component_Type );
	RegisterParent( SolutionVector_Type,           Stg_Component_Type );
	RegisterParent( ForceVector_Type,              SolutionVector_Type );
	RegisterParent( ForceTerm_Type,                Stg_Component_Type );
	RegisterParent( Assembler_Type, Stg_Class_Type );
	RegisterParent( FiniteElementContext_Type,     DomainContext_Type );
	RegisterParent( PETScMGSolver_Type, Stg_Component_Type );
	RegisterParent( MGOpGenerator_Type, Stg_Component_Type );
	RegisterParent( SROpGenerator_Type, MGOpGenerator_Type);

	return True;
}


