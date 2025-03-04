/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __KSP_PNS_h__
#define __KSP_PNS_h__
PetscErrorCode KSPBuildPressure_CB_Nullspace_BSSCR(KSP ksp);
PetscErrorCode KSPBuildPressure_Const_Nullspace_BSSCR(KSP ksp);
PetscErrorCode KSPRemovePressureNullspace_BSSCR(KSP ksp, Vec h);
#endif

