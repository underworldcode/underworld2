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

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "common-driver-utils.h"


/*
name[] is operator name
*/
PetscErrorCode BSSCR_MatInfoLog(PetscViewer v,Mat A,const char name[])
{
	MatInfo i;
	PetscReal nrm_1,nrm_f,nrm_inf;
	MatType mtype;
	PetscInt M,N;
	PetscViewerType vtype;
	PetscTruth isascii;
	
	PetscViewerGetType( v, &vtype );
	Stg_PetscObjectTypeCompare( (PetscObject)v,PETSC_VIEWER_ASCII,&isascii );
	if (!isascii) { PetscFunctionReturn(0); }
	
	MatGetSize( A, &M,&N );
	MatGetInfo(A,MAT_GLOBAL_SUM,&i);
	
	MatGetType( A, &mtype );
	MatNorm( A, NORM_1, &nrm_1 );
	MatNorm( A, NORM_FROBENIUS, &nrm_f );
	MatNorm( A, NORM_INFINITY, &nrm_inf );
	
	PetscViewerASCIIPrintf( v, "MatInfo: %s \n", name );
	PetscViewerASCIIPushTab(v);
	
	PetscViewerASCIIPrintf( v, "type=%s \n", mtype );
	PetscViewerASCIIPrintf( v, "dimension=%Dx%D \n", M,N );
	PetscViewerASCIIPrintf( v, "nnz=%D (total)\n", (PetscInt)i.nz_used );
	PetscViewerASCIIPrintf( v, "nnz=%D (allocated)\n", (PetscInt)i.nz_allocated );
	PetscViewerASCIIPrintf( v, "|A|_1         = %1.12e\n", nrm_1 );
	PetscViewerASCIIPrintf( v, "|A|_frobenius = %1.12e\n", nrm_f );
	PetscViewerASCIIPrintf( v, "|A|_inf       = %1.12e\n", nrm_inf );
	
	PetscViewerASCIIPopTab(v);
	
	PetscFunctionReturn(0);
}

/*
name[] is operator name
*/
PetscErrorCode BSSCR_VecInfoLog(PetscViewer v,Vec x,const char name[])
{
	PetscReal nrm_1,nrm_f,nrm_inf;
	VecType vectype;
	PetscInt M;
	PetscViewerType vtype;
	PetscTruth isascii;
	
	PetscViewerGetType( v, &vtype );
	Stg_PetscObjectTypeCompare( (PetscObject)v,PETSC_VIEWER_ASCII,&isascii );
	if (!isascii) { PetscFunctionReturn(0); }
	
	VecGetSize( x, &M);
	
	VecGetType( x, &vectype );
	VecNorm( x, NORM_1, &nrm_1 );
	VecNorm( x, NORM_2, &nrm_f );
	VecNorm( x, NORM_INFINITY, &nrm_inf );
	
	PetscViewerASCIIPrintf( v, "VecInfo: %s \n", name );
	PetscViewerASCIIPushTab(v);
	
	PetscViewerASCIIPrintf( v, "type=%s \n", vectype );
	PetscViewerASCIIPrintf( v, "dimension=%Dx1 \n", M);
	PetscViewerASCIIPrintf( v, "|x|_1   = %1.12e\n", nrm_1 );
	PetscViewerASCIIPrintf( v, "|x|_2   = %1.12e\n", nrm_f );
	PetscViewerASCIIPrintf( v, "|x|_inf = %1.12e\n", nrm_inf );
	
	PetscViewerASCIIPopTab(v);
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_StokesCreateOperatorSummary( Mat K, Mat G, Mat C, Vec f, Vec h, const char filename[] )
{
	MPI_Comm comm;
	PetscViewer v;
	char op_name[PETSC_MAX_PATH_LEN];	
	PetscTruth flg;
	
	PetscObjectGetComm( (PetscObject)K, &comm );
	PetscViewerASCIIOpen( comm, filename, &v );
	
	BSSCR_GeneratePetscHeader_for_viewer( v );
	PetscViewerASCIIPrintf( v, "\nInput matrices:\n");
	PetscViewerASCIIPushTab(v);
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_A11",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "A11: %s\n", op_name );	}
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_A12",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "A12: %s\n", op_name );	}
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_A22",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "A22: %s\n", op_name );	}
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_Smat",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "Smat: %s\n", op_name );	}
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_b1",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "b1: %s\n", op_name );	}
	
	PetscOptionsGetString( PETSC_NULL,"-stokes_b2",op_name,PETSC_MAX_PATH_LEN-1,&flg );
	if (flg) {	PetscViewerASCIIPrintf( v, "b2: %s\n", op_name );	}
	
	PetscViewerASCIIPopTab(v);
	PetscViewerASCIIPrintf( v, "\n");
	
	BSSCR_MatInfoLog(v,K,"stokes_A11");			PetscViewerASCIIPrintf( v, "\n");
	
	BSSCR_MatInfoLog(v,G,"stokes_A12");			PetscViewerASCIIPrintf( v, "\n");
	if (C) { BSSCR_MatInfoLog(v,C,"stokes_A22"); 		PetscViewerASCIIPrintf( v, "\n");	}	
	
	BSSCR_VecInfoLog(v,f,"stokes_b1");			PetscViewerASCIIPrintf( v, "\n");
	BSSCR_VecInfoLog(v,h,"stokes_b2");			PetscViewerASCIIPrintf( v, "\n");
	
	Stg_PetscViewerDestroy(&v );
	
	PetscFunctionReturn(0);
}
