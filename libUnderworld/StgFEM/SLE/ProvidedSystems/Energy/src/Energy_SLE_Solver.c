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
#include "Energy_SLE_Solver.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type Energy_SLE_Solver_Type = "Energy_SLE_Solver";

PetscTruth Energy_SLE_HasNullSpace( Mat A );

void* Energy_SLE_Solver_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Energy_SLE_Solver);
	Type                                                      type = Energy_SLE_Solver_Type;
	Stg_Class_DeleteFunction*                              _delete = _Energy_SLE_Solver_Delete;
	Stg_Class_PrintFunction*                                _print = _Energy_SLE_Solver_Print;
	Stg_Class_CopyFunction*                                  _copy = _Energy_SLE_Solver_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = Energy_SLE_Solver_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Energy_SLE_Solver_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Energy_SLE_Solver_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Energy_SLE_Solver_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _SLE_Solver_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _SLE_Solver_Destroy;
	SLE_Solver_SolverSetupFunction*                   _solverSetup = _Energy_SLE_Solver_SolverSetup;
	SLE_Solver_SolveFunction*                               _solve = _Energy_SLE_Solver_Solve;
	SLE_Solver_GetResidualFunc*                       _getResidual = _Energy_SLE_GetResidual;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _Energy_SLE_Solver_New(  ENERGY_SLE_SOLVER_PASSARGS  );
}

Energy_SLE_Solver* Energy_SLE_Solver_New( Name name, Bool useStatSolve, int statReps ) {
	Energy_SLE_Solver* self = (Energy_SLE_Solver*) Energy_SLE_Solver_DefaultNew( name );

	Energy_SLE_Solver_InitAll( self, useStatSolve, statReps ) ;

	return self;
}

/* Creation implementation / Virtual constructor */
Energy_SLE_Solver* _Energy_SLE_Solver_New(  ENERGY_SLE_SOLVER_DEFARGS  )
{
	Energy_SLE_Solver* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Energy_SLE_Solver) );
	self = (Energy_SLE_Solver*) _SLE_Solver_New(  SLE_SOLVER_PASSARGS  );
	
	/* Virtual info */
	return self;
}
	
/* Initialisation implementation */
void _Energy_SLE_Solver_Init( Energy_SLE_Solver* self ) {
	self->isConstructed = True;
	self->residual = NULL;
	//self->ksp = NULL;
	self->ksp = PETSC_NULL;
}
void Energy_SLE_Solver_InitAll( Energy_SLE_Solver* solver, Bool useStatSolve, int statReps ) {
	Energy_SLE_Solver* self = (Energy_SLE_Solver*)solver;

	SLE_Solver_InitAll( self, useStatSolve, statReps );
}

void _Energy_SLE_Solver_Delete( void* sle ) {
	Energy_SLE_Solver* self = (Energy_SLE_Solver*)sle;

	//FreeObject( self->ksp );
	Stg_KSPDestroy(&self->ksp );

	if( self->residual != PETSC_NULL ) {
		//FreeObject( self->residual );
		Stg_VecDestroy(&self->residual );
	}
}

void _Energy_SLE_Solver_Print( void* solver, Stream* stream ) {
	Energy_SLE_Solver* self = (Energy_SLE_Solver*)solver;
	
	/* Set the Journal for printing informations */
	Stream* standard_SLE_SolverStream = stream;
	
	/* General info */
	Journal_Printf( standard_SLE_SolverStream, "Energy_SLE_Solver (ptr): %p\n", self );

	/* other info */
	Journal_Printf( standard_SLE_SolverStream, "\tksp (ptr): %p", self->ksp );
}


void* _Energy_SLE_Solver_Copy( void* standardSleSolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Energy_SLE_Solver*	self = (Energy_SLE_Solver*)standardSleSolver;
	Energy_SLE_Solver*	newEnergySleSolver;

	newEnergySleSolver = _SLE_Solver_Copy( self, dest, deep, nameExt, ptrMap );
	
	newEnergySleSolver->ksp = self->ksp;
	
	return (void*)newEnergySleSolver;
}

void _Energy_SLE_Solver_AssignFromXML( void* sleSolver, Stg_ComponentFactory* cf, void* data ) {
	Energy_SLE_Solver	*self = (Energy_SLE_Solver*)sleSolver;

	assert( self && Stg_CheckType( self, Energy_SLE_Solver ) );
	assert( cf && Stg_CheckType( cf, Stg_ComponentFactory ) );

	_SLE_Solver_AssignFromXML( self, cf, data );
}

/* Build */
void _Energy_SLE_Solver_Build( void* sleSolver, void* standardSLE ) {
	Energy_SLE_Solver*	self = (Energy_SLE_Solver*)sleSolver;
	SystemLinearEquations*	sle = (SystemLinearEquations*) standardSLE;

    /* bail here if we don't have an sle... this function will be called from sle.c */
    if( sle == NULL )
       return;
	StiffnessMatrix*	stiffMat = (StiffnessMatrix*)sle->stiffnessMatrices->data[0];

	Journal_DPrintf( self->debug, "In %s()\n", __func__ );
	Stream_IndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
	Journal_DPrintf( self->debug, "building a standard L.A. solver for the \"%s\" matrix.\n", stiffMat->name );

	Stg_Component_Build( stiffMat, standardSLE, False );

   if( self->ksp == PETSC_NULL ){
		KSPCreate( sle->comm, &self->ksp );
      KSPSetOptionsPrefix( self->ksp, "EnergySolver_" );
   }

	Stream_UnIndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
}


