/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "MemoryPool.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type MemoryPool_Type = "MemoryPool";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
MemoryPool* _MemoryPool_New(  MEMORYPOOL_DEFARGS  )
{
	MemoryPool *self = NULL;

	self = (MemoryPool*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	self->numMemChunks = 1;
	self->delta = delta;
	self->elementSize = elementSize;
	self->numElements = numElements;
	self->numInitialElements = numElements;
	self->numElementsFree = numElements;
	self->callbackFunc = NULL;
	self->callbackFuncArg = NULL;

	return self;
}

MemoryPool* MemoryPool_NewFunc( SizeT elementSize, int numElements, int delta )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(MemoryPool);
	Type                              type = MemoryPool_Type;
	Stg_Class_DeleteFunction*      _delete = _MemoryPool_DeleteFunc;
	Stg_Class_PrintFunction*        _print = _MemoryPool_PrintFunc;
	Stg_Class_CopyFunction*          _copy = NULL;

	
	MemoryPool* self;
	
	self = _MemoryPool_New(  MEMORYPOOL_PASSARGS  );

	/* Virtual functions */
	_MemoryPool_Init( self );
	return self;
}

void _MemoryPool_Init( MemoryPool* self ){
	int i = 0;
	
	/* General info */
	
	assert( self );
	_Stg_Class_Init ((Stg_Class*) self);
	
	assert( self->numMemChunks == 1 );

	self->chunks = (MemChunk*)Memory_Alloc_Bytes_Unnamed( sizeof(MemChunk)*self->numMemChunks, "char*" );
	self->chunks[self->numMemChunks-1].memory = (char*)Memory_Alloc_Bytes_Unnamed( self->elementSize * self->numElements, "char" );
	memset( self->chunks[self->numMemChunks-1].memory, 0, self->elementSize * self->numElements );
	self->chunks[self->numMemChunks-1].numFree = self->numElements;
	self->chunks[self->numMemChunks-1].maxFree = self->numElements;

	self->pool = (char**)Memory_Alloc_Bytes_Unnamed( sizeof( char* ) * self->numElements, "char*" );
	memset( self->pool, 0, sizeof(char*) * self->numElements );
	
	for( i=0; i<self->numElements; i++ ){
		self->pool[i] = &(self->chunks[self->numMemChunks-1].memory[i*self->elementSize]);
	}
}

