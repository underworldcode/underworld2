/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

/*

Performs y = ( G^T G )^{-1} ( G^T K G ) ( G^T G )^{-1} x
This preconditioner is defined as a PC rather than a MatShell
as we can define and compute the inverse easily without the
need of an additional KSP. Compare this to Q_S = G^T Q_K^-1 G,
where Q_S^-1 can only be performed implicitly via an iterative
method.

*/
#if 1

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

#include "pc_GtKG.h"

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>

#define PCTYPE_GtKG "gtkg"


/* private data */

typedef struct {
	Mat         G, K;   /* G \in [M x N], K \in [M x M] */
	Mat         M;      /* the velocity mass matrix */
	Vec         inv_diag_M;
	PetscTruth  form_GtG; /* don't allow anything else yet */
	Mat         GtG;
	KSP         ksp;
	Vec         s,t,X;  /* s \in [M], t \in [N], X \in [M] */
	PetscTruth  monitor_activated;
	PetscTruth  monitor_rhs_consistency;
} _PC_GtKG;

typedef _PC_GtKG* PC_GtKG;


/* private prototypes */
PetscErrorCode BSSCR_PCApply_GtKG( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_PCApplyTranspose_GtKG( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_BSSCR_PCApply_GtKG_diagonal_scaling( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_BSSCR_PCApplyTranspose_GtKG_diagonal_scaling( PC pc, Vec x, Vec y );
PetscErrorCode BSSCR_PCSetUp_GtKG( PC pc );




PetscErrorCode BSSCR_pc_warn( PC pc, const char func_name[] ) 
{
	const PCType type;
	PCGetType( pc, &type );
	
	if( strcmp(type,PCTYPE_GtKG)!=0 ) {
		printf("Warning(%s): PC type (%s) should be gtkg \n",func_name, type );
		PetscFunctionReturn(0);
	}
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_pc_error( PC pc, const char func_name[] ) 
{
	const PCType type;
	PCGetType( pc, &type );
	
	if( strcmp(type,PCTYPE_GtKG)!=0 ) {
		printf("Error(%s): PC type (%s) should be gtkg \n",func_name, type );
		PetscFinalize();
		exit(0);
	}
	PetscFunctionReturn(0);
}

void BSSCR_get_number_nonzeros_AIJ( Mat A, PetscInt *nnz )
{
	MatInfo info;
	
	MatGetInfo( A, MAT_GLOBAL_SUM, &info );
	*nnz = info.nz_used;
	
}


/*
I should not modify setup called!!
This is handled via petsc.
*/
PetscErrorCode BSSCR_PCSetUp_GtKG( PC pc )
{
	PC_GtKG    ctx = (PC_GtKG)pc->data;
	PetscReal  fill;
	Mat        Ident;
	Vec        diag;
	PetscInt   M,N, m,n;
	MPI_Comm   comm;
	PetscInt   nnz_I, nnz_G;
	const MatType mtype;
	const char *prefix;
	PetscTruth wasSetup;
	
	
	
	if( ctx->K == PETSC_NULL ) {
		Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: K not set" );
	}
	if( ctx->G == PETSC_NULL ) {
		Stg_SETERRQ( PETSC_ERR_SUP, "gtkg: G not set" );
	}
	
	PetscObjectGetComm( (PetscObject)ctx->K, &comm ); 
	
	
	/* Check for existence of objects and trash any which exist */
	if( ctx->form_GtG == PETSC_TRUE && ctx->GtG != PETSC_NULL ) {
		Stg_MatDestroy(&ctx->GtG );
		ctx->GtG = PETSC_NULL;
	}
	
	if( ctx->s != PETSC_NULL ) {
		Stg_VecDestroy(&ctx->s );
		ctx->s = PETSC_NULL;
	}
	if( ctx->X != PETSC_NULL ) {
		Stg_VecDestroy(&ctx->X );
		ctx->X = PETSC_NULL;
	}
	if( ctx->t != PETSC_NULL ) {
		Stg_VecDestroy(&ctx->t );
		ctx->t = PETSC_NULL;
	}
	if( ctx->inv_diag_M != PETSC_NULL ) {
		Stg_VecDestroy(&ctx->inv_diag_M );
		ctx->inv_diag_M = PETSC_NULL;
	}
	
	
	
	/* Create vectors */
	MatGetVecs( ctx->K, &ctx->s, &ctx->X );
	MatGetVecs( ctx->G, &ctx->t, PETSC_NULL );
	
	if( ctx->M != PETSC_NULL ) {
		MatGetVecs( ctx->K, &ctx->inv_diag_M, PETSC_NULL );
		MatGetDiagonal( ctx->M, ctx->inv_diag_M );
		VecReciprocal( ctx->inv_diag_M );
		
		/* change the pc_apply routines */
		pc->ops->apply          = BSSCR_BSSCR_PCApply_GtKG_diagonal_scaling;
		pc->ops->applytranspose = BSSCR_BSSCR_PCApplyTranspose_GtKG_diagonal_scaling;
	}
	
	
	/* Assemble GtG */
	MatGetSize( ctx->G, &M, &N );
	MatGetLocalSize( ctx->G, &m, &n );
	
	MatGetVecs( ctx->G, PETSC_NULL, &diag );
	VecSet( diag, 1.0 );
	
	MatCreate( comm, &Ident );
	MatSetSizes( Ident, m,m , M, M );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
        MatSetUp(Ident);
#endif

	MatGetType( ctx->G, &mtype );
	MatSetType( Ident, mtype );
	
	if( ctx->M == PETSC_NULL ) {
		MatDiagonalSet( Ident, diag, INSERT_VALUES );
	}
	else {
		MatDiagonalSet( Ident, ctx->inv_diag_M, INSERT_VALUES );
	}
	
	BSSCR_get_number_nonzeros_AIJ( Ident, &nnz_I );
	BSSCR_get_number_nonzeros_AIJ( ctx->G, &nnz_G );
	//fill = 1.0;
	/* 
	Not sure the best way to estimate the fill factor.
	GtG is a laplacian on the pressure space. 
	This might tell us something useful...
	*/
	fill = (PetscReal)(nnz_G)/(PetscReal)( nnz_I );
	MatPtAP( Ident, ctx->G, MAT_INITIAL_MATRIX, fill, &ctx->GtG );
	
	Stg_MatDestroy(&Ident);
	Stg_VecDestroy(&diag );
	
	
	Stg_KSPSetOperators( ctx->ksp, ctx->GtG, ctx->GtG, SAME_NONZERO_PATTERN );
	
	if (!pc->setupcalled) {	
		wasSetup = PETSC_FALSE;
		
		PCGetOptionsPrefix( pc,&prefix );
		KSPSetOptionsPrefix( ctx->ksp, prefix );
		KSPAppendOptionsPrefix( ctx->ksp, "pc_gtkg_" ); /* -pc_GtKG_ksp_type <type>, -ksp_GtKG_pc_type <type> */
	}
	else {
		wasSetup = PETSC_TRUE;
	}	
	
	
//	if (!wasSetup && pc->setfromoptionscalled) {
	if (!wasSetup) {
		KSPSetFromOptions(ctx->ksp);
	}
	
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCDestroy_GtKG( PC pc )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	
	if( ctx == PETSC_NULL ) {	PetscFunctionReturn(0); }
	
	if( ctx->form_GtG == PETSC_TRUE && ctx->GtG != PETSC_NULL ) {
		Stg_MatDestroy(&ctx->GtG );
	}
	if( ctx->ksp != PETSC_NULL ) {	Stg_KSPDestroy(&ctx->ksp );		}
	if( ctx->s != PETSC_NULL ) {	Stg_VecDestroy(&ctx->s );		}
	if( ctx->X != PETSC_NULL ) {	Stg_VecDestroy(&ctx->X );		}
	if( ctx->t != PETSC_NULL ) {	Stg_VecDestroy(&ctx->t );		}
	if( ctx->inv_diag_M != PETSC_NULL ) {
		Stg_VecDestroy(&ctx->inv_diag_M );
		ctx->inv_diag_M = PETSC_NULL;
	}
	
	PetscFree( ctx );
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCView_GtKG( PC pc, PetscViewer viewer )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	PetscViewerASCIIPushTab(viewer); //1

	if( ctx->M == PETSC_NULL ) {
		PetscViewerASCIIPrintf( viewer, "gtkg: Standard \n" );
	}else {
		PetscViewerASCIIPrintf( viewer, "gtkg: Least Squares Commutator \n" );
	}
	
	PetscViewerASCIIPrintf( viewer, "gtkg-ksp \n" );
	PetscViewerASCIIPrintf(viewer,"---------------------------------\n");
	PetscViewerASCIIPushTab(viewer);
		KSPView( ctx->ksp, viewer );
	PetscViewerASCIIPopTab(viewer);
	PetscViewerASCIIPrintf(viewer,"---------------------------------\n");
	
	PetscViewerASCIIPopTab(viewer); //1
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_Lp_monitor( KSP ksp, PetscInt index )
{
	PetscInt max_it;
	PetscReal rnorm;
	KSPConvergedReason reason;
	
	KSPGetIterationNumber( ksp, &max_it );
	KSPGetResidualNorm( ksp, &rnorm );
	KSPGetConvergedReason( ksp, &reason );
	if (ksp->reason > 0) {
		PetscPrintf(((PetscObject)ksp)->comm,"\t<Lp(%d)>: Linear solve converged. its.=%.4d ; |r|=%5.5e ; Reason=%s\n", 
				index, max_it, rnorm, KSPConvergedReasons[reason] );
	} else {
		PetscPrintf(((PetscObject)ksp)->comm,"\t<Lp(%d)>: Linear solve did not converge. its.=%.4d ; |r|=%5.5e ; Reason=%s\n", 
				index, max_it, rnorm, KSPConvergedReasons[reason]);
	}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCBFBTSubKSPMonitor( KSP ksp, PetscInt index, PetscLogDouble time )
{
        PetscInt max_it;
        PetscReal rnorm;
        KSPConvergedReason reason;

        KSPGetIterationNumber( ksp, &max_it );
        KSPGetResidualNorm( ksp, &rnorm );
        KSPGetConvergedReason( ksp, &reason );
        PetscPrintf(((PetscObject)ksp)->comm,"    PCBFBTSubKSP (%d): %D Residual norm; r0 %12.12e, r %12.12e: Reason %s: Time %5.5e \n", 
		index, max_it, ksp->rnorm0, rnorm, KSPConvergedReasons[reason], time );

        PetscFunctionReturn(0);
}



/* 
Checks rhs of Lp systems is in the null space of Lp, i.e. {rhs} \centerdot {null_space} = 0
Since Lp should contain the null space {1}, we just check the \sum_i rhs_i = 0
*/
PetscErrorCode BSSCRBSSCR_Lp_monitor_check_rhs_consistency( KSP ksp, Vec rhs, PetscInt index )
{
	PetscScalar dot;
#if 0
	Vec one;
	
	VecDuplicate( rhs, &one );
	VecSet( one, 1.0 );
	VecDot( rhs, one, &dot );
	if( PetscAbsReal(dot) > 1.0e-8 ) {
		PetscPrintf(ksp->comm,"    ($D) Lp z = r: ******* WARNING ******* RHS is not consistent. {b}.{1} = %5.5e \n", index, dot );
	}
	
	Stg_VecDestroy(&one );
	PetscFunctionReturn(0);
#endif
#if 0	
	VecSum( rhs, &dot );
	if( PetscAbsReal(dot) > 1.0e-8 ) {
		PetscPrintf(((PetscObject)ksp)->comm,"    (%D) Lp z = r: ******* WARNING ******* RHS is not consistent. {b}.{1} = %5.5e \n", 
				index, dot );
		BSSCR_VecRemoveConstNullspace( rhs, PETSC_NULL );
	}
#endif
	PetscFunctionReturn(0);
}


/* 
Performs y <- S^{-1} x 
S^{-1} = ( G^T G )^{-1} G^T K G ( G^T G )^{-1}
*/
PetscErrorCode BSSCR_PCApply_GtKG( PC pc, Vec x, Vec y )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	KSP ksp;
	Mat K, G;
	Vec s,t,X;
	PetscLogDouble t0,t1;	

	ksp = ctx->ksp;
	K = ctx->K;
	G = ctx->G;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,x,1);		}
	PetscGetTime(&t0);
	KSPSolve( ksp, x, t ); /* t <- GtG_inv x */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {	BSSCR_PCBFBTSubKSPMonitor(ksp,1,(t1-t0));		}
	
	MatMult( G, t, s ); /* s <- G t */
	MatMult( K, s, X ); /* X <- K s */
	MatMultTranspose( G, X, t ); /* t <- Gt X */
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,t,2);		}
	PetscGetTime(&t0);
	KSPSolve( ksp, t, y ); /* y <- GtG_inv t */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {	BSSCR_PCBFBTSubKSPMonitor(ksp,2,(t1-t0));		}
	
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
PetscErrorCode BSSCR_PCApplyTranspose_GtKG( PC pc, Vec x, Vec y )
{
	
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	KSP ksp;
	Mat K, G;
	Vec s,t,X;
	PetscLogDouble t0,t1;
	
	ksp = ctx->ksp;
	K = ctx->K;
	G = ctx->G;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,x,1);		}
	PetscGetTime(&t0);
	KSPSolve( ksp, x, t ); /* t <- GtG_inv x */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {	BSSCR_PCBFBTSubKSPMonitor(ksp,1,(t1-t0));		}
	
	MatMult( G, t, s ); /* s <- G t */
	MatMultTranspose( K, s, X ); /* X <- K^T s */
	MatMultTranspose( G, X, t ); /* t <- Gt X */
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,t,2);		}
	PetscGetTime(&t0);
	KSPSolve( ksp, t, y ); /* y <- GtG_inv t */
	PetscGetTime(&t1);
	if (ctx->monitor_activated) {	BSSCR_PCBFBTSubKSPMonitor(ksp,2,(t1-t0));		}
	
	PetscFunctionReturn(0);
}


