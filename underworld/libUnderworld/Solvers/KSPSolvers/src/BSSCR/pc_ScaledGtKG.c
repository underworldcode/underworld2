/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#if 0
/*

Performs y = ( G^T G )^{-1} ( G^T K G ) ( G^T G )^{-1} x


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>

#include "common-driver-utils.h"

#include <petscversion.h>
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include "petsc/private/pcimpl.h"
     #include "petsc/private/kspimpl.h"
  #else
     #include "petsc-private/pcimpl.h"
     #include "petsc-private/kspimpl.h"
  #endif
#else
  #include "private/pcimpl.h"
  #include "private/kspimpl.h"
#endif


#include "pc_ScaledGtKG.h"

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#define PCTYPE_SCGtKG "scgtkg"


/* private data */

typedef struct {
	Mat         B, Bt, F, C;   /* Bt \in [M x N], F \in [M x M] */
	Vec         X1,X2, Y1,Y2;  /* the scaling vectors */
	Vec         s,t,X;         /* s \in [M], t \in [N], X \in [M] */
	KSP         ksp_BBt;       /* The user MUST provide this operator */
	PetscTruth  BBt_has_cnst_nullspace;
	PetscTruth  monitor_activated;
} _PC_SC_GtKG;

typedef _PC_SC_GtKG* PC_SC_GtKG;


