/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#if 0
#include <petsc.h>
#include <petscvec.h>
#include <petscmat.h>
#include <petscksp.h>

#include "common-driver-utils.h"
#include "stokes_block_scaling.h"
#include "stokes_mvblock_scaling.h"
typedef struct {
      int num_nsp_vecs;
      Vec *nsp_vecs;
} nsp_data_t;

PetscErrorCode BSSCR_MatContainsConstNullSpace(Mat A, Vec nsp_vec, PetscTruth *has) {
    PetscScalar sum;
    PetscReal nrm;
    Vec l, r;

    MatGetVecs(A, &r, &l); /* l = A r */

    if(nsp_vec == PETSC_NULL) {
	PetscInt N;

	VecGetSize(r, &N);
	sum  = 1.0/(PetscScalar)N;
	VecSet(r, sum);
    }
    else {
	VecSum(nsp_vec, &sum);
	sum = 1.0/sum;
	VecScale(nsp_vec, sum);
	VecCopy(nsp_vec, r);
    }

    MatMult(A, r, l); /* {l} = [A] {r} */

    VecNorm(l, NORM_2, &nrm);
    if(nrm < 1.e-7)
	*has = PETSC_TRUE;
    else
	*has = PETSC_FALSE;

    Stg_VecDestroy(&l);
    Stg_VecDestroy(&r);

    PetscFunctionReturn(0);
}



PetscErrorCode BSSCR_MatGtKinvG_ContainsConstantNullSpace(
		Mat K, Mat G, Mat M,Vec t1, Vec ustar,  Vec r, Vec l,
		KSP ksp, PetscTruth *has_cnst_nullsp )
{
	PetscInt N;
	PetscScalar sum;
	PetscReal nrm;
	
	
	VecGetSize(l,&N);
	sum  = 1.0/N;
	VecSet(l,sum);
	
	/* [S] {l} = {r} */
	MatMult( G,l, t1 );
	KSPSolve( ksp, t1, ustar );
	MatMultTranspose( G, ustar, r );
	if ( M ) {
		VecScale( r, -1.0 );
		MatMultAdd( M,l, r, r );
		VecScale( r, -1.0 );
	}
	VecNorm(r,NORM_2,&nrm);
	if (nrm < 1.e-7) {
//		PetscPrintf( PETSC_COMM_WORLD, "Constant null space detected, " );
		*has_cnst_nullsp = PETSC_TRUE;
	}
	else {
//		PetscPrintf( PETSC_COMM_WORLD, "Constant null space not present, " );
		*has_cnst_nullsp = PETSC_FALSE;
	}
//	PetscPrintf( PETSC_COMM_WORLD, "|| [S]{1} || = %G€n", nrm );
	
	
	PetscFunctionReturn(0);
}



PetscErrorCode BSSCR_VecRemoveConstNullspace(Vec v, Vec nsp_vec) {
    PetscScalar sum;
    PetscInt N;

    VecGetSize(v, &N);
    if(N > 0) {
	if(nsp_vec == PETSC_NULL) {
	    VecSum(v, &sum);
	    sum  = sum/(-1.0*(PetscScalar)N);
	    VecShift(v, sum);
	}
	else {
	    Vec r;
	    PetscScalar *nsp_array, *r_array;
	    int n;
	    int ii;

	    VecDot(v, nsp_vec, &sum);

	    VecDuplicate(nsp_vec, &r);
	    VecGetArray(nsp_vec, &nsp_array);
	    VecGetArray(r, &r_array);
	    VecGetLocalSize(nsp_vec, &n);
	    for(ii = 0; ii < n; ii++) {
		if(nsp_array[ii] > 1e-10)
		    r_array[ii] = -sum;
	    }
	    VecRestoreArray(nsp_vec, &nsp_array);
	    VecRestoreArray(r, &r_array);

	    VecAXPY(v, 1.0, r);

	    Stg_VecDestroy(&r);
	}
    }

    PetscFunctionReturn(0);
}

/**************************************************************
 This should be currently unused.
 But it's still here in case we need it.
 **************************************************************/
PetscErrorCode BSSCR_CheckNullspace(KSP ksp_S, Mat S, Vec h_hat, MatStokesBlockScaling BA, Vec * _nsp_vec) {
    /*
     * Check if S contains nulls space. If we have a scaling, we need to scale the
     * usual constant nullspace vector before checking.
     */
    char name[PETSC_MAX_PATH_LEN];
    PetscTruth found,flg2,removens;
    Vec nsp_vec;
    PetscTruth has_cnst_nullspace;
    
    found = PETSC_FALSE;
    removens = PETSC_FALSE;
    PetscOptionsGetTruth(PETSC_NULL, "-remove_pressure_null_space", &removens, &found );
    //PetscOptionsGetString( PETSC_NULL, "-no_remove_nullspace", name, PETSC_MAX_PATH_LEN-1, &flg );
    if( removens ) {
	PetscPrintf( PETSC_COMM_WORLD, "\t* Checking for NullSpace \n");

	/* Create a vector to scale. */
	MatGetVecs(S, &nsp_vec, PETSC_NULL);
	VecSet(nsp_vec, 1.0);
	/* Scale newly created vector. */
	//PetscOptionsGetString(PETSC_NULL, "-no_scale", name, PETSC_MAX_PATH_LEN - 1, &flg);
	//PetscOptionsGetString(PETSC_NULL, "-mv_scale", name, PETSC_MAX_PATH_LEN - 1, &flg2);
	//if(!flg || flg2) {
	if( BA ) {
	    Vec R2;
	    VecNestGetSubVec(BA->Rz, 1, &R2);
	    VecPointwiseDivide(nsp_vec, nsp_vec, R2);
	}

	BSSCR_MatContainsConstNullSpace(S, nsp_vec, &has_cnst_nullspace);
	if (has_cnst_nullspace) {
	    MatNullSpace nsp;
	    nsp_data_t nsp_data;

	    PetscPrintf( PETSC_COMM_WORLD, "\t* Configuring Seg KSP with NullSpace \n");
	    MatNullSpaceCreate( PETSC_COMM_WORLD, PETSC_FALSE, PETSC_NULL, PETSC_NULL, &nsp );
	    nsp_data.num_nsp_vecs = 1;
	    nsp_data.nsp_vecs = &nsp_vec;
	    MatNullSpaceSetFunction(nsp, BSSCR_NSPRemoveAll, &nsp_data);
        #if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR <6) )
	    KSPSetNullSpace( ksp_S, nsp );
        #else
        MatSetNullSpace( S, nsp);
        #endif
	    MatNullSpaceDestroy( nsp );

	    /* remove from rhs */
	    BSSCR_VecRemoveConstNullspace( h_hat, nsp_vec );
	}
    }
    *_nsp_vec=nsp_vec;
    PetscFunctionReturn(0);
}
#endif

