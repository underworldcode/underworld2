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

#include "common-driver-utils.h"

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
#include "Solvers/SLE/src/SLE.h" /* to give the AugLagStokes_SLE type */
#include "Solvers/KSPSolvers/src/KSPSolvers.h" /* for __KSP_COMMON */
#include "BSSCR.h"
#include "writeMatVec.h"

#if( (PETSC_VERSION_MAJOR==2) && (PETSC_VERSION_MINOR==3) && (PETSC_VERSION_SUBMINOR==0) )
#define FILE_OPTION PETSC_FILE_RDONLY
#endif
#if( (PETSC_VERSION_MAJOR==2) && (PETSC_VERSION_MINOR==3) && (PETSC_VERSION_SUBMINOR>=2) )
#define FILE_OPTION FILE_MODE_READ
#endif
#if( PETSC_VERSION_MAJOR==3 )
#define FILE_OPTION FILE_MODE_READ
#endif


PetscErrorCode BSSCR_FormSchurApproximation1( Mat A11, Mat A12, Mat A21, Mat A22, Mat *_Shat, PetscTruth sym );
PetscErrorCode BSSCR_FormSchurApproximationDiag( Mat A11, Mat A12, Mat A21, Mat A22, Mat *_Shat, PetscTruth sym );

PetscErrorCode BSSCR_BSSCR_StokesReadPCSchurMat_binary( MPI_Comm comm, Mat *S )
{
	PetscViewer mat_view_file;
	char	op_name[PETSC_MAX_PATH_LEN];
	PetscTruth flg;

	PetscOptionsGetString( PETSC_NULL,"-stokes_Smat",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	*S = PETSC_NULL;
	if (flg) {
		if (!S)	Stg_SETERRQ(1,"Memory space for Smat is NULL");

		PetscViewerBinaryOpen( comm, op_name,  FILE_OPTION, &mat_view_file );
		Stg_MatLoad( mat_view_file, MATAIJ, S );
		Stg_PetscViewerDestroy(&mat_view_file );
	}

	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_BSSCR_StokesReadPCSchurMat_ascii( MPI_Comm comm, Mat *S )
{
	char	op_name[PETSC_MAX_PATH_LEN];
	PetscTruth flg;

	PetscOptionsGetString( PETSC_NULL,"-stokes_Smat",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	*S = PETSC_NULL;
	if (flg) {
		if (!S)	Stg_SETERRQ(1,"Memory space for Smat is NULL");
		Stg_SETERRQ(1,"Currently Disabled");
		//MatAIJLoad_MatrixMarket( comm, op_name, S );
	}

	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_StokesReadPCSchurMat( MPI_Comm comm, Mat *S )
{
	char	op_name[PETSC_MAX_PATH_LEN];
	PetscTruth flg;


	PetscOptionsGetString( PETSC_NULL,"-stokes_ascii",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg==PETSC_TRUE) {
		BSSCR_BSSCR_StokesReadPCSchurMat_ascii( comm, S );
	}
	else {
		BSSCR_BSSCR_StokesReadPCSchurMat_binary( comm, S );
	}

	PetscFunctionReturn(0);
}




PetscErrorCode BSSCR_StokesCreatePCSchur( Mat K, Mat G, PC pc_S )
{
	char pc_type[PETSC_MAX_PATH_LEN];
	PetscTruth flg;


	PetscOptionsGetString( PETSC_NULL, "-Q22_pc_type", pc_type, PETSC_MAX_PATH_LEN-1, &flg );
	if( !flg ) {
		Stg_SETERRQ( PETSC_ERR_SUP, "OPTION: -Q22_pc_type must be set" );
	}


	/* 1. define S pc to be "none" */

	if( strcmp(pc_type,"none")==0 ) { /* none */
		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"none\" \n" );
		PCSetType( pc_S, "none" );
	}
	else if( strcmp(pc_type,"uw")==0 ) { /* diag */
		Mat S, Amat;
		MatStructure mstruct;
		MPI_Comm comm;

		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"uw\" \n" );

		PetscObjectGetComm( (PetscObject)pc_S, &comm );
		S = PETSC_NULL;
		BSSCR_StokesReadPCSchurMat( comm, &S );
		if (!S) {	Stg_SETERRQ(1,"Must indicate location of file for SchurPC matrix with the stokes_Smat option");	}

		Stg_PCGetOperators( pc_S, &Amat, &Amat, &mstruct );
		Stg_PCSetOperators( pc_S, Amat, S, SAME_NONZERO_PATTERN );

		MatView( S, PETSC_VIEWER_STDOUT_WORLD );
	}
	else { /* not valid option */
		Stg_SETERRQ( PETSC_ERR_SUP, "OPTION: -seg_schur_pc_type is not valid" );
	}

	PetscFunctionReturn(0);
}



PetscErrorCode BSSCR_BSSCR_StokesCreatePCSchur2(
   Mat K, Mat G, Mat D, Mat C, Mat Smat, PC pc_S,
   PetscTruth sym, KSP_BSSCR * bsscrp )
{
	char pc_type[PETSC_MAX_PATH_LEN];
	PetscTruth flg;


	PetscOptionsGetString( PETSC_NULL, "-Q22_pc_type", pc_type, PETSC_MAX_PATH_LEN-1, &flg );
	if( !flg ) {
	    strcpy(pc_type, "uw");
	    //Stg_SETERRQ( PETSC_ERR_SUP, "OPTION: -Q22_pc_type must be set" );
	}


	/* 1. define S pc to be "none" */

	if( strcmp(pc_type,"none")==0 ) { /* none */
		/* Mat Amat,Pmat; */
		/* MatStructure mstruct; */

		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"none\" \n" );
		/* PCGetOperators( pc_S, &Amat, &Pmat, &mstruct ); */
		/* PCSetOperators( pc_S, Amat, Amat, SAME_NONZERO_PATTERN ); */
		PCSetType( pc_S, "none" );
	}
	else if( strcmp(pc_type,"uw")==0 ) { /* diag */
	    Mat Amat,Pmat;
		MatStructure mstruct;

		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"uw\" \n" );

		if (!Smat) {	Stg_SETERRQ(1,"Smat cannot be NULL if -Q22_pc_type = uw");	}

		Stg_PCGetOperators( pc_S, &Amat, &Pmat, &mstruct );
		Stg_PCSetOperators( pc_S, Amat, Smat, SAME_NONZERO_PATTERN );
	}
	else if( strcmp(pc_type,"uwscale")==0 ) { /* diag */
	    Mat Amat, Shat, Pmat;
		MatStructure mstruct;

		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"uwscale\" \n" );

		if (!Smat) {	Stg_SETERRQ(1,"Smat cannot be NULL if -Q22_pc_type = uwscale");	}

		BSSCR_FormSchurApproximation1( K, G, D, C, &Shat, sym );

		Stg_PCGetOperators( pc_S, &Amat, &Pmat, &mstruct );
		Stg_PCSetOperators( pc_S, Amat, Shat, SAME_NONZERO_PATTERN );

		Stg_MatDestroy(&Shat);
	}
	else if( strcmp(pc_type,"gkgdiag")==0 ) { /* diag */
	    Mat Amat, Shat, Pmat;
		MatStructure mstruct;

		PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"gkgdiag\" \n" );

		if (!Smat) {	Stg_SETERRQ(1,"Smat cannot be NULL if -Q22_pc_type = uwscale");	}

		BSSCR_FormSchurApproximationDiag( K, G, D, C, &Shat, sym );

		Stg_PCGetOperators( pc_S, &Amat, &Pmat, &mstruct );
		Stg_PCSetOperators( pc_S, Amat, Shat, SAME_NONZERO_PATTERN );

		Stg_MatDestroy(&Shat);
	}
	else if( strcmp(pc_type,"gtkg")==0 ) { /* GtKG */
	    PetscPrintf(PETSC_COMM_WORLD,"  Setting schur_pc to \"gtkg\" \n" );

	    /* Build the schur pc GtKG */
	    PCSetType( pc_S, "gtkg" );
	    //AugLagStokes_SLE * stokesSLE = (AugLagStokes_SLE*)bsscrp->st_sle;
        StokesBlockKSPInterface* Solver = bsscrp->solver;
	    Mat M=0;
	    if (Solver->vmStiffMat){
		M = Solver->vmStiffMat->matrix;
	    }
	    BSSCR_PCGtKGSet_Operators( pc_S, K, G, M );
	    //BSSCR_PCGtKGAttachNullSpace( pc_S );
	}
	else { /* not valid option */
		Stg_SETERRQ( PETSC_ERR_SUP, "OPTION: -Q22_pc_type is not valid" );
	}

	PetscFunctionReturn(0);
}