/* private prototypes */
PetscErrorCode BSSCR_PCApply_ScGtKG( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_PCApplyTranspose_ScGtKG( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_PCSetUp_GtKG( PC pc );


PetscErrorCode BSSCR_BSSCR_pc_error_ScGtKG( PC pc, const char func_name[] ) 
{
	const PCType type;
	PCGetType( pc, &type );
	
	if( strcmp(type,PCTYPE_SCGtKG)!=0 ) {
		printf("Error(%s): PC type (%s) should be scgtkg. \n",func_name, type );
		PetscFinalize();
		exit(0);
	}
	PetscFunctionReturn(0);
}


void BSSCR_BSSCR_get_number_nonzeros_AIJ_ScGtKG( Mat A, PetscInt *nnz )
{
	MatInfo info;
	
	MatGetInfo( A, MAT_GLOBAL_SUM, &info );
	*nnz = info.nz_used;
	
}

PetscErrorCode BSSCR_PCScGtKGBBtContainsConstantNullSpace( PC pc, PetscTruth *has_cnst_nullsp )
{
	PC_SC_GtKG    ctx = (PC_SC_GtKG)pc->data;
	PetscInt N;
	PetscScalar sum;
	PetscReal nrm;
	Vec l,r;
	Mat BBt,A;
	
	
	Stg_KSPGetOperators( ctx->ksp_BBt, &BBt, PETSC_NULL, PETSC_NULL );
	A = BBt;
	
	MatGetVecs( A, &r, &l ); // l = A r
	
	
	VecGetSize(r,&N);
	sum  = 1.0/N;
	VecSet(r,sum);
	
	/* scale */
	VecPointwiseMult( r, r, ctx->Y2 );
	/* {l} = [A] {r} */
	MatMult( A,r, l );
	/* scale */
	VecPointwiseMult( l, l, ctx->X2 );
	
	VecNorm(l,NORM_2,&nrm);
	if (nrm < 1.e-7) {
		*has_cnst_nullsp = PETSC_TRUE;
	}
	else {
		*has_cnst_nullsp = PETSC_FALSE;
	}
	
	Stg_VecDestroy(&l);
	Stg_VecDestroy(&r);
	
	PetscFunctionReturn(0);
}



/*
I should not modify setup called!!
This is handled via petsc.
*/
PetscErrorCode BSSCR_PCSetUp_ScGtKG( PC pc )
{
	PC_SC_GtKG    ctx = (PC_SC_GtKG)pc->data;
	
	if( ctx->F == PETSC_NULL ) {   Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: F not set" );   }
	if( ctx->Bt == PETSC_NULL ) {  Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: Bt not set" );  }
	
	if(ctx->ksp_BBt==PETSC_NULL) {
		BSSCR_PCScGtKGUseStandardBBtOperator( pc ) ;
	}
	
	BSSCR_PCScGtKGBBtContainsConstantNullSpace( pc, &ctx->BBt_has_cnst_nullspace );
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		PetscPrintf( PETSC_COMM_WORLD, "\t* Detected prescence of constant nullspace in BBt-C\n" );
	}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCDestroy_ScGtKG( PC pc )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	
	if( ctx == PETSC_NULL ) {	PetscFunctionReturn(0); }
	
	if( ctx->ksp_BBt != PETSC_NULL ) {	Stg_KSPDestroy(&ctx->ksp_BBt );		}
	
	if( ctx->s != PETSC_NULL ) {		Stg_VecDestroy(&ctx->s );		}
	if( ctx->X != PETSC_NULL ) {		Stg_VecDestroy(&ctx->X );		}
	if( ctx->t != PETSC_NULL ) {		Stg_VecDestroy(&ctx->t );		}
	
	if( ctx->X1 != PETSC_NULL ) {		Stg_VecDestroy(&ctx->X1 );		}
	if( ctx->X2 != PETSC_NULL ) {		Stg_VecDestroy(&ctx->X2 );		}
	if( ctx->Y1 != PETSC_NULL ) {		Stg_VecDestroy(&ctx->Y1 );		}
	if( ctx->Y2 != PETSC_NULL ) {		Stg_VecDestroy(&ctx->Y2 );		}
	
	PetscFree( ctx );
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCView_ScGtKG( PC pc, PetscViewer viewer )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	PetscViewerASCIIPushTab(viewer); //1
	
	
	PetscViewerASCIIPrintf( viewer, "gtkg-ksp \n" );
	PetscViewerASCIIPrintf(viewer,"---------------------------------\n");
	PetscViewerASCIIPushTab(viewer);
		KSPView( ctx->ksp_BBt, viewer );
	PetscViewerASCIIPopTab(viewer);
	PetscViewerASCIIPrintf(viewer,"---------------------------------\n");
	
	PetscViewerASCIIPopTab(viewer); //1
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCRBSSCR_Lp_monitor_ScGtKG( KSP ksp, PetscInt index )
{
	PetscInt max_it;
	PetscReal rnorm;
	KSPConvergedReason reason;
	
	
	KSPGetIterationNumber( ksp, &max_it );
	KSPGetResidualNorm( ksp, &rnorm );
	KSPGetConvergedReason( ksp, &reason );
	if (reason >= 0) {
		PetscPrintf(((PetscObject)ksp)->comm,"\t<Lp(%d)>: Linear solve converged. its.=%.4d ; |r|=%5.5e ; Reason=%s\n", 
				index, max_it, rnorm, KSPConvergedReasons[reason] );
	} else {
		PetscPrintf(((PetscObject)ksp)->comm,"\t<Lp(%d)>: Linear solve did not converge. its.=%.4d ; |r|=%5.5e ; Residual reduction=%2.2e ; Reason=%s\n", 
				index, max_it, rnorm, (ksp->rnorm0/rnorm), KSPConvergedReasons[reason]);
	}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCScBFBTSubKSPMonitor( KSP ksp, PetscInt index, PetscLogDouble time )
{
	PetscInt max_it;
	PetscReal rnorm;
	KSPConvergedReason reason;
	
	KSPGetIterationNumber( ksp, &max_it );
	KSPGetResidualNorm( ksp, &rnorm );
	KSPGetConvergedReason( ksp, &reason );
	PetscPrintf(((PetscObject)ksp)->comm,"    PCScBFBTSubKSP (%d): %D Residual norm; r0 %12.12e, r %12.12e: Reason %s: Time %5.5e \n", 
			index, max_it, ksp->rnorm0, rnorm, KSPConvergedReasons[reason], time );
	
	PetscFunctionReturn(0);
}


/* 
Performs y <- S*^{-1} x 
  S*^{-1} = ( B' Bt' )^{-1} B' F' Bt' ( B' Bt' )^{-1}
where 
  F'  = X1 F Y1
  Bt' = X1 Bt Y2
  B'  = X2 B Y1

Thus, S*^{-1} = [ X2 B Y1 X1 Bt Y2 ]^{-1} . [ X2 B Y1 . X1 F Y1 . X1 Bt Y2 ] . [ X2 B Y1 X1 Bt Y2 ]^{-1}
              = Y2^{-1} ksp_BBt X2^{-1} . [ B' F' Bt' ] . Y2^{-1} ksp_BBt X2^{-1}
              = Y2^{-1} ksp_BBt . [ B Y1 . X1 F Y1 . X1 Bt ] . ksp_BBt X2^{-1}
*/
PetscErrorCode BSSCR_PCApply_ScGtKG( PC pc, Vec x, Vec y )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	KSP ksp;
	Mat F, Bt;
	Vec s,t,X;
	PetscLogDouble t0,t1;
	
	ksp = ctx->ksp_BBt;
	F = ctx->F;
	Bt = ctx->Bt;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	
	
	/* Apply scaled Poisson operator */
	/* scale x */ 
	/* ======================================================== 
	     NOTE:
	       I THINK TO OMIT THESE AS WE WANT TO UNSCALE THE 
	        PRECONDITIONER AS S IN THIS CASE IS NOT SCALED 
	======================================================== */
//	VecPointwiseDivide( x, x, ctx->X2 ); /* x <- x/X2 */  /* NEED TO BE SURE */
	
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
	  BSSCR_VecRemoveConstNullspace( x, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolve( ksp, x, t ); /* t <- GtG_inv x */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,1,(t1-t0));		
	}
	
	/* Apply Bt */
	MatMult( Bt, t, s ); /* s <- G t */
	VecPointwiseMult( s, s, ctx->X1 ); /* s <- s * X1 */
	
	
	/* Apply F */
	VecPointwiseMult( s, s, ctx->Y1 ); /* s <- s * Y1 */
	MatMult( F, s, X ); /* X <- K s */
	VecPointwiseMult( X, X, ctx->X1 ); /* X <- X * X1 */
	
	/* Apply B */
	VecPointwiseMult( X, X, ctx->Y1 ); /* X <- X * Y1 */
	MatMultTranspose( Bt, X, t ); /* t <- Gt X */
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		BSSCR_VecRemoveConstNullspace( t, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolve( ksp, t, y ); /* y <- GtG_inv t */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,2,(t1-t0));
	}
	
	VecPointwiseMult( y, y, ctx->Y2 ); /* y <- y/Y2 */ 
	
	
	/* undo modification made to x on entry */
//	VecPointwiseMult( x, x, ctx->X2 ); /* x <- x/X2 */ /* NEED TO BE SURE */
	
	
	PetscFunctionReturn(0);
}

/* 
Performs y <- S*^{-1} x 
  S*^{-1} = ( B' Bt' )^{-1} B' F' Bt' - C' ( B' Bt' )^{-1}
where 
  F'  = X1 F Y1
  Bt' = X1 Bt Y2
  B'  = X2 B Y1
  C'  = X2 C Y2

Thus, S*^{-1} = [ X2 B Y1 X1 Bt Y2 ]^{-1} . [ X2 B Y1 . X1 F Y1 . X1 Bt Y2 - X2 C Y2 ] . [ X2 B Y1 X1 Bt Y2 ]^{-1}
              = Y2^{-1} ksp_BBt X2^{-1} . [ B' F' Bt' - C' ] . Y2^{-1} ksp_BBt X2^{-1}
              = Y2^{-1} ksp_BBt . [ B Y1 . X1 F Y1 . X1 Bt - C ] . ksp_BBt X2^{-1}
*/
PetscErrorCode BSSCR_BSSCR_PCApply_ScGtKG_C( PC pc, Vec x, Vec y )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	KSP ksp;
	Mat F, Bt,C;
	Vec s,t,X;
	PetscLogDouble t0,t1;
	
	
	ksp = ctx->ksp_BBt;
	F = ctx->F;
	Bt = ctx->Bt;
	C = ctx->C;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	
	
	/* Apply scaled Poisson operator */
	/* scale x */ 
	/* ======================================================== 
	     NOTE:
	       I THINK TO OMIT THESE AS WE WANT TO UNSCALE THE 
	        PRECONDITIONER AS S IN THIS CASE IS NOT SCALED 
	======================================================== */
//	VecPointwiseDivide( x, x, ctx->X2 ); /* x <- x/X2 */  /* NEED TO BE SURE */
	
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		BSSCR_VecRemoveConstNullspace( x, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolve( ksp, x, t ); /* t <- GtG_inv x */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,1,(t1-t0));
	}
	
	/* Apply Bt */
	MatMult( Bt, t, s ); /* s <- G t */
	VecPointwiseMult( s, s, ctx->X1 ); /* s <- s * X1 */
	
	
	/* Apply F */
	VecPointwiseMult( s, s, ctx->Y1 ); /* s <- s * Y1 */
	MatMult( F, s, X ); /* X <- K s */
	VecPointwiseMult( X, X, ctx->X1 ); /* X <- X * X1 */
	
	/* Apply B */
	VecPointwiseMult( X, X, ctx->Y1 ); /* X <- X * Y1 */
	MatMultTranspose( Bt, X, s ); /* s <- Gt X */
	
	/* s <- s - C t */
	VecScale( s, -1.0 );
	MatMultAdd( C, t, s, s );
	VecScale( s, -1.0 );
	
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		BSSCR_VecRemoveConstNullspace( s, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolve( ksp, s, y ); /* y <- GtG_inv s */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,2,(t1-t0));
	}
	
	VecPointwiseMult( y, y, ctx->Y2 ); /* y <- y/Y2 */ 
	
	
	/* undo modification made to x on entry */
