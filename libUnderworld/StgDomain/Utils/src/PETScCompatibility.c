/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "PETScCompatibility.h"

/* wrapper functions for compatibility between Petsc version 3.2 and lower versions */

#if ((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=2))
   #undef __FUNCT__
   #define __FUNCT__ "Stg_MatNullSpaceDestroy"
   PetscErrorCode Stg_MatNullSpaceDestroy(Mat *A){
       return MatNullSpaceDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_MatDestroy"
   PetscErrorCode Stg_MatDestroy(Mat *A){
       return MatDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_VecDestroy"
   PetscErrorCode Stg_VecDestroy(Vec *A){
      return VecDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_KSPDestroy"
   PetscErrorCode Stg_KSPDestroy(KSP *A){
      return KSPDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_PCDestroy"
   PetscErrorCode Stg_PCDestroy(PC *A){
      return PCDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_SNESDestroy"
   PetscErrorCode Stg_SNESDestroy(SNES *A){
      return SNESDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_VecScatterDestroy"
   PetscErrorCode Stg_VecScatterDestroy(VecScatter *A){
      return VecScatterDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_ISDestroy"
   PetscErrorCode Stg_ISDestroy(IS *A){
      return ISDestroy(A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_PetscViewerDestroy"
   PetscErrorCode Stg_PetscViewerDestroy(PetscViewer *A){
      return PetscViewerDestroy(A);
   }

   #undef __FUNCT__  
   #define __FUNCT__ "Stg_MatLoad"
   PetscErrorCode Stg_MatLoad(PetscViewer viewer, const MatType outtype, Mat *newmat){
       PetscErrorCode ierr;
       ierr = MatCreate( PETSC_COMM_SELF, newmat );CHKERRQ(ierr);
       ierr = MatSetType( *newmat, outtype);CHKERRQ(ierr);
       return MatLoad( *newmat, viewer);
   }
   #undef __FUNCT__  
   #define __FUNCT__ "Stg_VecLoad"
   PetscErrorCode Stg_VecLoad(PetscViewer viewer, const VecType outtype, Vec *newvec){
       PetscErrorCode ierr;
       ierr = VecCreate( PETSC_COMM_SELF, newvec );CHKERRQ(ierr);
       ierr = VecSetType( *newvec, outtype);CHKERRQ(ierr);
       return VecLoad( *newvec, viewer);
   }
#else
   #undef __FUNCT__
   #define __FUNCT__ "Stg_MatNullSpaceDestroy"
   PetscErrorCode Stg_MatNullSpaceDestroy(Mat *A){
       return MatNullSpaceDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_MatDestroy"
   PetscErrorCode Stg_MatDestroy(Mat *A){
     return MatDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_VecDestroy"
   PetscErrorCode Stg_VecDestroy(Vec *A){
      return VecDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_KSPDestroy"
   PetscErrorCode Stg_KSPDestroy(KSP *A){
      return KSPDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_PCDestroy"
   PetscErrorCode Stg_PCDestroy(PC *A){
      return PCDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_SNESDestroy"
   PetscErrorCode Stg_SNESDestroy(SNES *A){
      return SNESDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_VecScatterDestroy"
   PetscErrorCode Stg_VecScatterDestroy(VecScatter *A){
      return VecScatterDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_ISDestroy"
   PetscErrorCode Stg_ISDestroy(IS *A){
      return ISDestroy(*A);
   }
   #undef __FUNCT__
   #define __FUNCT__ "Stg_PetscViewerDestroy"
   PetscErrorCode Stg_PetscViewerDestroy(PetscViewer *A){
      return PetscViewerDestroy(*A);
   }

   #undef __FUNCT__  
   #define __FUNCT__ "Stg_MatLoad"
   PetscErrorCode Stg_MatLoad(PetscViewer viewer, const MatType outtype,Mat *newmat){
       return MatLoad( viewer, outtype, newmat);
   }
   #undef __FUNCT__  
   #define __FUNCT__ "Stg_VecLoad"
   PetscErrorCode Stg_VecLoad(PetscViewer viewer, const VecType outtype, Vec *newvec){
       return VecLoad( viewer, outtype, newvec);
   }
#endif


