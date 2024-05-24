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
#include <petscksp.h>

#include "common-driver-utils.h"


/*

Stokes output:
  ---------------------------------
  Operator summary:
    K
    G
    f,
    h
    u
    p
  ---------------------------------
  Solution summary:
    max_u
    min_u
    average_u
    

    |r_1|
    |r_2|
---------------------------------
  Solver summary:
    name


---------------------------------
  Petsc build summary:

*/
PetscErrorCode BSSCR_stokes_output( PetscViewer v, Mat stokes_A, Vec stokes_b, Vec stokes_x, KSP ksp, PetscInt monitor_index )
{
	Mat K,G,D,C;
	Vec f,h, u,p;
	
	
	K = G = D = C = PETSC_NULL;
	f = h = PETSC_NULL;
	u = p = PETSC_NULL;
	
	MatNestGetSubMat( stokes_A, 0,0, &K );
	MatNestGetSubMat( stokes_A, 0,1, &G );
	MatNestGetSubMat( stokes_A, 1,0, &D );
	MatNestGetSubMat( stokes_A, 1,1, &C );
	
	VecNestGetSubVec( stokes_x, 0, &u );
	VecNestGetSubVec( stokes_x, 1, &p );
	
	VecNestGetSubVec( stokes_b, 0, &f );
	VecNestGetSubVec( stokes_b, 1, &h );
	
	
	PetscViewerASCIIPrintf( v, "Stokes Output:\n");
	PetscViewerASCIIPushTab( v );
	/*--------------------------------------------------------------------------------------------*/
	PetscViewerASCIIPrintf( v, "--------------------------------------------------\n");
	PetscViewerASCIIPrintf( v, "Operator summary:\n");
	PetscViewerASCIIPushTab( v );
	
	if (K) {	BSSCR_MatInfoLog(v,K, "stokes_A11");		PetscViewerASCIIPrintf( v, "\n");	}
	if (G) {	BSSCR_MatInfoLog(v,G, "stokes_A12");		PetscViewerASCIIPrintf( v, "\n");	}
	if (D){		BSSCR_MatInfoLog(v,D, "stokes_A21"); 		PetscViewerASCIIPrintf( v, "\n");	}
	if (C) {	BSSCR_MatInfoLog(v,C, "stokes_A22"); 		PetscViewerASCIIPrintf( v, "\n");	}
	
	if (f) {	BSSCR_VecInfoLog(v,f,"stokes_b1");		PetscViewerASCIIPrintf( v, "\n");	}
	if (h) {	BSSCR_VecInfoLog(v,h,"stokes_b2");		PetscViewerASCIIPrintf( v, "\n");	}
	
	PetscViewerASCIIPopTab( v );
	
	
	/*--------------------------------------------------------------------------------------------*/
	PetscViewerASCIIPrintf( v, "--------------------------------------------------\n");
	PetscViewerASCIIPrintf( v, "Solution summary:\n");
	PetscViewerASCIIPushTab( v );
	
	if (u) {	BSSCR_VecInfoLog(v,u,"x1");		PetscViewerASCIIPrintf( v, "\n");	}
	if (p) {	BSSCR_VecInfoLog(v,p,"x2");		PetscViewerASCIIPrintf( v, "\n");	}
	
	{
		PetscScalar s,sum;
		PetscReal r,max,min;
		PetscInt N, loc;
		double r1,r2;
		Vec K_d;
		PetscInt loc_max, loc_min;
		
		
		VecGetSize( u, &N );
		VecMax( u, &loc, &r );		PetscViewerASCIIPrintf( v, "u_max: %1.12e [%d] \n", r, loc );
		VecMin( u, &loc, &r );		PetscViewerASCIIPrintf( v, "u_min: %1.12e [%d] \n", r, loc );
		
		VecDot( u,u, &s );			PetscViewerASCIIPrintf( v, "u_rms: %1.12e \n", sqrt( PetscRealPart(s) )/N );
		
		VecDuplicate( u, &K_d );
		MatGetDiagonal( K, K_d );
		VecMax( K_d, &loc_max, &max );
		VecMin( K_d, &loc_min, &min );
		PetscViewerASCIIPrintf( v,"Algebraic contrast: max(K_d)=%.3e [%d] , min(K_d)=%.3e [%d] , max(K_d)/min(K_d) = %.8e \n", max,loc_max, min,loc_min, max/min );
		
		MatGetRowMax( K, K_d, PETSC_NULL );
		VecMax( K_d, &loc_max, &max );
		MatGetRowMin( K, K_d, PETSC_NULL );
		VecAbs( K_d );
		VecMin( K_d, &loc_min, &min );
		PetscViewerASCIIPrintf( v,"Algebraic contrast:   max(K)=%.3e [%d] , |min(K)|=%.3e [%d]  ,   max(K)/|min(K)| = %.8e \n", max,loc_max, min,loc_min, max/min );
		Stg_VecDestroy(&K_d );
		
		PetscViewerASCIIPrintf( v, "\n");
		
		VecGetSize( p, &N );
		VecMax( p, &loc, &r );		PetscViewerASCIIPrintf( v, "p_max:  %1.12e [%d] \n", r, loc );
		VecMin( p, &loc, &r );		PetscViewerASCIIPrintf( v, "p_min:  %1.12e [%d] \n", r, loc );
		
		VecDot( p,p, &s );			PetscViewerASCIIPrintf( v, "p_rms:  %1.12e \n", sqrt( PetscRealPart(s) )/N );
		
		VecSum( p, &sum );			PetscViewerASCIIPrintf( v, "sum(p): %1.12e \n", sum );
		
		PetscViewerASCIIPrintf( v, "\n");
		
		r1 = BSSCR_StokesMomentumResidual( K,G,f, u,p );
		PetscViewerASCIIPrintf( v, "|r1| = %1.12e <momentum> \n", r1 ); 
		r2 = BSSCR_StokesContinuityResidual( G,C,h, u,p );
		PetscViewerASCIIPrintf( v, "|r2| = %1.12e <continuity> \n", r2 ); 
		PetscViewerASCIIPrintf( v, "\n");
	}
	PetscViewerASCIIPopTab( v );
	
	
	/*--------------------------------------------------------------------------------------------*/
	if (ksp) {
		PetscViewerASCIIPrintf( v, "--------------------------------------------------\n");
		PetscViewerASCIIPrintf( v, "Solver summary:\n");
		PetscViewerASCIIPushTab( v );
		
		BSSCR_KSPLogSolve( v, monitor_index, ksp );
		BSSCR_BSSCR_KSPLogSolveSummary( v, monitor_index, ksp );
		PetscViewerASCIIPrintf( v, "\n");
		PetscViewerASCIIPopTab( v );
	}
	
	/*--------------------------------------------------------------------------------------------*/
	PetscViewerASCIIPrintf( v, "--------------------------------------------------\n");
	PetscViewerASCIIPrintf( v, "Petsc build summary:\n");
	PetscViewerASCIIPushTab( v );
	
	BSSCR_GeneratePetscHeader_for_viewer( v );
	PetscViewerASCIIPrintf( v, "\n");
	
	PetscViewerASCIIPopTab( v );
	/*--------------------------------------------------------------------------------------------*/
	
	
	PetscViewerASCIIPopTab(v);
	
	PetscFunctionReturn(0);
}