//	VecPointwiseMult( x, x, ctx->X2 ); /* x <- x/X2 */ /* NEED TO BE SURE */
	
	
	PetscFunctionReturn(0);
}



/* Need to check this one if correct */
/*
S^{-1} = ( G^T G )^{-1} G^T K G ( G^T G )^{-1}
       = A C A
S^{-T} = A^T (A C)^T
       = A^T C^T A^T, but A = G^T G which is symmetric
       = A C^T A
       = A G^T ( G^T K )^T A
       = A G^T K^T G A

*/
PetscErrorCode BSSCR_PCApplyTranspose_ScGtKG( PC pc, Vec x, Vec y )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	KSP ksp;
	Mat F, Bt;
	Vec s,t,X;
	PetscLogDouble t0,t1;
	
	
	ksp = ctx->ksp_BBt;
	F = ctx->F;
	Bt = ctx->Bt;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	
	
	/* Apply scaled Poisson operator */
	/* scale x */ 
	/* ======================================================== 
	     NOTE:
	       I THINK TO OMIT THESE AS WE WANT TO UNSCALE THE 
	        PRECONDITIONER AS S IN THIS CASE IS NOT SCALED 
	======================================================== */
//	VecPointwiseDivide( x, x, ctx->X2 ); /* x <- x/X2 */  /* NEED TO BE SURE */
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		BSSCR_VecRemoveConstNullspace( x, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolveTranspose( ksp, x, t ); /* t <- GtG_inv x */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,1,(t1-t0));
	}
	
	/* Apply Bt */
	MatMult( Bt, t, s ); /* s <- G t */
	VecPointwiseMult( s, s, ctx->X1 ); /* s <- s * X1 */
	
	
	/* Apply F */
	VecPointwiseMult( s, s, ctx->Y1 ); /* s <- s * Y1 */
	MatMultTranspose( F, s, X ); /* X <- K s */
	VecPointwiseMult( X, X, ctx->X1 ); /* X <- X * X1 */
	
	/* Apply B */
	VecPointwiseMult( X, X, ctx->Y1 ); /* X <- X * Y1 */
	MatMultTranspose( Bt, X, t ); /* t <- Gt X */
	
	if( ctx->BBt_has_cnst_nullspace == PETSC_TRUE ) {
		BSSCR_VecRemoveConstNullspace( t, PETSC_NULL );
	}
	PetscGetTime(&t0);
	KSPSolveTranspose( ksp, t, y ); /* y <- GtG_inv t */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {
		BSSCR_PCScBFBTSubKSPMonitor(ksp,2,(t1-t0));
	}
	
	VecPointwiseMult( y, y, ctx->Y2 ); /* y <- y/Y2 */ 
	
	
	/* undo modification made to x on entry */
