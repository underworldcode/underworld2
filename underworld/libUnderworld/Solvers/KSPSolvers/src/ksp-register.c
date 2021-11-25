/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>
#include <petsc.h>
#include <petscvec.h>
#include <petscmat.h>
#include <petscksp.h>
#include <petscpc.h>
#include <petscsnes.h>

#include <petscversion.h>
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include "petsc/private/kspimpl.h"
  #else
     #include "petsc-private/kspimpl.h"   /*I "petscksp.h" I*/
  #endif
#else
  #include "private/kspimpl.h"   /*I "petscksp.h" I*/
#endif

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>

#include "Solvers/KSPSolvers/src/KSPSolvers.h"
#include "BSSCR/petsccompat.h"

#include "Test/TestKSP.h"
#include "BSSCR/BSSCR.h"

/************************************************************************/
/*** This function is called from _StokesBlockKSPInterface_Initialise **************/
/************************************************************************/
#undef __FUNCT__  
#define __FUNCT__ "KSPRegisterAllKSP"
PetscErrorCode KSPRegisterAllKSP(const char path[])
{

    PetscFunctionBegin;
    KSPRegisterTEST(path);/* not sure if the path matters much here. everything still worked even though I had it wrong */
    KSPRegisterBSSCR (path);
    PetscFunctionReturn(0);
}

