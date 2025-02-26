/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/Discretisation/src/Discretisation.h>

#include "SystemSetup.h"


/* Textual name of this class */
const Type PETScMGSolver_Type = "PETScMGSolver";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PETScMGSolver* PETScMGSolver_New( Name name ) {
	/* Variables set in this function */
	SizeT                                                    _sizeOfSelf = sizeof(PETScMGSolver);
	Type                                                            type = PETScMGSolver_Type;
	Stg_Class_DeleteFunction*                                    _delete = _PETScMGSolver_Delete;
	Stg_Class_PrintFunction*                                      _print = _PETScMGSolver_Print;
	Stg_Class_CopyFunction*                                        _copy = NULL;
	Stg_Component_DefaultConstructorFunction*        _defaultConstructor = (void* (*)(Name))_PETScMGSolver_New;
	Stg_Component_ConstructFunction*                          _construct = _PETScMGSolver_AssignFromXML;
	Stg_Component_BuildFunction*                                  _build = _PETScMGSolver_Build;
	Stg_Component_InitialiseFunction*                        _initialise = _PETScMGSolver_Initialise;
	Stg_Component_ExecuteFunction*                              _execute = _PETScMGSolver_Execute;
	Stg_Component_DestroyFunction*                              _destroy = _PETScMGSolver_Destroy;
	AllocationType                                    nameAllocationType = NON_GLOBAL;
	MGSolver_SetCommFunc*                                    setCommFunc = PETScMGSolver_SetComm;
	MGSolver_SetMatrixFunc*                                setMatrixFunc = NULL;
	MGSolver_SetMaxIterationsFunc*                  setMaxIterationsFunc = NULL;
	MGSolver_SetRelativeToleranceFunc*          setRelativeToleranceFunc = NULL;
	MGSolver_SetAbsoluteToleranceFunc*          setAbsoluteToleranceFunc = NULL;
	MGSolver_SetUseInitialSolutionFunc*        setUseInitialSolutionFunc = NULL;
	MGSolver_SolveFunc*                                        solveFunc = NULL;
	MGSolver_SetupFunc*                                        setupFunc = PETScMGSolver_Setup;
	MGSolver_GetSolveStatusFunc*                      getSolveStatusFunc = NULL;
	MGSolver_GetIterationsFunc*                        getIterationsFunc = NULL;
	MGSolver_GetMaxIterationsFunc*                  getMaxIterationsFunc = NULL;
	MGSolver_GetResidualNormFunc*                    getResidualNormFunc = NULL;

	return _PETScMGSolver_New(  PETSCMGSOLVER_PASSARGS  );
}

PETScMGSolver* _PETScMGSolver_New(  PETSCMGSOLVER_DEFARGS  ) {
	PETScMGSolver*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PETScMGSolver) );
	//self = (PETScMGSolver*)_PETScMatrixSolver_New(  STG_COMPONENT_PASSARGS  );
	self = (PETScMGSolver*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */
	/* from the depreciated MatrixSolver_New func */
	self->setCommFunc = setCommFunc;
	self->setMatrixFunc = setMatrixFunc;
	self->setMaxIterationsFunc = setMaxIterationsFunc;
	self->setRelativeToleranceFunc = setRelativeToleranceFunc;
	self->setAbsoluteToleranceFunc = setAbsoluteToleranceFunc;
	self->setUseInitialSolutionFunc = setUseInitialSolutionFunc;

	self->solveFunc = solveFunc;
	self->setupFunc = setupFunc;

	self->getSolveStatusFunc = getSolveStatusFunc;
	self->getIterationsFunc = getIterationsFunc;
	self->getMaxIterationsFunc = getMaxIterationsFunc;
	self->getResidualNormFunc = getResidualNormFunc;

	/* PETScMGSolver info */
	_PETScMGSolver_Init( self );

	return self;
}