/*

Recomended usage:

Stg_KSPSetOperators( ksp_S, S, Shat, SAME_NONZERO_PATTERN );

KSPGetPC( ksp_S, &pc_S );
PCSetType( pc_S, "jacobi" );


NOTE: In pracise "jacobi" works better than keep the entire matrix and
factoring it using something like "cholesky"

*/
PetscErrorCode BSSCR_FormSchurApproximation1( Mat A11, Mat A12, Mat A21, Mat A22, Mat *_Shat, PetscTruth sym )
{
   Mat Shat, A21_cpy;
   Vec diag;

   MatGetVecs( A11, &diag, PETSC_NULL );
   MatGetDiagonal( A11, diag );
   VecReciprocal( diag );

/*    if( sym ) { */
/* #if( PETSC_VERSION_MAJOR <= 2 ) */
/*       MatTranspose( A12, &A21_cpy ); */
/* #else */
/*       MatTranspose( A12, MAT_INITIAL_MATRIX, &A21_cpy ); */
/* #endif */
/*       MatDiagonalScale(A21_cpy, PETSC_NULL, diag ); */
/*    } */
/*    else { */
      MatDuplicate( A21, MAT_COPY_VALUES, &A21_cpy );
      MatDiagonalScale(A21_cpy, PETSC_NULL, diag );
   /* } */

   MatMatMult( A21_cpy, A12, MAT_INITIAL_MATRIX, 1.2, &Shat );  /* A21 diag(K)^{-1} A12 */

   if( A22 != PETSC_NULL )
      MatAXPY( Shat, -1.0, A22, DIFFERENT_NONZERO_PATTERN ); /* S <- -C + A21 diag(K)^{-1} A12 */

   *(void**)_Shat = (void*)Shat;

   Stg_MatDestroy(&A21_cpy);
   Stg_VecDestroy(&diag);

   PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_FormSchurApproximationDiag( Mat A11, Mat A12, Mat A21, Mat A22, Mat *_Shat, PetscTruth sym )
{
   Mat Shat, A21_cpy;
   Vec diag;

   MatGetVecs( A11, &diag, PETSC_NULL );
   MatGetDiagonal( A11, diag );
   VecReciprocal( diag );

/*    if( sym ) { */
/* #if( PETSC_VERSION_MAJOR <= 2 ) */
/*       MatTranspose( A12, &A21_cpy ); */
/* #else */
/*       MatTranspose( A12, MAT_INITIAL_MATRIX, &A21_cpy ); */
/* #endif */
/*       MatDiagonalScale(A21_cpy, PETSC_NULL, diag ); */
/*    } */
/*    else { */
      MatDuplicate( A21, MAT_COPY_VALUES, &A21_cpy );
      MatDiagonalScale(A21_cpy, PETSC_NULL, diag );
   /* } */

   MatMatMult( A21_cpy, A12, MAT_INITIAL_MATRIX, 1.2, &Shat );  /* A21 diag(K)^{-1} A12 */

   if( A22 != PETSC_NULL )
      MatAXPY( Shat, -1.0, A22, DIFFERENT_NONZERO_PATTERN ); /* S <- -C + A21 diag(K)^{-1} A12 */


   Stg_MatDestroy(&A21_cpy);
   Stg_VecDestroy(&diag);

   MatGetVecs( Shat, &diag, PETSC_NULL );
   MatGetDiagonal( Shat, diag );

   MatZeroEntries( Shat );
   MatDiagonalSet( Shat, diag, INSERT_VALUES );

   *(void**)_Shat = (void*)Shat;

   Stg_VecDestroy(&diag);

   PetscFunctionReturn(0);
}