void *MemoryPool_Init( MemoryPool *self )
{
	/*TODO*/
	return NULL;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual Functions
*/
void _MemoryPool_PrintFunc ( void *memPool, Stream *stream )
{
	MemoryPool *self = NULL;
	
	self = (MemoryPool*) memPool;
	assert( self );
	assert( stream );

	/* print parent */
	_Stg_Class_Print( (void*) self, stream );

	/* general info */
	Journal_Printf( stream, "MemoryPool (ptr): (%p)\n", self );

	/* Virtual Info */

	/* MemoryPool Info */
	Journal_Printf( stream, "\tElementSize\t\t - %d\n", self->elementSize );
	Journal_Printf( stream, "\tNumElememts\t\t - %d\n", self->numElements );
	Journal_Printf( stream, "\tNumElememtsFree\t\t - %d\n", self->numElementsFree );
	Journal_Printf( stream, "\tNumMemChunks\t\t - %d\n", self->numMemChunks );
	Journal_Printf( stream, "\tDelta\t\t\t - %d\n", self->delta );
}

void _MemoryPool_DeleteFunc( void *memPool )
{
	int i = 0;
	MemoryPool *self = NULL;
	
	self = (MemoryPool*)memPool;
	assert (self);
	
	for( i=0; i<self->numMemChunks; i++ ){
		if( self->chunks[i].memory != ((char*)NULL) ){
			Memory_Free( self->chunks[i].memory );
		}
	}

	Memory_Free( self->chunks );
	Memory_Free( self->pool );
	
	_Stg_Class_Delete( self );
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void *MemoryPool_NewObjectFunc( SizeT elementSize, MemoryPool *memPool )
{
	int index = 0;
	int i = 0;
	char *result = NULL;
	
	assert( elementSize == memPool->elementSize );
	
label:	index = memPool->numElementsFree - 1;

	if( index < 0 ){
		MemoryPool_Extend( memPool );
		goto label;
	}

	result = (char*)(memPool->pool[--memPool->numElementsFree]);

	for( i=0; i<memPool->numMemChunks; i++ ){
		int numObjs = 0;

		numObjs = memPool->chunks[i].maxFree*memPool->elementSize;
		
		if( (result >= memPool->chunks[i].memory) && (result < (memPool->chunks[i].memory+numObjs)) ){
			memPool->chunks[i].numFree--;
			break;
		}
	}

	return (void*)result;
}
		
Bool MemoryPool_DeleteObject( MemoryPool *memPool, void *object )
{
	if( object != NULL ){
		int i = 0;
		int valid = 0;

		for ( i=0; i<memPool->numMemChunks; i++ ){
			int numObjs = 0;

			numObjs = memPool->chunks[i].maxFree*memPool->elementSize;
			
			if( ((char*)object >= memPool->chunks[i].memory) && ((char*)object < (memPool->chunks[i].memory+numObjs)) ){
				memPool->chunks[i].numFree++;
				valid = 1;
				break;
			}
		}

		if( valid ){
			memset( (char*)object, 0, memPool->elementSize );
			memPool->pool[memPool->numElementsFree++] = (char*)object;
			MemoryPool_Shrink( memPool );
			return (Bool)1;
		}
		else{
			return (Bool)0;
		}
	}
	else{
		return (Bool)0;
	}
}

void MemoryPool_Extend( MemoryPool *memPool )
{
	int i = 0;
	char **newPool;

	assert( memPool );

	memPool->numMemChunks++;

	memPool->chunks = (MemChunk*)Memory_Realloc( memPool->chunks, sizeof(MemChunk)*memPool->numMemChunks );
	assert( memPool->chunks );

	memPool->chunks[memPool->numMemChunks-1].memory = (char*)Memory_Alloc_Bytes_Unnamed( memPool->elementSize * memPool->delta, "int" );
	memset( memPool->chunks[memPool->numMemChunks-1].memory, 0, memPool->elementSize * memPool->delta );
	memPool->chunks[memPool->numMemChunks-1].numFree = memPool->delta;
	memPool->chunks[memPool->numMemChunks-1].maxFree = memPool->delta;

	newPool = (char**)Memory_Alloc_Bytes_Unnamed( sizeof(char*) * (memPool->numElements+memPool->delta), "char*" );
	assert( newPool );

	memcpy( newPool+memPool->delta, memPool->pool, sizeof(char*)*memPool->numElements );

	for( i=0; i<memPool->delta; i++ ){
		newPool[i] = &(memPool->chunks[memPool->numMemChunks-1].memory[i*memPool->elementSize]);
	}

	Memory_Free( memPool->pool );
	memPool->pool = newPool;
	memPool->numElements+=memPool->delta;
	memPool->numElementsFree=memPool->delta;

	if( memPool->callbackFunc ){
		memPool->callbackFunc( memPool->callbackFuncArg );
	}
}

void MemoryPool_Shrink( MemoryPool *memPool )
{
	int i = 0;
	Bool deleteFlag = (Bool)0;
	int chunkIdx = 0;
	char **newPool = (char**)0;
	int eCounter = 0;

	assert( memPool );

	if( memPool->numElementsFree < memPool->delta ){
		return;
	}

	for( i=0; i<memPool->numMemChunks; i++ )
	{
		if( memPool->chunks[i].numFree == memPool->chunks[i].maxFree ){
			deleteFlag = True;
			chunkIdx = i;
			break;
		}
	}

	if( deleteFlag ){
		int numObjs = 0;
		numObjs = memPool->chunks[chunkIdx].maxFree*memPool->elementSize;

		newPool = (char**)Memory_Alloc_Bytes_Unnamed( sizeof(char*) * (memPool->numElements-memPool->chunks[chunkIdx].maxFree), "char*" );

		eCounter = 0;
		for( i=0; i<memPool->numElements; i++ ){
			if( (memPool->pool[i] >= memPool->chunks[chunkIdx].memory) && 
					(memPool->pool[i] < (memPool->chunks[chunkIdx].memory+numObjs)) ){
			}
			else{
				newPool[eCounter++] = memPool->pool[i];
			}
		}

		Memory_Free( memPool->pool );
		memPool->pool = newPool;
		
		memPool->numElements-=memPool->chunks[chunkIdx].maxFree;
		memPool->numElementsFree-=memPool->chunks[chunkIdx].maxFree;

		Memory_Free( memPool->chunks[chunkIdx].memory );
		memPool->chunks[chunkIdx].memory = (char*)NULL;

		if( chunkIdx == (memPool->numMemChunks-1) ){
			memPool->numMemChunks--;

			if( memPool->numMemChunks>0 ){
				memPool->chunks = (MemChunk*)Memory_Realloc( memPool->chunks, sizeof(MemChunk)*memPool->numMemChunks );
				assert( memPool->chunks );
			}
		}
		else{
			memcpy( &(memPool->chunks[chunkIdx]), &(memPool->chunks[chunkIdx+1]), sizeof(MemChunk)*(memPool->numMemChunks - (chunkIdx+1)) );
			memPool->numMemChunks--;
			memPool->chunks = (MemChunk*)Memory_Realloc( memPool->chunks, sizeof(MemChunk)*memPool->numMemChunks );
			assert( memPool->chunks );
		}
	}
	
	if( memPool->callbackFunc ){
		memPool->callbackFunc( memPool->callbackFuncArg );
	}
}

void MemoryPool_SetCallbackFunc( MemoryPool *memPool, MemoryPool_ResizeCallbackFunc *f )
{
	assert( memPool );
	memPool->callbackFunc = f;
}

void MemoryPool_SetCallbackFuncArg( MemoryPool *memPool, void *callbackFuncArg )
{
	assert( memPool );
	memPool->callbackFuncArg = callbackFuncArg;
}



