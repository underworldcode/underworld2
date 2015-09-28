/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_Set_h__
#define __StGermain_Base_Container_Set_h__
	

	/* Textual name of this class */
	extern const Type Set_Type;

	/* Virtual function types */
	typedef void* (Set_UnionFunc)( void* set, void* operand );
	typedef void* (Set_IntersectionFunc)( void* set, void* operand );
	typedef void* (Set_SubtractionFunc)( void* set, void* operand );
	
	/* Support structures */
	
	/** Set class contents */
	#define __Set \
		/* General info */ \
		__Stg_Class \
		Dictionary*		dictionary; \
		\
		/* Virtual info */ \
		Set_UnionFunc*		_unionFunc; \
		Set_IntersectionFunc*	_intersectionFunc; \
		Set_SubtractionFunc*	_subtractionFunc; \
		\
		/* Set info ... */ \
		SizeT			_elSize; \
		BTree*			_btree;

	struct Set { __Set };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create an instance */
	#define Set_New( dictionary, elementType, compareFunc, dataCopyFunc, dataDeleteFunc ) \
		Set_New_all( dictionary, sizeof(elementType), compareFunc, dataCopyFunc, dataDeleteFunc )

	/* Create an instance with all parameters */
	Set* Set_New_all( 
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SET_DEFARGS \
                STG_CLASS_DEFARGS, \
                Set_UnionFunc*                    _unionFunc, \
                Set_IntersectionFunc*      _intersectionFunc, \
                Set_SubtractionFunc*        _subtractionFunc, \
                Dictionary*                       dictionary, \
                SizeT                            elementSize, \
                BTree_compareFunction*           compareFunc, \
                BTree_dataCopyFunction*         dataCopyFunc, \
                BTree_dataDeleteFunction*     dataDeleteFunc

	#define SET_PASSARGS \
                STG_CLASS_PASSARGS, \
	        _unionFunc,        \
	        _intersectionFunc, \
	        _subtractionFunc,  \
	        dictionary,        \
	        elementSize,       \
	        compareFunc,       \
	        dataCopyFunc,      \
	        dataDeleteFunc   

	Set* _Set_New(  SET_DEFARGS  );
	
	
	/* Initialise an instance */
	void Set_Init(
		Set*						self,
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Initialisation implementation functions */
	void _Set_Init(
		Set*						self, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _Set_Delete(
		void*						set );
	
	/* Print implementation */
	void _Set_Print(
		void*						set, 
		Stream*						stream );

	void* _Set_Union( void* set, void* operand );

	void* _Set_Intersection( void* set, void* operand );

	void* _Set_Subtraction( void* set, void* operand );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	Bool Set_Insert( void* set, void* data );

	#define Set_Union( self, operand ) \
		(self)->_unionFunc( self, operand )

	#define Set_Intersection( self, operand ) \
		(self)->_intersectionFunc( self, operand )

	#define Set_Subtraction( self, operand ) \
		(self)->_subtractionFunc( self, operand )

	void Set_Traverse( void* set, BTree_parseFunction* func, void* args );

	Bool Set_Exists( void* set, void* data );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _Set_BTreeUnion( void* data, void* set );

	void _Set_BTreeIntersection( void* data, void* pack );

	void _Set_BTreeSubtraction( void* data, void* pack );


#endif /* __StGermain_Base_Container_Set_h__ */

