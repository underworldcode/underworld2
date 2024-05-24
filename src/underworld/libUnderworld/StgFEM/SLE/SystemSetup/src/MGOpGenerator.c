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
const Type MGOpGenerator_Type = "MGOpGenerator";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MGOpGenerator* _MGOpGenerator_New(  MGOPGENERATOR_DEFARGS  ) {
	MGOpGenerator*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MGOpGenerator) );
	self = (MGOpGenerator*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	/* stuff previously housed in the MatrixSolver class */
	self->solver = malloc( sizeof( MGSolver_PETScData ) );
	self->solver->ksp = PETSC_NULL;
	self->solver->matrix = PETSC_NULL;
	self->solver->inversion = PETSC_NULL;
	self->solver->residual = PETSC_NULL;
	self->solver->curRHS = PETSC_NULL;
	self->solver->curSolution = PETSC_NULL;
	self->solver->expiredResidual = True;
	self->solver->matrixChanged = True;

	/* Virtual info */
	self->setNumLevelsFunc = setNumLevelsFunc;
	self->hasExpiredFunc = hasExpiredFunc;
	self->generateFunc = generateFunc;

	/* MGOpGenerator info */
	_MGOpGenerator_Init( self );

	return self;
}

void _MGOpGenerator_Init( MGOpGenerator* self ) {
	assert( self && Stg_CheckType( self, MGOpGenerator ) );

	self->nLevels = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MGOpGenerator_Delete( void* mgOpGenerator ) {
	MGOpGenerator*	self = (MGOpGenerator*)mgOpGenerator;

	assert( self && Stg_CheckType( self, MGOpGenerator ) );

	/* this stuff was previously taken care of in the MatrixSolver class */
	if( self->solver ) {
            /*if( self->solver->ksp != PETSC_NULL )         Stg_KSPDestroy(&self->solver->ksp );*/
		if( self->solver->matrix != PETSC_NULL )      Stg_MatDestroy(&self->solver->matrix );
		if( self->solver->inversion != PETSC_NULL )   Stg_MatDestroy(&self->solver->inversion );
		if( self->solver->residual != PETSC_NULL )    Stg_VecDestroy(&self->solver->residual );
		if( self->solver->curRHS != PETSC_NULL )      Stg_VecDestroy(&self->solver->curRHS );
		if( self->solver->curSolution != PETSC_NULL ) Stg_VecDestroy(&self->solver->curSolution );
		free( self->solver );
	}

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _MGOpGenerator_Print( void* mgOpGenerator, Stream* stream ) {
	MGOpGenerator*	self = (MGOpGenerator*)mgOpGenerator;
	
	/* Set the Journal for printing informations */
	Stream* mgOpGeneratorStream;
	mgOpGeneratorStream = Journal_Register( InfoStream_Type, (Name)"MGOpGeneratorStream"  );

	assert( self && Stg_CheckType( self, MGOpGenerator ) );

	/* Print parent */
	Journal_Printf( stream, "MGOpGenerator (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void _MGOpGenerator_AssignFromXML( void* mgOpGenerator, Stg_ComponentFactory* cf, void* data ) {
	MGOpGenerator*		self = (MGOpGenerator*)mgOpGenerator;

	assert( self && Stg_CheckType( self, MGOpGenerator ) );
	assert( cf );
}

void _MGOpGenerator_Build( void* mgOpGenerator, void* data ) {
}

void _MGOpGenerator_Initialise( void* mgOpGenerator, void* data ) {
}

void _MGOpGenerator_Execute( void* mgOpGenerator, void* data ) {
}

void _MGOpGenerator_Destroy( void* mgOpGenerator, void* data ) {
}

void _MGOpGenerator_SetNumLevels( void* mgOpGenerator, unsigned nLevels ) {
	MGOpGenerator*		self = (MGOpGenerator*)mgOpGenerator;

	assert( self && Stg_CheckType( self, MGOpGenerator ) );

	self->nLevels = nLevels;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void MGOpGenerator_SetMatrixSolver( void* mgOpGenerator, void* _solver ) {
	MGOpGenerator*	self = (MGOpGenerator*)mgOpGenerator;
	//MatrixSolver*	solver = (MatrixSolver*)_solver;
	KSP		solver = (KSP)_solver;

	assert( self && Stg_CheckType( self, MGOpGenerator ) );
	//assert( solver && Stg_CheckType( solver, MatrixSolver ) );

	//self->solver = solver;
	self->solver->ksp = solver;
}

unsigned MGOpGenerator_GetNumLevels( void* mgOpGenerator ) {
	MGOpGenerator*		self = (MGOpGenerator*)mgOpGenerator;

	assert( self && Stg_CheckType( self, MGOpGenerator ) );

	return self->nLevels;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


