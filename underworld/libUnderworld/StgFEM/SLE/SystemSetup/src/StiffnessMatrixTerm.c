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


#include "types.h"

#include "FiniteElementContext.h"
#include "StiffnessMatrixTerm.h"
#include "SolutionVector.h"
#include "StiffnessMatrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "EntryPoint.h"

/* Textual name of this class */
const Type StiffnessMatrixTerm_Type = "StiffnessMatrixTerm";

StiffnessMatrixTerm* StiffnessMatrixTerm_New(
		Name                                                 name,
		FiniteElementContext*                                context,
		StiffnessMatrix*                                     stiffnessMatrix,
		Swarm*                                               integrationSwarm,
		Stg_Component*                                       extraInfo )		
{
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*) _StiffnessMatrixTerm_DefaultNew( name );

	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, integrationSwarm, extraInfo );

	return self;
}

StiffnessMatrixTerm* _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_DEFARGS  )
{
	StiffnessMatrixTerm*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(StiffnessMatrixTerm) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (StiffnessMatrixTerm*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	self->_assembleElement = _assembleElement;
	
	return self;
}


void _StiffnessMatrixTerm_Init(
		void*                                                stiffnessMatrixTerm,
		FiniteElementContext*				                    context,
		StiffnessMatrix*                                     stiffnessMatrix,
		Swarm*                                               integrationSwarm,
		Stg_Component*                                       extraInfo )
{
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)  stiffnessMatrixTerm;
	
	self->isConstructed		= True;
	self->context				= context;
	self->debug					= Journal_MyStream( Debug_Type, self );
	self->extraInfo			= extraInfo;	
	self->integrationSwarm	= integrationSwarm;	
	self->stiffnessMatrix	= stiffnessMatrix;
	self->max_nElNodes		= 0; /* initialise to zero, in assembly routine it will change value */
	self->GNx					= NULL;
    
    if(stiffnessMatrix)
      StiffnessMatrix_AddStiffnessMatrixTerm( stiffnessMatrix, self );
}

void _StiffnessMatrixTerm_Delete( void* stiffnessMatrixTerm ) {
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	
	Journal_DPrintf( self->debug, "In %s - for %s\n", __func__, self->name );

	/* Stg_Class_Delete parent*/
	_Stg_Component_Delete( self );
}


void _StiffnessMatrixTerm_Print( void* stiffnessMatrixTerm, Stream* stream ) {
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;

	/* General info */
	Journal_Printf( stream, "StiffnessMatrixTerm (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* StiffnessMatrixTerm info */
	Journal_Printf( stream, "\tintegrationSwarm (ptr): %p\n", self->integrationSwarm );
	Journal_Printf( stream, "\textraInfo (ptr): %p\n", self->extraInfo );
}


void* _StiffnessMatrixTerm_Copy( void* stiffnessMatrixTerm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	StiffnessMatrixTerm*	self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	StiffnessMatrixTerm*	newStiffnessMatrixTerm;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newStiffnessMatrixTerm = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	newStiffnessMatrixTerm->extraInfo = self->extraInfo;
	if( deep ) {
		newStiffnessMatrixTerm->integrationSwarm = (Swarm*)Stg_Class_Copy( self->integrationSwarm, NULL, deep, nameExt, map );
	}
	else {
		newStiffnessMatrixTerm->integrationSwarm = self->integrationSwarm;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newStiffnessMatrixTerm;
}

void* _StiffnessMatrixTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(StiffnessMatrixTerm);
	Type                                                         type = StiffnessMatrixTerm_Type;
	Stg_Class_DeleteFunction*                                 _delete = _StiffnessMatrixTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _StiffnessMatrixTerm_Print;
	Stg_Class_CopyFunction*                                     _copy = _StiffnessMatrixTerm_Copy;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _StiffnessMatrixTerm_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _StiffnessMatrixTerm_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _StiffnessMatrixTerm_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _StiffnessMatrixTerm_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _StiffnessMatrixTerm_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _StiffnessMatrixTerm_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _StiffnessMatrixTerm_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
}

void _StiffnessMatrixTerm_AssignFromXML( void* stiffnessMatrixTerm, Stg_ComponentFactory* cf, void* data ) {
	StiffnessMatrixTerm*       self               = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	Swarm*                     swarm              = NULL;
	Stg_Component*             extraInfo;
	StiffnessMatrix*           stiffnessMatrix;
   FiniteElementContext*      context;
   
	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", FiniteElementContext, False, data );
	if( !context  )
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", FiniteElementContext, False, data  );

	stiffnessMatrix = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"StiffnessMatrix", StiffnessMatrix, False, data  ) ;
	swarm           = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", Swarm, True, data  ) ;
	extraInfo       = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ExtraInfo", Stg_Component, False, data  );

	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, swarm, extraInfo );
}

