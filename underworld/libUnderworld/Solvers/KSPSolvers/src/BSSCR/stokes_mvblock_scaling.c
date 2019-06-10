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
where L = diag(L1, L2), R = diag(R1,R2) are block diagonal
and L_i and R_i are both diagonal matrirces.

The scaling produces
( L1   )( K G )( R1   )( R1   )^-1(u) = ( L1   )(f)
(   L2 )( D C )(   R2 )(   R2 )   (p)   (   L2 )(h)
and we solve
( L1   )( K G )( R1   )(v) = ( L1   )(f)
(   L2 )( D C )(   R2 )(q)   (   L2 )(h)
or
( L1KR1 L1GR2 )(v) = (L1f)
( L2DR1 L2CR2 )(q)   (L2h)

The solution u,p is recovered via
(u) = ( R1   )(v)
(p)   (   R2 )(q) 

*/

#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "common-driver-utils.h"
#include "stokes_mvblock_scaling.h"

/* private prototypes */
PetscErrorCode BSSCR_MatStokesMVBlockDefaultBuildScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym  );
PetscErrorCode BSSCR_MatStokesMVBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );
PetscErrorCode BSSCR_MatMVBlock_ConstructScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym  );
PetscErrorCode BSSCR_mat_mvblock_invert_scalings( MatStokesBlockScaling BA );



/* A x = b -> A'x' = b' */
/*
Note this routine actually modifies the matrix and rhs b.
*/

// updated
PetscErrorCode BSSCR_MatStokesMVBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	Mat K,G,D,C;
	Vec L1,L2, R1,R2;
	Vec f,h, u,p;
	
	
	/* Get the scalings out the block mat data */
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Lz, 1, &L2 );
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	VecNestGetSubVec( BA->Rz, 1, &R2 );
	
	
	/* get the subblock solution and rhs */
	if( x != PETSC_NULL ) {
		VecNestGetSubVec( x, 0, &u );
		VecNestGetSubVec( x, 1, &p );
		
		VecPointwiseDivide( u, u,R1); /* x <- x * 1/R1 */
		VecPointwiseDivide( p, p,R2);
	}
	if( b != PETSC_NULL ) {
		VecNestGetSubVec( b, 0, &f );
		VecNestGetSubVec( b, 1, &h );
		
		VecPointwiseMult( f, f,L1); /* f <- f * L1 */
		VecPointwiseMult( h, h,L2);
	}
	
	
	/* Scale matrices */
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	MatNestGetSubMat( A, 1,1, &C );
	
	if( K != PETSC_NULL ) {		MatDiagonalScale( K, L1,R1 );		}
	if( G != PETSC_NULL ) {		MatDiagonalScale( G, L1,R2 );		}
	if( D != PETSC_NULL && !sym ) {	MatDiagonalScale( D, L2,R1 );		}
	if( C != PETSC_NULL ) {		MatDiagonalScale( C, L2,R2 );		}
	if( S != PETSC_NULL ) {		MatDiagonalScale( S, L2,R2 );		}

	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesMVBlockScalingCreate( MatStokesBlockScaling *_BA )
{
	MatStokesBlockScaling BA;
	PetscErrorCode ierr;

	ierr = PetscMalloc( sizeof(struct _p_MatStokesBlockScaling), &BA );
	
	BA->Lz = PETSC_NULL;
	BA->Rz = PETSC_NULL;
	
	BA->scaling_exists              = PETSC_FALSE;
	BA->scalings_have_been_inverted = PETSC_FALSE;
	BA->system_has_been_scaled      = PETSC_FALSE;
	
	*_BA = BA;
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesMVBlockScalingDestroy( MatStokesBlockScaling BA )
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
PetscErrorCode BSSCR_MatMVBlock_ConstructScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym )
{
	
	
	if( BA->scaling_exists == PETSC_FALSE ) {
		PetscInt M,N;
		PetscTruth is_block;
		
		/* check A is 2x2 block matrix */
		Stg_PetscObjectTypeCompare( (PetscObject)A, "block", &is_block );
		if (is_block==PETSC_FALSE) {
			Stg_SETERRQ( PETSC_ERR_SUP, "Only valid for MatType = block" );
		}
		MatGetSize( A, &M, &N );
		if ( (M!=2) || (N!=2) ) {
			Stg_SETERRQ2( PETSC_ERR_SUP, "Only valid for 2x2 block. Yours has dimension %Dx%D", M,N );
		}
		
		
		
		VecDuplicate( x, &BA->Lz ); 
		VecDuplicate( x, &BA->Rz );
		
		BA->scaling_exists = PETSC_TRUE;
	}
	
//	if( BA->user_build_scaling != PETSC_NULL ) {
//		BA->user_build_scaling( A,b,x,BA->scaling_ctx);
//	}
//	else {
	BSSCR_MatStokesMVBlockDefaultBuildScaling(BA,A,b,x, sym);
//	}
	
	BA->scalings_have_been_inverted = PETSC_FALSE;
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_mat_mvblock_invert_scalings( MatStokesBlockScaling BA )
{
	Vec L1,L2, R1,R2;
	
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Lz, 1, &L2 );
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	VecNestGetSubVec( BA->Rz, 1, &R2 );
	
	
	VecReciprocal(L1);
	VecReciprocal(L2);
	VecReciprocal(R1);
	VecReciprocal(R2);
	
	/* toggle inversion flag */
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {   BA->scalings_have_been_inverted = PETSC_FALSE;  }
	if( BA->scalings_have_been_inverted == PETSC_FALSE ) {  BA->scalings_have_been_inverted = PETSC_TRUE;   }
	
	PetscFunctionReturn(0);
}

