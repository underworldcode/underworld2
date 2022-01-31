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
#include "PtrMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type PtrMap_Type = "PtrMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PtrMap* PtrMap_New(
		unsigned					delta )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(PtrMap);
	Type                              type = PtrMap_Type;
	Stg_Class_DeleteFunction*      _delete = _PtrMap_Delete;
	Stg_Class_PrintFunction*        _print = _PtrMap_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

	return _PtrMap_New(  PTRMAP_PASSARGS  );
}


void PtrMap_Init(
		PtrMap*						self,
		unsigned					delta )
{
	/* General info */
	self->type = PtrMap_Type;
	self->_sizeOfSelf = sizeof(PtrMap);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _PtrMap_Delete;
	self->_print = _PtrMap_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* PtrMap info */
	_PtrMap_Init( self, delta );
}


PtrMap* _PtrMap_New(  PTRMAP_DEFARGS  )
{
	PtrMap*			self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PtrMap) );
	self = (PtrMap*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* PtrMap info */
	_PtrMap_Init( self, delta );
	
	return self;
}


void _PtrMap_Init(
		PtrMap*					self,
		unsigned				delta )
{
	/* General and Virtual info should already be set */
	
	/* PtrMap info */
	assert( self );
	
	self->delta = delta;
	self->maxTuples = self->delta;
	self->tupleTbl = Memory_Alloc_Array_Unnamed( PtrMapTuple, self->maxTuples );
	assert( self->tupleTbl ); /* TODO change this to a firewall, or something */

	self->tupleCnt = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PtrMap_Delete( void* ptrMap ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	
	/* Stg_Class_Delete the class itself */
	assert( self );
	
	if( self->tupleTbl )
		Memory_Free( self->tupleTbl );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _PtrMap_Print( void* ptrMap, Stream* stream ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	
	/* Set the Journal for printing informations */
	Stream*			myStream;
	myStream = Journal_Register( InfoStream_Type, "PtrMapStream" );

	/* Print parent */
	assert( self );
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "PtrMap (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* PtrMap info */
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void PtrMap_Append( void* ptrMap, void* key, void* ptr ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	unsigned		newTupleCnt;
	
	assert( self && key && ptr );
	
	if( PtrMap_Find( self, key ) != NULL ) {
		return;
	}
	
	newTupleCnt = self->tupleCnt + 1;
	if( newTupleCnt >= self->maxTuples ) {
		unsigned		factor;
		PtrMapTuple*		newTuples;
		
		factor = ceil( (float)(newTupleCnt - self->maxTuples) / (float)self->delta );
		self->maxTuples += factor * self->delta;
		
		newTuples = Memory_Alloc_Array_Unnamed( PtrMapTuple, self->maxTuples );
		assert( newTuples ); /* TODO change this */
		if( self->tupleTbl ) {
			memcpy( newTuples, self->tupleTbl, sizeof(PtrMapTuple) * self->tupleCnt );
			Memory_Free( self->tupleTbl );
		}
		self->tupleTbl = newTuples;
	}
	
	self->tupleTbl[self->tupleCnt].key = key;
	self->tupleTbl[self->tupleCnt].ptr = ptr;
	self->tupleCnt = newTupleCnt;
}


void* PtrMap_Find( void* ptrMap, void* key ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	unsigned		tuple_I;
	
	assert( self );
	
	if( key ) {
		for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
			if( self->tupleTbl[tuple_I].key == key )
				return self->tupleTbl[tuple_I].ptr;
		}
	}
	
	return NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


