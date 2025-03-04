/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*

Given a block system
A x = b,
or
( K G )(u) = (f)
( D C )(p)   (h)

We define a symmetrically scaled system,
L A R R^-1 x = L b,		A'x' = b'
where L = diag(L1, 1), R = diag(R1,1) are block diagonal
and L_i and R_i are both diagonal matrirces.

The scaling produces
( L1   )( K G )( R1   )( R1   )^-1(u) = ( L1   )(f)
Du + Cp = (h)
and we solve
( L1   )( K G )( R1   )(v) = ( L1   )(f)
 Du + Cp  = (h)
or
( L1*K*R1, L1*G )(v) = (L1*f)
( D*R1  ,  C    )(p) = ( h  )

The solution u,p is recovered via
(u) = (R1*v)
(p) = (p) 

*/

#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>

#include "common-driver-utils.h"
#include "stokes_Kblock_scaling.h"

/* private prototypes */
PetscErrorCode BSSCR_MatStokesKBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_mat_kblock_invert_scalings( MatStokesBlockScaling BA );



/* A x = b -> A'x' = b' */
/*
Note this routine actually modifies the matrix and rhs b.
*/

// updated
PetscErrorCode BSSCR_MatStokesKBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	Mat K,G,D;
	Vec L1,R1;
	Vec f,u;
	
	
	/* Get the scalings out the block mat data */
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	
	/* get the subblock solution and rhs */
	if( x != PETSC_NULL ) {
		VecNestGetSubVec( x, 0, &u );
		VecPointwiseDivide( u, u,R1); /* x <- x * 1/R1 */
	}
	if( b != PETSC_NULL ) {
		VecNestGetSubVec( b, 0, &f );
		VecPointwiseMult( f, f,L1); /* f <- f * L1 */
	}
	
	/* Scale matrices */
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	
	if( K != PETSC_NULL ) {		MatDiagonalScale( K, L1,R1 );		}
	if( G != PETSC_NULL ) {		MatDiagonalScale( G, L1,PETSC_NULL );		}
	if( D != PETSC_NULL && !sym ) {	MatDiagonalScale( D, PETSC_NULL,R1 );		}
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesKBlockScalingCreate( MatStokesBlockScaling *_BA )
{
	MatStokesBlockScaling BA;
	PetscErrorCode ierr;

	ierr = PetscMalloc( sizeof(struct _p_MatStokesBlockScaling), &BA );CHKERRQ(ierr);
	
	BA->Lz = PETSC_NULL;
	BA->Rz = PETSC_NULL;
	
	BA->scaling_exists              = PETSC_FALSE;
	BA->scalings_have_been_inverted = PETSC_FALSE;
	BA->system_has_been_scaled      = PETSC_FALSE;
	
	*_BA = BA;
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesKBlockScalingDestroy( MatStokesBlockScaling BA )
{
	
	if( BA->scaling_exists == PETSC_FALSE ) PetscFunctionReturn(0); 
	
	if( BA->Lz != PETSC_NULL ) {
		Stg_VecDestroy(&BA->Lz );
		BA->Lz = PETSC_NULL;
	}
	
	if( BA->Rz != PETSC_NULL ) {
		Stg_VecDestroy(&BA->Rz );
		BA->Rz = PETSC_NULL;
	}
	
	PetscFree( BA );
	
	PetscFunctionReturn(0);
}

/*
A is 2x2 block matrix
b and x are 2x1 block vectors
*/
// updated
PetscErrorCode BSSCR_MatKBlock_ConstructScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym )
{
	
	
	if( BA->scaling_exists == PETSC_FALSE ) {
      //PetscInt M,N;
		//PetscTruth is_block;
		
		/* check A is 2x2 block matrix */
		/* Stg_PetscTypeCompare( (PetscObject)A, "block", &is_block ); */
		/* if (is_block==PETSC_FALSE) { */
		/* 	Stg_SETERRQ( PETSC_ERR_SUP, "Only valid for MatType = block" ); */
		/* } */
		/* MatGetSize( A, &M, &N ); */
		/* if ( (M!=2) || (N!=2) ) { */
		/* 	Stg_SETERRQ2( PETSC_ERR_SUP, "Only valid for 2x2 block. Yours has dimension %Dx%D", M,N ); */
		/* } */
		
		VecDuplicate( x, &BA->Lz ); 
		VecDuplicate( x, &BA->Rz );
		
		BA->scaling_exists = PETSC_TRUE;
	}
	
//	if( BA->user_build_scaling != PETSC_NULL ) {
//		BA->user_build_scaling( A,b,x,BA->scaling_ctx);
//	}
//	else {
	BSSCR_MatStokesKBlockDefaultBuildScaling(BA,A,b,x, sym);
//	}
	
	BA->scalings_have_been_inverted = PETSC_FALSE;
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_mat_kblock_invert_scalings( MatStokesBlockScaling BA )
{
	Vec L1,R1;
	
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	
	VecReciprocal(L1);
	VecReciprocal(R1);

	/* toggle inversion flag */
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {   BA->scalings_have_been_inverted = PETSC_FALSE;  }
	if( BA->scalings_have_been_inverted == PETSC_FALSE ) {  BA->scalings_have_been_inverted = PETSC_TRUE;   }
	
	PetscFunctionReturn(0);
}

/* updated */
PetscErrorCode BSSCR_MatStokesKBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	if( BA->scaling_exists == PETSC_FALSE ) {
	    BSSCR_MatKBlock_ConstructScaling( BA,A,b,x, sym );
	}
	
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {
		BSSCR_mat_kblock_invert_scalings(BA); /* to undo inversion */
	}
	
	BSSCR_MatStokesKBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_TRUE;
	
	/*
	PetscPrintf( PETSC_COMM_WORLD, "Post Scaling \n");
	MatBlock_ReportOperatorScales(A);
	*/
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesKBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	
	if( BA->system_has_been_scaled == PETSC_FALSE ) {
		printf("Warning: MatBlock has not been scaled !! \n");
		PetscFunctionReturn(0);
	}
	
	if( BA->scalings_have_been_inverted == PETSC_FALSE ) {
		BSSCR_mat_kblock_invert_scalings(BA);
	}
	
	BSSCR_MatStokesKBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_FALSE;
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_MatStokesKBlockReportOperatorScales( Mat A, PetscTruth sym )
{
	Vec rA, rG;
	PetscInt loc;
	PetscReal min, max;
	Mat K,G,D,C;
	//PetscTruth is_block;
	
	
	/* check A is 2x2 block matrix */
	/* Stg_PetscTypeCompare( (PetscObject)A, "block", &is_block ); */
	/* if (is_block==PETSC_FALSE) { */
	/* 	Stg_SETERRQ( PETSC_ERR_SUP, "Only valid for MatType = block" ); */
	/* } */
	/* MatGetSize( A, &M, &N ); */
	/* if ( (M!=2) || (N!=2) ) { */
	/* 	Stg_SETERRQ2( PETSC_ERR_SUP, "Only valid for 2x2 block. Yours has dimension %Dx%D", M,N ); */
	/* } */
	
	
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	MatNestGetSubMat( A, 1,1, &C );
	
	
	MatGetVecs( K, PETSC_NULL, &rA );
	VecDuplicate( rA, &rG );
	
	/* Report the row max and mins */
	if (K!=PETSC_NULL) {
		MatGetRowMaxAbs( K, rA, PETSC_NULL );
		VecMax( rA, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(K) = %g \n", max );
		
		MatGetRowMinAbs( K, rA, PETSC_NULL );
		VecMin( rA, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(K) = %g \n\n", min );
	}
	
	if( G != PETSC_NULL ) {       
		MatGetRowMaxAbs( G, rG, PETSC_NULL );
		VecMax( rG, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(G) = %g \n", max );
		
		MatGetRowMinAbs( G, rG, PETSC_NULL );
		VecMin( rG, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(G) = %g \n", min );
	}
	
	if( D != PETSC_NULL && !sym ) {
                Vec rD;

                MatGetVecs( D, PETSC_NULL, &rD );
		MatGetRowMaxAbs( D, rD, PETSC_NULL );
		VecMax( rD, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(D) = %g \n", max );
		
		MatGetRowMinAbs( D, rD, PETSC_NULL );
		VecMin( rD, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(D) = %g \n", min );

                Stg_VecDestroy(&rD );
	}
	
	if( C != PETSC_NULL ) {
		Vec cG;

		MatGetVecs( G, &cG, PETSC_NULL );
		MatGetRowMaxAbs( C, cG, PETSC_NULL );
		VecMax( cG, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(C) = %g \n", max );
		
		MatGetRowMinAbs( C, cG, PETSC_NULL );
		VecMin( cG, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(C) = %g \n\n", min );
	
		Stg_VecDestroy(&cG);
	}
	
	
	Stg_VecDestroy(&rA );
	Stg_VecDestroy(&rG );
	
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesKBlockDefaultBuildScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym )
{
	Mat K;
	Vec rA;
	Vec L1, R1;
	
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	
	rA = L1;
	
	MatNestGetSubMat( A, 0,0, &K );
	
	/* Get diag of K */  
	MatGetDiagonal( K, rA);
	VecSqrt( rA );  
	VecReciprocal( rA );
	
	/* leave Shat alone as the Schur Compliment is unchanged by any consistent scaling of K */
	//MatGetDiagonal( Shat, rC);
	//VecSqrt( rC );  
	//VecReciprocal( rC );
	
	/* no scaling for pressure */
	//VecSet( rC, 1.0 );
	
	VecCopy( L1, R1 );
	
	PetscFunctionReturn(0);
}

