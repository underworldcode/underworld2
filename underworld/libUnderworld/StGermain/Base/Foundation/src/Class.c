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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Stg_Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Textual name of this class */
const Type Stg_Class_Type = "Stg_Class";


Stg_Class* _Stg_Class_New( SizeT _sizeOfSelf, 
		Type				type, 
		Stg_Class_DeleteFunction*	_delete, 
		Stg_Class_PrintFunction*	_print, 
		Stg_Class_CopyFunction*		_copy )
{
	Stg_Class*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_Class) );
	self = (Stg_Class*)Memory_Alloc_Bytes_Unnamed( _sizeOfSelf, type );
	memset( self, 0, _sizeOfSelf );
	self->_sizeOfSelf = _sizeOfSelf;
	self->_deleteSelf = True;
	
	/* General info */
	self->type = type;
	
	/* Virtual functions */
	self->_delete = _delete;
	self->_print = _print;
	self->_copy = _copy;
	
	/* Stg_Class info */
	_Stg_Class_Init( self );
	
	return self;
}



void _Stg_Class_Init( Stg_Class* self ) {
	/* General and Virtual info should already be set */
	
	/* Stg_Class info */
	self->nRefs = 0;
    
    self->locked = False;
}

void Stg_Class_Delete( void* _class ) {
	Stg_Class* self = (Stg_Class*)_class;
    if(self && !self->locked){
        self->_delete( _class );
    }
}

void _Stg_Class_Delete( void* _class ) {
   Stg_Class* self = (Stg_Class*)_class;

   if( self->_deleteSelf ) {
      Memory_Free( self );
   }
}

void Stg_Class_Print( void* _class, struct Stream* stream ) {
	Stg_Class* self = (Stg_Class*)_class;
	if( _class )
      self->_print( _class, stream );
}

void _Stg_Class_Print( void* _class, struct Stream* stream ) {
	Stg_Class* self = (Stg_Class*)_class;
	
	Journal_Firewall( (Bool)stream, stream, "Attempting to Print to stream that is NULL\n" );
	Journal_Firewall( (Bool)self, stream, "Attempting to Print class that is NULL\n" );
	
	Journal_Printf( stream, "Stg_Class (ptr): %p\n", self );
	Stream_Indent( stream );
	Journal_Printf( stream, "_sizeOfSelf: %lu\n", self->_sizeOfSelf );
	Journal_Printf( stream, "_deleteSelf: %s\n", self->_deleteSelf ? "Yes" : "No" );
	Journal_Printf( stream, "type: %s\n", self->type == Type_Invalid ? "Invalid" : self->type );
	Journal_Printf( stream, "_delete (func ptr): %p\n", self->_delete );
	Journal_Printf( stream, "_print (func ptr): %p\n", self->_print );
	Journal_Printf( stream, "_copy (func ptr): %p\n", self->_copy );
	Stream_UnIndent( stream );
}

void* Stg_Generic_Copy( 
	Stg_Class_CopyFunction* copyFunc,
	void* obj, 
	void* dest, 
	Bool deep, 
	Name nameExt, 
	struct PtrMap* ptrMap ) 
{
	void*		newObj;
	Bool		ownPtrMap = False;

	if ( obj == NULL ) {
		return NULL;
	}

	if ( copyFunc == NULL ) {
		/* TODO: change to Journal */
		printf( "Warning: attempting to copy a class with no copy method, return 'self'.\n" );
		return obj;
	}

	if( !ptrMap ) {
		ptrMap = PtrMap_New( 1 );
		ownPtrMap = True;
	}
	
	if ( (newObj = PtrMap_Find( ptrMap, obj )) == NULL ) {
		newObj = copyFunc( obj, dest, deep, nameExt, ptrMap );
	}

	if ( ownPtrMap && ptrMap ) {
		Stg_Class_Delete( ptrMap );
	}
	
	return (void*)newObj;
}

void* Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_Class* self = (Stg_Class*)_class;
	
	if ( !self ) {
		return NULL;
	}
	if ( !self->_copy ) {
		return NULL;
	}

	return Stg_Generic_Copy( self->_copy, _class, dest, deep, nameExt, ptrMap );
}

void* _Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_Class*	self = (Stg_Class*)_class;
	Stg_Class*	newClass;
	
	if( !dest ) {
		newClass = (Stg_Class*)Memory_Alloc_Bytes_Unnamed( self->_sizeOfSelf, self->type );
	}
	else {
		newClass = (Stg_Class*)dest;
	}
	PtrMap_Append( ptrMap, self, newClass );
	
	newClass->_sizeOfSelf = self->_sizeOfSelf;
	newClass->_deleteSelf = self->_deleteSelf;
	newClass->type = self->type;
	newClass->_delete = self->_delete;
	newClass->_print = self->_print;
	newClass->_copy = self->_copy;
	newClass->nRefs = self->nRefs;
	
	return (void*)newClass;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


Type Stg_Class_GetTypeFunc( void* _class ) {
	Stg_Class* self = (Stg_Class*)_class;
	
	return Stg_Class_GetTypeMacro( self );
}


void Stg_Class_AddRef( void* _class ) {
	Stg_Class*	self = (Stg_Class*)_class;

        if( self )
           self->nRefs++;
}

void Stg_Class_RemoveRef( void* _class ) {
	Stg_Class*	self = (Stg_Class*)_class;

	if( self && !(--self->nRefs) )
		Stg_Class_Delete( self );
}

void Stg_Class_Lock( void* _class ) { ((Stg_Class*)_class)->locked = True; }
void Stg_Class_Unlock( void* _class ) { ((Stg_Class*)_class)->locked = False; }





