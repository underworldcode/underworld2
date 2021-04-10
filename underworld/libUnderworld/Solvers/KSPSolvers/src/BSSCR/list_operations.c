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



#include "common-driver-utils.h"


#define BSSCR_report_op(op,v,i,func_name) \
  if((op)) { PetscViewerASCIIPrintf((v), "[%.2d] %s: valid operation \n", i, func_name ); } \
  else { PetscViewerASCIIPrintf((v),     "[%.2d] %s: OPERATION NOT DEFINED \n", i, func_name ); } \


PetscErrorCode BSSCR_MatListOperations( Mat A, PetscViewer v )
{
	MatOps ops = A->ops;
	MatType type;
	PetscInt i;
	
	MatGetType( A, &type );
	PetscViewerASCIIPrintf(v, "Operations available for MatType: %s \n", type );
	
	PetscViewerASCIIPushTab( v );
	
	i=0;
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->setvalues, v,  i++, "MatSetValues" );
	BSSCR_report_op( ops->getrow, v,     i++, "MatSetValues" );
	BSSCR_report_op( ops->restorerow, v, i++, "MatRestoreRow" );
	BSSCR_report_op( ops->mult, v,       i++, "MatMult" );
	BSSCR_report_op( ops->multadd, v,    i++, "MatMultAdd" );
	
	BSSCR_report_op( ops->multtranspose, v,    i++, "MatMultTranspose" );
	BSSCR_report_op( ops->multtransposeadd, v, i++, "MatMultTransposeAdd" );
	BSSCR_report_op( ops->solve, v,            i++, "MatSolve" );
	BSSCR_report_op( ops->solveadd, v,         i++, "MatSolveAdd" );
	BSSCR_report_op( ops->solvetranspose, v,   i++, "MatSolveTranspose" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->solvetransposeadd, v,  i++, "MatSolveTransposeAdd" );
	BSSCR_report_op( ops->lufactor, v,           i++, "MatLUFactor" );
	BSSCR_report_op( ops->choleskyfactor, v,     i++, "MatCholeskyFactor" );
#if( ( PETSC_VERSION_MAJOR >= 3 ) && ( PETSC_VERSION_MINOR > 0 ) )
	BSSCR_report_op( ops->sor, v,                i++, "MatSor" );
#else
	BSSCR_report_op( ops->relax, v,              i++, "MatRelax" );
#endif
	BSSCR_report_op( ops->transpose, v,          i++, "MatTranspose" );
	
	BSSCR_report_op( ops->getinfo, v,       i++, "MatGetInfo" );
	BSSCR_report_op( ops->equal, v,         i++, "MatEqual" );
	BSSCR_report_op( ops->getdiagonal, v,   i++, "MatGetDiagonal" );
	BSSCR_report_op( ops->diagonalscale, v, i++, "MatDiagonalScale" );
	BSSCR_report_op( ops->norm, v,          i++, "MatNorm" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->assemblybegin, v, i++, "MatAssemblyBegin" );
	BSSCR_report_op( ops->assemblyend, v,   i++, "MatAssemblyEnd" );
#if( ( ( PETSC_VERSION_MAJOR == 3 ) && ( PETSC_VERSION_MINOR < 1 ) ) || ( PETSC_VERSION_MAJOR <= 2 ) )
	BSSCR_report_op( ops->compress, v,      i++, "MatCompress" );
