/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PETSC_EXT_PC_BFBT_H__
#define __PETSC_EXT_PC_BFBT_H__

#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>

PetscErrorCode BSSCR_PCCreate_GtKG( PC pc );

PetscErrorCode BSSCR_PCGtKGSet_Operators( PC pc, Mat K, Mat G, Mat M );
PetscErrorCode BSSCR_PCGtKGGet_KSP( PC pc, KSP *ksp );
PetscErrorCode BSSCR_PCGtKGAttachNullSpace( PC pc );
PetscErrorCode BSSCR_PCGtKGSet_OperatorForAlgebraicCommutator( PC pc, Mat M );
PetscErrorCode BSSCR_PCGtKGSet_KSP( PC pc, KSP ksp );

#endif
