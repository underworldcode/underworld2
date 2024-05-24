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

writes out 

file1
  1) its, residual, time
  2) a summary of results
file2
  1) solver configuration

*/
PetscErrorCode BSSCR_solver_output( KSP ksp, PetscInt monitor_index, const char res_file[], const char solver_conf[] )
{
	PetscViewer v;
	MPI_Comm comm;
	PetscInt i,nargs;
	char **args;
	
	
	PetscObjectGetComm( (PetscObject)ksp, &comm );
	PetscViewerASCIIOpen( comm, res_file, &v );
	
	BSSCR_GeneratePetscHeader_for_viewer( v );
        PetscViewerASCIIPrintf( v, "\n");
	BSSCR_KSPLogSolve( v, monitor_index, ksp );
	BSSCR_BSSCR_KSPLogSolveSummary( v, monitor_index, ksp );
	
	Stg_PetscViewerDestroy(&v );
	
	//////////////////////////////////////////////////////////
	
	PetscObjectGetComm( (PetscObject)ksp, &comm );
	PetscViewerASCIIOpen( comm, solver_conf, &v );

		
	BSSCR_GeneratePetscHeader_for_viewer( v );
	PetscViewerASCIIPrintf( v, "\n\nSolver configuration: \n");

	PetscViewerASCIIPrintf( v, "\n1) Command line arguments supplied: \n");
	PetscViewerASCIIPushTab(v);
        PetscGetArgs( &nargs, &args );
        for( i=0; i<nargs; i++ ) {
                PetscViewerASCIIPrintf( v, "[%d]: %s \n", i, args[i] );
        }
	PetscViewerASCIIPopTab(v);

	PetscViewerASCIIPrintf( v, "\n2) Petsc KSPView(): \n");
	KSPView( ksp, v );
	
	Stg_PetscViewerDestroy(&v );
	
	
	PetscFunctionReturn(0);
}



#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPLogConvergenceRate"
PetscErrorCode BSSCR_KSPLogConvergenceRate(PetscViewer v,PetscInt monitor_index, KSP ksp)
{
	PetscInt i;
	PetscReal *rlog;
	PetscLogDouble *log;
	PetscInt nits;
	PetscReal rho;
	
	
	BSSCR_KSPLogGetResidualTimeHistory(ksp,monitor_index, &nits,&rlog,&log);
	PetscViewerASCIIPrintf( v, "#-----------------------------------------------------------------------------------------------------------------------\n");
	PetscViewerASCIIPrintf( v, "# it         |r|              |r_0|               |r|/|r_0|          -log10(r/r0)        rho     |rn|/|r0|^{1/n}        \n" );
	PetscViewerASCIIPrintf( v, "#-----------------------------------------------------------------------------------------------------------------------\n");
	rho = 0.0;
	i = 0;
	PetscViewerASCIIPrintf( v, "%1.4d %14.12e %14.12e %14.12e %14.12e %14.12e %14.12e\n", i, rlog[i],rlog[0],rlog[i]/rlog[0],-log10(rlog[i]/rlog[0]), rho, 0.0 );
	for( i=1; i<nits; i++ ) {
		rho = (-log10(rlog[i]/rlog[0]))-(-log10(rlog[i-1]/rlog[0]));
		PetscViewerASCIIPrintf( v, "%1.4d %14.12e %14.12e %14.12e %14.12e %14.12e %14.12e \n", 
				i, rlog[i],rlog[0],rlog[i]/rlog[0],-log10(rlog[i]/rlog[0]), rho, pow(rlog[i]/rlog[0], 1.0/(double)i) );
	}
	
	
	PetscFunctionReturn(0);
}
