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

#include "PopulationControl.h"

#include <stdio.h>

Bool PICellerator_PopulationControl_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentsRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( componentsRegister, EscapedRoutine_Type, (Name)"0", _EscapedRoutine_DefaultNew  );
	RegisterParent( EscapedRoutine_Type,      	 Stg_Component_Type );

	return True;
}


