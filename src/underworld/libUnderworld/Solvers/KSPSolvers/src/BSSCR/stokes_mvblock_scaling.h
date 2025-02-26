/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __STOKES_SOLVER_PETSC_EXT_MAT_MVBLOCK_SCALING_H__
#define __STOKES_SOLVER_PETSC_EXT_MAT_MVBLOCK_SCALING_H__


//typedef struct _p_MatStokesBlockScaling* MatStokesBlockScaling;


PetscErrorCode BSSCR_MatStokesMVBlockScalingCreate( MatStokesBlockScaling *_BA );
PetscErrorCode BSSCR_MatStokesMVBlockScalingDestroy( MatStokesBlockScaling BA );
PetscErrorCode BSSCR_MatStokesMVBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesMVBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesMVBlockReportOperatorScales( Mat A, PetscTruth sym );

#endif
