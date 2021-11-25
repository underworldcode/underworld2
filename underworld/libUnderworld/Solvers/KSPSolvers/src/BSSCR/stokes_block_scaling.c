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

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>

#include "common-driver-utils.h"
#include "stokes_block_scaling.h"

/* private prototypes */
PetscErrorCode BSSCR_MatStokesBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym );



/* A x = b -> A'x' = b' */
/*
Note this routine actually modifies the matrix and rhs b.
*/

// updated
PetscErrorCode BSSCR_MatStokesBlock_ApplyScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
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
PetscErrorCode BSSCR_MatStokesBlock_ApplyScaling2( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, Mat M, PetscTruth sym )
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
	if( M != PETSC_NULL ) {		MatDiagonalScale( M, L2,R2 );		}
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesBlockScalingCreate( MatStokesBlockScaling *_BA )
{
	MatStokesBlockScaling BA;
	
	PetscMalloc( sizeof(struct _p_MatStokesBlockScaling), &BA );
	
	BA->Lz = PETSC_NULL;
	BA->Rz = PETSC_NULL;
	
	BA->scaling_exists              = PETSC_FALSE;
	BA->scalings_have_been_inverted = PETSC_FALSE;
	BA->system_has_been_scaled      = PETSC_FALSE;
	
	*_BA = BA;
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesBlockScalingDestroy( MatStokesBlockScaling BA )
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
PetscErrorCode BSSCR_MatBlock_ConstructScaling( MatStokesBlockScaling BA, Mat A, Vec b, Vec x )
{
    if( BA->scaling_exists == PETSC_FALSE ) {
		
	VecDuplicate( x, &BA->Lz ); 
	VecDuplicate( x, &BA->Rz );
		
	BA->scaling_exists = PETSC_TRUE;	
	BSSCR_MatStokesBlockDefaultBuildScaling( BA,A );
	BA->scalings_have_been_inverted = PETSC_FALSE;
    }
	
    PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_mat_block_invert_scalings( MatStokesBlockScaling BA )
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
PetscErrorCode BSSCR_MatStokesBlockScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	if( BA->scaling_exists == PETSC_FALSE ) {
		BSSCR_MatBlock_ConstructScaling( BA,A,b,x );
	}
	
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {
		BSSCR_mat_block_invert_scalings(BA); /* to undo inversion */
	}
	
	BSSCR_MatStokesBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_TRUE;
	
	/*
	PetscPrintf( PETSC_COMM_WORLD, "Post Scaling \n");
	MatBlock_ReportOperatorScales(A);
	*/
	
	PetscFunctionReturn(0);
}
PetscErrorCode BSSCR_MatStokesBlockScaleSystem2( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, Mat M, PetscTruth sym )
{
	if( BA->scaling_exists == PETSC_FALSE ) {
		BSSCR_MatBlock_ConstructScaling( BA,A,b,x );
	}
	
	if( BA->scalings_have_been_inverted == PETSC_TRUE ) {
		BSSCR_mat_block_invert_scalings(BA); /* to undo inversion */
	}
	
	BSSCR_MatStokesBlock_ApplyScaling2(BA,A,b,x,S,M,sym);
	BA->system_has_been_scaled = PETSC_TRUE;
	
	/*
	PetscPrintf( PETSC_COMM_WORLD, "Post Scaling \n");
	MatBlock_ReportOperatorScales(A);
	*/
	
	PetscFunctionReturn(0);
}

// updated
PetscErrorCode BSSCR_MatStokesBlockUnScaleSystem( MatStokesBlockScaling BA, Mat A, Vec b, Vec x, Mat S, PetscTruth sym )
{
	
	if( BA->system_has_been_scaled == PETSC_FALSE ) {
		printf("Warning: MatBlock has not been scaled !! \n");
		PetscFunctionReturn(0);
	}
	
	if( BA->scalings_have_been_inverted == PETSC_FALSE ) {
		BSSCR_mat_block_invert_scalings(BA);
	}
	
	BSSCR_MatStokesBlock_ApplyScaling(BA,A,b,x,S,sym);
	BA->system_has_been_scaled = PETSC_FALSE;
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_MatStokesBlockReportOperatorScales( Mat A, PetscTruth sym )
{
	Vec rA, rG;
	PetscInt loc;
	PetscReal min, max;
	Mat K,G,D,C;
	
	MatNestGetSubMat( A, 0,0, &K );
	MatNestGetSubMat( A, 0,1, &G );
	MatNestGetSubMat( A, 1,0, &D );
	MatNestGetSubMat( A, 1,1, &C );
	
	
	MatGetVecs( K, PETSC_NULL, &rA );
	VecDuplicate( rA, &rG );
	
	/* Report the row max and mins */
	if (K!=PETSC_NULL) {
                PetscReal KNorm;
		MatGetRowMax( K, rA, PETSC_NULL );
		VecMax( rA, &loc, &max );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_max(K) = %g \n", max );
		
		MatGetRowMinAbs( K, rA, PETSC_NULL );
		VecMin( rA, &loc, &min );
		PetscPrintf( PETSC_COMM_WORLD, "Sup_min(K) = %g \n", min );

		MatNorm(K,NORM_1,&KNorm);
		PetscPrintf( PETSC_COMM_WORLD, "norm_1(K)  = %g \n\n",KNorm);
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
PetscErrorCode BSSCR_MatStokesBlockDefaultBuildScaling( MatStokesBlockScaling BA, Mat A )
{
	Mat K,G,D,C;
	Vec rG;
	PetscScalar rg2, rg, ra;  
	PetscInt N;
	Vec rA, rC;
	Vec L1,L2, R1,R2;
	
	
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
	MatGetRowMax( K, rA, PETSC_NULL );
	
	VecSqrt( rA );  
	VecReciprocal( rA );
	
	VecDot( rA,rA, &ra );
	VecGetSize( rA, &N );
	ra = PetscSqrtScalar( ra/N );
	
	
	/* Get magnitude of G */
	//px_MatGetAbsRowSum( G, rG );
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

