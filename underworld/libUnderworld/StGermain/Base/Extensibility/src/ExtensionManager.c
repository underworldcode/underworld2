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
#include "ExtensionManager.h"

#include "ExtensionInfo.h"
#include "SimpleExtensionInfo.h"
#include "ClassPtrExtensionInfo.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


/* Textual name of this class */
const Type ExtensionManager_Type = "Extension";


ExtensionManager* ExtensionManager_New_OfStruct( 
		Name				name, 
		SizeT				initialSize )
{
	/* Variables set in this function */
	SizeT                          _sizeOfSelf = sizeof(ExtensionManager);
	Type                                  type = ExtensionManager_Type;
	Stg_Class_DeleteFunction*          _delete = _ExtensionManager_Delete;
	Stg_Class_PrintFunction*            _print = _ExtensionManager_Print;
	Stg_Class_CopyFunction*              _copy = _ExtensionManager_Copy;
	void*                      _existingObject = NULL;
	void*                               _array = NULL;
	SizeT                             itemSize = 0;
	ExtensionManager*                       em = NULL;
	Index                                count = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _ExtensionManager_New(  EXTENSIONMANAGER_PASSARGS  );
}

ExtensionManager* ExtensionManager_New_OfExistingObject( 
		Name				name, 
		void*				_existingObject )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(ExtensionManager);
	Type                              type = ExtensionManager_Type;
	Stg_Class_DeleteFunction*      _delete = _ExtensionManager_Delete;
	Stg_Class_PrintFunction*        _print = _ExtensionManager_Print;
	Stg_Class_CopyFunction*          _copy = _ExtensionManager_Copy;
	SizeT                      initialSize = 0;
	void*                           _array = NULL;
	SizeT                         itemSize = 0;
	ExtensionManager*                   em = NULL;
	Index                            count = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _ExtensionManager_New(  EXTENSIONMANAGER_PASSARGS  );
}

ExtensionManager* ExtensionManager_New_OfArray(
		Name                            name,
		void*                           _array,
		SizeT                           itemSize,
		Index                           count )
{
	/* Variables set in this function */
	SizeT                          _sizeOfSelf = sizeof( ExtensionManager );
	Type                                  type = ExtensionManager_Type;
	Stg_Class_DeleteFunction*          _delete = _ExtensionManager_Delete;
	Stg_Class_PrintFunction*            _print = _ExtensionManager_Print;
	Stg_Class_CopyFunction*              _copy = _ExtensionManager_Copy;
	SizeT                          initialSize = 0;
	void*                      _existingObject = NULL;
	ExtensionManager*                       em = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _ExtensionManager_New(  EXTENSIONMANAGER_PASSARGS  );
}

ExtensionManager* ExtensionManager_New_OfExtendedArray(
		Name                            name,
		void*                           _array,
		ExtensionManager*               em,
		Index                           count )
{
	/* Variables set in this function */
	SizeT                          _sizeOfSelf = sizeof(ExtensionManager);
	Type                                  type = ExtensionManager_Type;
	Stg_Class_DeleteFunction*          _delete = _ExtensionManager_Delete;
	Stg_Class_PrintFunction*            _print = _ExtensionManager_Print;
	Stg_Class_CopyFunction*              _copy = _ExtensionManager_Copy;
	SizeT                          initialSize = 0;
	void*                      _existingObject = NULL;
	SizeT                             itemSize = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _ExtensionManager_New(  EXTENSIONMANAGER_PASSARGS  );
}

void ExtensionManager_Init_OfStruct( 
		void*				extension,
		Name				name, 
		SizeT				initialSize )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	
	/* General info */
	self->type = ExtensionManager_Type;
	self->_sizeOfSelf = sizeof(ExtensionManager);
	self->_deleteSelf = False;
		/** Ptr to extensions to existing object. */ \

	/* Virtual info */
	self->_delete = _ExtensionManager_Delete;
	self->_print = _ExtensionManager_Print;
	self->_copy = _ExtensionManager_Copy;
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	
	/* Extension info */
	_ExtensionManager_Init( 
		self, 
		initialSize, 
		NULL,		/* no existing object */
		NULL,		/* no arrays */
		0,
		NULL,
		0 );
}

