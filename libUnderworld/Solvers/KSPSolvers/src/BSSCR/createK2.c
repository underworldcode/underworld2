/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "createK2.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "Solvers/SLE/SLE.h" /* to give the AugLagStokes_SLE type */
#include "Solvers/KSPSolvers/KSPSolvers.h" /* for __KSP_COMMON */
#include "BSSCR.h"
#include "writeMatVec.h"
#undef __FUNCT__  
#define __FUNCT__ "bsscr_buildK2"
PetscErrorCode bsscr_buildK2(KSP ksp){
    KSP_BSSCR  *bsscr = (KSP_BSSCR *)ksp->data;
    Mat K,G,M, K2=0;
    Vec f2=0;
    //MatStokesBlockScaling BA=bsscr->BA;
    Stokes_SLE*  stokesSLE  = (Stokes_SLE*)bsscr->solver->st_sle;
    StokesBlockKSPInterface* Solver = bsscr->solver;
    //PetscErrorCode ierr;

    PetscFunctionBegin;
    K = stokesSLE->kStiffMat->matrix;
    G = stokesSLE->gStiffMat->matrix;
    if(bsscr->K2){
	Stg_MatDestroy(&bsscr->K2);
	bsscr->K2 =  PETSC_NULL;
    }
    switch (bsscr->k2type) {
    case (K2_DGMGD):
    {/* Should only do this one if scaling is turned off. */
	  Vec D;
	  //AugLagStokes_SLE * stokesSLE = (AugLagStokes_SLE*)bsscr->st_sle;

	  if (Solver->mStiffMat){      
	      MatGetVecs( K, PETSC_NULL, &D );
	      //MatGetRowMax( K, D, PETSC_NULL );
          MatGetDiagonal( K, D );
	      VecSqrt( D );  
	      M = Solver->mStiffMat->matrix;
	      bsscr_GMiGt(&K2,K,G,M);      /* K2 created */
	      MatDiagonalScale(K2, D, D ); /* K2 = D*G*inv(M)*Gt*D */
	      Stg_VecDestroy(&D);
	      PetscPrintf( PETSC_COMM_WORLD,  "\n\n-----  K2_DGMGD  ------\n\n");  }
	  else{
	      PetscPrintf( PETSC_COMM_WORLD,"The Augmented Lagrangian Method DGMGD was specified but the SLE has no mStiffMat on it.\n");
	      PetscPrintf( PETSC_COMM_WORLD,"You need to use the AugLagStokes_SLE class.\n");  }
    }
    break;
    case (K2_GMG): 
    {
	  //AugLagStokes_SLE * stokesSLE = (AugLagStokes_SLE*)bsscr->st_sle;
	  if (Solver->mStiffMat){      
	      M = Solver->mStiffMat->matrix;
	      bsscr_GMiGt(&K2,K,G,M);      /* K2 created */
	      PetscPrintf( PETSC_COMM_WORLD,  "\n\n-----  K2_GMG  ------\n\n");
	  }else{
	      PetscPrintf( PETSC_COMM_WORLD,"The Augmented Lagrangian Method GMG was specified but the SLE has no mStiffMat on it.\n");
	      PetscPrintf( PETSC_COMM_WORLD,"You need to use the AugLagStokes_SLE class.\n");
	  } 
    }
    break;
    case (K2_GG): 
    {
	  bsscr_GGt(&K2,K,G);      /* K2 created */
	  PetscPrintf( PETSC_COMM_WORLD,  "\n\n-----  K2_GG  ------\n\n"); 
    }
    break;
    case (K2_SLE): 
    {
	  //AugLagStokes_SLE * stokesSLE = (AugLagStokes_SLE*)bsscr->st_sle;
	  if (Solver->mStiffMat){      
	      K2 = Solver->mStiffMat->matrix;
	      if(Solver->jForceVec){ f2 = Solver->jForceVec->vector; }
	      PetscPrintf( PETSC_COMM_WORLD,  "\n\n-----  K2_SLE  ------\n\n");
	  }else{
	      PetscPrintf( PETSC_COMM_WORLD,"The Augmented Lagrangian Method SLE was specified but the Solver has no Matrix on it.\n");
	      PetscPrintf( PETSC_COMM_WORLD,"You need to set the K2 matrix on the StokesBlockKSPInterface class.\n");
	  } 
    }
    break;
    default:
	  PetscPrintf( PETSC_COMM_WORLD,  "\n\n-----  NO K2  ------\n\n");
    }
    bsscr->f2 = f2;
    bsscr->K2 = K2;
    PetscFunctionReturn(0);
}

/* computes K2 = D*G*inv(M)*Gt*D */
/* where D = diag(K)
         Gt = transpose(G)
	 M is Pressure Mass Matrix */