void _PETScMGSolver_Init( PETScMGSolver* self ) {
	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	self->mgData = malloc( sizeof( MGSolver_PETScData ) );

	/* from the depreciated MatrixSolver_Init func */
	self->mgData->comm = MPI_COMM_WORLD;
	KSPCreate( MPI_COMM_WORLD, &self->mgData->ksp );
	self->mgData->matrix = PETSC_NULL;
	self->mgData->inversion = PETSC_NULL;
	self->mgData->residual = PETSC_NULL;
	self->mgData->expiredResidual = True;
	self->mgData->matrixChanged = True;
        self->mgData->optionsReady = False;

	self->mgData->curRHS = PETSC_NULL;
	self->mgData->curSolution = PETSC_NULL;
	/* end of MatrixSolver_Init stuff */

	self->nLevels = 0;
	self->levels = NULL;
	self->opGen = NULL;
	self->solversChanged = True;
	self->opsChanged = True;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PETScMGSolver_Delete( void* matrixSolver ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	PETScMGSolver_Destruct( self );

	/* Delete the parent. */
	//_PETScMatrixSolver_Delete( self );
	_Stg_Component_Delete( self );
}

void _PETScMGSolver_Print( void* matrixSolver, Stream* stream ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	
	/* Set the Journal for printing informations */
	Stream* matrixSolverStream;
	matrixSolverStream = Journal_Register( InfoStream_Type, (Name)"PETScMGSolverStream"  );

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	/* Print parent */
	Journal_Printf( stream, "PETScMGSolver (ptr): (%p)\n", self );
	//_PETScMatrixSolver_Print( self, stream );
	_Stg_Component_Print( self, stream );
}

void _PETScMGSolver_AssignFromXML( void* matrixSolver, Stg_ComponentFactory* cf, void* data ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	Bool		pure;
	unsigned	nLevels;
	unsigned	nCycles;
	unsigned	nDownIts, nUpIts;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( cf );

	//_PETScMatrixSolver_AssignFromXML( self, cf, data );

	pure = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"pure", False  );
	nLevels = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"levels", 1  );
	nCycles = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"cycles", 1  );
	nDownIts = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"downIterations", 1  );
	nUpIts = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"upIterations", 1  );
   
   Journal_Firewall( nLevels>1, NULL, "In func %s: Multigrid required mgLevels>1", __func__ );

	self->pure = pure;
	PETScMGSolver_SetLevels( self, nLevels );
	PETScMGSolver_SetLevelCycles( self, nLevels - 1, nCycles );
	PETScMGSolver_SetAllDownIterations( self, nDownIts );
	PETScMGSolver_SetAllUpIterations( self, nUpIts );

	self->opGen = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"opGenerator", MGOpGenerator, True, data  );
	MGOpGenerator_SetMatrixSolver( self->opGen, self );
	MGOpGenerator_SetNumLevels( self->opGen, nLevels );
}

void _PETScMGSolver_Build( void* matrixSolver, void* data ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	if( self->opGen )
		Stg_Component_Build( self->opGen, data, False );
}

void _PETScMGSolver_Initialise( void* matrixSolver, void* data ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	if( self->opGen )
		Stg_Component_Initialise( self->opGen, data, False );
}

void _PETScMGSolver_Execute( void* matrixSolver, void* data ) {
}

void _PETScMGSolver_Destroy( void* matrixSolver, void* data ) {
}

void PETScMGSolver_SetComm( void* matrixSolver, MPI_Comm comm ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	PC		pc;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	//PETScMatrixSolver_SetComm( self, comm );
	self->mgData->comm = comm;
	if( self->mgData->ksp )
		Stg_KSPDestroy(&self->mgData->ksp );
	KSPCreate( comm, &self->mgData->ksp );

	if( self->pure )
		//PETScMatrixSolver_SetKSPType( self, PETScMatrixSolver_KSPType_Richardson );
		KSPSetType( self->mgData->ksp, KSPRICHARDSON );	
	else
		//PETScMatrixSolver_SetKSPType( self, PETScMatrixSolver_KSPType_FGMRes );
		KSPSetType( self->mgData->ksp, KSPFGMRES );
	//PETScMatrixSolver_SetPCType( self, PETScMatrixSolver_PCType_Multigrid );
	KSPGetPC( self->mgData->ksp, &pc );
	PCSetType( pc, PCMG );
}