/* 
Performs y <- S^{-1} x 
S^{-1} = ( G^T Di G )^{-1} G^T Di K Di G ( G^T Di G )^{-1}
where Di = diag(M)^{-1}
*/


PetscErrorCode BSSCR_BSSCR_PCApply_GtKG_diagonal_scaling( PC pc, Vec x, Vec y )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	KSP ksp;
	Mat K, G;
	Vec s,t,X,di;
	
	
	ksp = ctx->ksp;
	K = ctx->K;
	G = ctx->G;
	di = ctx->inv_diag_M;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,x,1);		}
	KSPSolve( ksp, x, t );       /* t <- GtG_inv x */
	if (ctx->monitor_activated) {	BSSCR_Lp_monitor(ksp,2);		}
	
	MatMult( G, t, s );          /* s <- G t */
	VecPointwiseMult( s, s,di ); /* s <- s * di */
	
	MatMult( K, s, X );          /* X <- K s */
	VecPointwiseMult( X, X,di ); /* X <- X * di */
	
	MatMultTranspose( G, X, t ); /* t <- Gt X */
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,t,2);		}
	KSPSolve( ksp, t, y );       /* y <- GtG_inv t */
	if (ctx->monitor_activated) {	BSSCR_Lp_monitor(ksp,2);		}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_BSSCR_PCApplyTranspose_GtKG_diagonal_scaling( PC pc, Vec x, Vec y )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	KSP ksp;
	Mat K, G;
	Vec s,t,X,di;
	
	
	ksp = ctx->ksp;
	K = ctx->K;
	G = ctx->G;
	di = ctx->inv_diag_M;
	s = ctx->s;
	t = ctx->t;
	X = ctx->X;
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,x,1);		}
	KSPSolve( ksp, x, t );       /* t <- GtG_inv x */
	if (ctx->monitor_activated) {	BSSCR_Lp_monitor(ksp,1);		}
	
	MatMult( G, t, s );          /* s <- G t */
	VecPointwiseMult( s, s,di ); /* s <- s * di */
	
	MatMultTranspose( K, s, X ); /* X <- K^T s */
	VecPointwiseMult( X, X,di ); /* X <- X * di */
	
	MatMultTranspose( G, X, t ); /* t <- Gt X */
	
	if (ctx->monitor_rhs_consistency) {		BSSCRBSSCR_Lp_monitor_check_rhs_consistency(ksp,t,2);		}
	KSPSolve( ksp, t, y );       /* y <- GtG_inv t */
	if (ctx->monitor_activated) {	BSSCR_Lp_monitor(ksp,2);		}
	
	PetscFunctionReturn(0);
}





