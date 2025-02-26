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

#include "types.h"
#include "shortcuts.h"
#include "VariableCondition_Register.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type VariableCondition_Register_Type = "VariableCondition_Register";


/* Global, default instantiation of the register... will be loaded with in built types (built in Init.c) */
VariableCondition_Register* variableCondition_Register = 0;


VariableCondition_Register* VariableCondition_Register_New(void) {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(VariableCondition_Register);
	Type                              type = VariableCondition_Register_Type;
	Stg_Class_DeleteFunction*      _delete = _VariableCondition_Register_Delete;
	Stg_Class_PrintFunction*        _print = _VariableCondition_Register_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

	return _VariableCondition_Register_New(  VARIABLECONDITION_REGISTER_PASSARGS  );
}

void VariableCondition_Register_Init( void* variableCondition_Register ) {
	VariableCondition_Register* self = (VariableCondition_Register*)variableCondition_Register;
	
	/* General info */
	self->type = VariableCondition_Register_Type;
	self->_sizeOfSelf = sizeof(VariableCondition_Register);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _VariableCondition_Register_Delete;
	self->_print = _VariableCondition_Register_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* VariableCondition_Register info */
	_VariableCondition_Register_Init( self );
}

VariableCondition_Register* _VariableCondition_Register_New(  VARIABLECONDITION_REGISTER_DEFARGS  )
{
	VariableCondition_Register* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(VariableCondition_Register) );
	self = (VariableCondition_Register*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	_VariableCondition_Register_Init( self );
	
	return self;
}

void _VariableCondition_Register_Init( void* variableCondition_Register ) {
	VariableCondition_Register* self = (VariableCondition_Register*)variableCondition_Register;
	
	/* General and Virtual info should already be set */
	
	/* VariableCondition_Register info */
	self->count = 0;
	self->_size = 8;
	self->_delta = 8;
	self->entry = Memory_Alloc_Array( VariableCondition_Register_Entry, self->_size, "VC_Register->entry" );
	memset( self->entry, 0, sizeof(VariableCondition_Register_Entry) * self->_size );
}

void _VariableCondition_Register_Delete( void* variableCondition_Register ) {
	VariableCondition_Register* self = (VariableCondition_Register*)variableCondition_Register;
	
	/* Assumes ownerships of the element types */
	if( self->entry ) {
		Memory_Free( self->entry );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}

void _VariableCondition_Register_Print( void* variableCondition_Register, Stream* stream ) {
	VariableCondition_Register* self = (VariableCondition_Register*)variableCondition_Register;
	#ifdef DEBUG
		VariableCondition_Index variableCondition_I;
	#endif
	
	/* Set the Journal for printing informations */
	Stream* variableCondition_RegisterStream = stream;
	
	/* General info */
	Journal_Printf( variableCondition_RegisterStream, "VariableCondition_Register (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self, variableCondition_RegisterStream );
	
	/* Virtual info */
	
	/* VariableCondition_Register info */
	Journal_Printf( variableCondition_RegisterStream, "\tcount: %u\n", self->count );
	Journal_Printf( variableCondition_RegisterStream, "\t_size: %lu\n", self->_size );
	Journal_Printf( variableCondition_RegisterStream, "\t_delta: %lu\n", self->_delta );
	
	Journal_Printf( variableCondition_RegisterStream, "\tvariableCondition (ptr): %p\n", self->entry );
	Journal_Printf( variableCondition_RegisterStream, "\tvariableCondition[0-%u]:\n", self->count );
	#ifdef DEBUG
		for( variableCondition_I = 0; variableCondition_I < self->count; variableCondition_I++ ) {
			Journal_Printf( variableCondition_RegisterStream, "\tentry[%u]:\n", variableCondition_I );
			Journal_Printf( variableCondition_RegisterStream, "\t\ttype: %s\n", self->entry[variableCondition_I].type );
			Journal_Printf( variableCondition_RegisterStream, "\t\tfactory (func ptr): %p\n", self->entry[variableCondition_I].factory );
		}
	#else
		Journal_Printf( variableCondition_RegisterStream, "...\n" );
	#endif
	Journal_Printf( variableCondition_RegisterStream, "\t]\n" );
}

VariableCondition_Index VariableCondition_Register_Add(
		void*					variableCondition_Register, 
		Type					type, 
		VariableCondition_Register_FactoryFunc*	factory )
{
	VariableCondition_Register*	self = (VariableCondition_Register*)variableCondition_Register;
	VariableCondition_Index	handle;
	
	if( self->count >= self->_size ) {
		SizeT currentSize = self->_size;

		self->_size += self->_delta;
		self->entry = Memory_Realloc_Array( self->entry, VariableCondition_Register_Entry, self->_size );
		memset( (Pointer)((ArithPointer)self->entry + (currentSize * sizeof(VariableCondition_Register_Entry)) ),
			0, sizeof(VariableCondition_Register_Entry) * (self->_size - currentSize) );
	}
	
	handle = self->count;
	self->entry[handle].type = type;
	self->entry[handle].factory = factory;
	self->count++;
	
	return handle;
}

VariableCondition_Index VariableCondition_Register_GetIndex( void* variableCondition_Register, Type type ) {
	VariableCondition_Register*	self = (VariableCondition_Register*)variableCondition_Register;
	VariableCondition_Index variableCondition_I;
	
	for( variableCondition_I = 0; variableCondition_I < self->count; variableCondition_I++ ) {
		if( self->entry[variableCondition_I].type == type ) {
			return variableCondition_I;
		}
	}
	/* type is likely to be given from input file in this case... treat comparison checks like for Name... do strcmp too */
	for( variableCondition_I = 0; variableCondition_I < self->count; variableCondition_I++ ) {
		if( strcmp( self->entry[variableCondition_I].type, type ) == 0 ) {
			return variableCondition_I;
		}
	}
	return (unsigned)-1;
}

VariableCondition_Register_Entry* _VariableCondition_Register_At( 
		void*					variableCondition_Register, 
		VariableCondition_Index			handle ) 
{
	VariableCondition_Register*	self = (VariableCondition_Register*)variableCondition_Register;
	
	return VariableCondition_Register_At( self, handle );
}

VariableCondition* VariableCondition_Register_CreateNew(
	void*	context,
	void* variableCondition_Register, 
	void* variable_Register, 
	void* conFunc_Register, 
	Type type, 
	void* dictionary, 
	void* data )
{
	VariableCondition_Register*  self = (VariableCondition_Register*)variableCondition_Register;
        VariableCondition_Index      variableTypeIndex = VariableCondition_Register_GetIndex( self, type );
        if( variableTypeIndex == (unsigned)-1 )
            Journal_Firewall( 0, Journal_Register( Error_Type, (Name)self->type  ), "No VariableCondition of type '%s' found", type );

	return VariableCondition_Register_At( self, variableTypeIndex )->factory( (AbstractContext*)context,
		(Variable_Register*)variable_Register,
		(ConditionFunction_Register*)conFunc_Register,
		(Dictionary*)dictionary,
		data );
}


