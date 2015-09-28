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
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "Solvers/SLE/SLE.h" /* to give the AugLagStokes_SLE type */
#include "Solvers/KSPSolvers/KSPSolvers.h" /* for __KSP_COMMON */

#include "BSSCR.h"
#include "writeMatVec.h"

/* creates and builds the "checker-board" null-space vectors for pressure
   and sets the t and v "Vec pointers" on the bsscr struct to point to them */
#undef __FUNCT__  
#define __FUNCT__ "KSPBuildPressure_CB_Nullspace_BSSCR" 
PetscErrorCode KSPBuildPressure_CB_Nullspace_BSSCR(KSP ksp)
{
    KSP_BSSCR        *bsscr = (KSP_BSSCR *)ksp->data;
    FeEquationNumber *eq_num = bsscr->solver->st_sle->pSolnVec->feVariable->eqNum;
    FeMesh           *feMesh = bsscr->solver->st_sle->pSolnVec->feVariable->feMesh; /* is the pressure mesh */
    unsigned          ijk[3];
    Vec               t, v;
    int numLocalNodes, globalNodeNumber, i, j, eq;
    MatStokesBlockScaling BA = bsscr->BA;
    PetscErrorCode ierr;
    Mat            Amat,Pmat, G;
    MatStructure   pflag;

    PetscFunctionBegin;
    /* get G matrix from Amat matrix operator on ksp */
    ierr=Stg_PCGetOperators(ksp->pc,&Amat,&Pmat,&pflag);CHKERRQ(ierr);
    MatNestGetSubMat( Amat, 0,1, &G );/* G should always exist */
    /* now create Vecs t and v to match size of G: i.e. pressure */ /* NOTE: not using "h" vector from ksp->vec_rhs because this part of the block vector doesn't always exist */
    MatGetVecs( G, &t, PETSC_NULL );/* t and v are destroyed in KSPDestroy_BSSCR */
    MatGetVecs( G, &v, PETSC_NULL );/* t and v such that can do G*t */

    numLocalNodes = Mesh_GetLocalSize( feMesh,  MT_VERTEX); /* number of nodes on current proc not counting any shadow nodes */
    for(j=0;j<numLocalNodes;j++){
	i = globalNodeNumber = Mesh_DomainToGlobal( feMesh, MT_VERTEX, j);
	RegularMeshUtils_Element_1DTo3D(feMesh, i, ijk);
	eq = eq_num->destinationArray[j][0];/* get global equation number -- 2nd arg is always 0 because pressure has only one dof */
	if(eq != -1){
	    if( (ijk[0]+ijk[1]+ijk[2])%2 ==0 ){
		VecSetValue(t,eq,1.0,INSERT_VALUES);
	    }
	    else{
		VecSetValue(v,eq,1.0,INSERT_VALUES);	    }}}

    VecAssemblyBegin( t );
    VecAssemblyEnd( t );
    VecAssemblyBegin( v );
    VecAssemblyEnd( v );

    /* Scaling the null vectors here because it easier at the moment *//* maybe should do this in the original scaling function */
    if( BA->scaling_exists == PETSC_TRUE ){
	Vec R2;
	/* Get the scalings out the block mat data */
	VecNestGetSubVec( BA->Rz, 1, &R2 );
	VecPointwiseDivide( t, t, R2); /* x <- x * 1/R2 */
	VecPointwiseDivide( v, v, R2);
    }

    bsscr_writeVec( t, "t", "Writing t vector");
    bsscr_writeVec( v, "v", "Writing v vector");

    bsscr->t=t;
    bsscr->v=v;

    PetscFunctionReturn(0);
}
/* creates and builds the "constant" null-space vector for pressure
   and sets the t "Vec pointer" on the bsscr struct to point to it */
