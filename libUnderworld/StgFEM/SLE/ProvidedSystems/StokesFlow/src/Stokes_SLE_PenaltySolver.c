/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"
#include "types.h"
#include "Stokes_SLE_PenaltySolver.h"

#include <assert.h>
#include <string.h>
#include <float.h>

#include "Stokes_SLE.h"

const Type Stokes_SLE_PenaltySolver_Type = "Stokes_SLE_PenaltySolver";

void* Stokes_SLE_PenaltySolver_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                              _sizeOfSelf = sizeof(Stokes_SLE_PenaltySolver);
    Type                                                      type = Stokes_SLE_PenaltySolver_Type;
    Stg_Class_DeleteFunction*                              _delete = _Stokes_SLE_PenaltySolver_Delete;
    Stg_Class_PrintFunction*                                _print = _Stokes_SLE_PenaltySolver_Print;
    Stg_Class_CopyFunction*                                  _copy = _Stokes_SLE_PenaltySolver_Copy;
    Stg_Component_DefaultConstructorFunction*  _defaultConstructor = Stokes_SLE_PenaltySolver_DefaultNew;
    Stg_Component_ConstructFunction*                    _construct = _Stokes_SLE_PenaltySolver_AssignFromXML;
    Stg_Component_BuildFunction*                            _build = _Stokes_SLE_PenaltySolver_Build;
    Stg_Component_InitialiseFunction*                  _initialise = _SLE_Solver_Initialise;
    Stg_Component_ExecuteFunction*                        _execute = _SLE_Solver_Execute;
    Stg_Component_DestroyFunction*                        _destroy = _SLE_Solver_Destroy;
    SLE_Solver_SolverSetupFunction*                   _solverSetup = _Stokes_SLE_PenaltySolver_SolverSetup;
    SLE_Solver_SolveFunction*                               _solve = _Stokes_SLE_PenaltySolver_Solve;
    SLE_Solver_GetResidualFunc*                       _getResidual = _Stokes_SLE_PenaltySolver_GetResidual;

    /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return _Stokes_SLE_PenaltySolver_New(  STOKES_SLE_PENALTYSOLVER_PASSARGS  );
}

Stokes_SLE_PenaltySolver* Stokes_SLE_PenaltySolver_New(
    Name                                        name )
{
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*) Stokes_SLE_PenaltySolver_DefaultNew( name );

    Stokes_SLE_PenaltySolver_InitAll( self );

    return self;
}


/* Creation implementation / Virtual constructor */
Stokes_SLE_PenaltySolver* _Stokes_SLE_PenaltySolver_New(  STOKES_SLE_PENALTYSOLVER_DEFARGS  )
{
    Stokes_SLE_PenaltySolver* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(Stokes_SLE_PenaltySolver) );
    self = (Stokes_SLE_PenaltySolver*) _SLE_Solver_New(  SLE_SOLVER_PASSARGS  );

    /* Virtual info */
    return self;
}


void _Stokes_SLE_PenaltySolver_Init( void* solver ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*)solver;

    self->isConstructed       = True;
}

void Stokes_SLE_PenaltySolver_InitAll( void* solver )
{
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*)solver;

    SLE_Solver_InitAll( self );
    _Stokes_SLE_PenaltySolver_Init( self );
}


void _Stokes_SLE_PenaltySolver_Delete( void* solver ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*)solver;
		
    Journal_DPrintf( self->debug, "In: %s \n", __func__);

    Stream_IndentBranch( StgFEM_Debug );

    Stream_UnIndentBranch( StgFEM_Debug );
}       


void _Stokes_SLE_PenaltySolver_Print( void* solver, Stream* stream ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*)solver;

    _SLE_Solver_Print( self, stream );
}


void* _Stokes_SLE_PenaltySolver_Copy( void* stokesSlePenaltySolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*)stokesSlePenaltySolver;
    Stokes_SLE_PenaltySolver*	newStokesSlePenaltySolver;
	
    newStokesSlePenaltySolver = _SLE_Solver_Copy( self, dest, deep, nameExt, ptrMap );
	
    return (void*) newStokesSlePenaltySolver;
}


