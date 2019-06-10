/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __DRIVERS_common_utils_h__
#define __DRIVERS_common_utils_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>
#include <petscpc.h>
#include <petscksp.h>
#include <petscsnes.h>

#include <petscversion.h>
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include "petsc/private/vecimpl.h"
     #include "petsc/private/matimpl.h"
     #include "petsc/private/pcimpl.h"
     #include "petsc/private/kspimpl.h"
     #include "petsc/private/snesimpl.h"
  #else
     #include "petsc-private/vecimpl.h"
     #include "petsc-private/matimpl.h"
     #include "petsc-private/pcimpl.h"
     #include "petsc-private/kspimpl.h"
     #include "petsc-private/snesimpl.h"
  #endif
#else
  #include "private/vecimpl.h"
  #include "private/matimpl.h"
  #include "private/pcimpl.h"
  #include "private/kspimpl.h"
  #include "private/snesimpl.h"
#endif

#include "../../../../StgDomain/Utils/src/PETScCompatibility.h"

#include "pc_GtKG.h"
#include "pc_ScaledGtKG.h"
#include "stokes_block_scaling.h"
#include "stokes_mvblock_scaling.h"
#include "ksp_scale.h"

/* BSSCR_create_execute_script.c */
int BSSCR_create_execute_script( void );


/* create_petsc_header.c */
PetscErrorCode BSSCR_GeneratePetscHeader_for_file( FILE *fd, MPI_Comm comm );
PetscErrorCode BSSCR_GeneratePetscHeader_for_viewer( PetscViewer viewer );


/* MatIdentityOperator.c */
//PetscErrorCode MatCreateIdentityOperator( Mat A, Mat *_I );


/* read_matrices.c */
//PetscErrorCode StokesReadOperators( MPI_Comm comm, Mat *K, Mat *G, Mat *D, Mat *C, Vec *f, Vec *h );


/* stokes_residual.c */
double BSSCR_StokesMomentumResidual( Mat K, Mat G, Vec F, Vec u, Vec p );
double BSSCR_StokesContinuityResidual( Mat G, Mat C, Vec H, Vec u, Vec p );


/* preconditioner.c */
PetscErrorCode BSSCR_StokesReadPCSchurMat( MPI_Comm comm, Mat *S );
PetscErrorCode BSSCR_StokesCreatePCSchur( Mat K, Mat G, PC pc_S );	

/* nullspace */
/*
PetscErrorCode BSSCR_MatGtKinvG_ContainsConstantNullSpace(
		Mat K, Mat G, Mat M,Vec t1, Vec ustar,  Vec r, Vec l,
		KSP ksp, PetscTruth *has_cnst_nullsp );
PetscErrorCode BSSCR_VecRemoveConstNullspace(Vec v, Vec nsp_vec);
*/
/* PetscErrorCode BSSCR_MatContainsConstantNullSpace( Mat A, PetscTruth *has_cnst_nullsp ); */
//PetscErrorCode BSSCR_MatContainsConstNullSpace(Mat A, Vec nsp_vec, PetscTruth *has);


/* timed residuals */
//PetscErrorCode KSPLogDestroyMonitor(KSP ksp);
PetscErrorCode BSSCR_KSPLogSetMonitor(KSP ksp,PetscInt len,PetscInt *monitor_index);
PetscErrorCode BSSCR_KSPLogGetTimeHistory(KSP ksp,PetscInt monitor_index,PetscInt *na,PetscLogDouble **log);
PetscErrorCode BSSCR_KSPLogGetResidualHistory(KSP ksp,PetscInt monitor_index,PetscInt *na,PetscReal **rlog);
PetscErrorCode BSSCR_KSPLogGetResidualTimeHistory(KSP ksp,PetscInt monitor_index,PetscInt *na,PetscReal **rlog,PetscLogDouble **tlog);
PetscErrorCode BSSCR_KSPLogSolve(PetscViewer v,PetscInt monitor_index,KSP ksp);
PetscErrorCode BSSCR_BSSCR_KSPLogSolveSummary(PetscViewer v,PetscInt monitor_index,KSP ksp);


/* operator summary */
PetscErrorCode BSSCR_MatInfoLog(PetscViewer v,Mat A,const char name[]);
PetscErrorCode BSSCR_VecInfoLog(PetscViewer v,Vec x,const char name[]);
PetscErrorCode BSSCR_StokesCreateOperatorSummary( Mat K, Mat G, Mat C, Vec f, Vec h, const char filename[] );


/* standard output */
PetscErrorCode BSSCR_solver_output( KSP ksp, PetscInt monitor_index, const char res_file[], const char solver_conf[] );
PetscErrorCode BSSCR_KSPLogConvergenceRate(PetscViewer v,PetscInt monitor_index, KSP ksp);

/* list available operations */
PetscErrorCode BSSCR_MatListOperations( Mat A, PetscViewer v );
PetscErrorCode BSSCR_VecListOperations( Vec x, PetscViewer v );
PetscErrorCode BSSCR_KSPListOperations( KSP ksp, PetscViewer v );
PetscErrorCode BSSCR_PCListOperations( PC pc, PetscViewer v );
PetscErrorCode BSSCR_SNESListOperations( SNES snes, PetscViewer v );


/* Register any petsc objects defined in drivers */
PetscErrorCode BSSCR_PetscExtStokesSolversInitialize( void );
PetscErrorCode BSSCR_PetscExtStokesSolversFinalize( void );

/* stokes block */
/*
PetscErrorCode BSSCR_CreateStokesBlockOperators( MPI_Comm comm, 
		Mat K, Mat G, Mat D, Mat C,
		Vec u, Vec p, Vec f, Vec h,
		Mat *A, Vec *x, Vec *b );
*/
/* stokes output */
PetscErrorCode BSSCR_stokes_output( PetscViewer v, Mat stokes_A, Vec stokes_b, Vec stokes_x, KSP ksp, PetscInt monitor_index );

//PetscErrorCode BSSCR_NSPRemoveAll(Vec v, void *_data);
//PetscErrorCode BSSCR_CheckNullspace(KSP ksp_S, Mat S, Vec h_hat, MatStokesBlockScaling BA, Vec * _nsp_vec);
#endif