void ExtensionManager_Init_OfExistingObject( 
		void*				extension,
		Name				name, 
		void*				_existingObject )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	
	/* General info */
	self->type = ExtensionManager_Type;
	self->_sizeOfSelf = sizeof(ExtensionManager);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ExtensionManager_Delete;
	self->_print = _ExtensionManager_Print;
	self->_copy = _ExtensionManager_Copy;
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	
	/* Extension info */
	_ExtensionManager_Init( 
		self, 
		0, 		/* not extending struct here */
		_existingObject,
		NULL,		/* no arrays */
		0,
		NULL,
		0 );
}

void ExtensionManager_Init_OfArray(
		void*                           extension,
		Name                            name,
		void*                           _array,
		SizeT                           itemSize,
		Index                           count )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	
	/* General info */
	self->type = ExtensionManager_Type;
	self->_sizeOfSelf = sizeof(ExtensionManager);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ExtensionManager_Delete;
	self->_print = _ExtensionManager_Print;
	self->_copy = _ExtensionManager_Copy;
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	
	/* Extension info */
	_ExtensionManager_Init( 
		self,
		0,		/* not extending struct here */
		NULL,		/* no existing item */
		_array,
		itemSize,
		NULL,		/* items are not extended */
		count );
}

void ExtensionManager_Init_OfExtendedArray(
		void*                           extension,
		Name                            name,
		void*                           _array,
		ExtensionManager*               em,
		Index                           count )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	
	/* General info */
	self->type = ExtensionManager_Type;
	self->_sizeOfSelf = sizeof(ExtensionManager);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ExtensionManager_Delete;
	self->_print = _ExtensionManager_Print;
	self->_copy = _ExtensionManager_Copy;
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	
	/* Extension info */
	_ExtensionManager_Init( 
		self,
		0,		/* not extending struct here */
		NULL,		/* no existing item */
		_array,
		0,		/* get itemSize from em */
		em,
		count );
}

ExtensionManager* _ExtensionManager_New(  EXTENSIONMANAGER_DEFARGS  )
{
	ExtensionManager* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ExtensionManager) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (ExtensionManager*)_Stg_Object_New(  STG_OBJECT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	_ExtensionManager_Init( self, initialSize, _existingObject, _array, itemSize, em, count );
	
	return self;
}

void _ExtensionManager_Init(
		void*				extension,
		SizeT				initialSize,
		void*				_existingObject,
		void*				_array,
		SizeT				itemSize,
		ExtensionManager*		em,
		Index				count )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	/* General and Virtual info should already be set */

	if ( _array ) {
		if ( em ) {
			/* arrays of extended structs take item size from em */
			itemSize = ExtensionManager_GetFinalSize( em );
		}

		/* if its an array, all sizes should be item size */
		initialSize = itemSize;
	}
	
	self->extInfos = Stg_ObjectList_New();
	/*self->objToExtensionMapper = HashTable_New( Stg_Class_Copy, NULL, Stg_Class_Delete, HASHTABLE_POINTER_KEY );*/

	/* Extension info */
	self->initialSize = initialSize;
	self->finalSize = self->initialSize;
	self->dataCopyFunc = NULL;

	self->_existingObject = _existingObject;
	self->_extensionsToExisting = NULL;
	/*
	if ( self->_existingObject ) {
		HashTable_InsertEntry( 
			self->objToExtensionMapper, 
			self->_existingObject, 
			sizeof(void*),
			HashTable_New( NULL, NULL, NULL, HASHTABLE_POINTER_KEY ),
			sizeof(HashTable) );
	}
	*/
	self->_array = _array;
	self->itemSize = itemSize;
	self->em = em;
	self->count = count;
}


void _ExtensionManager_Delete( void* extension ) {
	ExtensionManager* self = (ExtensionManager*)extension;
	
	if( self->_extensionsToExisting ) {
		Memory_Free( self->_extensionsToExisting );
	}
	
	Stg_Class_Delete( self->extInfos );
	/*Stg_Class_Delete( self->objToExtensionMapper );*/

	/* Stg_Class_Delete parent */
	_Stg_Object_Delete( self );
}