//	VecPointwiseMult( x, x, ctx->X2 ); /* x <- x/X2 */ /* NEED TO BE SURE */
	
	
	PetscFunctionReturn(0);
}


/* 
Performs y <- S^{-1} x 
S^{-1} = ( G^T Di G )^{-1} G^T Di K Di G ( G^T Di G )^{-1}
where Di = diag(M)^{-1}
*/



/*
Only the options related to GtKG should be set here.
*/
PetscErrorCode BSSCR_PCSetFromOptions_ScGtKG( PC pc )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	PetscTruth ivalue, flg;
	
	if(ctx->ksp_BBt!=PETSC_NULL) {
		PetscOptionsGetTruth( PETSC_NULL, "-pc_gtkg_monitor", &ivalue, &flg );
		BSSCR_PCScGtKGSetSubKSPMonitor( pc, ivalue );
		
	}
	
	
	PetscFunctionReturn(0);
}



/* ---- Exposed functions ---- */

PetscErrorCode BSSCR_PCCreate_ScGtKG( PC pc )
{
	PC_SC_GtKG      pc_data;
	PetscErrorCode  ierr;
	
	/* create memory for ctx */
	ierr = Stg_PetscNew( _PC_SC_GtKG,&pc_data);CHKERRQ(ierr);
	
	/* init ctx */
	pc_data->F   = PETSC_NULL;
	pc_data->Bt  = PETSC_NULL;
	pc_data->B   = PETSC_NULL;
	pc_data->BBt_has_cnst_nullspace = PETSC_FALSE;
	pc_data->ksp_BBt   = PETSC_NULL;
	pc_data->monitor_activated = PETSC_FALSE;
	
	pc_data->X1 = PETSC_NULL;
	pc_data->X2 = PETSC_NULL;
	pc_data->Y1 = PETSC_NULL;
	pc_data->Y2 = PETSC_NULL;
	
	pc_data->s = PETSC_NULL;
	pc_data->t = PETSC_NULL;
	pc_data->X = PETSC_NULL;
	
	
	
	/* set ctx onto pc */
	pc->data  = (void*)pc_data;
	
	ierr = PetscLogObjectMemory(pc,sizeof(_PC_SC_GtKG));CHKERRQ(ierr);
	
	/* define operations */
	pc->ops->setup   = BSSCR_PCSetUp_ScGtKG;
	pc->ops->view    = BSSCR_PCView_ScGtKG;
	pc->ops->destroy = BSSCR_PCDestroy_ScGtKG;
	pc->ops->setfromoptions = BSSCR_PCSetFromOptions_ScGtKG;
	
	pc->ops->apply          = BSSCR_PCApply_ScGtKG;
	pc->ops->applytranspose = BSSCR_PCApplyTranspose_ScGtKG;
	
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_PCScGtKGGetScalings( PC pc, Vec *X1, Vec *X2, Vec *Y1, Vec *Y2 )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	if( X1 != PETSC_NULL ) {  *X1 = ctx->X1;  }
	if( X2 != PETSC_NULL ) {  *X2 = ctx->X2;  }
	
	if( Y1 != PETSC_NULL ) {  *Y1 = ctx->Y1;  }
	if( Y2 != PETSC_NULL ) {  *Y2 = ctx->Y2;  }
	
	
	PetscFunctionReturn(0);
}


