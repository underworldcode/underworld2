/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <petsc.h>
#include <petscvec.h>
#include <petscmat.h>

#include "common-driver-utils.h"


// Calculate {r} = {F} - [K]{u} - [G]{p}
// Calculate || {r} ||_2
double BSSCR_StokesMomentumResidual( Mat K, Mat G, Vec F, Vec u, Vec p )
{
	Vec                     uStar;
	//      PetscReal       negOne = -1.0;
	PetscReal               f_norm;
	PetscReal               r1_norm;
	
	
	VecNorm( F, NORM_2, &f_norm );          // u_norm = || {uStar} ||_2
	
	VecDuplicate( u, &uStar );
	MatMult( K, u, uStar );                                 // {uStar} = [K]{u}
	MatMultAdd( G, p, uStar, uStar );               // {uStar} = {uStar} + [G]{p}
	VecAYPX( uStar, -1.0, F );                      // {uStar} = {F} - {uStar}
	VecNorm( uStar, NORM_2, &r1_norm );             // r_norm = || {uStar} ||_2
	Stg_VecDestroy(&uStar );
	/*
	printf("%s \n", __func__ );
	printf("\t||f - Ku - Gp||       = %g \n", r1_norm );
	printf("\t||f - Ku - Gp||/||f|| = %g \n", r1_norm/f_norm );
	*/
	
	//      return ( (double)(r1_norm/f_norm) );
	return ( (double)(r1_norm) );
}

// Calculate {r} = {H} - [G]^T{u} - [C]{p}
// Calculate || {r} ||_2
double BSSCR_StokesContinuityResidual( Mat G, Mat C, Vec H, Vec u, Vec p )
{
	Vec                     pStar;
	//      PetscReal       negOne = -1.0;
	PetscReal               r2_norm;
	PetscReal               u_norm;
	
	VecNorm( u, NORM_2, &u_norm );          // u_norm = || {uStar} ||_2
	
	VecDuplicate( H, &pStar );
	MatMultTranspose( G, u, pStar );                // {pStar} = [G]^T{u}
	if( C != PETSC_NULL ) {
		MatMultAdd( C, p, pStar, pStar );	/* {pStar} = {pStar} + [C] {p} */
	}
	VecAYPX( pStar, -1.0, H );                              // {pStar} = {H} - {pStar}
	
	VecNorm( pStar, NORM_2, &r2_norm );             // norm = || {pStar} ||_2
	Stg_VecDestroy(&pStar );
	
	/*
	printf("%s \n", __func__ );
	printf("\t||h - Du||       = %g \n", r2_norm );
	printf("\t||h - Du||/||u|| = %g \n", r2_norm/u_norm );
	*/
	
	//      return ( (double)(r2_norm/u_norm) );
	return ( (double)(r2_norm) );
}

