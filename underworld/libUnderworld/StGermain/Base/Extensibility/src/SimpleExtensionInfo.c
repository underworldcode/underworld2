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
#include "SimpleExtensionInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Textual name of this class */
const Type SimpleExtensionInfo_Type = "SimpleExtensionInfo";


/** allocate and initialise a new SimpleExtensionInfo. */
SimpleExtensionInfo* SimpleExtensionInfo_New( const Name name, SizeT size, Index count ) {
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(SimpleExtensionInfo);
	Type                                    type = SimpleExtensionInfo_Type;
	Stg_Class_DeleteFunction*            _delete = _SimpleExtensionInfo_Delete;
	Stg_Class_PrintFunction*              _print = _SimpleExtensionInfo_Print;
	Stg_Class_CopyFunction*                _copy = _SimpleExtensionInfo_Copy;
	ExtensionInfo_DataCopyFunction*    _dataCopy = _SimpleExtensionInfo_DataCopy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _SimpleExtensionInfo_New(  SIMPLEEXTENSIONINFO_PASSARGS  );
}


void SimpleExtensionInfo_Init( void* simpleExtensionInfo, const Name name, SizeT size, Index count ) {
	SimpleExtensionInfo* self = (SimpleExtensionInfo*)simpleExtensionInfo;

	/* General info */
	self->type = SimpleExtensionInfo_Type;
	self->_sizeOfSelf = sizeof(SimpleExtensionInfo);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _SimpleExtensionInfo_Delete;
	self->_print = _SimpleExtensionInfo_Print;
	self->_copy = _SimpleExtensionInfo_Copy;
	self->_dataCopy = _SimpleExtensionInfo_DataCopy;
	
	/* SimpleExtensionInfo info */
	_SimpleExtensionInfo_Init( self, (Name)name, size, count );
}


SimpleExtensionInfo* _SimpleExtensionInfo_New(  SIMPLEEXTENSIONINFO_DEFARGS  )
{
	SimpleExtensionInfo* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SimpleExtensionInfo) );
	self = (SimpleExtensionInfo*)_ExtensionInfo_New(  EXTENSIONINFO_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* SimpleExtensionInfo info */
	_SimpleExtensionInfo_Init( self, name, size, count );
	
	return self;
}


void _SimpleExtensionInfo_Init( SimpleExtensionInfo* self, Name name, SizeT size, Index count ) {
	/* General and Virtual info should already be set */
	/* SimpleExtensionInfo info */
	_ExtensionInfo_Init( (ExtensionInfo*)self, name, size, count );
}


void _SimpleExtensionInfo_Delete( void* simpleExtensionInfo ) {
	SimpleExtensionInfo* self = (SimpleExtensionInfo*)simpleExtensionInfo;

	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" delete a pointer of value NULL\n", SimpleExtensionInfo_Type );
	
	/* Purposely do nothing... the extension is assumed to be static (non-pointer) data */
	
	/* Delete parent */
	_ExtensionInfo_Delete( self );
}


void _SimpleExtensionInfo_Print( void* simpleExtensionInfo, Stream* stream ) {
	SimpleExtensionInfo* self = (SimpleExtensionInfo*)simpleExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" print a pointer of value NULL\n", SimpleExtensionInfo_Type );
	
	/* General info */
	Journal_Printf( (void*)stream, "SimpleExtensionInfo (%p):\n", self );
	_ExtensionInfo_Print( self, stream );
	Journal_Printf( (void*)stream, "\tNo print function implemented/assigned for this extension\n" );
}


void* _SimpleExtensionInfo_Copy( void* simpleExtensionInfo, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap  ) {
	SimpleExtensionInfo*	self = (SimpleExtensionInfo*)simpleExtensionInfo;
	SimpleExtensionInfo*	newSimpleExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, SimpleExtensionInfo_Type ), 
		"Attempting to \"%s\" copy a pointer of value NULL\n", SimpleExtensionInfo_Type );
	
	/* Copy parent */
	newSimpleExtensionInfo = (SimpleExtensionInfo*)_ExtensionInfo_Copy( self, dest, deep, nameExt, ptrMap );
	
	return newSimpleExtensionInfo;
}

void* _SimpleExtensionInfo_DataCopy( 
	void* simpleExtensionInfo, 
	void* source,
	void* dest, 
	Bool deep, 
	Name nameExt, 
	PtrMap* ptrMap )
{
	SimpleExtensionInfo* self = (SimpleExtensionInfo*)simpleExtensionInfo;
	
	memcpy( dest, source, self->size );

	return dest;
}

/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