void _ExtensionManager_Print( void* extension, Stream* stream ) {
	ExtensionManager* self = (ExtensionManager*)extension;
	
	/* General info */
	Journal_Printf( (void*) stream, "Extension (ptr):%p\n", self );
	
	/* Print parent */
	_Stg_Object_Print( self, stream );
	
	/* Virtual info */
	
	/* Extension info */
	Journal_Printf( (void*) stream, "\tinitialSize %lu\n", self->initialSize );
	Journal_Printf( (void*) stream, "\tfinalSize %lu\n", self->finalSize );
	Journal_Printf( (void*) stream, "\tcount: %u\n", self->extInfos->count );
	
	Journal_Printf( (void*) stream, "\textInfos (ptr): %p\n", (void*)self->extInfos );
	Stg_Class_Print( self->extInfos, stream );
	
	Journal_Printf( (void*)stream, "\t_extensionsToExisting (ptr): %p\n", self->_extensionsToExisting );
	Journal_Printf( (void*)stream, "\t_existingObject (ptr): %p\n", self->_existingObject );

	Journal_Printf( (void*)stream, "\t_array (ptr): %p\n", self->_array );
	Journal_Printf( (void*)stream, "\titemSize %lu\n", self->itemSize );
	Journal_Printf( (void*)stream, "\tem (ptr) %p\n", self->em );
	Journal_Printf( (void*)stream, "\tcount %u\n", self->count );
}


void* _ExtensionManager_Copy( void* extensionManager, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ExtensionManager*	self = (ExtensionManager*)extensionManager;
	ExtensionManager*	newExtensionManager;
	PtrMap*			map = ptrMap;
	int item_I;
	int ext_I;
	void* data;
	ExtensionInfo* srcInfo;

	newExtensionManager = (ExtensionManager*)_Stg_Object_Copy( self, dest, deep, nameExt, map );
	
	newExtensionManager->initialSize = self->initialSize;
	newExtensionManager->finalSize = self->finalSize;
	newExtensionManager->dataCopyFunc = self->dataCopyFunc;

	/* Of Existing */
	if ( self->_existingObject ) {
		/* ExtensionManager assumes the object is already copied */
		newExtensionManager->_existingObject = PtrMap_Find( map, self->_existingObject );
		Journal_Firewall(
			newExtensionManager->_existingObject != NULL,
			Journal_Register( Error_Type, __FILE__ ),
			"Copy Error: ExtensionManager copied before _existingObject\n" );
	}
	else {
		newExtensionManager->_existingObject = NULL;
	}
	
	if ( self->_extensionsToExisting ) {
		/* OfObject case */
		if( (newExtensionManager->_extensionsToExisting = PtrMap_Find( map, self->_extensionsToExisting )) == NULL && 
			self->_extensionsToExisting )
		{
			Index ext_I;
			ArithPointer offset;
			newExtensionManager->_extensionsToExisting = Memory_Alloc_Bytes( 
				self->finalSize, 
				"Extended data", 
				self->name );
			PtrMap_Append( map, self->_extensionsToExisting, newExtensionManager->_extensionsToExisting );
			offset = 0;
			for ( ext_I = 0; ext_I < self->extInfos->count; ++ext_I ) {
				ExtensionInfo* eInfo = (ExtensionInfo*)Stg_ObjectList_At( self->extInfos, ext_I );
				ExtensionInfo_DataCopy(
					eInfo,
					(void*)((ArithPointer)self->_extensionsToExisting + offset),
					(void*)((ArithPointer)newExtensionManager->_extensionsToExisting + offset ),
					True,
					nameExt,
					map );

				offset += ExtensionInfo_Size( eInfo );
				PtrMap_Append(
					map,
					(void*)((ArithPointer)self->_extensionsToExisting + offset),
					(void*)((ArithPointer)newExtensionManager->_extensionsToExisting + offset ) );
			}
		}
	}
	else {
		newExtensionManager->_extensionsToExisting = NULL;
	}

	if ( self->_array ) {
		/* Array case */
		newExtensionManager->_array = PtrMap_Find( map, self->_array );
		if ( newExtensionManager->_array == NULL ) {
			if ( self->em ) {
				/* ExtendedArray case */
				newExtensionManager->_array = ExtensionManager_CopyAllocation(
					self->em,
					self->_array,
					NULL,
					deep,
					nameExt,
					ptrMap,
					self->count );
				
			}
			else {
				/* OfArray case */
				newExtensionManager->_array = Memory_Alloc_Bytes( 
					self->itemSize * self->count, 
					"Base Type", 
					self->name );
				PtrMap_Append( map, self->_array, newExtensionManager->_array );

			}
		}

		/* Add each item of array to pointer map */
		for ( item_I = 0; item_I < self->count; ++item_I ) {
			PtrMap_Append(
				map,
				(void*)( ((ArithPointer)self->_array) + (self->itemSize * item_I) ),
				(void*)( ((ArithPointer)newExtensionManager->_array) +
					 (self->itemSize * item_I) ) );
		}
		
		/* Copy each item of the array */
		if ( self->dataCopyFunc ) {
			for ( item_I = 0; item_I < self->count; ++item_I ) {
				Stg_Generic_Copy( 
					self->dataCopyFunc,
					(void*)( ((ArithPointer)self->_array) + (self->itemSize * item_I) ),
					(void*)( ((ArithPointer)newExtensionManager->_array) + 
						(self->itemSize * item_I) ),
					deep,
					nameExt,
					ptrMap );
			}
		}
		else {
			memcpy( newExtensionManager->_array, self->_array, self->itemSize * self->count );
		}

		/* Copy the extensions */
		for ( ext_I = 0; ext_I < self->extInfos->count; ++ext_I ) {
			srcInfo = ExtensionInfo_At( self->extInfos, ext_I );
			data = PtrMap_Find( map, srcInfo->data );
			if ( data == NULL ) {
				data = Memory_Alloc_Bytes( 
					srcInfo->size * self->count, 
					"Extended data",
					self->name );
				PtrMap_Append( map, srcInfo->data, data );
			}
			for ( item_I = 0; item_I < self->count; ++item_I ) {
				ExtensionInfo_DataCopy( 
					srcInfo, 
					((void*)((ArithPointer)srcInfo->data + (srcInfo->itemSize * item_I) )),
					((void*)((ArithPointer)data + (srcInfo->itemSize * item_I) )),
					deep,
					nameExt,
					map );
				PtrMap_Append(
					map,
					((void*)((ArithPointer)srcInfo->data + (srcInfo->itemSize * item_I) )),
					((void*)((ArithPointer)data + (srcInfo->itemSize * item_I) )) );
			}
		}
	}
	else {
		newExtensionManager->_array = NULL;
	}

	newExtensionManager->itemSize = self->itemSize;
	newExtensionManager->em = (ExtensionManager*)Stg_Class_Copy( self->em, NULL, deep, nameExt, ptrMap );
	newExtensionManager->count = self->count;

	/* Must be copied after all the data ptrs have been appended to map */
	newExtensionManager->objToExtensionMapper = (HashTable*)Stg_Class_Copy( self->objToExtensionMapper, NULL, deep, nameExt, ptrMap );
	
	/* must be copied last bacause of array case */
	newExtensionManager->extInfos = (ExtensionInfoList*)Stg_Class_Copy( self->extInfos, NULL, deep, nameExt, ptrMap );

	return (void*)newExtensionManager;
}

