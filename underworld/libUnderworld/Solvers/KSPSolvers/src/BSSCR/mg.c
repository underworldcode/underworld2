/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "mg.h"

/******************************************************************************************************/
PetscErrorCode MG_inner_solver_mgContext_initialise(MGContext *mgCtx) {

    PetscTruth found;

    /* 1) Default values into the mg context */

    mgCtx->useAcceleratingSmoothingMG   = PETSC_FALSE;
    mgCtx->acceleratingSmoothingMGView  = PETSC_FALSE;

    mgCtx->smoothsToStartWith = 1;
    mgCtx->smoothsMax = 250;
    mgCtx->smoothingIncrement = 2;
    mgCtx->targetCyclesForTenfoldReduction = 3;
    mgCtx->totalMgCycleCount=0;
    mgCtx->totalSmoothingCount=0;

    /* 2) Read options from petsc dictionary */

    PetscOptionsGetTruth( PETSC_NULL, "-mg_accelerating_smoothing",               &mgCtx->useAcceleratingSmoothingMG,  &found );
    PetscOptionsGetTruth( PETSC_NULL, "-mg_accelerating_smoothing_view",          &mgCtx->acceleratingSmoothingMGView, &found );
    PetscOptionsGetInt(   PETSC_NULL, "-mg_smooths_to_start",                     &(mgCtx->smoothsToStartWith),         PETSC_NULL );
    PetscOptionsGetInt(   PETSC_NULL, "-mg_smooths_max",                          &(mgCtx->smoothsMax),                 PETSC_NULL );
    PetscOptionsGetInt(   PETSC_NULL, "-mg_smoothing_increment",                  &(mgCtx->smoothingIncrement),              PETSC_NULL );
    PetscOptionsGetInt(   PETSC_NULL, "-mg_target_cycles_10fold_reduction",       &(mgCtx->targetCyclesForTenfoldReduction), PETSC_NULL );

    mgCtx->currentNumberOfSmooths = mgCtx->smoothsToStartWith;

    if(mgCtx->acceleratingSmoothingMGView && mgCtx->useAcceleratingSmoothingMG) {
	PetscPrintf( PETSC_COMM_WORLD,  "\nSCR MG OPTIONS\n" );
	PetscPrintf( PETSC_COMM_WORLD,  "  Smooths to start with:     %d\n", mgCtx->smoothsToStartWith );
	PetscPrintf( PETSC_COMM_WORLD,  "  Smooths maximum:           %d\n", mgCtx->smoothsMax );
	PetscPrintf( PETSC_COMM_WORLD,  "  Smoothing increment:       %d\n", mgCtx->smoothingIncrement );
	PetscPrintf( PETSC_COMM_WORLD,  "  Iterations between checks: %d\n\n", mgCtx->targetCyclesForTenfoldReduction );
    }

    PetscFunctionReturn( 0 );
}

PetscErrorCode MG_inner_solver_pcmg_setup( KSP_BSSCR * bsscrp_self, MGContext *mgCtx, KSP ksp_inner, PC pc_MG, Mat K ) {

    PCSetType(pc_MG, PCMG);
    PCMGSetLevels(pc_MG, bsscrp_self->mg->nLevels, PETSC_NULL);

    //PCMGSetType(pc_MG, PC_MG_MULTIPLICATIVE); /* Breaks the accelrating MG */
    
    PCMGSetType(pc_MG, PC_MG_KASKADE);
    #if ((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=2) )
        PCMGSetGalerkin( pc_MG, PETSC_TRUE );
    #else
        PCMGSetGalerkin( pc_MG );
    #endif
    PCSetFromOptions(pc_MG);

    Stg_KSPSetOperators(ksp_inner, K, K, DIFFERENT_NONZERO_PATTERN);

    bsscrp_self->mg->mgData->ksp = ksp_inner;

    mgCtx->ksp = ksp_inner;
    mgCtx->pc = pc_MG;

    PETScMGSolver_UpdateOps(bsscrp_self->mg);

    /* If we are using dynamically adjusting smoother settings then
       this is implemented as a KSPMonitor (with side-effects)*/

    if( mgCtx->useAcceleratingSmoothingMG  )
	KSPMonitorSet( ksp_inner, KSPCycleEffectivenessMonitorAndAdjust, mgCtx, PETSC_NULL );

    PetscFunctionReturn(0);

}
PetscErrorCode MG_inner_solver_pcmg_shutdown( PC pc_MG ) {
    PetscErrorCode ierr;
    ierr = Stg_PCDestroy(&pc_MG );CHKERRQ(ierr);
    PetscFunctionReturn(0);
}
#undef __FUNCT__
#define __FUNCT__ "KSPCycleEffectivenessMonitorAndAdjust"
PetscErrorCode KSPCycleEffectivenessMonitorAndAdjust(KSP ksp, PetscInt n, PetscReal rnorm, void *_mgctx )
{
    PetscErrorCode          ierr;
    //PetscViewerASCIIMonitor viewer;
    PetscViewerASCIIMonitor viewer  = PETSC_VIEWER_STDOUT_(((PetscObject)ksp)->comm);
    MGContext * mgctx= (MGContext *)_mgctx;
    PetscFunctionBegin;

    if(n==0) {
	mgctx->smoothingCountThisSolve=0;
	mgctx->currentNumberOfSmooths = mgctx->smoothsToStartWith;
    }
    else if((n % mgctx->targetCyclesForTenfoldReduction) == 0 && mgctx->currentNumberOfSmooths < mgctx->smoothsMax) {
	mgctx->currentNumberOfSmooths += mgctx->smoothingIncrement;
    }

    ierr = PetscViewerASCIIMonitorCreate(((PetscObject)ksp)->comm,"stdout",0,&viewer);
    CHKERRQ(ierr);

    if(mgctx->acceleratingSmoothingMGView) {
	ierr = PetscViewerASCIIMonitorPrintf(viewer,"%3D MG smoothing cycles %d [%d] \n", n,
					     mgctx->currentNumberOfSmooths,mgctx->smoothingCountThisSolve);
	CHKERRQ(ierr);
    }

#if (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >= 9 )
  PCMGSetNumberSmooth(   mgctx->pc, mgctx->currentNumberOfSmooths );
#else
  PCMGSetNumberSmoothDown( mgctx->pc, mgctx->currentNumberOfSmooths );
  PCMGSetNumberSmoothUp(   mgctx->pc, mgctx->currentNumberOfSmooths );
#endif
    
    mgctx->totalMgCycleCount++;
    mgctx->smoothingCountThisSolve += mgctx->currentNumberOfSmooths;
    mgctx->totalSmoothingCount += mgctx->currentNumberOfSmooths;

    ierr = PetscViewerASCIIMonitorDestroy(viewer);
    CHKERRQ(ierr);

    PetscFunctionReturn(0);
}
double setupMG( KSP_BSSCR * bsscrp_self, KSP ksp_inner, PC pc_MG, Mat K, MGContext *mgCtx ){
    double mgSetupTime;

    mgSetupTime = MPI_Wtime();

    MG_inner_solver_mgContext_initialise( mgCtx );
    //MG_inner_solver_pcmg_create( ksp_inner, &pc_MG );
    KSPSetOptionsPrefix( ksp_inner, "A11_" );/* just in case.. for the moment */
    KSPSetFromOptions( ksp_inner );
    MG_inner_solver_pcmg_setup( bsscrp_self, mgCtx, ksp_inner, pc_MG, K );

    mgSetupTime = MPI_Wtime() - mgSetupTime;

    return mgSetupTime;
}
/********************************************************************************************************/
