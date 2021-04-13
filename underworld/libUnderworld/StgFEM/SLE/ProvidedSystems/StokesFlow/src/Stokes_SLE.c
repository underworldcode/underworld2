/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "StgFEM/Discretisation/src/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/src/SystemSetup.h"
#include "types.h"
#include "Stokes_SLE.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type Stokes_SLE_Type = "Stokes_SLE";

Stokes_SLE* Stokes_SLE_New(
	Name							name,
	FiniteElementContext*	context,
	SLE_Solver*					solver,
	Bool							isNonLinear,
	double						nonLinearTolerance,
	Iteration_Index			nonLinearMaxIterations,
	Bool							killNonConvergent,
	EntryPoint_Register*		entryPoint_Register,
	MPI_Comm						comm,
	StiffnessMatrix*			kStiffMat,
	StiffnessMatrix*			gStiffMat,
	StiffnessMatrix*			dStiffMat,
	StiffnessMatrix*			cStiffMat,
	SolutionVector*			uSolnVec,
	SolutionVector*			pSolnVec,
	ForceVector*				fForceVec,
	ForceVector*				hForceVec )
{
	Stokes_SLE* self = (Stokes_SLE*) _Stokes_SLE_DefaultNew( name );

	self->isConstructed = True;
	_SystemLinearEquations_Init( self, solver, NULL, context, False, isNonLinear, 
      nonLinearTolerance, nonLinearMaxIterations, killNonConvergent, 1,  "", "", entryPoint_Register, comm );
	_Stokes_SLE_Init( self, kStiffMat, gStiffMat, dStiffMat, cStiffMat, uSolnVec, pSolnVec, fForceVec, hForceVec );

	return self;
}

/* Creation implementation / Virtual constructor */
Stokes_SLE* _Stokes_SLE_New(  STOKES_SLE_DEFARGS  )
{
	Stokes_SLE* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stokes_SLE) );
	self = (Stokes_SLE*) _SystemLinearEquations_New(  SYSTEMLINEAREQUATIONS_PASSARGS  );

	/* Virtual info */

	return self;
}

void _Stokes_SLE_Init(
		void*                                               sle,
		StiffnessMatrix*                                    kStiffMat,
		StiffnessMatrix*                                    gStiffMat,
		StiffnessMatrix*                                    dStiffMat,
		StiffnessMatrix*                                    cStiffMat,
		SolutionVector*                                     uSolnVec,
		SolutionVector*                                     pSolnVec,
		ForceVector*                                        fForceVec,
		ForceVector*                                        hForceVec )
{
	Stokes_SLE* self = (Stokes_SLE*)sle;

	self->kStiffMat = kStiffMat;
	self->gStiffMat = gStiffMat;
	self->dStiffMat = dStiffMat;
	self->cStiffMat = cStiffMat;
	self->uSolnVec  = uSolnVec;
	self->pSolnVec  = pSolnVec;
	self->fForceVec = fForceVec;
	self->hForceVec = hForceVec;

        self->null_vector=NULL;

	/* add the vecs and matrices to the Base SLE class's dynamic lists, so they can be
	initialised and built properly */
	SystemLinearEquations_AddStiffnessMatrix( self, kStiffMat );
	SystemLinearEquations_AddStiffnessMatrix( self, gStiffMat );

	if ( dStiffMat )
		SystemLinearEquations_AddStiffnessMatrix( self, dStiffMat );
	if ( cStiffMat )
		SystemLinearEquations_AddStiffnessMatrix( self, cStiffMat );

	SystemLinearEquations_AddSolutionVector( self, uSolnVec );
	SystemLinearEquations_AddSolutionVector( self, pSolnVec );
	SystemLinearEquations_AddForceVector( self, fForceVec );
	SystemLinearEquations_AddForceVector( self, hForceVec );

}

void _Stokes_SLE_Print( void* sle, Stream* stream ) {
	Stokes_SLE* self = (Stokes_SLE*)sle;
	/* Set the Journal for printing informations */

	/* General info */
	Journal_Printf( stream, "Stokes_SLE (ptr): %p\n", self );
	_SystemLinearEquations_Print( self, stream );

	Journal_Printf( stream, "Name of discrete stress tensor (K) matrix = \"%s\" \n",self->kStiffMat->name );
	Journal_Printf( stream, "Name of discrete gradient (G) matrix = \"%s\" \n",self->gStiffMat->name );
	if (self->dStiffMat) {
		Journal_Printf( stream, "Name of discrete divergence (D) matrix = \"%s\" \n",self->dStiffMat->name );
	}
	else {
		Journal_Printf( stream, "No discrete divergence (D) matrix set up (Symmetric geometry).\n" );
	}
	if (self->cStiffMat) {
		Journal_Printf( stream, "Name of compressibility (C) matrix = \"%s\" \n",self->cStiffMat->name );
	}
	else {
		Journal_Printf( stream, "No compressibility (C) matrix set up (incompressible fluids)\n" );
	}
	Journal_Printf( stream, "Name of velocity (u) vector = \"%s\" \n",self->uSolnVec->name );
	Journal_Printf( stream, "Name of pressure (p) vector = \"%s\" \n",self->pSolnVec->name );

	Stg_Class_Print( self->solver, stream );
}

