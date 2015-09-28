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
#include <petscksp.h>
#include <petscpc.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "Solvers/KSPSolvers/KSPSolvers.h"

#include "common-driver-utils.h"
#include "BSSCR.h" /* includes StokesBlockKSPInterface.h */

#define KSPBSSCR         "bsscr"

void bsscr_summary(KSP_BSSCR * bsscrp_self, KSP ksp_S, KSP ksp_inner, 
		   Mat K,Mat K2,Mat D,Mat G,Mat C,Vec u,Vec p,Vec f,Vec h,Vec t,
		   double penaltyNumber,PetscTruth KisJustK,double mgSetupTime,double scrSolveTime,double a11SingleSolveTime){
    PetscTruth flg, found;
    PetscInt   uSize, pSize, lmax, lmin, iterations;
    PetscReal  rNorm, fNorm, uNorm, uNormInf, pNorm, pNormInf, p_sum, min, max;
    Vec q, qq, t2, t3;
    double solutionAnalysisTime;
      
      PetscPrintf( PETSC_COMM_WORLD,  "\n\nSCR Solver Summary:\n\n");
      if(bsscrp_self->mg)
	    PetscPrintf( PETSC_COMM_WORLD, "  Multigrid setup:        = %.4g secs \n", mgSetupTime);

      KSPGetIterationNumber( ksp_S, &iterations);
      PetscPrintf( PETSC_COMM_WORLD,     "  Pressure Solve:         = %.4g secs / %d its\n", scrSolveTime, iterations);
      KSPGetIterationNumber( ksp_inner, &iterations);
      PetscPrintf( PETSC_COMM_WORLD,     "  Final V Solve:          = %.4g secs / %d its\n\n", a11SingleSolveTime, iterations);
      /***************************************************************************************************************/
      flg = PETSC_TRUE; PetscOptionsGetTruth( PETSC_NULL, "-scr_ksp_solution_summary", &flg, &found );	
      if(flg) {	
	    PetscScalar KuNorm;
	    solutionAnalysisTime = MPI_Wtime();
	    VecGetSize( u, &uSize ); 
	    VecGetSize( p, &pSize );
	    VecDuplicate( u, &t2 );
	    VecDuplicate( u, &t3 );
	    MatMult( K, u, t3); VecNorm( t3, NORM_2, &KuNorm );
	    double angle, kdot;
	    if(penaltyNumber > 1e-10){/* should change this to ifK2built maybe */
		MatMult( K2, u, t2); VecNorm( t2, NORM_2, &rNorm );
		VecDot(t2,t3,&kdot);
		angle = (kdot/(rNorm*KuNorm));
		PetscPrintf( PETSC_COMM_WORLD,  "  <K u, K2 u>/(|K u| |K2 u|)    = %.6e\n", angle);
	    }
	    VecNorm( t, NORM_2, &rNorm ); /* t = f- G p  should be the formal residual vector, calculated on line 267 in auglag-driver-DGTGD.c */
	    VecDot(t3,t,&kdot);
	    angle = (kdot/(rNorm*KuNorm));
            PetscPrintf( PETSC_COMM_WORLD,  "  <K u, (f-G p)>/(|K u| |f- G p|)    = %.6e\n\n", angle);

	    MatMult( K, u, t2); VecNorm(t2, NORM_2, &KuNorm);
	    VecAYPX( t2, -1.0, t ); /* t2 <- -t2 + t  : t = f- G p  should be the formal residual vector, calculated on line 267 in auglag-driver-DGTGD.c*/
	    VecNorm( t2, NORM_2, &rNorm );	
	    VecNorm( f,  NORM_2, &fNorm );	
	    if(KisJustK){
		PetscPrintf( PETSC_COMM_WORLD,"Velocity back-solve with original K matrix\n");
		PetscPrintf( PETSC_COMM_WORLD,"Solved    K u = G p -f\n");
		PetscPrintf( PETSC_COMM_WORLD,"Residual with original K matrix\n");
		PetscPrintf( PETSC_COMM_WORLD,  "  |f - K u - G p|                       = %.12e\n", rNorm);
		PetscPrintf( PETSC_COMM_WORLD,  "  |f - K u - G p|/|f|                   = %.12e\n", rNorm/fNorm);
		if(penaltyNumber > 1e-10){/* means the restore_K flag was used */
		    //if(K2 && f2){
			MatAXPY(K,penaltyNumber,K2,DIFFERENT_NONZERO_PATTERN);/* Computes K = penaltyNumber*K2 + K */
			//VecAXPY(f,penaltyNumber,f2); /* f = penaltyNumber*f2 + f */
			KisJustK=PETSC_FALSE;
			MatMult( K, u, t2);
			MatMult( G, p, t);
			VecAYPX( t, -1.0, f ); /* t <- -t + f */
			VecAYPX( t2, -1.0, t ); /* t2 <- -t2 + t */
			VecNorm( t2, NORM_2, &rNorm );
			PetscPrintf( PETSC_COMM_WORLD,"Residual with K+K2 matrix and f rhs vector\n");
			PetscPrintf( PETSC_COMM_WORLD,  "  |(f) - (K + K2) u - G p|         = %.12e\n", rNorm);
			//}
		}
		
	    }
	    else{
		PetscPrintf( PETSC_COMM_WORLD,"Velocity back-solve with K+K2 matrix\n");
		PetscPrintf( PETSC_COMM_WORLD,"Solved    (K + K2) u = G p - (f)\n");
		PetscPrintf( PETSC_COMM_WORLD,"Residual with K+K2 matrix and f rhs vector\n");
		PetscPrintf( PETSC_COMM_WORLD,  "  |(f) - (K + K2) u - G p|         = %.12e\n", rNorm);
		PetscReal KK2Norm,KK2Normf;
		MatNorm(K,NORM_1,&KK2Norm);
		MatNorm(K,NORM_FROBENIUS,&KK2Normf);
		penaltyNumber = -penaltyNumber;
		MatAXPY(K,penaltyNumber,K2,DIFFERENT_NONZERO_PATTERN);/* Computes K = penaltyNumber*K2 + K */
		//VecAXPY(f,penaltyNumber,f2); /* f = penaltyNumber*f2 + f */
		KisJustK=PETSC_FALSE;
		MatMult( K, u, t2);    /* t2 = K*u  */
		MatMult( G, p, t);     /* t  = G*p  */
		VecAYPX( t, -1.0, f ); /* t <- f - t ; t = f - G*p  */
		VecAYPX( t2, -1.0, t ); /* t2 <- t - t2; t2 = f - G*p - K*u  */
		VecNorm( t2, NORM_2, &rNorm );
		PetscPrintf( PETSC_COMM_WORLD,"Residual with original K matrix\n");
		PetscPrintf( PETSC_COMM_WORLD,  "  |f - K u - G p|                       = %.12e\n", rNorm);
		PetscPrintf( PETSC_COMM_WORLD,  "  |f - K u - G p|/|f|                   = %.12e\n", rNorm/fNorm);
		PetscReal KNorm, K2Norm;
		MatNorm(K,NORM_1,&KNorm);	  MatNorm(K2,NORM_1,&K2Norm);
		PetscPrintf( PETSC_COMM_WORLD,"K and K2 norm_1    %.12e %.12e   ratio %.12e\n",KNorm,K2Norm,K2Norm/KNorm);
		MatNorm(K,NORM_INFINITY,&KNorm);  MatNorm(K2,NORM_INFINITY,&K2Norm);
		PetscPrintf( PETSC_COMM_WORLD,"K and K2 norm_inf  %.12e %.12e   ratio %.12e\n",KNorm,K2Norm,K2Norm/KNorm);
		MatNorm(K,NORM_FROBENIUS,&KNorm); MatNorm(K2,NORM_FROBENIUS,&K2Norm);
		PetscPrintf( PETSC_COMM_WORLD,"K and K2 norm_frob %.12e %.12e   ratio %.12e\n",KNorm,K2Norm,K2Norm/KNorm);
		PetscPrintf( PETSC_COMM_WORLD,"K+r*K2 norm_1    %.12e\n",KK2Norm);
		PetscPrintf( PETSC_COMM_WORLD,"K+r*K2 norm_frob %.12e\n",KK2Normf);
		penaltyNumber = -penaltyNumber;
		MatAXPY(K,penaltyNumber,K2,DIFFERENT_NONZERO_PATTERN);/* Computes K = penaltyNumber*K2 + K */
	    }
	    PetscPrintf( PETSC_COMM_WORLD,"\n");
	    PetscPrintf( PETSC_COMM_WORLD,  "  |K u|    = %.12e\n", KuNorm);
	    if(penaltyNumber > 1e-10){
		MatMult( K2, u, t2); VecNorm( t2, NORM_2, &rNorm );
		PetscPrintf( PETSC_COMM_WORLD,  "  |K2 u|   = %.12e\n", rNorm);
		PetscPrintf( PETSC_COMM_WORLD,"\n");
	    }


	    
	    VecDuplicate( p, &q );
	    MatMult( D, u, q );   /* q = G'*u = D*u */
	    VecNorm( u, NORM_2, &uNorm );	
	    VecNorm( q, NORM_2, &rNorm );	
	
	    PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|_2               = %.6e\n", rNorm );
	    PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|_2/|u|_2         = %.6e\n", sqrt( (double) uSize / (double) pSize ) * rNorm / uNorm);

	    VecDuplicate( p, &qq );
	    MatMultTranspose( G, u, qq );
	    VecNorm( qq, NORM_2, &rNorm );
	    PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|/|u|             = %.8e\n", rNorm/uNorm ); /* to compare directly with Uzawa */
	    
	    VecNorm( q, NORM_INFINITY, &rNorm );
	    PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|_infty/|u|_2     = %.6e\n", sqrt( (double) uSize ) * rNorm / uNorm);
	    /* create G'*u+C*p-h to check on this constraint */
	    /* already have q = D*u */
	    VecZeroEntries(qq);
	    if(C){
		MatMult( C, p, qq );
	    }
	    VecAYPX( q, 1.0, qq ); /* q = q+qq; G'*u + C*p*/
	    VecAXPY( q, -1.0, h ); /* q = q-h;  G'*u + C*p - h  */
	    VecNorm( q, NORM_2, &rNorm );
	    PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u + C p - h|        = %.8e  :constraint\n", rNorm );
	    
	    VecNorm( u, NORM_INFINITY, &uNormInf );
	    VecNorm( u, NORM_2,        &uNorm );
	    VecGetSize( u, &uSize );
	
	    VecNorm( p, NORM_INFINITY, &pNormInf );
	    VecNorm( p, NORM_2,        &pNorm );
	
	    PetscPrintf( PETSC_COMM_WORLD,  "  |u|_{\\infty}  = %.6e , u_rms = %.6e\n", 
	                 uNormInf, uNorm / sqrt( (double) uSize ) );
		
	    PetscPrintf( PETSC_COMM_WORLD,  "  |p|_{\\infty}  = %.6e , p_rms = %.6e\n",
	                 pNormInf, pNorm / sqrt( (double) pSize ) );

	    VecMax( u, &lmax, &max );
	    VecMin( u, &lmin, &min );
	    PetscPrintf( PETSC_COMM_WORLD,  "  min/max(u)    = %.6e [%d] / %.6e [%d]\n",min,lmin,max,lmax);

	    VecMax( p, &lmax, &max );
	    VecMin( p, &lmin, &min );
	    PetscPrintf( PETSC_COMM_WORLD,  "  min/max(p)    = %.6e [%d] / %.6e [%d]\n",min,lmin,max,lmax);
       
	    VecSum( p, &p_sum );
	    PetscPrintf( PETSC_COMM_WORLD,  "  \\sum_i p_i    = %.6e \n", p_sum );
	
	    solutionAnalysisTime = MPI_Wtime() - solutionAnalysisTime;
	
	    PetscPrintf( PETSC_COMM_WORLD,  "\n  Time for this analysis  = %.4g secs\n\n",solutionAnalysisTime);
	
	    Stg_VecDestroy(&t2 );
	    Stg_VecDestroy(&t3 );
	    Stg_VecDestroy(&q );
	    Stg_VecDestroy(&qq );	    
      }

}
