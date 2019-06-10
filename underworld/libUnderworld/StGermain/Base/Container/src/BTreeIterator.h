/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_BTreeIterator_h__
#define __StGermain_Base_Container_BTreeIterator_h__

#define MAX_DEPTH 100

	/** Textual name for BTreeIterator class. */
	extern const Type BTreeIterator_Type;
	
	/** Virtual Function Pointers */
	
	/** \def __BTreeIterator_Iterator See __BTree_Iterator */
	#define __BTreeIterator \
		__Stg_Class \
		BTree						*tree; \
		int							depth; \
		BTreeNode					*stack[MAX_DEPTH];
	
	struct BTreeIterator { __BTreeIterator };
	

	/** Constructor interface. */
	BTreeIterator* BTreeIterator_New( BTree *tree );
	
	BTreeIterator *_BTreeIterator_New(
			SizeT					_sizeOfSelf,
			Type					type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy
			);
	
	/** Init interface. */
	void _BTreeIterator_Init( BTreeIterator *self );
	
	void BTreeIterator_Init( BTreeIterator *self );

	/** Stg_Class_Delete Interface */
	void _BTreeIterator_DeleteFunc( void *self );

	/** Print Interface */
	void _BTreeIterator_PrintFunc( void *, Stream * );
	
	/** Public Functions **/
		/* Returns the data stored inside the Tree's first node ( will be used for iterating through the tree ) */
	void *BTreeIterator_First( BTreeIterator *self );
		
	/* Returns the data stored inside the Tree's next node ( will be used for iterating through the tree ) */
	void *BTreeIterator_Next( BTreeIterator *self );
	
#endif /* __StGermain_Base_Container_BTreeIterator_h__ */