void* _Stokes_SLE_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                            _sizeOfSelf = sizeof(Stokes_SLE);
	Type                                                                    type = Stokes_SLE_Type;
	Stg_Class_DeleteFunction*                                            _delete = _SystemLinearEquations_Delete;
	Stg_Class_PrintFunction*                                              _print = _Stokes_SLE_Print;
	Stg_Class_CopyFunction*                                                _copy = _SystemLinearEquations_Copy;
	Stg_Component_DefaultConstructorFunction*                _defaultConstructor = _Stokes_SLE_DefaultNew;
	Stg_Component_ConstructFunction*                                  _construct = _Stokes_SLE_AssignFromXML;
	Stg_Component_BuildFunction*                                          _build = _SystemLinearEquations_Build;
	Stg_Component_InitialiseFunction*                                _initialise = _SystemLinearEquations_Initialise;
	Stg_Component_ExecuteFunction*                                      _execute = _SystemLinearEquations_Execute;
	Stg_Component_DestroyFunction*                                      _destroy = _SystemLinearEquations_Destroy;
	SystemLinearEquations_LM_SetupFunction*                            _LM_Setup = _SystemLinearEquations_LM_Setup;
	SystemLinearEquations_MatrixSetupFunction*                      _matrixSetup = _SystemLinearEquations_MatrixSetup;
	SystemLinearEquations_VectorSetupFunction*                      _vectorSetup = _SystemLinearEquations_VectorSetup;
	SystemLinearEquations_UpdateSolutionOntoNodesFunc*  _updateSolutionOntoNodes = _SystemLinearEquations_UpdateSolutionOntoNodes;
	SystemLinearEquations_MG_SelectStiffMatsFunc*             _mgSelectStiffMats = _Stokes_SLE_MG_SelectStiffMats;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_Stokes_SLE_New(  STOKES_SLE_PASSARGS  );
}

void _Stokes_SLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data ) {
	Stokes_SLE*       self = (Stokes_SLE*)sle;
	StiffnessMatrix*  kStiffMat;
	StiffnessMatrix*  gStiffMat;
	StiffnessMatrix*  dStiffMat;
	StiffnessMatrix*  cStiffMat;
	SolutionVector*   uSolnVec;
	SolutionVector*   pSolnVec;
	ForceVector*      fForceVec;
	ForceVector*      hForceVec;

	/* Construct Parent */
	_SystemLinearEquations_AssignFromXML( self, cf, data );

	kStiffMat =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"StressTensorMatrix", StiffnessMatrix, True, data  );
	gStiffMat =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"GradientMatrix", StiffnessMatrix, True, data  );
	dStiffMat =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"DivergenceMatrix", StiffnessMatrix, False, data  );
	cStiffMat =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"CompressibilityMatrix", StiffnessMatrix, False, data  );

	uSolnVec  =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityVector", SolutionVector, True, data  );
	pSolnVec  =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"PressureVector", SolutionVector, True, data  );

	fForceVec =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ForceVector", ForceVector, True, data  );
	hForceVec =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ContinuityForceVector", ForceVector, True, data  );

	_Stokes_SLE_Init( self, kStiffMat, gStiffMat, dStiffMat, cStiffMat, uSolnVec, pSolnVec, fForceVec, hForceVec );
}


void _Stokes_SLE_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms ) {
	Stokes_SLE*	self = (Stokes_SLE*)_sle;

	/*
	** In this implementation, only the velocity matrix will have MG applied.
	*/

	*nSMs = 1;
	*sms = Memory_Alloc_Array( StiffnessMatrix*, 1, "Stokes_SLE" );
	(*sms)[0] = self->kStiffMat;
}

double Stokes_MomentumResidual( Stokes_SLE* self ) {
  
  /*
  Calc the residual of the momentum equation
    res = F - [K]u - [G]p
  */ 
  Mat K = self->kStiffMat->matrix;
  Mat G = self->gStiffMat->matrix;
  Vec F = self->fForceVec->vector;
  Vec u = self->uSolnVec->vector;
  Vec p = self->pSolnVec->vector;
  
  Vec                     uStar;
  PetscReal               r1_norm;
  
  VecDuplicate( u, &uStar );
  MatMult( K, u, uStar );                         // {uStar} = [K]{u}
  MatMultAdd( G, p, uStar, uStar );               // {uStar} = {uStar} + [G]{p}
  VecAYPX( uStar, -1.0, F );                      // {uStar} = {F} - {uStar}
  VecNorm( uStar, NORM_2, &r1_norm );             // r_norm = || {uStar} ||_2
  Stg_VecDestroy(&uStar );
  
  return ( (double)(r1_norm) );
}

double Stokes_ContinuityResidual( Stokes_SLE* self ) {
  /*
  Calc the residual of the momentum equation
    res = H - [G^T]u - [C]p
  */
  Mat G = self->gStiffMat->matrix;
  Vec H = self->hForceVec->vector;
  Vec u = self->uSolnVec->vector;
  Vec p = self->pSolnVec->vector;
  Mat C = PETSC_NULL;
  
  if (self->cStiffMat) C = self->cStiffMat->matrix;
  
  Vec                     pStar;
  PetscReal               r2_norm;
  
  VecDuplicate( H, &pStar );
  MatMultTranspose( G, u, pStar );                // {pStar} = [G]^T{u}
  if( C != PETSC_NULL ) {   
    MatMultAdd( C, p, pStar, pStar );	/* {pStar} = {pStar} + [C] {p} */
  }
  VecAYPX( pStar, -1.0, H );                      // {pStar} = {H} - {pStar}

  VecNorm( pStar, NORM_2, &r2_norm );             // norm = || {pStar} ||_2
  Stg_VecDestroy(&pStar );

  return ( (double)(r2_norm) );
}