void ExtensionManager_SetDataCopyFunc( void* extensionManager, Stg_Class_CopyFunction* dataCopyFunc ) {
	ExtensionManager* self = (ExtensionManager*)extensionManager;

	self->dataCopyFunc = dataCopyFunc;
}

ExtensionInfo_Index ExtensionManager_AddExtensionInfo( void* extensionManager, void* extensionInfo ) {
	ExtensionManager* self = (ExtensionManager*)extensionManager;
	ExtensionInfo* newExtension = (ExtensionInfo*) extensionInfo;

	ExtensionInfo_Index	handle;
	SizeT			offset;
	SizeT			oldFinalSize;

	Journal_Firewall(
		self->lockDown == False,
		Journal_MyStream( Error_Type, self ),
		"Error in func %s: Trying to add extension \"%s\" to ExtensionManager \"%s\" when it is in lock down mode "
		"(No more extensions can be added).\n"
		"Please ensure that the source code is not adding to the wrong ExtensionManager or contact the developer.\n",
		__func__,
		newExtension->key,
		self->name );
	
	handle = self->extInfos->count;
	offset = ExtensionManager_Size( self ); /* aligned, size of everything */
	
	if ( self->_array == NULL ) {
		/* Struct or Object case */
		oldFinalSize = ExtensionManager_Size( self );
		
		Stg_ObjectList_Append( self->extInfos, newExtension ); 
		ExtensionInfo_Register( newExtension, offset, self, handle, NULL );

		self->finalSize = ExtensionManager_Size( self );
	
		/* if we are extending an existing object, allocate more memory */
		if( self->_existingObject ) {
			/*
			HashTable* ht = HashTable_FindEntry( self->objToExtensionMapper, self->_existingObject, sizeof(HashTable), HashTable );
			*/
			if (! self->_extensionsToExisting ) {
				self->_extensionsToExisting = Memory_Alloc_Bytes( self->finalSize, "Extended data", self->name );
				/* Use 0 as size of data because its a don't care */
				/*HashTable_InsertEntry( ht, newExtension->key, sizeof(Name), self->_extensionsToExisting, 0 ); */
			} 
			else {
				/* 
				HashTable* newHt;
				Index ext_I;
				void* curPointer;
				*/
				
				self->_extensionsToExisting = Memory_Realloc( self->_extensionsToExisting, self->finalSize );
				memset( 
					(void*)((ArithPointer)self->_extensionsToExisting + (ArithPointer)oldFinalSize), 
					0, 
					self->finalSize - oldFinalSize );
				/* rehash the lot */
				/*
				newHt = HashTable_New( NULL, NULL, NULL, HASHTABLE_POINTER_KEY );
				curPointer = self->_extensionsToExisting;
				for ( ext_I = 0; ext_I < self->extInfos->count; ++ext_I ) {
					ExtensionInfo* eInfo = (ExtensionInfo*)Stg_ObjectList_At( self->extInfos, ext_I );
					HashTable_InsertEntry( newHt, eInfo->key, sizeof(Name), curPointer, 0 );
					curPointer = (void*)((ArithPointer)curPointer + (ArithPointer)ExtensionInfo_Size( eInfo ));
				}
				HashTable_ReplaceEntry( self->objToExtensionMapper, self->_existingObject, sizeof(void*), newHt, sizeof(HashTable) );
				*/
			}
			
		}
		return handle;
	}
	else {
		/* Array or ExtendedArray case */
		void* data;

		/*HashTable* ht;*/
		void* curObj;
		void* curExtData;
		Index obj_I;
		
		/* each extension stores a pointer to array of extended memory */
		/* created the array of extended data */
		data = Memory_Alloc_Bytes( newExtension->size * self->count, "Extended data", self->name );
	
		/* Add the extension.
		 * Note that in this case, offset convention is retained for the purpose of
		 * size calculation and is NOT to be used */
		Stg_ObjectList_Append( self->extInfos, newExtension );
		ExtensionInfo_Register( newExtension, offset, self, handle, data );

		/*
		if ( ExtensionManager_OfExtendedArray( self ) ) {
			ht = self->em->objToExtensionMapper;
		}
		else {
			ht = self->objToExtensionMapper;
		}
		*/
		
		curObj = self->_array;
		curExtData = data;
		for ( obj_I = 0; obj_I < self->count; ++obj_I ) {
			/*
			HashTable* extHt = HashTable_FindEntry( ht, curObj, sizeof(void*), HashTable );
			HashTable_InsertEntry( extHt, newExtension->key, sizeof(Name), curExtData, 0 );
			*/

			curObj = (void*)((ArithPointer)curObj + (ArithPointer)self->itemSize);
			curExtData = (void*)((ArithPointer)curExtData + (ArithPointer)ExtensionInfo_Size( newExtension ) );
		}

		if ( self->em ) {
			return handle + self->em->extInfos->count;
		}
		
		return handle;
	}
		
}
	