#undef __FUNCT__  
#define __FUNCT__ "bsscr_DGMiGtD" 
PetscErrorCode bsscr_DGMiGtD( Mat *_K2, Mat K, Mat G, Mat M){
    Mat K2;
    Vec diag;
    Vec Mdiag;
    Mat MinvGt;
    Mat Gtrans;
    PetscErrorCode ierr;

    PetscFunctionBegin;
    MatGetVecs( K, &diag, PETSC_NULL );
    MatGetDiagonal( K, diag );
    VecSqrt(diag);
    //VecReciprocal(diag);/* trying something different here */                                                                                                                     
    MatGetVecs( M, &Mdiag, PETSC_NULL );
    MatGetDiagonal( M, Mdiag );
    VecReciprocal(Mdiag);
    #if( PETSC_VERSION_MAJOR <= 2 )
    ierr=MatTranspose(G, &Gtrans);CHKERRQ(ierr);
    #else
    ierr=MatTranspose(G, MAT_INITIAL_MATRIX,&Gtrans);CHKERRQ(ierr);
    #endif
    ierr=MatConvert(Gtrans, MATSAME, MAT_INITIAL_MATRIX, &MinvGt);CHKERRQ(ierr);/* copy Gtrans -> MinvGt */
    MatDiagonalScale(MinvGt, Mdiag, PETSC_NULL);/* Minv*Gtrans */
    /* MAT_INITIAL_MATRIX -> creates K2 matrix : PETSC_DEFAULT for fill ratio: run with -info to find what it should be*/
    ierr=MatMatMult( G, MinvGt, MAT_INITIAL_MATRIX, PETSC_DEFAULT, &K2);CHKERRQ(ierr);/* K2 = G*Minv*Gtrans */
    MatDiagonalScale(K2, diag, diag );/* K2 = D*K2*D  = D*G*Minv*Gtrans*D */
    Stg_MatDestroy(&Gtrans);
    Stg_MatDestroy(&MinvGt);
    Stg_VecDestroy(&diag);
    Stg_VecDestroy(&Mdiag);
    *_K2=K2;
    PetscFunctionReturn(0);
}
/* computes K2 = G*inv(M)*Gt */
/* where Gt = transpose(G)
	 M is Pressure Mass Matrix */
#undef __FUNCT__  
#define __FUNCT__ "bsscr_GMiGt" 
PetscErrorCode bsscr_GMiGt( Mat *_K2, Mat K, Mat G, Mat M){
    Mat K2;
    Vec Mdiag;
    Mat MinvGt;
    Mat Gtrans;
    PetscErrorCode ierr;

    PetscFunctionBegin;
    MatGetVecs( M, &Mdiag, PETSC_NULL );
    MatGetDiagonal( M, Mdiag );
    VecReciprocal(Mdiag);
    #if( PETSC_VERSION_MAJOR <= 2 )
    ierr=MatTranspose(G, &Gtrans);CHKERRQ(ierr);
    #else
    ierr=MatTranspose(G, MAT_INITIAL_MATRIX,&Gtrans);CHKERRQ(ierr);
    #endif
    ierr=MatConvert(Gtrans, MATSAME, MAT_INITIAL_MATRIX, &MinvGt);CHKERRQ(ierr);/* copy Gtrans -> MinvGt */
    MatDiagonalScale(MinvGt, Mdiag, PETSC_NULL);/* Minv*Gtrans */
    /* MAT_INITIAL_MATRIX -> creates K2 matrix : PETSC_DEFAULT for fill ratio: run with -info to find what it should be*/
    ierr=MatMatMult( G, MinvGt, MAT_INITIAL_MATRIX, PETSC_DEFAULT, &K2);CHKERRQ(ierr);/* K2 = G*Minv*Gtrans */
    Stg_MatDestroy(&Gtrans);
    Stg_MatDestroy(&MinvGt);
    Stg_VecDestroy(&Mdiag);
    *_K2=K2;
    PetscFunctionReturn(0);
}
/* computes K2 = G*Gt */
#undef __FUNCT__  
#define __FUNCT__ "bsscr_GGt" 
PetscErrorCode bsscr_GGt( Mat *_K2, Mat K, Mat G){
    Mat K2;
    Mat Gtrans;
    PetscErrorCode ierr;

    PetscFunctionBegin;
    #if( PETSC_VERSION_MAJOR <= 2 )
    ierr=MatTranspose(G, &Gtrans);CHKERRQ(ierr);
    #else
    ierr=MatTranspose(G, MAT_INITIAL_MATRIX,&Gtrans);CHKERRQ(ierr);
    #endif
    /* MAT_INITIAL_MATRIX -> creates K2 matrix : PETSC_DEFAULT for fill ratio: run with -info to find what it should be*/
    ierr=MatMatMult( G, Gtrans, MAT_INITIAL_MATRIX, PETSC_DEFAULT, &K2);CHKERRQ(ierr);
    Stg_MatDestroy(&Gtrans);
    *_K2=K2;
    PetscFunctionReturn(0);
}