void _Energy_SLE_Solver_Initialise( void* sleSolver, void* standardSLE ) {
	Energy_SLE_Solver*	self = (Energy_SLE_Solver*)sleSolver;
	SystemLinearEquations*	sle = (SystemLinearEquations*) standardSLE;

    /* bail here if we don't have an sle... this function will be called from sle.c */
    if( sle == NULL )
       return;
	
	/* Initialise parent. */
	_SLE_Solver_Initialise( self, sle );
}

void _Energy_SLE_Solver_Execute( void* sleSolver, void* data ) {
}
	
void _Energy_SLE_Solver_Destroy( void* sleSolver, void* data ) {
}

void _Energy_SLE_Solver_SolverSetup( void* sleSolver, void* standardSLE ) {
	Energy_SLE_Solver* self = (Energy_SLE_Solver*)sleSolver;
	SystemLinearEquations* sle = (SystemLinearEquations*) standardSLE;
	StiffnessMatrix*	stiffMat = (StiffnessMatrix*)sle->stiffnessMatrices->data[0];
	
	Journal_DPrintf( self->debug, "In %s()\n", __func__ );
	Stream_IndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
	
	Journal_DPrintf( self->debug, "Initialising the L.A. solver for the \"%s\" matrix.\n", stiffMat->name );
	Stg_KSPSetOperators( self->ksp, stiffMat->matrix, stiffMat->matrix, DIFFERENT_NONZERO_PATTERN );
    KSPSetFromOptions( self->ksp );
	Stream_UnIndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
	
	if( self->maxIterations > 0 ) {
		KSPSetTolerances( self->ksp, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT, self->maxIterations );
	}

	VecDuplicate( ((ForceVector**)sle->forceVectors->data)[0]->vector, &self->residual );
}


void _Energy_SLE_Solver_Solve( void* sleSolver, void* standardSLE ) {
	Energy_SLE_Solver*     self       = (Energy_SLE_Solver*)sleSolver;
	SystemLinearEquations* sle        = (SystemLinearEquations*) standardSLE;
	Iteration_Index        iterations;
        PetscTruth isNull;
        MatNullSpace nullsp;

	
	Journal_DPrintf( self->debug, "In %s - for standard SLE solver\n", __func__ );
	Stream_IndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
	
	if ( (sle->stiffnessMatrices->count > 1 ) ||
		(sle->forceVectors->count > 1 ) )
	{
		Stream* warning = Stream_RegisterChild( StgFEM_Warning, self->type );
		Journal_Printf( warning, "Warning: Energy solver unable to solve more that one matrix/vector.\n" );
	}

	if ( sle->solutionVectors->count > 1 ) {
		Stream* warning = Stream_RegisterChild( StgFEM_Warning, self->type );
		Journal_Printf( warning, "Warning: More than 1 solution vector provided to standard sle. Ignoring second and subsequent"
			" solution vectors.\n" );
	}

    isNull = Energy_SLE_HasNullSpace(((StiffnessMatrix**)sle->stiffnessMatrices->data)[0]->matrix);
    if(isNull) {
      MatNullSpaceCreate(PETSC_COMM_WORLD, PETSC_TRUE, 0, PETSC_NULL, &nullsp);
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR <6) )
      KSPSetNullSpace(self->ksp, nullsp);
#else
      MatSetNullSpace(((StiffnessMatrix**)sle->stiffnessMatrices->data)[0]->matrix, nullsp);
#endif
    }

	KSPSolve( self->ksp,
		    ((ForceVector*) sle->forceVectors->data[0])->vector, 
		    ((SolutionVector*) sle->solutionVectors->data[0])->vector );
	KSPGetIterationNumber( self->ksp, &iterations );

	Journal_DPrintf( self->debug, "Solved after %u iterations.\n", iterations );
	Stream_UnIndentBranch( StgFEM_SLE_ProvidedSystems_Energy_Debug );
	
	/* calculate the residual */
   /* TODO: need to make this optional */
	MatMult( ((StiffnessMatrix**)sle->stiffnessMatrices->data)[0]->matrix, 
		 ((SolutionVector**)sle->solutionVectors->data)[0]->vector, 
		 self->residual );
	VecAYPX( self->residual, -1.0, ((ForceVector**)sle->forceVectors->data)[0]->vector );
    
    if(isNull)
      Stg_MatNullSpaceDestroy(&nullsp);
}


Vec _Energy_SLE_GetResidual( void* sleSolver, Index fv_I ) {
	Energy_SLE_Solver*	self = (Energy_SLE_Solver*)sleSolver;

	return self->residual;
}

PetscTruth Energy_SLE_HasNullSpace( Mat A ) {
    PetscInt N;
    PetscScalar sum;
    PetscReal nrm, Anorm;
    PetscTruth isNull;
    Vec r, l;

    MatGetVecs(A, &r, &l); /*  l=A*r */

    /* Calculate norm(A*r,1)/n/norm(A,1).

       1-norm of vector gives sum of elements so divide by n to get
       average size.
       Then divide by 1-norm of matrix to scale result
       to avoid false positive null space detection.
     */
    VecGetSize(r, &N);
    sum = 1.0/(PetscScalar)N;
    VecSet(r, sum);
    MatMult(A, r, l); /* l=A*r */

    /* 1-norm should be quick enough for sparse matrix */
    MatNorm(A,NORM_1,&Anorm);
    VecNorm(l, NORM_1, &nrm);/* norm(A*r,1)/n */

    if(nrm/Anorm < 1.e-7)
      isNull = PETSC_TRUE;
    else
      isNull = PETSC_FALSE;

    Stg_VecDestroy(&l);
    Stg_VecDestroy(&r);

    return isNull;
}


