/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "common-driver-utils.h"
#include "stokes_block_scaling.h"
#include "stokes_Kblock_scaling.h"
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "Solvers/SLE/SLE.h" /* to give the AugLagStokes_SLE type */
#include "Solvers/KSPSolvers/KSPSolvers.h" /* for __KSP_COMMON */

#include "BSSCR.h"

#undef __FUNCT__  
#define __FUNCT__ "KSPUnscale_BSSCR" 
PetscErrorCode KSPUnscale_BSSCR(KSP ksp)
{
    KSP_BSSCR  *bsscr = (KSP_BSSCR *)ksp->data;
    Mat            Amat,Pmat;
    Vec            B,X;
    Mat            ApproxS;
    MatStokesBlockScaling BA=bsscr->BA;
    PetscTruth     sym;
    MatStructure   pflag;
    PetscErrorCode ierr;

    PetscFunctionBegin;
    X             = ksp->vec_sol;
    B             = ksp->vec_rhs;
    ApproxS       = bsscr->preconditioner->matrix;
    sym           = bsscr->DIsSym;
    ierr=Stg_PCGetOperators(ksp->pc,&Amat,&Pmat,&pflag);CHKERRQ(ierr);
    if( BA->scaling_exists == PETSC_FALSE ){
	PetscPrintf( PETSC_COMM_WORLD, "SCALING does not EXIST in %s\n", __func__);
    }else{
	if( DEFAULT == bsscr->scaletype ){
	    PetscPrintf( PETSC_COMM_WORLD, "Operator scales (pre-Unscaling)\n");
	    BSSCR_MatStokesBlockReportOperatorScales(Amat, sym);
	    BSSCR_MatStokesBlockUnScaleSystem( BA, Amat, B, X, ApproxS, sym);
	    PetscPrintf( PETSC_COMM_WORLD, "Operator scales (post-Unscaling)\n");
	    BSSCR_MatStokesBlockReportOperatorScales(Amat, sym);
	}
	if( KONLY == bsscr->scaletype ){
	    PetscPrintf( PETSC_COMM_WORLD, "Operator scales KONLY (pre-Unscaling)\n");
	    BSSCR_MatStokesKBlockReportOperatorScales(Amat, sym);
	    BSSCR_MatStokesKBlockUnScaleSystem( BA, Amat, B, X, ApproxS, sym);
	    PetscPrintf( PETSC_COMM_WORLD, "Operator scales KONLY (post-Unscaling)\n");
	    BSSCR_MatStokesKBlockReportOperatorScales(Amat, sym);
	}
    }
    bsscr->scaled = PETSC_FALSE;
    PetscFunctionReturn(0);
}
/*
  KSPScale_BSSCR constructs scaling and applies scaling to system.
  Want to do both here when doing nonlinear iterations to keep the
  scaling up to date with each iteration.
 */
#undef __FUNCT__  
#define __FUNCT__ "KSPScale_BSSCR" 
PetscErrorCode KSPScale_BSSCR(KSP ksp)
{
    KSP_BSSCR  *bsscr = (KSP_BSSCR *)ksp->data;
    Mat            Amat,Pmat;
    Vec            B,X;
    Mat            ApproxS;
    MatStokesBlockScaling BA=bsscr->BA;
    PetscTruth     sym;
    MatStructure   pflag;
    PetscErrorCode ierr;

    PetscFunctionBegin;
    X             = ksp->vec_sol;
    B             = ksp->vec_rhs;
    ApproxS       = bsscr->preconditioner->matrix;
    sym           = bsscr->DIsSym;
    ierr=Stg_PCGetOperators(ksp->pc,&Amat,&Pmat,&pflag);CHKERRQ(ierr);

    if( DEFAULT == bsscr->scaletype ){
	if( BA->scaling_exists ){
	    BSSCR_MatStokesBlockDefaultBuildScaling( BA, Amat );/* rebuild scaling vectors on struct */
	    BA->scalings_have_been_inverted = PETSC_FALSE;
	}else{
	    BSSCR_MatBlock_ConstructScaling( BA, Amat, B, X );/* allocates scaling vectors then calls the above function */
	    BA->scalings_have_been_inverted = PETSC_FALSE;/* above function sets this but I am making it explicit here */
	}
	PetscPrintf( PETSC_COMM_WORLD, "Operator scales (pre-scaling)\n");
	BSSCR_MatStokesBlockReportOperatorScales(Amat, sym);
	BSSCR_MatStokesBlockScaleSystem( BA, Amat, B, X, ApproxS, sym );
	PetscPrintf( PETSC_COMM_WORLD, "Operator scales (post-scaling)\n");
	BSSCR_MatStokesBlockReportOperatorScales(Amat, sym);
	bsscr->scaled = PETSC_TRUE;
    }

    if( KONLY == bsscr->scaletype ){
	if( BA->scaling_exists ){
	    BSSCR_MatStokesKBlockDefaultBuildScaling( BA, Amat );/* rebuild scaling vectors on struct */
	    BA->scalings_have_been_inverted = PETSC_FALSE;
	}else{
	    BSSCR_MatKBlock_ConstructScaling( BA, Amat, B, X );/* allocates scaling vectors then calls the above function */
	    BA->scalings_have_been_inverted = PETSC_FALSE;/* above function sets this but I am making it explicit here */
	}
	PetscPrintf( PETSC_COMM_WORLD, "Operator scales KONLY (pre-scaling)\n");
	BSSCR_MatStokesKBlockReportOperatorScales(Amat, sym);
	BSSCR_MatStokesKBlockScaleSystem( BA, Amat, B, X, ApproxS, sym );
	PetscPrintf( PETSC_COMM_WORLD, "Operator scales KONLY (post-scaling)\n");
	BSSCR_MatStokesKBlockReportOperatorScales(Amat, sym);
	bsscr->scaled = PETSC_TRUE;
    }
    PetscFunctionReturn(0);
}