ExtensionInfo_Index ExtensionManager_AddArray( 
	void* extension, 
	Name extensionName, 
	SizeT size, 
	Index count )
{
	ExtensionInfo* newExtInfo = (ExtensionInfo*)SimpleExtensionInfo_New( extensionName, ExtensionManager_Align( size ), count );
	return ExtensionManager_AddExtensionInfo( extension, newExtInfo );
}

ExtensionInfo_Index ExtensionManager_AddClassPtrArray( 
	void* extension, 
	Name extensionName, 
	Stg_Class_CopyFunction* copyFunc, 
	Index count ) {

	ExtensionInfo* newExtInfo = (ExtensionInfo*)ClassPtrExtensionInfo_New( extensionName, copyFunc, count );
	return ExtensionManager_AddExtensionInfo( extension, newExtInfo );
}

ExtensionInfo_Index ExtensionManager_GetHandle( void* extension, Name extensionName ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	ExtensionInfo_Index			result = 0;
	ExtensionInfo_Index			offset_I;

	if ( self->em ) {
		result = ExtensionManager_GetHandle( self->em, extensionName );
		if ( result != (unsigned)-1 ) {
			return result;
		}
		/* not found, so the result must be the prev + current extInfos handle if any */
		result = self->em->extInfos->count;
	}
	
	for( offset_I = 0; offset_I < self->extInfos->count; offset_I++ ) {
		if( strcmp( extensionName, ExtensionInfo_At( self->extInfos, offset_I )->key ) == 0 ) {
			return result + offset_I;
		}
	}
	
	return (unsigned)-1;
}