void _StiffnessMatrixTerm_Build( void* stiffnessMatrixTerm, void* data ) {
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	
	Journal_DPrintf( self->debug, "In %s - for %s\n", __func__, self->name );
	Stream_IndentBranch( StgFEM_Debug );
	
	/* ensure integrationSwarm is built */
	Stg_Component_Build( self->integrationSwarm, data, False );

	if ( self->extraInfo ) 
		Stg_Component_Build( self->extraInfo, data, False );
		
	Stream_UnIndentBranch( StgFEM_Debug );
}


void _StiffnessMatrixTerm_Initialise( void* stiffnessMatrixTerm, void* data ) {
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	
	Journal_DPrintf( self->debug, "In %s - for %s\n", __func__, self->name );
	Stream_IndentBranch( StgFEM_Debug );

	Stg_Component_Initialise( self->integrationSwarm, data, False );
	if ( self->extraInfo ) 
		Stg_Component_Initialise( self->extraInfo, data, False );
	
	Stream_UnIndentBranch( StgFEM_Debug );
}

void _StiffnessMatrixTerm_Execute( void* stiffnessMatrixTerm, void* data ) {
}

void _StiffnessMatrixTerm_Destroy( void* stiffnessMatrixTerm, void* data ) {
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	/* free GNx memory */
	if( self->GNx ) Memory_Free( self->GNx );
}

void StiffnessMatrixTerm_AssembleElement( 
			void*                             stiffnessMatrixTerm, 
			StiffnessMatrix*                  stiffnessMatrix, 
			Element_LocalIndex                lElement_I,
			SystemLinearEquations*            sle,
			FiniteElementContext*             context,
			double**                          elStiffMatToAdd ) 
{
	StiffnessMatrixTerm* self = (StiffnessMatrixTerm*)stiffnessMatrixTerm;

	self->_assembleElement( self, stiffnessMatrix, lElement_I, sle, context, elStiffMatToAdd );
}
	
void _StiffnessMatrixTerm_AssembleElement( 
			void*                             stiffnessMatrixTerm, 
			StiffnessMatrix*                  stiffnessMatrix, 
			Element_LocalIndex                lElement_I,
			SystemLinearEquations*            sle,
			FiniteElementContext*             context,
			double**                          elStiffMatToAdd ) 
{
	StiffnessMatrixTerm* self        = (StiffnessMatrixTerm*)stiffnessMatrixTerm;
	Stream*    errorStream = Journal_Register( Error_Type, (Name)self->type  );

	Journal_Printf( errorStream, "Error in func %s for %s '%s' - "
			"This function is the default function which should never be called - "
			"Please set this virtual function with appropriate application dependent function.\n",
			__func__, self->type, self->name );
	abort();
}	

void StiffnessMatrixTerm_SetAssembleElementFunction( void* stiffnessMatrixTerm, StiffnessMatrixTerm_AssembleElementFunction* assembleElementFunction ) {
	StiffnessMatrixTerm* self        = (StiffnessMatrixTerm*)stiffnessMatrixTerm;

	self->_assembleElement = assembleElementFunction;
}