/* updated */
PetscErrorCode BSSCR_MatStokesMVBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	if( BA->scaling_exists == PETSC_FALSE ) {
	    BSSCR_MatMVBlock_ConstructScaling( BA,A,b,x, sym );
	}
	
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {
		BSSCR_mat_mvblock_invert_scalings(BA); /* to undo inversion */
	}
	
	BSSCR_MatStokesMVBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_TRUE;
	
	/*
	PetscPrintf( PETSC_COMM_WORLD, "Post Scaling \n");
	MatBlock_ReportOperatorScales(A);
	*/
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesMVBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	
	if( BA->system_has_been_scaled == PETSC_FALSE ) {
		printf("Warning: MatBlock has not been scaled !! \n");
		PetscFunctionReturn(0);
	}
	
	if( BA->scalings_have_been_inverted == PETSC_FALSE ) {
		BSSCR_mat_mvblock_invert_scalings(BA);
	}
	
	BSSCR_MatStokesMVBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_FALSE;
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_MatStokesMVBlockReportOperatorScales( Mat A, PetscTruth sym )
{
	Vec rA, rG;
	PetscInt loc,M,N;
	PetscReal min, max;
	Mat K,G,D,C;
	PetscTruth is_block;
	
	
	/* check A is 2x2 block matrix */
	Stg_PetscObjectTypeCompare( (PetscObject)A, "block", &is_block );
	if (is_block==PETSC_FALSE) {
		Stg_SETERRQ( PETSC_ERR_SUP, "Only valid for MatType = block" );
	}
	MatGetSize( A, &M, &N );
	if ( (M!=2) || (N!=2) ) {
		Stg_SETERRQ2( PETSC_ERR_SUP, "Only valid for 2x2 block. Yours has dimension %Dx%D", M,N );
	}
	
	
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	MatNestGetSubMat( A, 1,1, &C );
	
	
	MatGetVecs( K, PETSC_NULL, &rA );
	VecDuplicate( rA, &rG );
	
	/* Report the row max and mins */
	if (K!=PETSC_NULL) {
		MatGetRowMax( K, rA, PETSC_NULL );
		VecMax( rA, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(K) = %g \n", max );
		
		MatGetRowMinAbs( K, rA, PETSC_NULL );
		VecMin( rA, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(K) = %g \n\n", min );
	}
	
	if( G != PETSC_NULL ) {       
		MatGetRowMax( G, rG, PETSC_NULL );
		VecMax( rG, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(G) = %g \n", max );
		
		MatGetRowMinAbs( G, rG, PETSC_NULL );
		VecMin( rG, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(G) = %g \n", min );
	}
	
	if( D != PETSC_NULL && !sym ) {
                Vec rD;

                MatGetVecs( D, PETSC_NULL, &rD );
		MatGetRowMax( D, rD, PETSC_NULL );
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
		MatGetRowMax( C, cG, PETSC_NULL );
		VecMax( cG, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(C) = %g \n", max );
		
		MatGetRowMin( C, cG, PETSC_NULL );
		VecMin( cG, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(C) = %g \n\n", min );
	
		Stg_VecDestroy(&cG);
	}
	
	
	Stg_VecDestroy(&rA );
	Stg_VecDestroy(&rG );
	
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesMVBlockDefaultBuildScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, PetscTruth sym )
{
	Mat K,G,D,C;
	Vec rG;
	PetscScalar rg2, rg, ra;  
	PetscInt N;
	Vec rA, rC;
	Vec L1,L2, R1,R2;
	Mat S;
	
	VecNestGetSubVec( BA->Lz, 0, &L1 );
	VecNestGetSubVec( BA->Lz, 1, &L2 );
	
	VecNestGetSubVec( BA->Rz, 0, &R1 );
	VecNestGetSubVec( BA->Rz, 1, &R2 );
	
	rA = L1;
	rC = L2;
	
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	MatNestGetSubMat( A, 1,1, &C );
	
	VecDuplicate( rA, &rG );
	
	
	/* Get magnitude of K */  
	//px_MatGetAbsRowSum( K, rA );
        //MatGetRowMax( K, rA, PETSC_NULL );
	MatGetDiagonal( K, rA);

	VecSqrt( rA );  
	VecReciprocal( rA );
	
	/* VecDot( rA,rA, &ra ); */
	/* VecGetSize( rA, &N ); */
	/* ra = PetscSqrtScalar( ra/N ); */
	
	
	/* Get magnitude of G */
	//px_MatGetAbsRowSum( G, rG );
	//MatGetRowMax( G, rG, PETSC_NULL );

	Mat A21_cpy;
	Mat Shat;
	Vec diag; /* same as rA*rA */
	
	
	MatGetVecs( K, &diag, PETSC_NULL );
	MatGetDiagonal( K, diag );
	VecReciprocal( diag );

	if( sym )
#if( PETSC_VERSION_MAJOR <= 2 )
	    MatTranspose( G, &A21_cpy );
#else
	    MatTranspose( G, MAT_INITIAL_MATRIX, &A21_cpy );
#endif
	else {
	    //MatNestGetSubMat( A, 1, 0, &A21 );
	    MatDuplicate( D, MAT_COPY_VALUES, &A21_cpy );
	}
	
	MatDiagonalScale( A21_cpy, PETSC_NULL, diag );
	MatMatMult( A21_cpy, G, MAT_INITIAL_MATRIX, 1.2, &Shat );  /* A21 diag(K)^{-1} A12 */
	if( C != PETSC_NULL ){
	    MatAXPY( Shat, -1.0, C, DIFFERENT_NONZERO_PATTERN ); /* S <- -C + A21 diag(K)^{-1} A12 */
	}

	Stg_MatDestroy(&A21_cpy );
		
	/* should now have Shat = Gt.rA.rA.G -C */
	//MatGetRowMax( Shat, rC, PETSC_NULL );
	MatGetDiagonal( Shat, rC);
	VecSqrt( rC );  
	VecReciprocal( rC );
	
	/* going to try just setting it to identity */
	/* so no scaling for pressure -- no better */
	//VecSet( rC, 1.0 );
	
	Stg_MatDestroy(&Shat );


	VecCopy( L1, R1 );
	VecCopy( L2, R2 );

	PetscFunctionReturn(0);
}
