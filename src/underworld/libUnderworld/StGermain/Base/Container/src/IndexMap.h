/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_IndexMap_h__
#define __StGermain_Base_Container_IndexMap_h__
	
	
	/** Textual name of this class */
	extern const Type IndexMap_Type;

	/* Virtual function types */
	
	/** IndexMap class contents */
	typedef struct IndexMapTuple {
		Index			key;
		Index			idx;
	} IndexMapTuple;
	
	#define __IndexMap \
		/* General info */ \
		__Stg_Class \
		\
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* IndexMap info ... */ \
		unsigned				delta; \
		unsigned				maxTuples; \
		unsigned				tupleCnt; \
		IndexMapTuple*				tupleTbl;

	struct IndexMap { __IndexMap };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a default IndexMap */
	#define INDEXMAP_DEFAULT_DELTA		10
	
	#define IndexMap_New() \
		IndexMap_New_Param( INDEXMAP_DEFAULT_DELTA )
	
	/* Create a IndexMap */
	IndexMap* IndexMap_New_Param( 
		unsigned					delta );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define INDEXMAP_DEFARGS \
                STG_CLASS_DEFARGS, \
                unsigned  delta

	#define INDEXMAP_PASSARGS \
                STG_CLASS_PASSARGS, \
	        delta

	IndexMap* _IndexMap_New(  INDEXMAP_DEFARGS  );
	
	
	/* Initialise a IndexMap */
	void IndexMap_Init(
		IndexMap*					self,
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _IndexMap_Init(
		IndexMap*					self,
		unsigned					delta );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _IndexMap_Delete( void* indexMap );
	
	/* Print mesh implementation */
	void _IndexMap_Print( void* indexMap, Stream* stream );
	
	/* Copy implementation */
	#define IndexMap_Copy( self ) \
		(IndexMap*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IndexMap_DeepCopy( self ) \
		(IndexMap*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _IndexMap_Copy( void* indexMap, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	#define IndexMap_Size( self ) \
		(self)->tupleCnt
	
	void IndexMap_Append( void* indexMap, Index key, Index idx );
	
	Index IndexMap_Find( void* indexMap, Index key );
	
	void IndexMap_Remap( void* indexMap, void* mapThrough );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __StGermain_Base_Container_IndexMap_h__ */