#endif
	BSSCR_report_op( ops->setoption, v,     i++, "MatSetOption" );
	BSSCR_report_op( ops->zeroentries, v,   i++, "MatZeroEntries" );
	
	BSSCR_report_op( ops->zerorows, v,               i++, "Stg_MatZeroRows" );
	BSSCR_report_op( ops->lufactorsymbolic, v,       i++, "MatLUFactorSymbolic" );
	BSSCR_report_op( ops->lufactornumeric, v,        i++, "MatLUFactorNumeric" );
	BSSCR_report_op( ops->choleskyfactorsymbolic, v, i++, "MatCholeskyFactorSymbolic" );
	BSSCR_report_op( ops->choleskyfactornumeric, v,  i++, "MatCholeskyFactorNumeric" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");

	BSSCR_report_op( ops->ilufactorsymbolic, v,  i++, "MatILUFactorSymbolic" );
	BSSCR_report_op( ops->iccfactorsymbolic, v,  i++, "MatICCFactorSymbolic" );


	
	BSSCR_report_op( ops->duplicate, v,     i++, "MatDuplicate" );
	BSSCR_report_op( ops->forwardsolve, v,  i++, "MatForwardSolve" );
	BSSCR_report_op( ops->backwardsolve, v, i++, "MatBackwardSolve" );
	BSSCR_report_op( ops->ilufactor, v,     i++, "MatILUFactor" );
	BSSCR_report_op( ops->iccfactor, v,     i++, "MatICCFactor" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->axpy, v,            i++, "MatAXPY" );

#if( (PETSC_VERSION_MAJOR == 3) &&  (PETSC_VERSION_MINOR > 7) )
    BSSCR_report_op( ops->createsubmatrices, v,  i++, "MatCreateSubMatrices" );
#else
    BSSCR_report_op( ops->getsubmatrices, v,  i++, "MatGetSubMatrices" );
#endif

    BSSCR_report_op( ops->increaseoverlap, v, i++, "MatIncreaseOverlap" );
	BSSCR_report_op( ops->getvalues, v,       i++, "MatGetValues" );
	BSSCR_report_op( ops->copy, v,            i++, "MatCopy" );
	
	BSSCR_report_op( ops->getrowmax, v,   i++, "MatGetRowMax" );
	BSSCR_report_op( ops->scale, v,       i++, "MatScale" );
	BSSCR_report_op( ops->shift, v,       i++, "MatShift" );
	BSSCR_report_op( ops->diagonalset, v, i++, "MatDiagonalSet" );
#if( ( ( PETSC_VERSION_MAJOR == 3 ) && ( PETSC_VERSION_MINOR < 1 ) ) || ( PETSC_VERSION_MAJOR <= 2 ) )
	BSSCR_report_op( ops->iludtfactor, v, i++, "MatILUDTFactor" );
#endif
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");

	BSSCR_report_op( ops->getrowij, v,        i++, "MatGetRowIJ" );
	BSSCR_report_op( ops->restorerowij, v,    i++, "MatRestoreRowIJ" );
	BSSCR_report_op( ops->getcolumnij, v,     i++, "MatGetColumnIJ" );
	BSSCR_report_op( ops->restorecolumnij, v, i++, "MatRestoreColumnIJ" );
	
	BSSCR_report_op( ops->fdcoloringcreate, v, i++, "MatFDColoringCreate" );
	BSSCR_report_op( ops->coloringpatch, v,    i++, "MatColoringPatch" );
	BSSCR_report_op( ops->setunfactored, v,    i++, "MatSetUnFactored" );
	BSSCR_report_op( ops->permute, v,          i++, "MatPermute" );
	BSSCR_report_op( ops->setvaluesblocked, v, i++, "MatSetValuesBlocked" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");

#if( (PETSC_VERSION_MAJOR == 3) &&  (PETSC_VERSION_MINOR > 7) )
    BSSCR_report_op( ops->createsubmatrix, v,  i++, "MatCreateSubMatrix" );
#else
    BSSCR_report_op( ops->getsubmatrix, v,  i++, "MatGetSubMatrix" );
#endif

	BSSCR_report_op( ops->destroy, v,       i++, "MatDestroy" );
	BSSCR_report_op( ops->view, v,          i++, "MatView" );
	BSSCR_report_op( ops->convertfrom, v,   i++, "MatConvertFrom" );

	


	BSSCR_report_op( ops->setlocaltoglobalmapping, v, i++, "MatSetLocalToGlobalMapping" );
	BSSCR_report_op( ops->setvalueslocal, v,          i++, "MatSetValuesLocal" );
	BSSCR_report_op( ops->zerorowslocal, v,           i++, "Stg_MatZeroRowsLocal" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->getrowmaxabs, v,    i++, "MatGetRowMaxAbs" );
	BSSCR_report_op( ops->convert, v,         i++, "MatConvert" );
//    BSSCR_report_op( ops->setcoloring, v,     i++, "MatSetColoring" );  NOT SURE IF THIS IS AVAIL IN 3.8

	BSSCR_report_op( ops->setvaluesadifor, v, i++, "MatSetValuesAdicfor" );
	
	BSSCR_report_op( ops->fdcoloringapply, v,              i++, "MatFDCloringApply" );
	BSSCR_report_op( ops->setfromoptions, v,               i++, "MatSetFromOptions" );
	BSSCR_report_op( ops->multconstrained, v,              i++, "MatMultConstrained" );
	BSSCR_report_op( ops->multtransposeconstrained, v,     i++, "MatMultTransposeConstrained" );
	/* data member depreciated from PETSc v. 3.0.0 */
#if( PETSC_VERSION_MAJOR <= 2 )
	BSSCR_report_op( ops->ilufactorsymbolicconstrained, v, i++, "MatILUFactorSymbolicConstrained" );
#endif	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
#if( ( ( PETSC_VERSION_MAJOR == 3 ) && ( PETSC_VERSION_MINOR < 2 ) ) )
	BSSCR_report_op( ops->permutesparsify, v, i++, "MatPermuteSparsify" );
#endif
	BSSCR_report_op( ops->mults, v,           i++, "MatMults" );
	BSSCR_report_op( ops->solves, v,          i++, "MatSolves" );
	BSSCR_report_op( ops->getinertia, v,      i++, "MatGetInertia" );
	BSSCR_report_op( ops->load, v,            i++, "MatLoad" );
	
	BSSCR_report_op( ops->issymmetric, v,             i++, "MatIsSymmetric" );
	BSSCR_report_op( ops->ishermitian, v,             i++, "MatIsHermitian" );
	BSSCR_report_op( ops->isstructurallysymmetric, v, i++, "MatIsStructurallySymmetric" );
#if( ( ( PETSC_VERSION_MAJOR == 3 ) && ( PETSC_VERSION_MINOR < 1 ) ) || ( PETSC_VERSION_MAJOR <= 2 ) )
	BSSCR_report_op( ops->pbrelax, v,                 i++, "MatPBRelax" );
#endif
	BSSCR_report_op( ops->getvecs, v,                 i++, "MatGetVec" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	// BSSCR_report_op( ops->matmult, v,         i++, "MatMatMult" );  removed for petsc 3.13.2 compat.
	BSSCR_report_op( ops->matmultsymbolic, v, i++, "MatMatMultSymbolic" );
	BSSCR_report_op( ops->matmultnumeric, v,  i++, "MatMatMultNumeric" );
	// BSSCR_report_op( ops->ptap, v,            i++, "MatPtAP" );     removed for petsc 3.13.2 compat.
	BSSCR_report_op( ops->ptapsymbolic, v,    i++, "MatPtAPSymbolic" );
	
	BSSCR_report_op( ops->ptapnumeric, v,              i++, "MatPtAPNumeric" );




	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");



	BSSCR_report_op( ops->conjugate, v,           i++, "MatConjugate" );
	//BSSCR_report_op( ops->setsizes, v,            i++, "MatSetSizes" );
	
	BSSCR_report_op( ops->setvaluesrow, v,              i++, "MatSetValuesRow" );
	BSSCR_report_op( ops->realpart, v,                  i++, "MatRealPart" );
	BSSCR_report_op( ops->imaginarypart, v,             i++, "MatImaginaryPart" );
	BSSCR_report_op( ops->getrowuppertriangular, v,     i++, "MatGetRowUpperTriangular" );
	BSSCR_report_op( ops->restorerowuppertriangular, v, i++, "MatRestoreRowUpperTriangular" );
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->matsolve, v,           i++, "MatMatSolve" );
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR <6) )
	BSSCR_report_op( ops->getredundantmatrix, v, i++, "MatGetRedundantMatrix" );
