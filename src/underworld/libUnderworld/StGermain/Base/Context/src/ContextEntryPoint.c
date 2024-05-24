/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "ContextEntryPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

/* Textual name of this class */
const Type ContextEntryPoint_Type = "ContextEntryPoint";


ContextEntryPoint* ContextEntryPoint_New( const Name name, unsigned int castType ) {
	/* Variables set in this function */
	SizeT                       _sizeOfSelf = sizeof(ContextEntryPoint);
	Type                               type = ContextEntryPoint_Type;
	Stg_Class_DeleteFunction*       _delete = _EntryPoint_Delete;
	Stg_Class_PrintFunction*         _print = _EntryPoint_Print;
	Stg_Class_CopyFunction*           _copy = NULL;
	EntryPoint_GetRunFunction*      _getRun = _ContextEntryPoint_GetRun;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _ContextEntryPoint_New(  CONTEXTENTRYPOINT_PASSARGS  );
}

void ContextEntryPoint_Init( void* contextEntryPoint, Name name, unsigned int castType ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	
	/* General info */
	self->type = ContextEntryPoint_Type;
	self->_sizeOfSelf = sizeof(ContextEntryPoint);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _EntryPoint_Delete;
	self->_print = _EntryPoint_Print;
	self->_copy = NULL;
	self->_getRun = _ContextEntryPoint_GetRun;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, GLOBAL );
	_EntryPoint_Init( (EntryPoint*)self, castType );
	
	/* ContextEntryPoint info */
	_ContextEntryPoint_Init( self );
}

ContextEntryPoint* _ContextEntryPoint_New(  CONTEXTENTRYPOINT_DEFARGS  )
{
	ContextEntryPoint* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ContextEntryPoint) );
	self = (ContextEntryPoint*)_EntryPoint_New(  ENTRYPOINT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* ContextEntryPoint info */
	_ContextEntryPoint_Init( self );
	
	return self;
}

void _ContextEntryPoint_Init( ContextEntryPoint* self ) {
	/* General and Virtual info should already be set */
	
	/* ContextEntryPoint info */
}


Func_Ptr _ContextEntryPoint_GetRun( void* contextEntryPoint ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	
	switch( self->castType ) {
		case ContextEntryPoint_Dt_CastType:
			return (Func_Ptr)_ContextEntryPoint_Run_Dt;
		
		case ContextEntryPoint_Step_CastType:
			return (Func_Ptr)_ContextEntryPoint_Run_Step;
		
		default:
			return (Func_Ptr)_EntryPoint_GetRun( self );
	}
}


double _ContextEntryPoint_Run_Dt( void* contextEntryPoint, void* data0 ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	Hook_Index hookIndex;
	double result = 0.0;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _ContextEntryPoint_Run_Dt, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		result = ((ContextEntryPoint_Dt_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0 );
	}
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif

	return result;
}

void _ContextEntryPoint_Run_Step( void* contextEntryPoint, void* data0, double data1 ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	Hook_Index hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _ContextEntryPoint_Run_Step, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		((ContextEntryPoint_Step_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0, data1 );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}



