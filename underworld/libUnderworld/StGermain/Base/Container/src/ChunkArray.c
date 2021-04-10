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
#include "ChunkArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type ChunkArray_Type = "ChunkArray";

ChunkArray *_ChunkArray_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		)
{
	ChunkArray *self = NULL;
	
	self = (ChunkArray*)malloc( _sizeOfSelf );
	memset( self, 0, _sizeOfSelf );
	
	self->_sizeOfSelf = _sizeOfSelf;
	self->_deleteSelf = True;
	self->type = type;
	self->_delete = _delete;
	self->_print = _print;
	self->_copy = _copy;
	
	return self;
}

void _ChunkArray_Init( ChunkArray* self )
{
    int i = 0;
    
    assert(self);

    self->maxChunkEntries = CHUNK_ARRAY_DELTA;
    self->chunkToUse = INVALID;
    
    self->chunks = (Chunk*)NULL;
    self->chunks = (Chunk*)malloc( sizeof(Chunk)*self->maxChunkEntries );
    memset(self->chunks, 0, sizeof(Chunk)*self->maxChunkEntries);
    for(i=0; i<self->maxChunkEntries; i++){
        self->chunks[i].chunkId = INVALID;
        self->chunks[i].numFree = INVALID;
    }
    
    assert(self->chunks);
}
	
ChunkArray*
ChunkArray_NewFunc
(
    int         elementSize,
    int         numElementsPerChunk
)
{
    ChunkArray      *self     = NULL;

    self = _ChunkArray_New(
		sizeof(ChunkArray),
		ChunkArray_Type,
		_ChunkArray_Delete,
		_ChunkArray_Print,
		NULL);
		

    assert(self);
    assert(elementSize > 0);
    assert(numElementsPerChunk > 0);

    self->numChunks = 0;
    self->elementSize = elementSize;

    if(numElementsPerChunk < 65535){
        self->numElementsPerChunk = numElementsPerChunk;
    }
    else{
        return NULL;
    }

    _ChunkArray_Init(self);
    return self;
}


void _ChunkArray_Print( void *self, Stream *myStream )
{
	ChunkArray *chArray = NULL;

	chArray = (ChunkArray*) self;
	assert( chArray );

	/* print parent */
	_Stg_Class_Print( (void*) chArray, myStream );

	/* general info */
	Journal_Printf( myStream, "CHunkArray (ptr): (%p)\n", chArray );
}

void
_ChunkArray_Delete
(
    void    *chunkArray
)
{
    int        i       = 0;
    ChunkArray          *self   = NULL;
    
    self = (ChunkArray*)chunkArray;
    assert (self);
    
    for( i=0; i<self->maxChunkEntries; i++ ){
        if( self->chunks[i].numFree != INVALID ){
            free( self->chunks[i].memory );
            free( self->chunks[i].freeList );
        }
    }

    free ( self->chunks );
    free( self );
}

int
ChunkArray_CreateChunk
(
    ChunkArray      *chunkArray,
    int         pos
)
{
    assert( chunkArray );

    //printf("creating chunk at %d\n", pos);
    if((pos) < chunkArray->maxChunkEntries){
    }
    else{
        int  i = 0;

        chunkArray->maxChunkEntries += CHUNK_ARRAY_DELTA;
        chunkArray->chunks = (Chunk*)realloc(chunkArray->chunks, sizeof(Chunk)*chunkArray->maxChunkEntries );

        assert(chunkArray->chunks);
    	memset(&chunkArray->chunks[chunkArray->maxChunkEntries-CHUNK_ARRAY_DELTA], 0, sizeof(Chunk)*CHUNK_ARRAY_DELTA);
        
        for(i=(chunkArray->maxChunkEntries-CHUNK_ARRAY_DELTA); i<chunkArray->maxChunkEntries; i++){
            chunkArray->chunks[i].chunkId = INVALID;
            chunkArray->chunks[i].numFree = INVALID;
        }
    }

    {
        int        idx     = 0;
        int        i       = 0;
        int        j       = 0;
        
        idx = pos;

        chunkArray->chunks[idx].memory = (char*)NULL;
        chunkArray->chunks[idx].memory = (char*)malloc(
                                        sizeof(char) * chunkArray->elementSize * chunkArray->numElementsPerChunk);
        memset(chunkArray->chunks[idx].memory, 0, sizeof(char)*chunkArray->elementSize * chunkArray->numElementsPerChunk);
    
        assert(chunkArray->chunks[idx].memory);

        chunkArray->chunks[idx].chunkId = idx;

        chunkArray->chunks[idx].freeList = (char**)NULL;
        chunkArray->chunks[idx].freeList = (char**)malloc(
                                        sizeof(char*) * chunkArray->numElementsPerChunk);
        
        assert(chunkArray->chunks[idx].freeList);

        chunkArray->chunks[idx].numFree = chunkArray->numElementsPerChunk;

        for(i=0,j=0; i<chunkArray->numElementsPerChunk*chunkArray->elementSize; i+=chunkArray->elementSize, j++){
            chunkArray->chunks[idx].freeList[j] = &(chunkArray->chunks[idx].memory[i]);
        }

	    ++chunkArray->numChunks;

        return idx;
    }
}

