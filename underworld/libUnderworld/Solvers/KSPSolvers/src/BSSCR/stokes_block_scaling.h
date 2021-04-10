/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __STOKES_MAT_BLOCK_SCALING_H__
#define __STOKES_MAT_BLOCK_SCALING_H__

struct _p_MatStokesBlockScaling {
      Vec             Lz, Rz;
      PetscTruth      scaling_exists, scalings_have_been_inverted, system_has_been_scaled; 
};

typedef struct _p_MatStokesBlockScaling* MatStokesBlockScaling;


PetscErrorCode BSSCR_MatStokesBlockScalingCreate( MatStokesBlockScaling *_BA );
PetscErrorCode BSSCR_MatStokesBlockScalingDestroy( MatStokesBlockScaling BA );
PetscErrorCode BSSCR_MatStokesBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesBlockScaleSystem2( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, Mat M, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatStokesBlockReportOperatorScales( Mat A, PetscTruth sym );
PetscErrorCode BSSCR_MatBlock_ConstructScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x );
PetscErrorCode BSSCR_MatStokesBlockDefaultBuildScaling( MatStokesBlockScaling BA, Mat A );
PetscErrorCode BSSCR_mat_block_invert_scalings( MatStokesBlockScaling BA );

#endif
