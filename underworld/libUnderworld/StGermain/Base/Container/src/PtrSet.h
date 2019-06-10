/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_PtrSet_h__
#define __StGermain_Base_Container_PtrSet_h__
	

	/* Textual name of this class */
	extern const Type PtrSet_Type;

	/* Virtual function types */
	
	/* Support structures */
	
	/** PtrSet class contents */
	#define __PtrSet \
		/* General info */ \
		__Set \
		\
		/* Virtual info */ \
		\
		/* PtrSet info ... */ \

	struct PtrSet { __PtrSet };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create an instance with all parameters */
	PtrSet* PtrSet_New( 
		Dictionary*					dictionary );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PTRSET_DEFARGS \
                SET_DEFARGS

	#define PTRSET_PASSARGS \
                SET_PASSARGS

	PtrSet* _PtrSet_New(  PTRSET_DEFARGS  );
	
	
	/* Initialise an instance */
	void PtrSet_Init(
		PtrSet*						self,
		Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _PtrSet_Init(
		PtrSet*						self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _PtrSet_Delete(
		void*						ptrSet );
	
	/* Print implementation */
	void _PtrSet_Print(
		void*						ptrSet, 
		Stream*						stream );

	void* _PtrSet_Union( void* ptrSet, void* operand );

	void* _PtrSet_Intersection( void* ptrSet, void* operand );

	void* _PtrSet_Subtraction( void* ptrSet, void* operand );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	int _PtrSet_CompareData( void* left, void* right );

	void _PtrSet_DeleteData( void* data );


#endif /* __StGermain_Base_Container_PtrSet_h__ */

