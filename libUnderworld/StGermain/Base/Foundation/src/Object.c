/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "CommonRoutines.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type Stg_Object_Type = "Stg_Object";

Index _Stg_Object_Counter = 0;
static const char _Stg_Object_Unnamed[] = "Unnamed";

Stg_Object* _Stg_Object_New(  STG_OBJECT_DEFARGS  )
{
	Stg_Object* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_Object) );
	self = (Stg_Object*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* Stg_Object info */
	_Stg_Object_Init( self, name, nameAllocationType );
	
	return self;
}


void _Stg_Object_Init( Stg_Object* self, Name name, AllocationType nameAllocationType ) {
	/* General and Virtual info should already be set */
	
	/* Stg_Object info */
	if( !name || strlen( name ) == 0 ) {
		Stg_asprintf( &self->name, "%s-%u", _Stg_Object_Unnamed, _Stg_Object_Counter );
		_Stg_Object_Counter += 1;
	}
	else if ( GLOBAL == nameAllocationType ) {
		self->name = name;
	}
	else {
		self->name = StG_Strdup( name );
	}

	self->nameAllocationType = nameAllocationType;	
}


void _Stg_Object_Delete( void* object ) {
	Stg_Object* self = (Stg_Object*)object;
	
	if ( GLOBAL != self->nameAllocationType )
		Memory_Free( self->name );

	/* Delete parent class */
	_Stg_Class_Delete( self );
}


void _Stg_Object_Print( void* object, struct Stream* stream) {
	Stg_Object* self = (Stg_Object*)object;
	
	/* General info */
	Journal_Printf( stream, "Stg_Object (ptr): %p\n", (void*)self );
	Stream_Indent( stream );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	
	/* Stg_Object */
	Journal_Printf( stream, "name: %s\n", self->name );
	Journal_Printf( stream, "nameAllocationType: %s\n", self->nameAllocationType == GLOBAL ? "GLOBAL" : "NON_GLOBAL" );
	
	Stream_UnIndent( stream );
}

void* _Stg_Object_Copy( void* object, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_Object*	self = (Stg_Object*)object;
	Stg_Object*	newObject;
	
	newObject = (Stg_Object*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* TODO: if we are not deep copying we should not copy the name, just the pointer.  There
	 * is a problem with this; will try to fix it later. */
	
	if( nameExt ) {
		unsigned	nameLen = strlen( self->name );
		
		newObject->name = Memory_Alloc_Array_Unnamed( char, nameLen + strlen( nameExt ) + 1 );
		memcpy( newObject->name, self->name, nameLen );
		strcpy( newObject->name + nameLen, nameExt );
	}
	else {
		newObject->name = StG_Strdup( self->name );
	}
	
	newObject->nameAllocationType = NON_GLOBAL;
	
	return newObject;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void Stg_Object_SetName( void* object, Name name )
{
	Stg_Object* self = (Stg_Object*)object;
	
	if ( GLOBAL == self->nameAllocationType )
	{
		self->name = name;
	}
	else
	{
      char* keepPtr = self->name;
		self->name = StG_Strdup( name );
      Memory_Free( keepPtr );

	}
}


/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


Name _Stg_Object_GetNameFunc( void* object ) {
	Stg_Object* self = (Stg_Object*)object;
	
	return _Stg_Object_GetNameMacro( self );
}

Name Stg_Object_AppendSuffix( void* object, Name suffix ) {
	Stg_Object* self = (Stg_Object*)object;
	Name        name;

	Stg_asprintf( &name, "%s-%s", _Stg_Object_GetNameMacro( self ), suffix );

	return name;
}