#endif
	BSSCR_report_op( ops->getrowmin, v,          i++, "MatGetRowMin" );
	BSSCR_report_op( ops->getcolumnvector, v,    i++, "MatGetColumnVector" );
	
	
	PetscViewerASCIIPopTab( v );
	
	PetscFunctionReturn(0);
}


PetscErrorCode BSSCR_VecListOperations( Vec x, PetscViewer v )
{
	VecOps ops = x->ops;
	VecType type;
	PetscInt i;
	
	VecGetType( x, &type );
	PetscViewerASCIIPrintf(v, "Operations available for VecType: %s \n", type );
	
	PetscViewerASCIIPushTab( v );
	
	
	
	PetscViewerASCIIPopTab( v );
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_KSPListOperations( KSP ksp, PetscViewer v )
{
	KSPOps ops = ksp->ops;
	KSPType type;
	PetscInt i;
	
	KSPGetType( ksp, &type );
	PetscViewerASCIIPrintf(v, "Operations available for KSPType: %s \n", type );
	
	PetscViewerASCIIPushTab( v );
	
	i=0;
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->buildsolution, v,  i++, "KSPBuildSolution" );
	BSSCR_report_op( ops->buildresidual, v,  i++, "KSPBuildResidual" );
	BSSCR_report_op( ops->solve, v,          i++, "KSPSolve" );
	BSSCR_report_op( ops->setup, v,          i++, "KSPSetUp" );
	BSSCR_report_op( ops->setfromoptions, v, i++, "KSPSetFromOptions" );
	
	BSSCR_report_op( ops->publishoptions, v,               i++, "KSPPublishOptions" );
	BSSCR_report_op( ops->computeextremesingularvalues, v, i++, "KSPComputeExtremeSingularValues" );
	BSSCR_report_op( ops->computeeigenvalues, v,           i++, "KSPComputeEigenvalues" );
	BSSCR_report_op( ops->destroy, v, i++, "BSSCR_KSPDestroy" );
	BSSCR_report_op( ops->view, v,    i++, "KSPView" );
	
	PetscViewerASCIIPopTab( v );
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_PCListOperations( PC pc, PetscViewer v )
{
	PCOps ops = pc->ops;
	PCType type;
	PetscInt i;
	
	PCGetType( pc, &type );
	PetscViewerASCIIPrintf(v, "Operations available for PCType: %s \n", type );
	
	PetscViewerASCIIPushTab( v );
	
	i=0;
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->setup, v,           i++, "PCSetUp" );
	BSSCR_report_op( ops->apply, v,           i++, "PCApply" );
	BSSCR_report_op( ops->applyrichardson, v, i++, "PCApplyRichardson" );
	BSSCR_report_op( ops->applyBA, v,         i++, "PCApplyBA" );
	BSSCR_report_op( ops->applytranspose, v,  i++, "PCApplyTranspose" );
	
	BSSCR_report_op( ops->applyBAtranspose, v,  i++, "PCApplyBATranspose" );
	BSSCR_report_op( ops->setfromoptions, v,    i++, "PCSetFromOptions" );
	BSSCR_report_op( ops->presolve, v,          i++, "PCPreSolve" );
	BSSCR_report_op( ops->postsolve, v,         i++, "PCPostSolve" );
	BSSCR_report_op( ops->getfactoredmatrix, v, i++, "PCGetFactoredMatrix" );
	
	
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");
	BSSCR_report_op( ops->applysymmetricleft, v,  i++, "PCApplySymmetricLeft" );
	BSSCR_report_op( ops->applysymmetricright, v, i++, "PCApplySymmetricRight" );
	BSSCR_report_op( ops->setuponblocks, v,       i++, "PCSetupOnBlocks" );
	BSSCR_report_op( ops->destroy, v,             i++, "PCDestroy" );
	BSSCR_report_op( ops->view, v,                i++, "PCView" );
	
	
	PetscViewerASCIIPopTab( v );
	
	PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_SNESListOperations( SNES snes, PetscViewer v )
{
	SNESOps ops = snes->ops;
	SNESType type;
	PetscInt i;
	
	SNESGetType( snes, &type );
	PetscViewerASCIIPrintf(v, "Operations available for SNESType: %s \n", type );
	
	PetscViewerASCIIPushTab( v );
	
	i=0;
	PetscViewerASCIIPrintf(v, "------------------------------------------------\n");


	BSSCR_report_op( ops->computescaling, v,  i++, "SNESComputeScaling" );
	BSSCR_report_op( ops->update, v,          i++, "SNESUpdate" );
	BSSCR_report_op( ops->converged, v,       i++, "SNESConverged" );
	
	BSSCR_report_op( ops->setup, v,           i++, "SNESSetUp" );
	BSSCR_report_op( ops->solve, v,           i++, "SNESSolve" );
	BSSCR_report_op( ops->view, v,            i++, "SNESView" );
	BSSCR_report_op( ops->setfromoptions, v,  i++, "SNESSetFromOptions" );
	BSSCR_report_op( ops->destroy, v,         i++, "SNESDestroy" );
	
	PetscViewerASCIIPopTab( v );
	
	PetscFunctionReturn(0);
}
