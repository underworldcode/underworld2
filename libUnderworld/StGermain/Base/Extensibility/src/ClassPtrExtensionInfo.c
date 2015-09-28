/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "ExtensionManager.h"
#include "ExtensionInfo.h"
#include "ClassPtrExtensionInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Textual name of this class */
const Type ClassPtrExtensionInfo_Type = "ClassPtrExtensionInfo";


/** allocate and initialise a new ClassPtrExtensionInfo. */
ClassPtrExtensionInfo* ClassPtrExtensionInfo_New( const Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(ClassPtrExtensionInfo);
	Type                                    type = ClassPtrExtensionInfo_Type;
	Stg_Class_DeleteFunction*            _delete = _ClassPtrExtensionInfo_Delete;
	Stg_Class_PrintFunction*              _print = _ClassPtrExtensionInfo_Print;
	Stg_Class_CopyFunction*                _copy = _ClassPtrExtensionInfo_Copy;
	ExtensionInfo_DataCopyFunction*    _dataCopy = _ClassPtrExtensionInfo_DataCopy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;
	SizeT                         size = ZERO;

	return _ClassPtrExtensionInfo_New(  CLASSPTREXTENSIONINFO_PASSARGS  );
}


void ClassPtrExtensionInfo_Init( void* arrayExtensionInfo, const Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*)arrayExtensionInfo;

	/* General info */
	self->type = ClassPtrExtensionInfo_Type;
	self->_sizeOfSelf = sizeof(ClassPtrExtensionInfo);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ClassPtrExtensionInfo_Delete;
	self->_print = _ClassPtrExtensionInfo_Print;
	self->_copy = _ClassPtrExtensionInfo_Copy;
	self->_dataCopy = _ClassPtrExtensionInfo_DataCopy;
	
	/* ClassPtrExtensionInfo info */
	_ClassPtrExtensionInfo_Init( self, (Name)name, copyFunc, count );
}


ClassPtrExtensionInfo* _ClassPtrExtensionInfo_New(  CLASSPTREXTENSIONINFO_DEFARGS  )
{
	ClassPtrExtensionInfo* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ClassPtrExtensionInfo) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	size = sizeof(ClassPtrExtensionInfo_PtrClass);

	self = (ClassPtrExtensionInfo*)_ExtensionInfo_New(  EXTENSIONINFO_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* ClassPtrExtensionInfo info */
	_ClassPtrExtensionInfo_Init( self, name, copyFunc, count );
	
	return self;
}


void _ClassPtrExtensionInfo_Init( ClassPtrExtensionInfo* self, Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	/* General and Virtual info should already be set */
	/* ClassPtrExtensionInfo info */
	_ExtensionInfo_Init( (ExtensionInfo*)self, name, sizeof(ClassPtrExtensionInfo_PtrClass), count );
}


void _ClassPtrExtensionInfo_Delete( void* arrayExtensionInfo ) {
	ClassPtrExtensionInfo*	self = (ClassPtrExtensionInfo*)arrayExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" delete a pointer of value NULL\n", ClassPtrExtensionInfo_Type );
	
	/* Delete parent */
	_ExtensionInfo_Delete( self );
}


void _ClassPtrExtensionInfo_Print( void* arrayExtensionInfo, Stream* stream ) {
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*)arrayExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" print a pointer of value NULL\n", ClassPtrExtensionInfo_Type );
	
	/* General info */
	Journal_Printf( (void*)stream, "ClassPtrExtensionInfo (%p):\n", self );
	_ExtensionInfo_Print( self, stream );
}


void* _ClassPtrExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap  ) {
	ClassPtrExtensionInfo*	self = (ClassPtrExtensionInfo*)extensionInfo;
	ClassPtrExtensionInfo*	newClassPtrExtensionInfo;

	/* Copy parent */
	newClassPtrExtensionInfo = (ClassPtrExtensionInfo*)_ExtensionInfo_Copy( self, dest, deep, nameExt, ptrMap );

	newClassPtrExtensionInfo->copyFunc = self->copyFunc;
	
	return newClassPtrExtensionInfo;
}

void* _ClassPtrExtensionInfo_DataCopy(
		void* extensionInfo,
		void* source,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap )
{
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*) extensionInfo;
	ClassPtrExtensionInfo_PtrClass* srcArray = (ClassPtrExtensionInfo_PtrClass*)source;
	ClassPtrExtensionInfo_PtrClass* destArray = (ClassPtrExtensionInfo_PtrClass*)dest;
	int item_I;

	for ( item_I = 0; item_I < self->count; ++item_I ) {
		destArray[item_I].ptr = Stg_Generic_Copy( self->copyFunc, srcArray[item_I].ptr, NULL, deep, nameExt, ptrMap );
	}

	return dest;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