void* ExtensionManager_GetOFunc( void* extension, void* ptr, ExtensionInfo_Index handle ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_GetOMacro( self, ptr, handle );
}

void* ExtensionManager_GetCFunc( void* extension, void* ptr, ExtensionInfo_Index handle ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_GetCMacro( self, ptr, handle );
}

void* ExtensionManager_GetAFunc( void* extension, void* itemPtr, ExtensionInfo_Index handle ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_GetAMacro( self, itemPtr, handle );
}

void* ExtensionManager_GetExtendedAFunc( void* extension, void* itemPtr, ExtensionInfo_Index handle ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_GetExtendedAMacro( self, itemPtr, handle );
}

void* ExtensionManager_GetFunc( void* extension, void* ptr, ExtensionInfo_Index handle ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_GetMacro( self, ptr, handle );
}

void* ExtensionManager_HashGet( void* extension, void* ptr, Name key ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	/*

	HashTable* extHt;
	void* result;
	
	if ( ExtensionManager_OfExtendedArray( self ) ) {
		return ExtensionManager_HashGet( self->em, ptr, key );
	}
	extHt = HashTable_FindEntry( self->objToExtensionMapper, ptr, sizeof(void*), HashTable );
	if ( extHt == NULL ) {
		return NULL;
	}
	result = HashTable_FindEntry( extHt, key, sizeof(Name), void);
	*/
/* Uncomment this code to allow string compare to work */
/*
	if ( result == NULL && key != NULL ) {
		Index ext_I;
		for ( ext_I = 0; ext_I == self->extInfos->count; ++ext_I ) {
			if ( strcmp( key, ExtensionInfo_At( self->extInfos, ext_I )->key ) == 0 ) {
				result = HashTable_FindEntry( self->objToExtensionMapper, ExtensionInfo_At( self->extInfos, ext_I )->key, sizeof(void*), HashTable );
				break;
			}
		}
	}
*/
/*
	return result;
*/
	Journal_Firewall( False, Journal_Register( ErrorStream_Type, self->type ),
		"%s() currently disabled.\n", __func__ );
	return NULL;
}

SizeT ExtensionManager_SizeFunc( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return ExtensionManager_SizeMacro( self );
}

Bool ExtensionManager_OfExistingFunc( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return (Bool)ExtensionManager_OfExistingMacro( self );
}

Bool ExtensionManager_OfArrayFunc( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return (Bool)ExtensionManager_OfArrayMacro( self );
}

Bool ExtensionManager_OfExtendedArrayFunc( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return (Bool)ExtensionManager_OfExtendedArrayMacro( self );
}

SizeT ExtensionManager_GetFinalSizeFunc( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return ExtensionManager_GetFinalSizeMacro( self );
}

SizeT ExtensionManager_AlignFunc( SizeT size ) {
	return ExtensionManager_AlignMacro( size );
}