void _Stokes_SLE_PenaltySolver_Build( void* solver, void* stokesSLE ) {
    Stokes_SLE_PenaltySolver*	self  = (Stokes_SLE_PenaltySolver*)solver;

    Journal_DPrintf( self->debug, "In %s\n", __func__ );
    Stream_IndentBranch( StgFEM_Debug );
    Stream_UnIndentBranch( StgFEM_Debug );
}

void _Stokes_SLE_PenaltySolver_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data ) {
    Stokes_SLE_PenaltySolver* self         = (Stokes_SLE_PenaltySolver*) solver;

    _SLE_Solver_AssignFromXML( self, cf, data );
	
    _Stokes_SLE_PenaltySolver_Init( self );
}

void _Stokes_SLE_PenaltySolver_Execute( void* solver, void* data ) {
}

void _Stokes_SLE_PenaltySolver_Destroy( void* solver, void* data ) {
}

void _Stokes_SLE_PenaltySolver_Initialise( void* solver, void* stokesSLE ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*) solver;
    Stokes_SLE*             sle  = (Stokes_SLE*)             stokesSLE;
	
    /* Initialise Parent */
    _SLE_Solver_Initialise( self, sle );
}

/* SolverSetup */
void _Stokes_SLE_PenaltySolver_SolverSetup( void* solver, void* stokesSLE ) {
    Stokes_SLE_PenaltySolver* self = (Stokes_SLE_PenaltySolver*) solver;
	
    Journal_DPrintf( self->debug, "In %s:\n", __func__ );
    Stream_IndentBranch( StgFEM_Debug );
    Stream_UnIndentBranch( StgFEM_Debug );
}