#undef __FUNCT__  
#define __FUNCT__ "KSPBuildPressure_Const_Nullspace_BSSCR" 
PetscErrorCode KSPBuildPressure_Const_Nullspace_BSSCR(KSP ksp)
{
    KSP_BSSCR        *bsscr = (KSP_BSSCR *)ksp->data;
    FeEquationNumber *eq_num = bsscr->solver->st_sle->pSolnVec->feVariable->eqNum;
    FeMesh           *feMesh = bsscr->solver->st_sle->pSolnVec->feVariable->feMesh; /* is the pressure mesh */
    unsigned          ijk[3];
    Vec               t;
    int numLocalNodes, globalNodeNumber, i, eq;
    MatStokesBlockScaling BA = bsscr->BA;
    PetscErrorCode ierr;
    Mat            Amat,Pmat, G;
    MatStructure   pflag;

    PetscFunctionBegin;
    /* get G matrix from Amat matrix operator on ksp */
    ierr=Stg_PCGetOperators(ksp->pc,&Amat,&Pmat,&pflag);CHKERRQ(ierr);
    MatNestGetSubMat( Amat, 0,1, &G );/* G should always exist */
    /* now create Vec t to match size of G: i.e. pressure */ /* NOTE: not using "h" vector from ksp->vec_rhs because this part of the block vector doesn't always exist */
    MatGetVecs( G, &t, PETSC_NULL );/* t is destroyed in KSPDestroy_BSSCR */

    VecSet(t, 1.0);

    VecAssemblyBegin( t );
    VecAssemblyEnd( t );

    /* Scaling the null vectors here because it easier at the moment */
    if( BA->scaling_exists == PETSC_TRUE ){
	Vec R2;
	/* Get the scalings out the block mat data */
	VecNestGetSubVec( BA->Rz, 1, &R2 );
	VecPointwiseDivide( t, t, R2); /* x <- x * 1/R2 */
    }
    bsscr_writeVec( t, "t", "Writing t vector");
    bsscr->t=t;

    PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "KSPRemovePressureNullspace_BSSCR" 
PetscErrorCode KSPRemovePressureNullspace_BSSCR(KSP ksp, Vec h_hat)
{
    KSP_BSSCR            *bsscr = (KSP_BSSCR *)ksp->data;
    MatStokesBlockScaling BA    = bsscr->BA;
    PetscErrorCode ierr;
    PetscScalar norm, a, a1, a2, hnorm, pnorm, gnorm;
    Vec t2, t,v;
    Mat            Amat,Pmat, D;
    MatStructure   pflag;
    double nstol   = bsscr->nstol; /* set in KSPCreate_BSSCR */

    PetscFunctionBegin;
    t=bsscr->t;
    v=bsscr->v;
    /* get G matrix from Amat matrix operator on ksp */
    ierr=Stg_PCGetOperators(ksp->pc,&Amat,&Pmat,&pflag);CHKERRQ(ierr);
    //MatNestGetSubMat( Amat, 0,1, &G );/* G should always exist */
    MatNestGetSubMat( Amat, 1,0, &D );/* D should always exist */
    /* now create Vec t2 to match left hand size of G: i.e. velocity */
    MatGetVecs( D, &t2, PETSC_NULL );
    MatNorm(D,NORM_INFINITY,&gnorm); /* seems like not a bad estimate of the largest eigenvalue for this matrix */
    if(t){/* assumes that v and t are initially set to PETSC_NULL (in KSPCreate_BSSCR ) */
	MatMultTranspose( D, t, t2);
	VecNorm(t2, NORM_2, &norm);
	VecNorm(t, NORM_2, &a);
	norm=norm/a;/* so we are using unit null vector */
	if(norm < nstol*gnorm){/* then t in NS of G */
	    VecDot(t,h_hat, &a1);
	    VecDot(t,t, &a2);
	    a=-a1/a2;
	    VecAXPY(h_hat, a, t);
	    VecDot(t,h_hat, &a1);
	    PetscPrintf( PETSC_COMM_WORLD, "\n\t* Found t NS norm= %g : Dot = %g\n\n", norm, a1);
	}
    }
    if(v){
	MatMultTranspose( D, v, t2);
	VecNorm(t2, NORM_2, &norm);
	VecNorm(v, NORM_2, &a);
	norm=norm/a;/* so we are using unit null vector */
	if(norm < nstol*gnorm){/* then v in NS of G */
	    VecDot(v,h_hat, &a1);
	    VecDot(v,v, &a2);
	    a=-a1/a2;
	    VecAXPY(h_hat, a, v);
	    VecDot(v,h_hat, &a1);
	    PetscPrintf( PETSC_COMM_WORLD, "\n\t* Found v NS norm= %g : Dot = %g\n\n", norm, a1);
	}
    }
    bsscr_writeVec( h_hat, "h", "Writing h_hat Vector in Solver");
    Stg_VecDestroy(&t2);

    PetscFunctionReturn(0);
}