void *
ChunkArray_NewObjectFunc
(
    SizeT           elementSize,
    ChunkArray      *chunkArray
)
{
    char                        *result     = NULL;
    int                objectID    = ChunkArray_NewObjectIDFunc(elementSize, chunkArray);
    int                chunkID     = objectID >> 16;
    int                arrayIdx    = objectID & TWO_EXP16;

    if(chunkID < chunkArray->maxChunkEntries && chunkArray->chunks[chunkID].numFree != INVALID){
        if(arrayIdx < chunkArray->numElementsPerChunk*chunkArray->elementSize){
            result = (char*) ChunkArray_ObjectAt(chunkArray, objectID);
        }
        else{
            assert(0);
        }
    }
    else{
        assert(0);
    }

    return result;
}

unsigned int
ChunkArray_NewObjectIDFunc
(
    SizeT           elementSize,
    ChunkArray      *chunkArray
)
{
    int             index       = 0;
    Chunk           *chunk      = NULL;
    unsigned int    resultID    = 0;
    unsigned int    chunkID     = 0;
    unsigned int    arrayIdx    = 0;
    
    assert(elementSize == chunkArray->elementSize);
    
    if(chunkArray->chunkToUse == INVALID){
        chunkArray->chunkToUse = ChunkArray_CreateChunk(chunkArray, 0);
    }
    
    chunk = &(chunkArray->chunks[chunkArray->chunkToUse]);

    assert(chunk);

label:    index = chunk->numFree - 1;
    if( index < 0 ){

        chunkArray->chunkToUse = ChunkArray_GetChunkWithFreeSlots(chunkArray);
        
        if(chunkArray->chunkToUse == INVALID){
            int chunkSlot = ChunkArray_GetFreeChunkSlot(chunkArray);

            if(chunkSlot==INVALID){
                chunkArray->chunkToUse = ChunkArray_CreateChunk(chunkArray, chunkArray->maxChunkEntries);
                assert( chunkArray->chunkToUse != INVALID );
            }
            else{
                chunkArray->chunkToUse = ChunkArray_CreateChunk(chunkArray, chunkSlot);
                assert( chunkArray->chunkToUse != INVALID );
            }
        }
        
        chunk = &(chunkArray->chunks[chunkArray->chunkToUse]);
        goto label;
    }

    assert(chunk->chunkId < TWO_EXP16);

    chunkID = chunk->chunkId;
    arrayIdx = ((long int)(chunk->freeList[--chunk->numFree]) - (long int)chunk->memory);

    resultID = resultID | (chunkID << 16);
    resultID = resultID | (arrayIdx);
    
    return resultID;
}

int
ChunkArray_GetFreeChunkSlot
(
    ChunkArray      *chunkArray
)
{
    int i = 0;

    assert(chunkArray);

    for(i=0; i<chunkArray->maxChunkEntries; i++){
        if(chunkArray->chunks[i].numFree == INVALID){
            return i;
        }
    }

    return INVALID;
}

int
ChunkArray_GetChunkWithFreeSlots
(
    ChunkArray      *chunkArray
)
{
    int i = 0;
    int leastNumFree = 1<<30;
    int leastNumFreeIdx = INVALID;

    assert(chunkArray);

    for(i=0; i<chunkArray->maxChunkEntries; i++){
        if(chunkArray->chunks[i].numFree > 0){
            if(chunkArray->chunks[i].numFree < leastNumFree){
                leastNumFree = chunkArray->chunks[i].numFree;
                leastNumFreeIdx = i;
            }
        }
    }

    return leastNumFreeIdx;
}
        
