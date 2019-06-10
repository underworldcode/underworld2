
/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "ObjectAdaptor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type Stg_ObjectAdaptor_Type = "Stg_ObjectAdaptor";


Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfClass( void* dataPtr, Name name, Bool iOwn, Bool isGlobal ) {
	/* Variables set in this function */
	SizeT                                     _sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	Type                                             type = Stg_ObjectAdaptor_Type;
	Stg_Class_DeleteFunction*                     _delete = _Stg_ObjectAdaptor_Delete;
	Stg_Class_PrintFunction*                       _print = _Stg_ObjectAdaptor_Print;
	Stg_Class_CopyFunction*                         _copy = _Stg_ObjectAdaptor_Copy;
	Bool                                       isStgClass = True;
	Stg_ObjectAdaptor_DeletePointerFunction*    ptrDelete = NULL;
	Stg_ObjectAdaptor_PrintPointerFunction*      ptrPrint = NULL;
	Stg_ObjectAdaptor_CopyPointerFunction*        ptrCopy = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _Stg_ObjectAdaptor_New(  STG_OBJECTADAPTOR_PASSARGS  ); 
}


void Stg_ObjectAdaptor_InitOfClass( Stg_ObjectAdaptor* self, void* dataPtr, Name name, Bool iOwn, Bool isGlobal ) {
	/* General info */
	self->type = Stg_ObjectAdaptor_Type;
	self->_sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Stg_ObjectAdaptor_Delete;
	self->_print = _Stg_ObjectAdaptor_Print;
	self->_copy = _Stg_ObjectAdaptor_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );

	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, True, NULL, NULL, NULL );
}


Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfPointer(
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	Type                              type = Stg_ObjectAdaptor_Type;
	Stg_Class_DeleteFunction*      _delete = _Stg_ObjectAdaptor_Delete;
	Stg_Class_PrintFunction*        _print = _Stg_ObjectAdaptor_Print;
	Stg_Class_CopyFunction*          _copy = _Stg_ObjectAdaptor_Copy;
	Bool                        isStgClass = False;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _Stg_ObjectAdaptor_New(  STG_OBJECTADAPTOR_PASSARGS  ); 
}


void Stg_ObjectAdaptor_InitOfPointer( 
		Stg_ObjectAdaptor* 				self, 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	/* General info */
	self->type = Stg_ObjectAdaptor_Type;
	self->_sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Stg_ObjectAdaptor_Delete;
	self->_print = _Stg_ObjectAdaptor_Print;
	self->_copy = _Stg_ObjectAdaptor_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, GLOBAL );

	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, False, ptrDelete, ptrPrint, ptrCopy );
}


Stg_ObjectAdaptor* _Stg_ObjectAdaptor_New(  STG_OBJECTADAPTOR_DEFARGS  )
{
	Stg_ObjectAdaptor* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_ObjectAdaptor) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = GLOBAL;

	self = (Stg_ObjectAdaptor*)_Stg_Object_New(  STG_OBJECT_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, isStgClass, ptrDelete, ptrPrint, ptrCopy );
	
	return self;
}


void _Stg_ObjectAdaptor_Init(
		Stg_ObjectAdaptor*				self, 
		void*						dataPtr, 
		Bool						iOwn, 
		Bool						isGlobal, 
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	self->dataPtr = dataPtr;
	self->iOwn = iOwn;
	self->isGlobal = isGlobal;
	self->isStgClass = isStgClass;
	if( isStgClass ) {
		self->ptrDelete = NULL;
		self->ptrPrint = NULL;
		self->ptrCopy = NULL;
	}
	else {
		self->ptrDelete = ptrDelete;
		self->ptrPrint = ptrPrint;
		self->ptrCopy = ptrCopy;
	}
	
	Journal_Firewall( 
		(Bool)self->dataPtr, 
		Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
		"Constructing a Stg_ObjectAdaptor where there the data pointer is NULL is illegal.\n" );
	if( self->isGlobal ) {
		Journal_Firewall( 
			!self->iOwn, 
			Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
			"Constructing a Stg_ObjectAdaptor where both iOwn and isGlobal are true is illegal.\n" );
	}
}


