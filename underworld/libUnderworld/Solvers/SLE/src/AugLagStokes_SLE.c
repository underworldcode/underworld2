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
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include "types.h"
#include "AugLagStokes_SLE.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type AugLagStokes_SLE_Type = "AugLagStokes_SLE";

/* Creation implementation / Virtual constructor */
AugLagStokes_SLE* _AugLagStokes_SLE_New(  AUGLAGSTOKES_SLE_DEFARGS  )
{
	AugLagStokes_SLE* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(AugLagStokes_SLE) );
	self = (AugLagStokes_SLE*) _Stokes_SLE_New(  STOKES_SLE_PASSARGS  );
	
	/* Virtual info */
	
	return self;
}

void _AugLagStokes_SLE_Init( 
		void*                                             sle, 
		StiffnessMatrix*                                  k2StiffMat,
		StiffnessMatrix*                                  mStiffMat,
		ForceVector*	        			  f2ForceVec,
		ForceVector*	        			  jForceVec,
		double            penaltyNumber,
		double            hFactor,
		StiffnessMatrix*                                  vmStiffMat,
		ForceVector*	        			  vmForceVec
		 )
{
	AugLagStokes_SLE* self = (AugLagStokes_SLE*)sle;

	self->k2StiffMat = k2StiffMat;
	self->f2ForceVec = f2ForceVec;
	self->penaltyNumber = penaltyNumber;
	self->hFactor       = hFactor;
	self->mStiffMat = mStiffMat;
	self->jForceVec = jForceVec;
	self->vmStiffMat = vmStiffMat;
	self->vmForceVec = vmForceVec;

	/* add the vecs and matrices to the Base SLE class's dynamic lists, so they can be 
	initialised and built properly */
	
	
	if (k2StiffMat ) 
	    SystemLinearEquations_AddStiffnessMatrix( self, k2StiffMat );
		
	if (f2ForceVec ) 		
	    SystemLinearEquations_AddForceVector( self, f2ForceVec );

	if (mStiffMat ) 
	    SystemLinearEquations_AddStiffnessMatrix( self, mStiffMat );

	if (jForceVec ) 		
	    SystemLinearEquations_AddForceVector( self, jForceVec );
	
	if (vmStiffMat ) 
	    SystemLinearEquations_AddStiffnessMatrix( self, vmStiffMat );

	if (vmForceVec ) 		
	    SystemLinearEquations_AddForceVector( self, vmForceVec );
	
}

void _AugLagStokes_SLE_Print( void* sle, Stream* stream ) {
	AugLagStokes_SLE* self = (AugLagStokes_SLE*)sle;
	/* Set the Journal for printing informations */
	
	/* General info */
	Journal_Printf( stream, "AugLagStokes_SLE (ptr): %p\n", self );
	_Stokes_SLE_Print( self, stream );

	if (self->k2StiffMat) {
		Journal_Printf( stream, "Name of 2nd discrete stress tensor (K) matrix = \"%s\" \n",self->k2StiffMat->name );
	}
	if (self->mStiffMat) {
		Journal_Printf( stream, "Name of Mass Matrix (M) matrix = \"%s\" \n",self->mStiffMat->name );
	}
	
	
	
	
}

void* _AugLagStokes_SLE_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                            _sizeOfSelf = sizeof(AugLagStokes_SLE);
	Type                                                                    type = AugLagStokes_SLE_Type;
	Stg_Class_DeleteFunction*                                            _delete = _SystemLinearEquations_Delete;
	Stg_Class_PrintFunction*                                              _print = _AugLagStokes_SLE_Print;
	Stg_Class_CopyFunction*                                                _copy = _SystemLinearEquations_Copy;
	Stg_Component_DefaultConstructorFunction*                _defaultConstructor = _AugLagStokes_SLE_DefaultNew;
	Stg_Component_ConstructFunction*                                  _construct = _AugLagStokes_SLE_AssignFromXML;
	Stg_Component_BuildFunction*                                          _build = _SystemLinearEquations_Build;
	Stg_Component_InitialiseFunction*                                _initialise = _SystemLinearEquations_Initialise;
	Stg_Component_ExecuteFunction*                                      _execute = _SystemLinearEquations_Execute;
	Stg_Component_DestroyFunction*                                      _destroy = _SystemLinearEquations_Destroy;
	SystemLinearEquations_LM_SetupFunction*                            _LM_Setup = _SystemLinearEquations_LM_Setup;
	SystemLinearEquations_MatrixSetupFunction*                      _matrixSetup = _SystemLinearEquations_MatrixSetup;
	SystemLinearEquations_VectorSetupFunction*                      _vectorSetup = _SystemLinearEquations_VectorSetup;
	SystemLinearEquations_UpdateSolutionOntoNodesFunc*  _updateSolutionOntoNodes = _SystemLinearEquations_UpdateSolutionOntoNodes;
	SystemLinearEquations_MG_SelectStiffMatsFunc*             _mgSelectStiffMats = _AugLagStokes_SLE_MG_SelectStiffMats;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_AugLagStokes_SLE_New(  AUGLAGSTOKES_SLE_PASSARGS  );
}

void _AugLagStokes_SLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data ) {
	AugLagStokes_SLE*       self = (AugLagStokes_SLE*)sle;
	StiffnessMatrix*  k2StiffMat;
	ForceVector*	  f2ForceVec;
	StiffnessMatrix*  mStiffMat;
	ForceVector*	  jForceVec;
	double            penaltyNumber;
	double            hFactor;
	StiffnessMatrix*  vmStiffMat;
	ForceVector*	  vmForceVec;

	/* Construct Parent */
	_Stokes_SLE_AssignFromXML( self, cf, data );

	k2StiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"2ndStressTensorMatrix", StiffnessMatrix, False, data  );
	f2ForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"2ndForceVector", ForceVector, False, data  );
	penaltyNumber   = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"penaltyNumber", 0.0  );
	hFactor         = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"hFactor", 0.0  );
	mStiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"MassMatrix", StiffnessMatrix, False, data  );
	jForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"JunkForceVector", ForceVector, False, data  );
	vmStiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityMassMatrix", StiffnessMatrix, False, data  );
	vmForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VMassForceVector", ForceVector, False, data  );
	
	_AugLagStokes_SLE_Init( self, k2StiffMat, mStiffMat, f2ForceVec, jForceVec, penaltyNumber, hFactor, vmStiffMat, vmForceVec );
}


void _AugLagStokes_SLE_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms ) {
	AugLagStokes_SLE*	self = (AugLagStokes_SLE*)_sle;
	
	/*
	** In this implementation, only the velocity matrix will have MG applied.
	*/
	
	*nSMs = 1;
	if (self->k2StiffMat)
		*nSMs = 2; 
	
	*sms = Memory_Alloc_Array( StiffnessMatrix*, *nSMs, "AugLagStokes_SLE" );
	(*sms)[0] = self->kStiffMat;
	
	if (self->k2StiffMat)
		(*sms)[1] = self->k2StiffMat;
	
	fprintf(stderr,"There are %d stiffness matrices in the MG list ... \n",*nSMs);		
	fprintf(stderr, "Name of discrete stress tensor (K) matrix = \"%s\" \n",self->kStiffMat->name );
	
	if (self->k2StiffMat) {
		fprintf(stderr,"Name of 2nd discrete stress tensor (K) matrix = \"%s\" \n",self->k2StiffMat->name );
	}
	
	
	
	
}



