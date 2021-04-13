/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"

#include "units.h"
#include "types.h"
#include "IndexMap.h"
#include "PtrMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type IndexMap_Type = "IndexMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IndexMap* IndexMap_New_Param(
		unsigned					delta )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(IndexMap);
	Type                              type = IndexMap_Type;
	Stg_Class_DeleteFunction*      _delete = _IndexMap_Delete;
	Stg_Class_PrintFunction*        _print = _IndexMap_Print;
	Stg_Class_CopyFunction*          _copy = _IndexMap_Copy;

	return _IndexMap_New(  INDEXMAP_PASSARGS  );
}


void IndexMap_Init(
		IndexMap*					self,
		unsigned					delta )
{
	/* General info */
	self->type = IndexMap_Type;
	self->_sizeOfSelf = sizeof(IndexMap);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _IndexMap_Delete;
	self->_print = _IndexMap_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* IndexMap info */
	_IndexMap_Init( self, delta );
}


IndexMap* _IndexMap_New(  INDEXMAP_DEFARGS  )
{
	IndexMap*			self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IndexMap) );
	self = (IndexMap*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* IndexMap info */
	_IndexMap_Init( self, delta );
	
	return self;
}


void _IndexMap_Init(
		IndexMap*				self,
		unsigned				delta )
{
	/* General and Virtual info should already be set */
	
	/* IndexMap info */
	assert( self );
	
	self->delta = delta;
	self->maxTuples = self->delta;
	self->tupleTbl = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
	assert( self->tupleTbl ); /* TODO change this to a firewall, or something */
	self->tupleCnt = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IndexMap_Delete( void* indexMap ) {
	IndexMap*			self = (IndexMap*)indexMap;
	
	/* Stg_Class_Delete the class itself */
	assert( self );
	
	if( self->tupleTbl ) {
		Memory_Free( self->tupleTbl );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _IndexMap_Print( void* indexMap, Stream* stream ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		tuple_I;
	
	/* Set the Journal for printing informations */
	Stream*			myStream;
	myStream = Journal_Register( InfoStream_Type, "IndexMapStream" );

	/* Print parent */
	assert( self );
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "IndexMap (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IndexMap info */
	for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
		Journal_Printf( myStream, "\ttuple[%d]: %d -> %d\n", tuple_I, self->tupleTbl[tuple_I].key, self->tupleTbl[tuple_I].idx );
	}
}


void* _IndexMap_Copy( void* indexMap, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	IndexMap*	self = (IndexMap*)indexMap;
	IndexMap*	newIndexMap;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newIndexMap = (IndexMap*)_Stg_Class_Copy( self, dest, deep, nameExt, map );
	
	newIndexMap->dictionary = self->dictionary;
	newIndexMap->delta = self->delta;
	newIndexMap->maxTuples = self->maxTuples;
	newIndexMap->tupleCnt = self->tupleCnt;
	
	if( deep ) {
		if( (newIndexMap->tupleTbl = (IndexMapTuple*)PtrMap_Find( map, self->tupleTbl )) == NULL && self->tupleTbl ) {
			newIndexMap->tupleTbl = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
			memcpy( newIndexMap->tupleTbl, self->tupleTbl, sizeof(IndexMapTuple) * self->maxTuples );
			PtrMap_Append( map, self->tupleTbl, newIndexMap->tupleTbl );
		}
	}
	else {
		newIndexMap->tupleTbl = self->tupleTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newIndexMap;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void IndexMap_Append( void* indexMap, Index key, Index idx ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		newTupleCnt;
	
	assert( self && key != -1 && idx != -1 );
	
	if( IndexMap_Find( self, key ) != -1 ) {
		return;
	}
	
	newTupleCnt = self->tupleCnt + 1;
	if( newTupleCnt >= self->maxTuples ) {
		unsigned		factor;
		IndexMapTuple*		newTuples;
		
		factor = ceil( (float)(newTupleCnt - self->maxTuples) / (float)self->delta );
		self->maxTuples += factor * self->delta;
		
		newTuples = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
		assert( newTuples ); /* TODO change this */
		if( self->tupleTbl ) {
			memcpy( newTuples, self->tupleTbl, sizeof(IndexMapTuple) * self->tupleCnt );
			Memory_Free( self->tupleTbl );
		}
		self->tupleTbl = newTuples;
	}
	
	self->tupleTbl[self->tupleCnt].key = key;
	self->tupleTbl[self->tupleCnt].idx = idx;
	self->tupleCnt = newTupleCnt;
}


Index IndexMap_Find( void* indexMap, Index key ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		tuple_I;
	
	assert( self );
	
	if( key != -1 ) {
		for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
			if( self->tupleTbl[tuple_I].key == key )
				return self->tupleTbl[tuple_I].idx;
		}
	}
	
	return -1;
}


void IndexMap_Remap( void* indexMap, void* mapThrough ) {
	IndexMap*	self = (IndexMap*)indexMap;
	Index		tuple_I;
	
	for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
		self->tupleTbl[tuple_I].idx = IndexMap_Find( mapThrough, self->tupleTbl[tuple_I].idx );
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/