int
ChunkArray_DeleteObject
(
       ChunkArray       *chunkArray,
       void             *object
)
{
    if( object != NULL ){
        int                 i           = 0;
        int                 valid       = 0;
        int                 chunkIdx    = 0;

        for ( i=0; i<chunkArray->maxChunkEntries; i++ ){
            
            if( chunkArray->chunks[i].memory != NULL &&
                ((char*)object >= chunkArray->chunks[i].memory) && 
                ((char*)object < (chunkArray->chunks[i].memory+(chunkArray->numElementsPerChunk*chunkArray->elementSize))) ){
                valid = 1;
                chunkIdx = i;
                break;
            }
        }

        if( valid ){
            memset(object, 0, chunkArray->elementSize);
            chunkArray->chunks[chunkIdx].freeList[chunkArray->chunks[chunkIdx].numFree++] = (char*)object;

            ChunkArray_Shrink( chunkArray );

            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}

int
ChunkArray_DeleteObjectID
(
       ChunkArray       *chunkArray,
       unsigned int          objectId
)
{
    int chunkID  = objectId >> 16;
    int arrayIdx = objectId & TWO_EXP16;
    char         *objPtr  = NULL;
        
    if(chunkID < chunkArray->maxChunkEntries && chunkArray->chunks[chunkID].numFree != INVALID){
        if(arrayIdx < chunkArray->numElementsPerChunk*chunkArray->elementSize){
            if (chunkArray->chunks[chunkID].chunkId != chunkID){
                return FAILURE;
            } 
            objPtr = ChunkArray_ObjectAt(chunkArray, objectId);

            if( (arrayIdx < chunkArray->numElementsPerChunk*chunkArray->elementSize) &&
                ((char*)objPtr >= chunkArray->chunks[chunkID].memory) && 
                ((char*)objPtr < (chunkArray->chunks[chunkID].memory+(chunkArray->numElementsPerChunk*chunkArray->elementSize))) ){
                
                memset(objPtr, 0, chunkArray->elementSize);
                chunkArray->chunks[chunkID].freeList[chunkArray->chunks[chunkID].numFree++] = (char*)objPtr;
                ChunkArray_Shrink( chunkArray );
            }
            else{
                return FAILURE;
            }
        }
        else{
            return FAILURE;
        }
    }
    else{
        return FAILURE;
    }
    
    return SUCCESS;    
}

void
ChunkArray_Shrink
(
    ChunkArray      *chunkArray
)
{
    int                     i                = 0;
    int                     deleteFlag       = 0;
    int                     chunkIdx         = 0;
    int                     shrinkChunkArray = 0;

    assert( chunkArray );

    for(i=0; i<chunkArray->maxChunkEntries; i++){
        //printf("\t\tmaxChunks %d chunk id %d numFree %d\n", chunkArray->maxChunkEntries, chunkArray->chunks[i].chunkId, chunkArray->chunks[i].numFree);
        if(chunkArray->chunks[i].numFree == chunkArray->numElementsPerChunk){
            deleteFlag = 1;
            chunkIdx = i;
            break;
        }
    }

    //printf("current live objects %d\n", currObjsAlive);
    if(deleteFlag){
        Chunk *c = (Chunk*)NULL;

       
        c = &(chunkArray->chunks[chunkIdx]);

        free(c->freeList);
        free(c->memory);
        memset(c, 0, sizeof(Chunk));
        c->chunkId = INVALID;
        c->numFree = INVALID;
        
        chunkArray->numChunks--;

#if 1
label:  shrinkChunkArray = 1;
        for(i=chunkArray->maxChunkEntries-1; i>=(chunkArray->maxChunkEntries-CHUNK_ARRAY_DELTA); i--){
            if(chunkArray->chunks[i].numFree != INVALID){
                shrinkChunkArray = 0;
                break;
            }
            else{

            }
        }

        if(shrinkChunkArray && chunkArray->maxChunkEntries>CHUNK_ARRAY_DELTA){
            chunkArray->maxChunkEntries-=CHUNK_ARRAY_DELTA;
            
            if(chunkArray->numChunks > chunkArray->maxChunkEntries) 
            {
                assert(0);
            }
            
            chunkArray->chunks = (Chunk*)realloc(chunkArray->chunks, sizeof(Chunk)*(chunkArray->maxChunkEntries));

            chunkArray->chunkToUse = ChunkArray_GetChunkWithFreeSlots(chunkArray);
            
            /*printf("shrinking chunks array, maxChunks %d numChunks %d LiveObjects %d\n", chunkArray->maxChunkEntries, chunkArray->numChunks, currObjsAlive);*/
            
            goto label;

        }
#endif
    }
}

char* ChunkArray_ObjectAt(ChunkArray *chunkArray, unsigned int objectId)
{
    unsigned int chunkID = objectId >> 16;
    unsigned int arrayIdx = objectId & TWO_EXP16;
    
    return (char*)&(chunkArray->chunks[chunkID].memory[arrayIdx]);
}


