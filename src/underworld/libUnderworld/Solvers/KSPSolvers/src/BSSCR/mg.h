/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __BSSCR_MG_h__
#define __BSSCR_MG_h__

#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
#include "Solvers/KSPSolvers/src/KSPSolvers.h"

#include "common-driver-utils.h"
#include "BSSCR.h" /* includes StokesBlockKSPInterface.h */

#define KSPBSSCR         "bsscr"

typedef struct {
  KSP ksp;
  PC pc;

  PetscTruth useAcceleratingSmoothingMG;
  PetscTruth acceleratingSmoothingMGView;

  /* mg_accelerating_smoothing options */
  
  PetscInt smoothsMax;
  PetscInt smoothsToStartWith;
  PetscInt currentNumberOfSmooths;
  PetscInt smoothingIncrement;
  PetscInt targetCyclesForTenfoldReduction;
  
  PetscInt smoothingCountThisSolve;
  PetscInt totalSmoothingCount;
  PetscInt totalMgCycleCount;  
} MGContext;

PetscErrorCode KSPCycleEffectivenessMonitorAndAdjust(KSP ksp, PetscInt n, PetscReal rnorm, void *_mgctx );
//PetscErrorCode MG_inner_solver_mgContext_initialise(MGContext *mgCtx);
PetscErrorCode MG_inner_solver_pcmg_shutdown( PC pc_MG );
//PetscErrorCode BSSCR_mgPCApply( void *ctx, Vec x, Vec y );
//PetscErrorCode BSSCR_mgPCAccelerating( void *ctx, Vec x, Vec y );
double setupMG( KSP_BSSCR * bsscrp_self, KSP ksp_inner, PC pc_MG, Mat K, MGContext *mgCtx );
#endif

