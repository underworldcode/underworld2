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
#include <StgFEM/StgFEM.h>

#include "types.h"
#include "PICelleratorContext.h"

/* Textual name of this class */
const Type PICelleratorContext_Type = "PICelleratorContext";

/* Constructors ------------------------------------------------------------------------------------------------*/

PICelleratorContext* PICelleratorContext_New( 
	Name			name,
	double		start,
	double		stop,
	MPI_Comm		communicator,
	Dictionary*	dictionary )
{
	PICelleratorContext* self = _PICelleratorContext_DefaultNew( name );

	self->isConstructed = True;
	_AbstractContext_Init( (AbstractContext*) self );
	_DomainContext_Init( (DomainContext*) self );	
	_FiniteElementContext_Init( (FiniteElementContext*) self );
	_PICelleratorContext_Init( self );

	return self;
}	

PICelleratorContext* _PICelleratorContext_New(  PICELLERATORCONTEXT_DEFARGS  ) {
	PICelleratorContext* self;
	
	/* Allocate memory */
	self = (PICelleratorContext*)_FiniteElementContext_New(  FINITEELEMENTCONTEXT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	return self;
}

void* _PICelleratorContext_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(PICelleratorContext);
	Type                                                      type = PICelleratorContext_Type;
	Stg_Class_DeleteFunction*                              _delete = _PICelleratorContext_Delete;
	Stg_Class_PrintFunction*                                _print = _PICelleratorContext_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _PICelleratorContext_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _PICelleratorContext_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _AbstractContext_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _AbstractContext_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = (Stg_Component_ExecuteFunction*)_AbstractContext_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _PICelleratorContext_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	AbstractContext_SetDt*                                  _setDt = _PICelleratorContext_SetDt;
	double                                               startTime = 0;
	double                                                stopTime = 0;
	MPI_Comm                                          communicator = MPI_COMM_WORLD;
	Dictionary*                                         dictionary = NULL;

	return (void*) _PICelleratorContext_New(  PICELLERATORCONTEXT_PASSARGS  );
}

void _PICelleratorContext_Init( void* context ) {
	PICelleratorContext* self = (PICelleratorContext*)context;
	self->isConstructed = True;
}


/* Virtual Functions -------------------------------------------------------------------------------------------------------------*/

void _PICelleratorContext_Delete( void* context ) {
	PICelleratorContext* self = (PICelleratorContext*)context;
	
	Journal_DPrintf( self->debug, "In: %s()\n", __func__ );

	/* Stg_Class_Delete parent */
	_FiniteElementContext_Delete( self );
}

void _PICelleratorContext_Destroy( void* component, void* data ) {
	PICelleratorContext* self = (PICelleratorContext*)component;
	
	_FiniteElementContext_Destroy( self, data );
}

void _PICelleratorContext_Print( void* context, Stream* stream ) {
	PICelleratorContext* self = (PICelleratorContext*)context;
	
	/* General info */
	Journal_Printf( (void*) stream, "PICelleratorContext (ptr): %p\n", self );
	
	/* Print parent */
	_FiniteElementContext_Print( self, stream );

}

void _PICelleratorContext_SetDt( void* context, double dt ) {
	PICelleratorContext* self = (PICelleratorContext*)context;
	
	self->dt = dt;
}


/* Public Functions ----------------------------------------------------------------------------------------------------*/



/* EntryPoint Hooks ----------------------------------------------------------------------------------------------------*/

void _PICelleratorContext_AssignFromXML( void* context, Stg_ComponentFactory *cf, void* data ){
	PICelleratorContext* self = (PICelleratorContext*) context;
	
	_FiniteElementContext_AssignFromXML( context, cf, data );

	_PICelleratorContext_Init( self );
}


