/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_PtrMap_h__
#define __StGermain_Base_Container_PtrMap_h__
	

	/** Textual name of this class */
	extern const Type PtrMap_Type;

	/* Virtual function types */
	
	/** PtrMap class contents */
	typedef struct PtrMapTuple {
		void*			key;
		void*			ptr;
	} PtrMapTuple;
	
	#define __PtrMap \
		/* General info */ \
		__Stg_Class \
		\
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* PtrMap info ... */ \
		unsigned				delta; \
		unsigned				maxTuples; \
		unsigned				tupleCnt; \
		PtrMapTuple*				tupleTbl;

	struct PtrMap { __PtrMap };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a PtrMap */
	PtrMap* PtrMap_New( 
		unsigned					delta );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PTRMAP_DEFARGS \
                STG_CLASS_DEFARGS, \
                unsigned  delta

	#define PTRMAP_PASSARGS \
                STG_CLASS_PASSARGS, \
	        delta

	PtrMap* _PtrMap_New(  PTRMAP_DEFARGS  );
	
	
	/* Initialise a PtrMap */
	void PtrMap_Init(
		PtrMap*						self,
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _PtrMap_Init(
		PtrMap*						self,
		unsigned					delta );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _PtrMap_Delete( void* ptrMap );
	
	/* Print mesh implementation */
	void _PtrMap_Print( void* ptrMap, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	void PtrMap_Append( void* ptrMap, void* key, void* ptr );
	
	void* PtrMap_Find( void* ptrMap, void* key );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __StGermain_Base_Container_PtrMap_h__ */

