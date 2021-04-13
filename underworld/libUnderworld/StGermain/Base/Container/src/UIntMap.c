/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "UIntMap.h"


typedef struct {
	unsigned	curItem;
	unsigned*	keys;
	unsigned*	vals;
} UIntMap_ParseStruct;


/* Textual name of this class */
const Type UIntMap_Type = "UIntMap";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

UIntMap* UIntMap_New() {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(UIntMap);
	Type                              type = UIntMap_Type;
	Stg_Class_DeleteFunction*      _delete = _UIntMap_Delete;
	Stg_Class_PrintFunction*        _print = _UIntMap_Print;
	Stg_Class_CopyFunction*          _copy = _UIntMap_Copy;

	return _UIntMap_New(  UINTMAP_PASSARGS  );
}

UIntMap* _UIntMap_New(  UINTMAP_DEFARGS  ) {
	UIntMap* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(UIntMap) );
	self = (UIntMap*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */

	/* UIntMap info */
	_UIntMap_Init( self );

	return self;
}

void _UIntMap_Init( UIntMap* self ) {
	self->btree = BTree_New( UIntMap_DataCompare, UIntMap_DataCopy, UIntMap_DataDelete, NULL, 
				  BTREE_NO_DUPLICATES );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _UIntMap_Delete( void* generator ) {
	UIntMap*	self = (UIntMap*)generator;

	FreeObject( self->btree );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _UIntMap_Print( void* generator, Stream* stream ) {
	UIntMap*	self = (UIntMap*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, "UIntMapStream" );

	/* Print parent */
	Journal_Printf( stream, "UIntMap (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void* _UIntMap_Copy( void* generator, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	UIntMap*	self = (UIntMap*)generator;
	UIntMap*	newUIntMap;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newUIntMap = (UIntMap*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newUIntMap;
#endif

	return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void UIntMap_Insert( void* map, unsigned key, unsigned value ) {
	UIntMap*	self = (UIntMap*)map;
	BTreeNode*	node;

	assert( self );

	node = BTree_FindNode( self->btree, &key );
	if( node )
		((unsigned*)node->data)[1] = value;
	else {
		unsigned	data[2];

		data[0] = key;
		data[1] = value;
		BTree_InsertNode( self->btree, data, 2 * sizeof(unsigned) );
		self->size = self->btree->nodeCount;
	}
}

void UIntMap_Remove( void* map, unsigned key ) {
	UIntMap*	self = (UIntMap*)map;
	BTreeNode*	node;

	assert( self );

	node = BTree_FindNode( self->btree, &key );
	assert( node );
	BTree_DeleteNode( self->btree, node );
}

void UIntMap_Clear( void* map ) {
	UIntMap*	self = (UIntMap*)map;

	assert( self );

	FreeObject( self->btree );
	self->btree = BTree_New( UIntMap_DataCompare, UIntMap_DataCopy, UIntMap_DataDelete, NULL, 
				  BTREE_NO_DUPLICATES );
}

Bool UIntMap_Map( void* map, unsigned key, unsigned* value ) {
	UIntMap*	self = (UIntMap*)map;
	BTreeNode*	node;

	assert( self );

	node = BTree_FindNode( self->btree, &key );
	if( node ) {
		*value = ((unsigned*)node->data)[1];
		return True;
	}

	return False;
}

void UIntMap_GetItems( void* map, unsigned* nItems, unsigned** keys, unsigned** values ) {
	UIntMap*		self = (UIntMap*)map;
	UIntMap_ParseStruct	parseStruct;

	assert( self );

	parseStruct.curItem = 0;
	parseStruct.keys = Memory_Alloc_Array_Unnamed( unsigned, self->size );
	parseStruct.vals = Memory_Alloc_Array_Unnamed( unsigned, self->size );
	BTree_ParseTree( self->btree, UIntMap_ParseNode, &parseStruct );

	*nItems = self->size;
	*keys = parseStruct.keys;
	*values = parseStruct.vals;
}

unsigned UIntMap_GetSize( void* map ) {
	UIntMap*	self = (UIntMap*)map;

	assert( self );

	return self->size;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int UIntMap_DataCompare( void* left, void* right ) {
	if( ((unsigned*)left)[0] > ((unsigned*)right)[0] )
		return 1;
	else if( ((unsigned*)left)[0] < ((unsigned*)right)[0] )
		return -1;
	else
		return 0;
}

void UIntMap_DataCopy( void** dstData, void* data, SizeT size ) {
	*dstData = Memory_Alloc_Array_Unnamed( unsigned, 2 );
	((unsigned*)(*dstData))[0] = ((unsigned*)data)[0];
	((unsigned*)(*dstData))[1] = ((unsigned*)data)[1];
}

void UIntMap_DataDelete( void* data ) {
	Memory_Free( data );
}

void UIntMap_ParseNode( void* data, void* _parseStruct ) {
	UIntMap_ParseStruct*	parseStruct = (UIntMap_ParseStruct*)_parseStruct;

	assert( data );
	assert( parseStruct );

	parseStruct->keys[parseStruct->curItem] = ((unsigned*)data)[0];
	parseStruct->vals[parseStruct->curItem++] = ((unsigned*)data)[1];
}