void _Stg_ObjectAdaptor_Delete( void* objectAdaptor ) {
	Stg_ObjectAdaptor*	self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	if( !self->isGlobal ) {
		if( self->isStgClass ) {
			if( self->iOwn && self->dataPtr ) {
				Stg_Class_Delete( self->dataPtr );
			}
		}
		else {
			if( self->iOwn && self->dataPtr ) {
				if( self->ptrDelete ) {
					self->ptrDelete( self->dataPtr );
				}
				Memory_Free( self->dataPtr );
			}
		}
	}
	self->dataPtr = NULL;
	
	/* Delete parent class */
	_Stg_Object_Delete( self );
}


void _Stg_ObjectAdaptor_Print( void* objectAdaptor, struct Stream* stream ) {
	Stg_ObjectAdaptor*	self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	/* General info */
	Journal_Printf( stream, "Stg_ObjectAdaptor (ptr): %p\n", self );
	Stream_Indent( stream );
	
	_Stg_Object_Print( self, stream );
	
	Journal_Printf( stream, "iOwn: %s\n", self->iOwn ? "Yes" : "No" );
	Journal_Printf( stream, "isGlobal: %s\n", self->isGlobal ? "Yes" : "No" );
	Journal_Printf( stream, "isStgClass: %s\n", self->isStgClass ? "Yes" : "No" );
	
	if( self->isStgClass ) {
		Stg_Class_Print( self->dataPtr, stream );
	}
	else {
		if( self->ptrPrint ) {
			self->ptrPrint( self->dataPtr, stream );
		}
		else {
			Journal_Printf( stream, "Don't know how to print the pointer (it is not to a StGermain class)\n" );
		}
	}
	
	Stream_UnIndent( stream );
}


void* _Stg_ObjectAdaptor_Copy( void* objectAdaptor, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	Stg_ObjectAdaptor* newTuple;

	newTuple = (Stg_ObjectAdaptor*)_Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );
	
	newTuple->isGlobal = self->isGlobal;
	newTuple->isStgClass = self->isStgClass;
	
	Journal_Firewall( deep, Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), "Shallow copy not yet implemented\n" );
	if( deep ) {
		if( self->isGlobal ) {
			newTuple->iOwn = False;
			newTuple->dataPtr = self->dataPtr;
		}
		else {
			newTuple->iOwn = True;
			
			Journal_Firewall( 
				self->iOwn, 
				Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
				"Deep copy for Stg_ObjectAdaptors that do not own the non-global data is not yet implemented\n" );
			
			/* TODO: DOES NOT check if the object has already been copied (i.e. use ptrMap). Beacuse if we assume
			   ownership, we could not do so without an instance count machanism. */
			/* if not in ptr map */
			if( self->isStgClass ) {
				newTuple->dataPtr = Stg_Class_Copy( self->dataPtr, 0, deep, nameExt, ptrMap );
			}
			else {
				Journal_Firewall( 
					self->ptrCopy != NULL, 
					Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
					"Deep copy for pointer based Stg_ObjectAdaptors requires a copy function attached... not provided!\n" );
				newTuple->dataPtr = self->ptrCopy( self->dataPtr, 0, deep, nameExt, ptrMap );
			}
			/* else if in ptr map ... copy ptr value from there. */
		}
	}
	
	return newTuple;
}
	
/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void* Stg_ObjectAdaptor_ObjectFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_ObjectMacro( self );
}


Bool Stg_ObjectAdaptor_IOwnFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IOwnMacro( self );
}


Bool Stg_ObjectAdaptor_IsGlobalFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IsGlobalMacro( self );
}


Bool Stg_ObjectAdaptor_IsStgClassFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IsStgClassMacro( self );
}


/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


