/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __PETSC_EXT_PC_SCALEDBFBT_H__
#define __PETSC_EXT_PC_SCALEDBFBT_H__

#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>

PetscErrorCode BSSCR_PCCreate_ScGtKG( PC pc );

PetscErrorCode BSSCR_PCScGtKGGetScalings( PC pc, Vec *X1, Vec *X2, Vec *Y1, Vec *Y2 );
PetscErrorCode BSSCR_PCScGtKGSetOperators( PC pc, Mat F, Mat Bt, Mat B, Mat C );
PetscErrorCode BSSCR_PCScGtKGAttachNullSpace( PC pc );
PetscErrorCode BSSCR_PCScGtKGGetKSP( PC pc, KSP *ksp );
PetscErrorCode BSSCR_PCScGtKGSetKSP( PC pc, KSP ksp );
PetscErrorCode BSSCR_PCScGtKGSetSubKSPMonitor( PC pc, PetscTruth flg );
PetscErrorCode BSSCR_PCScGtKGUseStandardScaling( PC pc );
PetscErrorCode BSSCR_PCScGtKGUseStandardBBtOperator( PC pc );

#endif