void PETScMGSolver_Setup( void* matrixSolver, void* rhs, void* solution ) { 
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	Bool		rebuildOps;
	double          wallTime;
	PetscErrorCode  ec;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	wallTime = MPI_Wtime();
		
	//_MatrixSolver_Setup( self, rhs, solution );
	self->mgData->curRHS = rhs;
	self->mgData->curSolution = solution;
	self->mgData->expiredResidual = True;

   	PetscPrintf( PETSC_COMM_WORLD, "_MatrixSolver_Setup %g\n", MPI_Wtime() - wallTime);


	if( self->opGen )
		rebuildOps = MGOpGenerator_HasExpired( self->opGen );
	else
		rebuildOps = False;
	if( !rebuildOps ) {
		unsigned		l_i;

		for( l_i = 1; l_i < self->nLevels; l_i++ ) {
			if( !self->levels[l_i].R || !self->levels[l_i].P ) {
				rebuildOps = True;
				break;
			}
		}
	}

	if( self->solversChanged || rebuildOps || self->opsChanged ) {
		wallTime = MPI_Wtime();
		PETScMGSolver_UpdateSolvers( self );
	   	PetscPrintf( PETSC_COMM_WORLD, "PETScMGSolver_UpdateSolvers %g\n", MPI_Wtime() - wallTime);
	}
	if( rebuildOps ) {
		wallTime = MPI_Wtime();
		PETScMGSolver_UpdateOps( self );
		PetscPrintf( PETSC_COMM_WORLD,  "PETScMGSolver_UpdateOps %g\n", MPI_Wtime() - wallTime);

	}
	//if( self->matrixChanged || self->solversChanged || rebuildOps || self->opsChanged ) {
	if( self->mgData->matrixChanged || self->solversChanged || rebuildOps || self->opsChanged ) {
		wallTime = MPI_Wtime();
		PETScMGSolver_UpdateMatrices( self );
		PETScMGSolver_UpdateWorkVectors( self );
		PetscPrintf( PETSC_COMM_WORLD, "PETScMGSolver_UpdateMats-WorkVecs %g\n", MPI_Wtime() - wallTime); 
	}

	self->solversChanged = False;
	//self->matrixChanged = False;
	self->mgData->matrixChanged = False;
	self->opsChanged = False;

        //if( !self->optionsReady ) {
        if( !self->mgData->optionsReady ) {
		KSPSetOptionsPrefix( self->mgData->ksp, "A11_" );
                ec = KSPSetFromOptions( self->mgData->ksp );
                CheckPETScError( ec );
                //self->optionsReady = True;
                self->mgData->optionsReady = True;
        }
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void PETScMGSolver_SetLevels( void* matrixSolver, unsigned nLevels ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	PETScMGSolver_DestructLevels( self );

	self->nLevels = nLevels;
	self->levels = AllocArray( PETScMGSolver_Level, nLevels );
	for( l_i = 0; l_i < nLevels; l_i++ ) {
		PETScMGSolver_Level*	level = self->levels + l_i;

		level->nDownIts = 1;
		level->nUpIts = (l_i == 0) ? 0 : 1;
		level->nCycles = 1;
		level->R = NULL;
		level->P = NULL;
		level->A = NULL;
		level->workRes = NULL;
		level->workSol = NULL;
		level->workRHS = NULL;
	}
}

//void PETScMGSolver_SetRestriction( void* matrixSolver, unsigned levelInd, void* R ) {
void PETScMGSolver_SetRestriction( void* matrixSolver, unsigned levelInd, void* _R ) {
	PETScMGSolver*		self 	= (PETScMGSolver*)matrixSolver;
	PETScMGSolver_Level*	level;
	Mat			R	= (Mat)_R;
    int equal=0;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( levelInd < self->nLevels && levelInd > 0 );
	//assert( !R || Stg_CheckType( R, PETScMatrix ) );

	level = self->levels + levelInd;
	if( level->R != R )
		self->opsChanged = True;

    if(level->P == level->R) equal=1;/* need to test for equality first as petsc will set mat to PETSC_NULL on destroy */

	if( level->R != PETSC_NULL ){
		Stg_MatDestroy(&level->R );
		if(equal)
		    level->P = PETSC_NULL;
    }
	level->R = R;
}

void PETScMGSolver_SetProlongation( void* matrixSolver, unsigned levelInd, void* _P ) {
	PETScMGSolver*		self 	= (PETScMGSolver*)matrixSolver;
	PETScMGSolver_Level*	level;
	Mat			P	= (Mat)_P;
    int equal=0;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( levelInd < self->nLevels && levelInd > 0 );
	//assert( !P || Stg_CheckType( P, PETScMatrix ) );

	level = self->levels + levelInd;
	if( level->P != P )
		self->opsChanged = True;

    if(level->P == level->R) equal=1;/* need to test for equality first as petsc will set mat to PETSC_NULL on destroy */

	if( level->P != PETSC_NULL ) {
		Stg_MatDestroy(&level->P );
		if(equal)
		    level->R = PETSC_NULL;
	}
	level->P = P;
}

void PETScMGSolver_SetLevelDownIterations( void* matrixSolver, unsigned level, unsigned nIts ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( level < self->nLevels );

	self->levels[level].nDownIts = nIts;
	self->solversChanged = True;
}

void PETScMGSolver_SetLevelUpIterations( void* matrixSolver, unsigned level, unsigned nIts ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( level < self->nLevels );
	assert( level > 0 );

	self->levels[level].nUpIts = nIts;
	self->solversChanged = True;
}

void PETScMGSolver_SetLevelCycles( void* matrixSolver, unsigned level, unsigned nCycles ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	assert( level < self->nLevels );
	assert( level > 0 );

	self->levels[level].nCycles = nCycles;
	self->solversChanged = True;
}

void PETScMGSolver_SetAllDownIterations( void* matrixSolver, unsigned nIts ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	for( l_i = 1; l_i < self->nLevels; l_i++ )
		PETScMGSolver_SetLevelDownIterations( self, l_i, nIts );
}

void PETScMGSolver_SetAllUpIterations( void* matrixSolver, unsigned nIts ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	for( l_i = 1; l_i < self->nLevels; l_i++ )
		PETScMGSolver_SetLevelUpIterations( self, l_i, nIts );
}

unsigned PETScMGSolver_GetNumLevels( void* matrixSolver ) {
	PETScMGSolver*	self = (PETScMGSolver*)matrixSolver;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	return self->nLevels;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void PETScMGSolver_UpdateOps( PETScMGSolver* self ) {
	PC		pc;
	Mat		*pOps, *rOps;
	PetscErrorCode	ec;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	ec = KSPGetPC( self->mgData->ksp, &pc );
	CheckPETScError( ec );

	MGOpGenerator_Generate( self->opGen, (Mat**)&pOps, (Mat**)&rOps );

	for( l_i = 1; l_i < self->nLevels; l_i++ ) {
		//assert( Stg_CheckType( pOps[l_i], PETScMatrix ) );
		//assert( Stg_CheckType( rOps[l_i], PETScMatrix ) );

		PETScMGSolver_SetProlongation( self, l_i, pOps[l_i] );
#if( ((PETSC_VERSION_MAJOR==2) && (PETSC_VERSION_MINOR==3) && (PETSC_VERSION_SUBMINOR==3)) || (PETSC_VERSION_MAJOR==3) )
			ec = PCMGSetInterpolation( pc, l_i, pOps[l_i] );
#else
			ec = PCMGSetInterpolate( pc, l_i, pOps[l_i] );
#endif
		CheckPETScError( ec );

		PETScMGSolver_SetRestriction( self, l_i, rOps[l_i] );
		ec = PCMGSetRestriction( pc, l_i, rOps[l_i] );
		CheckPETScError( ec );
	}

	FreeArray( pOps );
	FreeArray( rOps );
}

void PETScMGSolver_UpdateMatrices( PETScMGSolver* self ) {
	Stream*			stream;
	PETScMGSolver_Level*	level;
	PC			pc;
	KSP			levelKSP;
	PetscErrorCode		ec;
	unsigned		l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );
	//assert( self->matrix && Stg_CheckType( self->matrix, PETScMatrix ) );

	stream = Journal_Register( InfoStream_Type, (Name)"general" ); assert( stream  );
	Journal_Printf( stream, "Updating MG matrices ...\n" );

	ec = KSPGetPC( self->mgData->ksp, &pc );
	CheckPETScError( ec );

	for( l_i = self->nLevels - 1; l_i < self->nLevels; l_i-- ) {
		level = self->levels + l_i;

		if( l_i == self->nLevels - 1 )
			//level->A = (PETScMatrix*)self->matrix;
			level->A = self->mgData->matrix;
		else {
                    if( level->A )
                        MatPtAP( self->levels[l_i + 1].A, self->levels[l_i + 1].P, MAT_REUSE_MATRIX, 1.0, &level->A );
                    else
                        MatPtAP( self->levels[l_i + 1].A, self->levels[l_i + 1].P, MAT_INITIAL_MATRIX, 1.0, &level->A );
			//Matrix_PtAP( self->levels[l_i + 1].A, self->levels[l_i + 1].P, (void**)&level->A );

/*
			if( self->levels[l_i + 1].P )
				MatPtAP( self->levels[l_i + 1].A, self->levels[l_i + 1].P, MAT_REUSE_MATRIX, 1.0, &level->A );
			else
				MatPtAP( self->levels[l_i + 1].A, self->levels[l_i + 1].P, MAT_INITIAL_MATRIX, 1.0, &level->A );
*/
		}

		ec = PCMGGetSmootherDown( pc, l_i, &levelKSP );
		CheckPETScError( ec );
		//ec = Stg_KSPSetOperators( levelKSP, level->A->petscMat, level->A->petscMat, DIFFERENT_NONZERO_PATTERN );
		ec = Stg_KSPSetOperators( levelKSP, level->A, level->A, DIFFERENT_NONZERO_PATTERN );
		CheckPETScError( ec );
		//ec = PCMGSetResidual( pc, l_i, PCMGDefaultResidual, level->A->petscMat );
		ec = PCMGSetResidual( pc, l_i, Stg_PCMGDefaultResidual, level->A );
		CheckPETScError( ec );

		if( l_i > 0 ) {
			PCMGGetSmootherUp( pc, l_i, &levelKSP );
			//ec = Stg_KSPSetOperators( levelKSP, level->A->petscMat, level->A->petscMat, DIFFERENT_NONZERO_PATTERN );
			ec = Stg_KSPSetOperators( levelKSP, level->A, level->A, DIFFERENT_NONZERO_PATTERN );
			CheckPETScError( ec );
		}
	}

	Journal_Printf( stream, "done\n" );
}

void PETScMGSolver_UpdateWorkVectors( PETScMGSolver* self ) {
	PETScMGSolver_Level*	level;
	PC			pc;
	//unsigned		size;
	PetscInt		size, vecSize;
	PetscErrorCode		ec;
	unsigned		l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	if( self->nLevels == 1 )
		return;

	ec = KSPGetPC( self->mgData->ksp, &pc );
	CheckPETScError( ec );

	for( l_i = 0; l_i < self->nLevels; l_i++ ) {
		level = self->levels + l_i;

		//Matrix_GetLocalSize( level->A, &size, NULL );
		MatGetLocalSize( level->A, &size, PETSC_NULL );

                if( level->workRes )
                    VecGetLocalSize( level->workRes, &vecSize );

		if( l_i > 0 && (!level->workRes || /*Vector_GetLocalSize( level->workRes )*/vecSize != size) ) {
			if( level->workRes )
				Stg_VecDestroy(&level->workRes );
			//	FreeObject( level->workRes );
			//Vector_Duplicate( self->curSolution, (void**)&level->workRes );
			//Vector_SetLocalSize( level->workRes, size );
			//ec = PCMGSetR( pc, l_i, level->workRes->petscVec );
			VecCreate( MPI_COMM_WORLD, &level->workRes );
			VecSetSizes( level->workRes, size, PETSC_DECIDE );
			VecSetFromOptions( level->workRes );
#if( PETSC_VERSION_MAJOR <= 2 && PETSC_VERSION_MINOR >= 3 && PETSC_VERSION_SUBMINOR >= 3 )
			VecSetOption( level->workRes, VEC_IGNORE_NEGATIVE_INDICES );
#elif( PETSC_VERSION_MAJOR >= 3 )
			VecSetOption( level->workRes, VEC_IGNORE_NEGATIVE_INDICES, PETSC_TRUE );
#endif
			ec = PCMGSetR( pc, l_i, level->workRes );
			CheckPETScError( ec );
		}

		if( l_i < self->nLevels - 1 ) {
                    if( level->workSol )
			VecGetLocalSize( level->workSol, &vecSize );
                    if( !level->workSol || /*Vector_GetLocalSize( level->workSol )*/vecSize != size ) {
				if( level->workSol )
					Stg_VecDestroy(&level->workSol );
				//	FreeObject( level->workSol );
				//Vector_Duplicate( self->curSolution, (void**)&level->workSol );
				//Vector_SetLocalSize( level->workSol, size );
				//ec = PCMGSetX( pc, l_i, level->workSol->petscVec );
				VecCreate( MPI_COMM_WORLD, &level->workSol );
				VecSetSizes( level->workSol, size, PETSC_DECIDE );
				VecSetFromOptions( level->workSol );
#if( PETSC_VERSION_MAJOR <= 2 && PETSC_VERSION_MINOR >= 3 && PETSC_VERSION_SUBMINOR >= 3 )
				VecSetOption( level->workSol, VEC_IGNORE_NEGATIVE_INDICES );
#elif( PETSC_VERSION_MAJOR >= 3 )
				VecSetOption( level->workSol, VEC_IGNORE_NEGATIVE_INDICES, PETSC_TRUE );
#endif
				ec = PCMGSetX( pc, l_i, level->workSol );
				CheckPETScError( ec );
			}

                    if( level->workRHS )
			VecGetLocalSize( level->workRHS, &vecSize );
                    if( !level->workRHS || /*Vector_GetLocalSize( level->workRHS )*/vecSize != size ) {
				if( level->workRHS )
					Stg_VecDestroy(&level->workRHS );
				//	FreeObject( level->workRHS );
				//Vector_Duplicate( self->curSolution, (void**)&level->workRHS );
				//Vector_SetLocalSize( level->workRHS, size );
				//ec = PCMGSetRhs( pc, l_i, level->workRHS->petscVec );
				VecCreate( MPI_COMM_WORLD, &level->workRHS );
				VecSetSizes( level->workRHS, size, PETSC_DECIDE );
				VecSetFromOptions( level->workRHS );
#if( PETSC_VERSION_MAJOR <= 2 && PETSC_VERSION_MINOR >= 3 && PETSC_VERSION_SUBMINOR >= 3 )
				VecSetOption( level->workRHS, VEC_IGNORE_NEGATIVE_INDICES );
#elif( PETSC_VERSION_MAJOR >= 3 )
				VecSetOption( level->workRHS, VEC_IGNORE_NEGATIVE_INDICES, PETSC_TRUE );
#endif
				ec = PCMGSetRhs( pc, l_i, level->workRHS );
				CheckPETScError( ec );
			}
		}
	}
}

void PETScMGSolver_UpdateSolvers( PETScMGSolver* self ) {
	PETScMGSolver_Level*	level;
	PC			pc;
	KSP			levelKSP;
	PC			levelPC;
	PetscErrorCode		ec;
	unsigned		l_i;
	PetscTruth              smoothers_differ, flag;
	PetscMPIInt             size;
        MPI_Comm                comm;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	ec = KSPGetPC( self->mgData->ksp, &pc );
	CheckPETScError( ec );

	ec = PCMGSetLevels( pc, self->nLevels, PETSC_NULL );
	CheckPETScError( ec );
	ec = PCMGSetType( pc, PC_MG_MULTIPLICATIVE );
	CheckPETScError( ec );

	ec=PetscOptionsGetTruth( PETSC_NULL, "-pc_mg_different_smoothers", &smoothers_differ, &flag ); CheckPETScError(ec);

	ec=PetscObjectGetComm( (PetscObject)pc, &comm ); CheckPETScError(ec);
	MPI_Comm_size( comm, &size );

	for( l_i = 1; l_i < self->nLevels; l_i++ ) {
		level = self->levels + l_i;

		printf("Configuring MG level %d \n", l_i );
		ec = PCMGGetSmootherDown( pc, l_i, &levelKSP );
		CheckPETScError( ec );
		if(smoothers_differ==PETSC_TRUE) { ec=KSPAppendOptionsPrefix( levelKSP, "down_" ); CheckPETScError(ec); }
		ec = KSPSetType( levelKSP, KSPRICHARDSON ); CheckPETScError( ec );
		ec = KSPGetPC( levelKSP, &levelPC ); CheckPETScError( ec );

		if(size==1) {
		  ec = PCSetType( levelPC, PCSOR ); CheckPETScError( ec );
		}
		/* This does not work - bug with the order the operators are created I guess */
		/* For parallel jobs you best bet is to use the command line args and let petsc work it out */
		/*
		else {
		  KSP *sub_ksp;
		  PetscInt k, n_local, first_local;
		  PC sub_pc;

		  PCSetType( levelPC, PCBJACOBI );
		  KSPSetUp( levelKSP );
		  PCBJacobiGetSubKSP( levelPC, &n_local,&first_local,&sub_ksp);
		  for(k=0;k<n_local;k++ ) {
		    KSPSetType( sub_ksp[k], KSPFGMRES );
		    KSPGetPC( sub_ksp[k], &sub_pc );
		    PCSetType( sub_pc, PCSOR );
		  }
		}
		*/
		ec = KSPSetTolerances( levelKSP, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT, level->nDownIts ); CheckPETScError( ec );
		if( l_i == self->nLevels - 1 ) { 
		  ec = KSPSetInitialGuessNonzero( levelKSP, PETSC_TRUE );  CheckPETScError( ec );
		} 
		else {  ec = KSPSetInitialGuessNonzero( levelKSP, PETSC_FALSE ); CheckPETScError( ec );  }

		ec = PCMGGetSmootherUp( pc, l_i, &levelKSP ); CheckPETScError( ec );
		if(smoothers_differ==PETSC_TRUE) { ec=KSPAppendOptionsPrefix( levelKSP, "up_" ); CheckPETScError(ec); }
		ec = KSPSetType( levelKSP, KSPRICHARDSON ); CheckPETScError( ec );
		ec = KSPGetPC( levelKSP, &levelPC ); CheckPETScError( ec );
		if(size==1) {
		  ec = PCSetType( levelPC, PCSOR ); CheckPETScError( ec );
		}
		ec = KSPSetTolerances( levelKSP, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT, level->nUpIts ); CheckPETScError( ec );
		ec = KSPSetInitialGuessNonzero( levelKSP, PETSC_TRUE ); CheckPETScError( ec );

		ec = PCMGSetCyclesOnLevel( pc, l_i, level->nCycles ); CheckPETScError( ec );
	}
}

void PETScMGSolver_Destruct( PETScMGSolver* self ) {
	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	PETScMGSolver_DestructLevels( self );
	/*KillObject( self->opGen );*/
}

void PETScMGSolver_DestructLevels( PETScMGSolver* self ) {
	unsigned	l_i;

	assert( self && Stg_CheckType( self, PETScMGSolver ) );

	for( l_i = 0; l_i < self->nLevels; l_i++ ) {
		PETScMGSolver_Level*	level = self->levels + l_i;

        if(level->R == level->P){
          if( level->R != PETSC_NULL ){
            Stg_MatDestroy(&level->R );
            level->P =  PETSC_NULL;
          }
        }else{/* not same so test individually */
          if( level->R != PETSC_NULL ) Stg_MatDestroy(&level->R );
          if( level->P != PETSC_NULL ) Stg_MatDestroy(&level->P );
        }

        if( level->workRes )
          Stg_VecDestroy(&level->workRes );
        if( level->workSol )
          Stg_VecDestroy(&level->workSol );
        if( level->workRHS )
          Stg_VecDestroy(&level->workRHS );
	}

	KillArray( self->levels );
	self->nLevels = 0;
	self->solversChanged = True;
	self->opsChanged = True;
}


