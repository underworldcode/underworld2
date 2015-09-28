/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_MemoryPool_h__
#define __StGermain_Base_Container_MemoryPool_h__

	/** Textual name for List class. */
	extern const Type MemoryPool_Type;
	
	typedef struct MemChunk_t{
		char *memory;
		unsigned int maxFree;
		unsigned int numFree;
	}MemChunk;

	typedef void ( MemoryPool_ResizeCallbackFunc ) ( void * );
	
	/** \def __List See __List */
	#define __MemoryPool \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* MemoryPool info */ \
		SizeT	elementSize; \
		int		numInitialElements; \
		int		numElements; \
		int		numElementsFree; \
		int		numMemChunks; \
		int		delta; \
		MemChunk	*chunks; \
		char	**pool; \
		MemoryPool_ResizeCallbackFunc *callbackFunc; \
		void	*callbackFuncArg;

	struct MemoryPool { __MemoryPool };
	
	/** Constructor interface. */
	#define MemoryPool_New( type, numElements, delta )\
		MemoryPool_NewFunc( sizeof(type), numElements, delta )


	MemoryPool* MemoryPool_NewFunc( SizeT elementSize, int numElements, int delta );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MEMORYPOOL_DEFARGS \
                STG_CLASS_DEFARGS, \
                int  elementSize, \
                int  numElements, \
                int        delta

	#define MEMORYPOOL_PASSARGS \
                STG_CLASS_PASSARGS, \
	        elementSize, \
	        numElements, \
	        delta      

	MemoryPool* _MemoryPool_New(  MEMORYPOOL_DEFARGS  );

	
	/** Init interface. */
	void *MemoryPool_Init( MemoryPool* self );
	
	void _MemoryPool_Init( MemoryPool* self );
	
	/** Stg_Class_Delete interface. */
		/** Stg_Class delete function */
	void _MemoryPool_DeleteFunc ( void *memPool );
	
	/** Print interface. */
		/** Stg_Class print function */
	void _MemoryPool_PrintFunc ( void *memPool, Stream* stream );

	/** Public functions */
#define MemoryPool_NewObject( type, memPool ) \
	(type*)MemoryPool_NewObjectFunc( sizeof(type), memPool )

	void *MemoryPool_NewObjectFunc( SizeT elementSize, MemoryPool *memPool );
		
	Bool MemoryPool_DeleteObject( MemoryPool *memPool, void *object );

	void MemoryPool_Extend( MemoryPool *memPool );
	
	void MemoryPool_Shrink( MemoryPool *memPool );

	void MemoryPool_SetCallbackFunc( MemoryPool *memPool, MemoryPool_ResizeCallbackFunc *f );
	void MemoryPool_SetCallbackFuncArg( MemoryPool *memPool, void *callbackFuncArg );
	
	/** Private Functions */
	
#endif /* __StGermain_Base_Container_MemoryPool_h__ */


