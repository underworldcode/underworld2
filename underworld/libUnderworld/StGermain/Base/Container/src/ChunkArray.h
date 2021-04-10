/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __StGermain_Base_Container_ChunkArray_h__
#define __StGermain_Base_Container_ChunkArray_h__

#define CHUNK_ARRAY_DELTA 10
#define INVALID -1
#define SUCCESS 1
#define FAILURE 0
#define TWO_EXP16 65535

extern const Type ChunkArray_Type;

    struct Chunk{
        char *memory;
        int numFree;
        int chunkId;
        char **freeList;
    };

    typedef void ( ChunkArray_ResizeCallbackFunc ) ( void * );
    
	#define __ChunkArray \
		__Stg_Class \
        	SizeT                  elementSize; \
        	int                    numElementsPerChunk; \
       		int                    numChunks; \
        	struct Chunk           *chunks; \
        	int                    maxChunkEntries; \
        	int                    chunkToUse;
    
	struct ChunkArray {__ChunkArray};
    

ChunkArray*
ChunkArray_NewFunc
(
    int         elementSize,
    int         numElementsPerChunk
);

ChunkArray *_ChunkArray_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		);
	
void _ChunkArray_Init( ChunkArray* self );

#define ChunkArray_New(type, numElementsPerChunk) \
    ChunkArray_NewFunc(sizeof(type), numElementsPerChunk);

int
ChunkArray_GetChunkWithFreeSlots
(
    ChunkArray      *chunkArray
);

int
ChunkArray_GetFreeChunkSlot
(
    ChunkArray      *chunkArray
);

void
_ChunkArray_Delete
(
    void    *chunkArray
);

void _ChunkArray_Print( void *self, Stream *myStream );

int
ChunkArray_CreateChunk
(
    ChunkArray      *chunkArray,
    int         pos
);

void *
ChunkArray_NewObjectFunc
(
    SizeT           elementSize,
    ChunkArray      *chunkArray
);

unsigned int
ChunkArray_NewObjectIDFunc
(
    SizeT           elementSize,
    ChunkArray      *chunkArray
);
        
int
ChunkArray_DeleteObject
(
       ChunkArray       *chunkArray,
       void             *object
);

int
ChunkArray_DeleteObjectID
(
       ChunkArray       *chunkArray,
       unsigned int          objectId
);

void
ChunkArray_Shrink
(
    ChunkArray      *chunkArray
);

    /** Public functions */
#define ChunkArray_NewObject( type, chunkArray ) \
    (type*)ChunkArray_NewObjectFunc( sizeof(type), chunkArray )

#define ChunkArray_NewObjectID( type, chunkArray ) \
    ChunkArray_NewObjectIDFunc( sizeof(type), chunkArray )

/*#define ChunkArray_ObjectAt(chunkArray, objectId) \
        (char*)(chunkArray->chunks[objectId >> 16].freeList[objectId & TWO_EXP16])*/

char* ChunkArray_ObjectAt(ChunkArray *chunkArray, unsigned int objectId);

#endif
