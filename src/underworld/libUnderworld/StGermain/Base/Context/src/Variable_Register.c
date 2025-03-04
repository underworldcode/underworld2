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

#include "types.h"
#include "shortcuts.h"
#include "Variable.h"
#include "Variable_Register.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


const Type Variable_Register_Type = "Variable_Register";


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

Variable_Register* Variable_Register_New(void)
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Variable_Register);
	Type                              type = Variable_Register_Type;
	Stg_Class_DeleteFunction*      _delete = _Variable_Register_Delete;
	Stg_Class_PrintFunction*        _print = _Variable_Register_Print;
	Stg_Class_CopyFunction*          _copy = _Variable_Register_Copy;
 
	return _Variable_Register_New(  VARIABLE_REGISTER_PASSARGS  );
}


void Variable_Register_Init(Variable_Register* self)
{ 
	/* General info */
	self->type = Variable_Register_Type;
	self->_sizeOfSelf = sizeof(Variable_Register);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Variable_Register_Delete;
	self->_print = _Variable_Register_Print;
	self->_copy = _Variable_Register_Copy;
	
	_Stg_Class_Init((Stg_Class*)self);
	
	/* Stg_Class info */
	_Variable_Register_Init(self);
}


Variable_Register* _Variable_Register_New(  VARIABLE_REGISTER_DEFARGS  )
{
	Variable_Register*	self;
	
	/* Allocate memory */
	self = (Variable_Register*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* Virtual info */
	
	/* Stg_Class info */
	_Variable_Register_Init(self);
	
	return self;
}


void _Variable_Register_Init(void* variable_Register)
{ 
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	self->count = 0;
	self->_size = 8;
	self->_delta = 8;
	self->_variable = Memory_Alloc_Array( StgVariable*, self->_size, "Variable_Register->_variable" );
	memset(self->_variable, 0, sizeof(StgVariable*)*self->_size);
	self->errorStream = Journal_Register( Error_Type, self->type );
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _Variable_Register_Delete(void* variable_Register)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	if (self->_variable) Memory_Free(self->_variable);

	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete(self);
}

void _Variable_Register_Print(void* variable_Register, Stream* stream)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	/* Set the Journal for printing informations */
	Stream* variable_RegisterStream = Journal_Register( InfoStream_Type,	"Variable_RegisterStream");
	
	/* General info */
	Journal_Printf( variable_RegisterStream, "Variable_Register (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( variable_RegisterStream, "\tcount: %u\n", self->count);
	Journal_Printf( variable_RegisterStream, "\t_size: %lu\n", self->_size);
	Journal_Printf( variable_RegisterStream, "\t_delta: %lu\n", self->_delta);
	Journal_Printf( variable_RegisterStream, "\t_variable (ptr): %p\n", self->_variable);
	if (self->_variable)
	{
		StgVariable_Index	var_I;
		
		for (var_I = 0; var_I < self->count; var_I++)
			Stg_Class_Print(self->_variable[var_I], stream);
	}
	
	/* Print parent */
	_Stg_Class_Print(self, variable_RegisterStream);
}

void* _Variable_Register_Copy( void* vr, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {

	Variable_Register* self = (Variable_Register*)vr;
	Variable_Register* newVariableRegister;
	int ii;

	newVariableRegister = (Variable_Register*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newVariableRegister );

	newVariableRegister->count = self->count;
	newVariableRegister->_size = self->_size;
	newVariableRegister->_variable = Memory_Alloc_Array( StgVariable*, self->_size, "Variable_Register->_variable" );
	memset(newVariableRegister->_variable, 0, sizeof(StgVariable*)*self->_size);

	for ( ii = 0; ii < self->count; ++ii ) {
		if ( (newVariableRegister->_variable[ii] = (StgVariable*)PtrMap_Find( ptrMap, self->_variable[ii] )) == NULL ) {
			newVariableRegister->_variable[ii] = (StgVariable*)Stg_Class_Copy( self->_variable[ii], NULL, deep, nameExt, ptrMap );
		}
	}

	return newVariableRegister;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/

StgVariable_Index Variable_Register_Add(void* variable_Register, StgVariable* variable)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	StgVariable_Index		handle;
	
	if (self->count >= self->_size)
	{
		SizeT currentSize = self->_size;

		self->_size += self->_delta;
		self->_variable = Memory_Realloc_Array( self->_variable, StgVariable*, self->_size );
		memset( (Pointer)((ArithPointer)self->_variable + (currentSize * sizeof(StgVariable*)) ), 0, 
			sizeof(StgVariable*) * (self->_size - currentSize) );
	}
	
	handle = self->count++;
	self->_variable[handle] = variable;

	return handle;
}


void Variable_Register_BuildAll(void* variable_Register)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	StgVariable_Index		var_I;
	
	for (var_I = 0; var_I < self->count; var_I++)
		Stg_Component_Build( self->_variable[var_I], 0, False );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

StgVariable_Index Variable_Register_GetIndex(void* variable_Register, Name name)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	StgVariable_Index		var_I;
	
	for (var_I = 0; var_I < self->count; var_I++)
	{
		if (self->_variable[var_I]->name && !strcmp(name, self->_variable[var_I]->name))
			return var_I;
	}
	
	return (StgVariable_Index)-1;
}


StgVariable* Variable_Register_GetByName(void* variable_Register, Name name)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	StgVariable_Index		varIndex;
	
	if( (varIndex = Variable_Register_GetIndex( self, name )) != (StgVariable_Index)-1 )
		return self->_variable[varIndex];
	
	return NULL;
}


StgVariable* Variable_Register_GetByIndex(void* variable_Register, StgVariable_Index varIndex ) {
	Variable_Register*	self = (Variable_Register*)variable_Register;
	Journal_DFirewall(  (varIndex < self->count), self->errorStream,
		"Error: Given variable index %d not between 0 and variable count %d.\n",
		varIndex, self->count );
		
	return  self->_variable[varIndex];
}