/*
F & Bt must different to PETSC_NULL
B may be PETSC_NULL
C can be PETSC_NULL
*/
PetscErrorCode BSSCR_PCScGtKGSetOperators( PC pc, Mat F, Mat Bt, Mat B, Mat C )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	
	ctx->F  = F;
	ctx->Bt = Bt;
	ctx->B  = B;
	ctx->C  = C;
	
	if( C != PETSC_NULL ) {
		pc->ops->apply          = BSSCR_BSSCR_PCApply_ScGtKG_C;
		pc->ops->applytranspose = PETSC_NULL;
	}
	
	/* Create vectors */
	if( ctx->s == PETSC_NULL ) {  MatGetVecs( ctx->F, &ctx->s, PETSC_NULL );  }
	if( ctx->X == PETSC_NULL ) {  MatGetVecs( ctx->F, PETSC_NULL, &ctx->X );  }
	if( ctx->t == PETSC_NULL ) {  MatGetVecs( ctx->Bt, &ctx->t, PETSC_NULL ); }
	
	if( ctx->F == PETSC_NULL ) {   Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: F not set" );   }
	if( ctx->Bt == PETSC_NULL ) {  Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: Bt not set" );  }
	
	if( ctx->X1 == PETSC_NULL ) {  MatGetVecs( ctx->F, &ctx->X1, PETSC_NULL );  }
	if( ctx->Y1 == PETSC_NULL ) {  MatGetVecs( ctx->F, &ctx->Y1, PETSC_NULL );  }
	if( ctx->X2 == PETSC_NULL ) {  MatGetVecs( ctx->Bt, &ctx->X2, PETSC_NULL ); }
	if( ctx->Y2 == PETSC_NULL ) {  MatGetVecs( ctx->Bt, &ctx->Y2, PETSC_NULL ); }
	
	VecSet( ctx->X1, 1.0 );
	VecSet( ctx->Y1, 1.0 );
	VecSet( ctx->X2, 1.0 );
	VecSet( ctx->Y2, 1.0 );
	
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_PCScGtKGAttachNullSpace( PC pc )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	MatNullSpace nsp;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	/* Attach a null space */
	MatNullSpaceCreate( PETSC_COMM_WORLD, PETSC_TRUE, PETSC_NULL, PETSC_NULL, &nsp );
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR <6) )
	KSPSetNullSpace( ctx->ksp_BBt, nsp );