void* ExtensionManager_Malloc( void* extension, Index count ) {
	ExtensionManager* self = (ExtensionManager*)extension;
	void* result;

	/*Index obj_I;*/
	/*Index ext_I;*/
	void* curObj;

	if ( ExtensionManager_OfExisting( self ) ) {
		return NULL;
	}
	
	result = Memory_Alloc_Array_Bytes( ExtensionManager_GetFinalSize( (self) ), (count), "Extended object", (self)->name );
	memset( result, 0, ExtensionManager_GetFinalSize( self ) * count );

	if ( ExtensionManager_GetFinalSize( self ) > 0 ) {
		curObj = result;
		/*
		for ( obj_I = 0; obj_I < count; ++obj_I ) {
			HashTable* objHt = HashTable_New( NULL, NULL, NULL, HASHTABLE_POINTER_KEY );
			HashTable_InsertEntry( self->objToExtensionMapper, curObj, sizeof(void*), objHt, sizeof(HashTable ) );

			HashTable_InsertEntry( objHt, NULL, sizeof(Name), curObj, 0 );
			curObj = (void*)((ArithPointer)curObj + (ArithPointer)self->initialSize);
			
			for ( ext_I = 0; ext_I < self->extInfos->count; ++ext_I ) {
				ExtensionInfo* eInfo = (ExtensionInfo*)Stg_ObjectList_At( self->extInfos, ext_I );

				HashTable_InsertEntry( objHt, eInfo->key, sizeof(Name), curObj, 0 );

				curObj = (void*)((ArithPointer)curObj + (ArithPointer)ExtensionInfo_Size( eInfo ));
			}
		}
		*/
	}
	
	return result;
}

void* ExtensionManager_CopyAllocation(
		void* extension,
		void* array,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap,
		Index count )
{
	ExtensionManager* self = (ExtensionManager*)extension;
	Bool ownMap = False;
	void* srcCurrent;
	void* destCurrent;
	int item_I;
	int ext_I;

	ArithPointer offset;

	if ( array == NULL ) {
		return NULL;
	}
	if ( !ptrMap ) {
		ptrMap = PtrMap_New( 1 );
		ownMap = True;
	}
	
	if ( dest == NULL ) {
		dest = PtrMap_Find( ptrMap, array );
		if ( dest == NULL ) {
			dest = Memory_Alloc_Bytes( 
				ExtensionManager_GetFinalSizeFunc( self ) * count, 
				"Extended object", 
				self->name );
			PtrMap_Append( ptrMap, array, dest );
		}
	}

	for ( item_I = 0; item_I < count; ++item_I ) {
		if ( self->dataCopyFunc ) {
			Stg_Generic_Copy(
				self->dataCopyFunc,
				(void*)(((ArithPointer)array) + (ExtensionManager_GetFinalSize(self) * item_I)),
				(void*)(((ArithPointer)dest) + (ExtensionManager_GetFinalSize(self)* item_I)),
				deep,
				nameExt,
				ptrMap );
		}
		else {
			memcpy(
				(void*)(((ArithPointer)dest) + (ExtensionManager_GetFinalSize(self)* item_I)),
				(void*)(((ArithPointer)array) + (ExtensionManager_GetFinalSize(self) * item_I)),
				self->initialSize );
		}
		PtrMap_Append( 
			ptrMap,
			(void*)(((ArithPointer)array) + (ExtensionManager_GetFinalSize(self) * item_I)),
			(void*)(((ArithPointer)dest) + (ExtensionManager_GetFinalSize(self)* item_I)) );

		srcCurrent = ExtensionManager_At( self, array, item_I );
		destCurrent = ExtensionManager_At( self, dest, item_I );
		
		/* each extension */
		offset = self->initialSize;
		for ( ext_I = 0; ext_I < self->extInfos->count; ++ext_I ){
			ExtensionInfo_DataCopy( 
				Stg_ObjectList_At( self->extInfos, ext_I ),
				(void*)((ArithPointer)srcCurrent + (ArithPointer)offset),
				(void*)((ArithPointer)destCurrent + (ArithPointer)offset),
				deep,
				nameExt,
				ptrMap );

			offset += ExtensionInfo_At( self->extInfos, ext_I )->size;
		}
	}

	if ( ownMap ) {
		Stg_Class_Delete( ptrMap );
	}

	return dest;
}


void* ExtensionManager_MemsetFunc( void* extension, void* ptr, int value, Index count ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_MemsetMacro( self, ptr, value, count );
}

void ExtensionManager_FreeFunc( void* extension, void* ptr ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	ExtensionManager_FreeMacro( self, ptr );
}

void* ExtensionManager_AtFunc( void* extension, void* ptr, Index index ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	return ExtensionManager_AtMacro( self, ptr, index );
}

void ExtensionManager_SetLockDown( void* extension, Bool lockDown ) {
	ExtensionManager*			self = (ExtensionManager*)extension;
	
	self->lockDown = lockDown;
}
Bool ExtensionManager_GetLockDown( void* extension ) {
	ExtensionManager*			self = (ExtensionManager*)extension;

	return self->lockDown;
}





