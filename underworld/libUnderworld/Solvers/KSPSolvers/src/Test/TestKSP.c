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

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include <Solvers/Solvers.h>

#include "petsccompat.h"

#include "TestKSP.h"

#define KSPTEST         "test"

EXTERN_C_BEGIN
EXTERN PetscErrorCode PETSCKSP_DLLEXPORT KSPCreate_TEST(KSP);
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "KSPRegisterTEST"
PetscErrorCode PETSCKSP_DLLEXPORT KSPRegisterTEST(const char path[])
{
    PetscErrorCode ierr;

    PetscFunctionBegin;
    ierr = Stg_KSPRegister(KSPTEST, path, "KSPCreate_TEST", KSPCreate_TEST );CHKERRQ(ierr);
    PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "KSPSolve_TEST"
PetscErrorCode  KSPSolve_TEST(KSP ksp)
{
    PetscPrintf( PETSC_COMM_WORLD, "----- In KSP Solver %s\n",__func__);
    PetscFunctionReturn(0);
    
}


#undef __FUNCT__  
#define __FUNCT__ "KSPDestroy_TEST" 
PetscErrorCode KSPDestroy_TEST(KSP ksp)
{
    PetscErrorCode ierr;
    PetscFunctionBegin;
    ierr = KSPDefaultDestroy(ksp);CHKERRQ(ierr);
    PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "KSPSetFromOptions_TEST"
PetscErrorCode KSPSetFromOptions_TEST(KSP ksp)
{

    PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "KSPView_TEST" 
PetscErrorCode KSPView_TEST(KSP ksp,PetscViewer viewer)
{
    PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "KSPSetUp_TEST" 
PetscErrorCode KSPSetUp_TEST(KSP ksp)
{
    PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "KSPCreate_TEST"
PetscErrorCode PETSCKSP_DLLEXPORT KSPCreate_TEST(KSP ksp)
{
    PetscErrorCode ierr;
    PetscFunctionBegin;

    #if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >= 2 ) )
    ierr = KSPSetSupportedNorm(ksp,KSP_NORM_NONE,PC_LEFT,1);CHKERRQ(ierr);
    #endif
    /*
       Sets the functions that are associated with this data structure 
       (in C++ this is the same as defining virtual functions)
    */
    ksp->ops->setup                = KSPSetUp_TEST;
    ksp->ops->solve                = KSPSolve_TEST;
    ksp->ops->destroy              = KSPDestroy_TEST;
    ksp->ops->view                 = KSPView_TEST;
    ksp->ops->setfromoptions       = KSPSetFromOptions_TEST;
    ksp->ops->buildsolution        = KSPDefaultBuildSolution;
    ksp->ops->buildresidual        = KSPDefaultBuildResidual;

    
    PetscFunctionReturn(0);
}
EXTERN_C_END