#else
    Mat A;
    KSPGetOperators(ctx->ksp_BBt,&A,NULL);//Note: DOES NOT increase the reference counts of the matrix, so you should NOT destroy them. 
    MatSetNullSpace( A, nsp);
#endif
	/* 
	NOTE: This does NOT destroy the memory for nsp, it just decrements the nsp->refct, so that
	the next time MatNullSpaceDestroy() is called, the memory will be released. The next time this
	is called will be by KSPDestroy();
	*/
	MatNullSpaceDestroy( nsp );
		
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCScGtKGGetKSP( PC pc, KSP *ksp )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	
	if( ksp != PETSC_NULL ) {
		(*ksp) = ctx->ksp_BBt;
	}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCScGtKGSetKSP( PC pc, KSP ksp )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	
	if( ctx->ksp_BBt != PETSC_NULL ) {
		Stg_KSPDestroy(&ctx->ksp_BBt);
	}
	
	PetscObjectReference( (PetscObject)ksp );
	ctx->ksp_BBt = ksp;
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_PCScGtKGSetSubKSPMonitor( PC pc, PetscTruth flg )
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	ctx->monitor_activated = flg;
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_PCScGtKGUseStandardScaling( PC pc ) 
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	Mat K,G,D,C;
	Vec rG;
	PetscScalar rg2, rg, ra;  
	PetscInt N;
	Vec rA, rC;
	Vec L1,L2, R1,R2;
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	
	L1 = ctx->X1;
	L2 = ctx->X2;
	
	R1 = ctx->Y1;
	R2 = ctx->Y2;
	
	
	rA = L1;
	rC = L2;
	
	K = ctx->F;
	G = ctx->Bt;
	D = ctx->B;
	C = ctx->C;
	
	VecDuplicate( rA, &rG );
	
	/* Get magnitude of K */  
	MatGetRowMax( K, rA, PETSC_NULL );
	
	VecSqrt( rA );  
	VecReciprocal( rA );
	
	VecDot( rA,rA, &ra );
	VecGetSize( rA, &N );
	ra = PetscSqrtScalar( ra/N );
	
	
	/* Get magnitude of G */
	MatGetRowMax( G, rG, PETSC_NULL );
	
	VecDot( rG, rG, &rg2 );
	VecGetSize( rG, &N );
	rg = PetscSqrtScalar(rg2/N);
	//      printf("rg = %f \n", rg );
	
	VecSet( rC, 1.0/(rg*ra) );
	
	Stg_VecDestroy(&rG );
	
	VecCopy( L1, R1 );
	VecCopy( L2, R2 );
	
	PetscFunctionReturn(0);
}