/*
Only the options related to GtKG should be set here.
*/
PetscErrorCode BSSCR_PCSetFromOptions_GtKG( PC pc )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	PetscTruth ivalue, flg;
	
	PetscOptionsGetTruth( PETSC_NULL, "-pc_gtkg_monitor", &ivalue, &flg );
	if( flg==PETSC_TRUE ) {
		ctx->monitor_activated = ivalue;
	}
	
	PetscOptionsGetTruth( PETSC_NULL, "-pc_gtkg_monitor_rhs_consistency", &ivalue, &flg );
	if( flg==PETSC_TRUE ) {
		ctx->monitor_rhs_consistency = ivalue;
	}
	
	
	PetscFunctionReturn(0);
}



/* ---- Exposed functions ---- */

PetscErrorCode BSSCR_PCCreate_GtKG( PC pc )
{
	PC_GtKG         pc_data;
	PetscErrorCode  ierr;
	
	/* create memory for ctx */
	ierr = Stg_PetscNew( _PC_GtKG,&pc_data);CHKERRQ(ierr);
	
	/* init ctx */
	pc_data->K   = PETSC_NULL;
	pc_data->G   = PETSC_NULL;
	pc_data->M   = PETSC_NULL;
	pc_data->GtG = PETSC_NULL;
	pc_data->form_GtG          = PETSC_TRUE;
	pc_data->ksp               = PETSC_NULL;
	pc_data->monitor_activated = PETSC_FALSE;
	pc_data->monitor_rhs_consistency = PETSC_FALSE;
	
	pc_data->s = PETSC_NULL;
	pc_data->t = PETSC_NULL;
	pc_data->X = PETSC_NULL;
	pc_data->inv_diag_M = PETSC_NULL;
	
	/* create internals */
	KSPCreate( ((PetscObject)pc)->comm, &pc_data->ksp );
	
	
	/* set ctx onto pc */
	pc->data  = (void*)pc_data;
	
	ierr = PetscLogObjectMemory(pc,sizeof(_PC_GtKG));CHKERRQ(ierr);
	
	/* define operations */
	pc->ops->setup   = BSSCR_PCSetUp_GtKG;
	pc->ops->view    = BSSCR_PCView_GtKG;
	pc->ops->destroy = BSSCR_PCDestroy_GtKG;
	pc->ops->setfromoptions = BSSCR_PCSetFromOptions_GtKG;
	
	pc->ops->apply          = BSSCR_PCApply_GtKG;
	pc->ops->applytranspose = BSSCR_PCApplyTranspose_GtKG;
	
	
	PetscFunctionReturn(0);
}