void _Stokes_SLE_PenaltySolver_Solve( void* solver,void* stokesSLE ) {
    Stokes_SLE_PenaltySolver* self            = (Stokes_SLE_PenaltySolver*)solver;	
    Stokes_SLE*             sle             = (Stokes_SLE*)stokesSLE;
    /* Create shortcuts to stuff needed on sle */
    Mat                     kMatrix         = sle->kStiffMat->matrix;
    Mat                     gradMat         = sle->gStiffMat->matrix;
    Mat                     divMat          = NULL;
    Mat                     C_Mat           = sle->cStiffMat->matrix;
    Vec                     uVec            = sle->uSolnVec->vector;
    Vec                     pVec            = sle->pSolnVec->vector;
    Vec                     fVec            = sle->fForceVec->vector;
    Vec                     hVec            = sle->hForceVec->vector;
    Vec     		hTempVec;
    Vec    			fTempVec;
    Vec                     penalty;
    Mat    			GTrans, kHat;
    KSP			ksp_v;
    double	 		negOne=-1.0;
    double	 		one=1.0;
    Mat    			C_InvMat;
    Vec    			diagC;
    PC			pc;
    int                 rank;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    Journal_DPrintf( self->debug, "In %s():\n", __func__ );

    VecDuplicate( hVec, &hTempVec );
    VecDuplicate( fVec, &fTempVec );
    VecDuplicate( pVec, &diagC );
	
    if( sle->dStiffMat == NULL ) {
        Journal_DPrintf( self->debug, "Div matrix == NULL : Problem is assumed to be symmetric. ie Div = GTrans \n");
#if( PETSC_VERSION_MAJOR <= 2 )
        MatTranspose( gradMat, &GTrans );
#else
        MatTranspose( gradMat, MAT_INITIAL_MATRIX, &GTrans );
#endif
        divMat = GTrans;
    }
    else {

       MatType type;
       PetscInt size[2];

        MatGetType( sle->dStiffMat->matrix, &type );
        MatGetLocalSize( sle->dStiffMat->matrix, size + 0, size + 1 );

        /* make a copy we can play with */
        MatCreate( sle->comm, &GTrans );
        MatSetSizes( GTrans, size[0], size[1], PETSC_DECIDE, PETSC_DECIDE );
        MatSetType( GTrans, type );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
        MatSetUp(GTrans);
#endif
        MatCopy( sle->dStiffMat->matrix, GTrans, DIFFERENT_NONZERO_PATTERN );
        divMat = GTrans;

    }

    Stokes_SLE_PenaltySolver_MakePenalty( self, sle, &penalty );

    /* Create CInv */
    MatGetDiagonal( C_Mat, diagC );
    VecReciprocal( diagC );
    VecPointwiseMult( diagC, penalty, diagC );
    { /* Print the maximum and minimum penalties in my system. */
        PetscInt idx;
        PetscReal min, max;

        VecMin( diagC, &idx, &min );
        VecMax( diagC, &idx, &max );
        if( rank == 0 ) {
           printf( "PENALTY RANGE:\n" );
           printf( "  MIN: %e\n", min );
           printf( "  MAX: %e\n", max );
        }
    }
    MatDiagonalSet( C_Mat, diagC, INSERT_VALUES );
    C_InvMat = C_Mat;				/* Use pointer CInv since C has been inverted */

    /* Build RHS : rhs = f - GCInv h */
    MatMult( C_InvMat, hVec, hTempVec ); /* hTempVec = C_InvMat * hVec */
    VecScale( hTempVec, -1.0 );
    MatMult( gradMat, hTempVec, fTempVec );
#if 0
    VecPointwiseMult( fTempVec, penalty, fTempVec );
    { /* Print the maximum and minimum penalties in my system. */
        PetscInt idx;
        PetscReal min, max;

        VecMin( fTempVec, &idx, &min );
        VecMax( fTempVec, &idx, &max );
        printf( "PENALTY RANGE:\n" );
        printf( "  MIN: %e\n", min );
        printf( "  MAX: %e\n", max );
    }
#endif
    VecAXPY( fTempVec, 1.0, fVec );
    /*MatMultAdd( gradMat, hTempVec, fVec, fTempVec );*/

    /* Build G CInv GTrans */
/* 	MatTranspose( gradMat, &GTrans ); */
/* 	 since CInv is diagonal we can just scale mat entries by the diag vector */
    MatDiagonalScale( divMat, diagC, PETSC_NULL );  /*  Div = CInve Div */
    MatMatMult( gradMat, divMat, MAT_INITIAL_MATRIX, PETSC_DEFAULT, &kHat );
    /*MatDiagonalScale( kHat, penalty, PETSC_NULL );*/
    MatScale( kHat, -1.0 );
    MatAXPY( kMatrix, 1.0, kHat, SAME_NONZERO_PATTERN );
	
    /* Setup solver context and make sure that it uses a direct solver */
    KSPCreate( sle->comm, &ksp_v );
    Stg_KSPSetOperators( ksp_v, kMatrix, kMatrix, DIFFERENT_NONZERO_PATTERN );
    KSPSetType( ksp_v, KSPPREONLY );
    KSPGetPC( ksp_v, &pc );
    PCSetType( pc, PCLU );
    KSPSetFromOptions( ksp_v );

    KSPSolve( ksp_v, fTempVec, uVec );
	
    /* Recover p */
    if( sle->dStiffMat == NULL ) {

/* 		 since Div was modified when C is diagonal, re build the transpose */
        if( GTrans != PETSC_NULL )
            Stg_MatDestroy(&GTrans );

#if( PETSC_VERSION_MAJOR <= 2 )
        MatTranspose( gradMat, &GTrans );
#else
        MatTranspose( gradMat, MAT_INITIAL_MATRIX, &GTrans );
#endif
        divMat = GTrans;
    }
    else {
/* 		 never modified Div_null so set divMat to point back to it */
        divMat = sle->dStiffMat->matrix;
    }

    MatMult( divMat, uVec, hTempVec );    /* hTemp = Div v */
    VecAYPX( hTempVec, negOne, hVec );    /* hTemp = H - hTemp   : hTemp = H - Div v */
    MatMult( C_InvMat, hTempVec, pVec );  /* p = CInv hTemp      : p = CInv ( H - Div v ) */
	
    Stg_MatDestroy(&kHat );
    if( fTempVec != PETSC_NULL ) Stg_VecDestroy(&fTempVec );
    if( hTempVec != PETSC_NULL ) Stg_VecDestroy(&hTempVec );
    if( diagC != PETSC_NULL )    Stg_VecDestroy(&diagC );
    if( ksp_v != PETSC_NULL )   Stg_KSPDestroy(&ksp_v );
    if( GTrans != PETSC_NULL )   Stg_MatDestroy(&GTrans );
}


