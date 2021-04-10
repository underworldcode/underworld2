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
#include <PICellerator/PICellerator.h>

#include "Rheology.h"

Bool Underworld_Rheology_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

	Stg_ComponentRegister_Add( componentRegister, ConstitutiveMatrixCartesian_Type, (Name)"0", _ConstitutiveMatrixCartesian_DefaultNew  );
	RegisterParent( ConstitutiveMatrix_Type,          StiffnessMatrixTerm_Type );
	RegisterParent( ConstitutiveMatrixCartesian_Type, ConstitutiveMatrix_Type );

	return True;
}