/*
Builds 
  B Y1 X1 Bt 
and creates a ksp when C=0, otherwise it builds 
  B Y1 X1 Bt - C
*/
PetscErrorCode BSSCR_PCScGtKGUseStandardBBtOperator( PC pc ) 
{
	PC_SC_GtKG ctx = (PC_SC_GtKG)pc->data;
	PetscReal  fill;
	Mat        diag_mat,C;
	Vec        diag;
	PetscInt   M,N, m,n;
	MPI_Comm   comm;
	PetscInt   nnz_I, nnz_G;
	MatType    mtype;
	const char *prefix;
	Mat BBt;
	KSP ksp;
	PetscTruth ivalue, flg, has_cnst_nullsp;
	
	
	BSSCR_BSSCR_pc_error_ScGtKG( pc, __func__ );
	
	/* Assemble BBt */
	MatGetSize( ctx->Bt, &M, &N );
	MatGetLocalSize( ctx->Bt, &m, &n );
	
	MatGetVecs( ctx->Bt, PETSC_NULL, &diag );
	
	/* Define diagonal matrix Y1 X1 */
	VecPointwiseMult( diag, ctx->Y1, ctx->X1 );
	
	PetscObjectGetComm( (PetscObject)ctx->F, &comm ); 
	MatCreate( comm, &diag_mat );
	MatSetSizes( diag_mat, m,m , M, M );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
        MatSetUp(diag_mat);
#endif
	MatGetType( ctx->Bt, &mtype );
	MatSetType( diag_mat, mtype );
	
	MatDiagonalSet( diag_mat, diag, INSERT_VALUES );
	
	/* Build operator B Y1 X1 Bt */
	BSSCR_BSSCR_get_number_nonzeros_AIJ_ScGtKG( diag_mat, &nnz_I );
	BSSCR_BSSCR_get_number_nonzeros_AIJ_ScGtKG( ctx->Bt, &nnz_G );
	/* 
	Not sure the best way to estimate the fill factor.
	BBt is a laplacian on the pressure space. 
	This might tell us something useful...
	*/
	fill = (PetscReal)(nnz_G)/(PetscReal)( nnz_I );
	MatPtAP( diag_mat, ctx->Bt, MAT_INITIAL_MATRIX, fill, &BBt );
	
	Stg_MatDestroy(&diag_mat );
	Stg_VecDestroy(&diag );
	
	
	C = ctx->C;
	if( C !=PETSC_NULL ) {
		MatAXPY( BBt, -1.0, C, DIFFERENT_NONZERO_PATTERN );
	}
	
	
	/* Build the solver */
	KSPCreate( ((PetscObject)pc)->comm, &ksp );
	
	Stg_KSPSetOperators( ksp, BBt, BBt, SAME_NONZERO_PATTERN );
	
	PCGetOptionsPrefix( pc,&prefix );
	KSPSetOptionsPrefix( ksp, prefix );
	KSPAppendOptionsPrefix( ksp, "pc_gtkg_" ); /* -pc_GtKG_ksp_type <type>, -ksp_GtKG_pc_type <type> */
	
	BSSCR_PCScGtKGSetKSP( pc, ksp );
	
	BSSCR_MatContainsConstNullSpace( BBt, NULL, &has_cnst_nullsp );
	if( has_cnst_nullsp == PETSC_TRUE ) {
		BSSCR_PCScGtKGAttachNullSpace( pc );
	}
	
	PetscOptionsGetTruth( PETSC_NULL, "-pc_gtkg_monitor", &ivalue, &flg );
	BSSCR_PCScGtKGSetSubKSPMonitor( pc, ivalue );
	
	Stg_KSPDestroy(&ksp);
	Stg_MatDestroy(&BBt);
	
	PetscFunctionReturn(0);
}

#endif

