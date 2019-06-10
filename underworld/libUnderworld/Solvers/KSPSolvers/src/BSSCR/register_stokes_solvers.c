/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>

#include "common-driver-utils.h"
#include "pc_GtKG.h"
#include "pc_ScaledGtKG.h"


PetscErrorCode BSSCR_PetscExtStokesSolversInitialize( void )
{
    Stg_PCRegister( "gtkg", "Solvers/KSPSolvers/src/BSSCR", "BSSCR_PCCreate_GtKG", BSSCR_PCCreate_GtKG );
	//Stg_PCRegister( "bfbt", "Solvers/KSPSolvers/src/BSSCR", "BSSCR_PCCreate_GtKG", BSSCR_PCCreate_GtKG );
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_PetscExtStokesSolversFinalize( void )
{
	
#if ((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=4))
//  PCFinalizePackage();
#else
//  PCRegisterDestroy();
#endif
	
	PetscFunctionReturn(0);
}



