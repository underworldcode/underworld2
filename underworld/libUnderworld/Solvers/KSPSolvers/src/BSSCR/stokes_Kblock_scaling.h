/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __STOKES_SOLVER_PETSC_EXT_MAT_KBLOCK_SCALING_H__
#define __STOKES_SOLVER_PETSC_EXT_MAT_KBLOCK_SCALING_H__


//typedef struct _p_MatStokesBlockScaling* MatStokesBlockScaling;


PetscErrorCode BSSCR_MatStokesKBlockScalingCreate( MatStokesBlockScaling *_BA );
PetscErrorCode BSSCR_MatStokesKBlockScalingDestroy( MatStokesBlockScaling BA );
PetscErrorCode BSSCR_MatStokesKBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesKBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesKBlockReportOperatorScales( Mat A, PetscTruth sym );

#endif