/*
K & G must different to PETSC_NULL
M can be PETSC_NULL
*/
PetscErrorCode BSSCR_PCGtKGSet_Operators( PC pc, Mat K, Mat G, Mat M )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	BSSCR_pc_error( pc, "__func__" );
	
	ctx->K = K;
	ctx->G = G;
	ctx->M = M;
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCGtKGSet_OperatorForAlgebraicCommutator( PC pc, Mat M )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	BSSCR_pc_error( pc, "__func__" );
	
	ctx->M = M;
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCGtKGAttachNullSpace( PC pc )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	MatNullSpace nsp;
	
	BSSCR_pc_error( pc, "__func__" );
	
	/* Attach a null space */
	MatNullSpaceCreate( PETSC_COMM_WORLD, PETSC_TRUE, PETSC_NULL, PETSC_NULL, &nsp );
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR <6) )
	KSPSetNullSpace( ctx->ksp, nsp );
#else
    Mat A;
    KSPGetOperators(ctx->ksp,&A,NULL);//Note: DOES NOT increase the reference counts of the matrix, so you should NOT destroy them. 
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

PetscErrorCode BSSCR_PCGtKGGet_KSP( PC pc, KSP *ksp )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	BSSCR_pc_error( pc, "__func__" );
	
	
	if( ksp != PETSC_NULL ) {
		(*ksp) = ctx->ksp;
	}
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCGtKGSet_KSP( PC pc, KSP ksp )
{
	PC_GtKG ctx = (PC_GtKG)pc->data;
	
	BSSCR_pc_error( pc, "__func__" );
	
	
	if( ctx->ksp != PETSC_NULL ) {
		Stg_KSPDestroy(&ctx->ksp);
	}
	ctx->ksp = ksp;
	
	PetscFunctionReturn(0);
}
#endif