Vec _Stokes_SLE_PenaltySolver_GetResidual( void* solver, Index fv_I ) {
/* 	 TODO */
    return NULL;
}

void Stokes_SLE_PenaltySolver_MakePenalty( Stokes_SLE_PenaltySolver* self, Stokes_SLE* sle, Vec* _penalty ) {
    Vec fVec = sle->fForceVec->vector, hVec = sle->hForceVec->vector, penalty, lambda;
    Mat kMat = sle->kStiffMat->matrix;
    FeMesh *mesh = sle->kStiffMat->rowVariable->feMesh;
    FeVariable *velField = sle->kStiffMat->rowVariable;
    FeEquationNumber *eqNum = velField->eqNum;
    IArray *inc;
    PetscScalar *lambdaVals, lambdaMin, *penaltyVals;
    int numDofs, numLocalElems, nodeCur, numLocalNodes, rank, eq;
    SolutionVector *solVec = sle->uSolnVec;
    double *velBackup;
    Vec vecBackup;
    int ii, jj, kk;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    numDofs = Mesh_GetDimSize( mesh );
    numLocalElems = FeMesh_GetElementLocalSize( mesh );
    numLocalNodes = FeMesh_GetNodeLocalSize( mesh );

    velBackup = (double*)malloc( numLocalNodes*numDofs*sizeof(double) );
    for( ii = 0; ii < numLocalNodes; ii++ )
        FeVariable_GetValueAtNode( velField, ii, velBackup + ii*numDofs );

    VecDuplicate( hVec, &penalty );
    VecGetArray( penalty, &penaltyVals );

    VecDuplicate( fVec, &lambda );
    MatGetDiagonal( kMat, lambda );
    {
        PetscInt idx;
        PetscReal min, max;

        VecMin( lambda, &idx, &min );
        VecMax( lambda, &idx, &max );
        if( rank == 0 ) {
           printf( "LAMBDA RANGE:\n" );
           printf( "  MIN: %e\n", min );
           printf( "  MAX: %e\n", max );
        }
    }

    vecBackup = solVec->vector;
    solVec->vector = lambda;
    SolutionVector_UpdateSolutionOntoNodes( solVec );

    inc = IArray_New();
    lambdaVals = (double*)malloc( numDofs*sizeof(double) );

    for( ii = 0; ii < numLocalElems; ii++ ) {

        lambdaMin = DBL_MAX;

        FeMesh_GetElementNodes( mesh, ii, inc );
        for( jj = 0; jj < inc->size; jj++ ) {

            nodeCur = inc->ptr[jj];
            FeVariable_GetValueAtNode( velField, nodeCur, lambdaVals );

            for( kk = 0; kk < numDofs; kk++ ) {

                eq = eqNum->destinationArray[nodeCur][kk];
                if( eq == -1 )
                    continue;

/*
                eq = *(int*)STreeMap_Map( eqNum->ownedMap, &eq );

                VecGetValues( lambda, 1, &eq, &lambdaVal );
*/

                if( lambdaVals[kk] < 0.0 )
                    printf( "%g\n",  lambdaVals[kk] );
                if( lambdaVals[kk] < lambdaMin )
                    lambdaMin = lambdaVals[kk];

            }
        }

        penaltyVals[ii] = lambdaMin;

    }

    if( lambdaVals ) free( lambdaVals );
    Stg_Class_Delete( inc );

    solVec->vector = vecBackup;

    for( ii = 0; ii < numLocalNodes; ii++ )
        FeVariable_SetValueAtNode( velField, ii, velBackup + ii*numDofs );
    if( velBackup ) free( velBackup );
    FeVariable_SyncShadowValues( velField );

    Stg_VecDestroy(&lambda );
    
    VecRestoreArray( penalty, &penaltyVals );
    VecAssemblyBegin( penalty );
    VecAssemblyEnd( penalty );

    {
        PetscInt idx;
        PetscReal min, max;

        VecMin( penalty, &idx, &min );
        VecMax( penalty, &idx, &max );
        if( rank == 0 ) {
           printf( "SEMI-PENALTY RANGE:\n" );
           printf( "  MIN: %e\n", min );
           printf( "  MAX: %e\n", max );
        }
    }

    *_penalty = penalty;
}

