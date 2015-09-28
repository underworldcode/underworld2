/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_BTree_h__
#define __StGermain_Base_Container_BTree_h__

#define MAX_DEPTH 100

	/** Textual name for BTree class. */
	extern const Type BTree_Type;
	
	/** Virtual Function Pointers */
		/** This is a function type to be provided by the user for doing comparisons on node data */
	typedef int		(BTree_compareFunction)		( void *, void * );
		
		/** This is a function type to be provided by the user for copying node data, when a new node is being created */
	typedef void	(BTree_dataCopyFunction)	( void **, void *, SizeT );
		
		/** This is a function type to be provided by the user for deleting node data, when node is being deleted */
	typedef void	(BTree_dataDeleteFunction)	( void * );

		/** This is a function type to be provided by the user for printing the contents of a node's data */
	typedef void	(BTree_dataPrintFunction)	( void *, Stream * );
	
		/** This is a function type to be provided by the user, which can be applied to each node on the tree later on.
		 * The first argument is the data stored inside a node and the second argument can be a single argument or a 
		 * struct of arguments that needs to be passed to the user-supplied function */
	typedef void	(BTree_parseFunction)		( void *, void * );

	/** \def __BTree See __BTree */
	#define __BTree \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* BTree info */ \
		ChunkArray					*pool; \
		BTreeNode					*root; \
		BTree_compareFunction*		compareFunction; \
		BTree_dataCopyFunction*		dataCopyFunction; \
		BTree_dataDeleteFunction*	dataDeleteFunction; \
		BTree_dataPrintFunction*	dataPrintFunction; \
		BTreeProperty				property; \
		Index						nodeCount;
		
	struct BTree { __BTree };
	

	/** Constructor interface. */
	BTree* BTree_New( 
		BTree_compareFunction*		compareFunction,
		BTree_dataCopyFunction*		dataCopyFunction,
		BTree_dataDeleteFunction*	dataDeleteFunction,
		BTree_dataPrintFunction*	dataPrintFunction,
		BTreeProperty				property );
	
	BTree *_BTree_New(
			SizeT					_sizeOfSelf,
			Type					type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy
			);
	
	/** Init interface. */
	void _BTree_Init( BTree* self );
	
	void BTree_Init( BTree* self );

	/** Stg_Class_Delete Interface */
	void _BTree_DeleteFunc_Helper( BTreeNode *, BTree_dataDeleteFunction *, ChunkArray *pool );
	void _BTree_DeleteFunc( void *self );

	/** Print Interface */
	void _BTree_PrintFunc_Helper( BTreeNode *, BTree_dataPrintFunction *, Stream * );
	void _BTree_PrintFunc( void *, Stream * );
	
	/** Stg_Class_Delete interface. */
	
	/** Print interface. */

	/** Public functions */
		/** Inserts a new node into the tree */
	int BTree_InsertNode ( BTree *tree, void *newNodeData, SizeT );
		
		/** Sets the compare function to be used by the tree */
	void BTree_SetCompareFunction ( BTree *tree, BTree_compareFunction *compareFunction );
	
		/** Attempts to find a node in the tree */
	BTreeNode *BTree_FindNode( BTree *tree, void *data );
	
		/** This function is similar to the one above and returns the number of hops needed to reach the target node */
	BTreeNode *BTree_FindNodeAndHops( BTree *tree, void *data, int *hops );
	
		/** Deletes a node from the tree */
	void BTree_DeleteNode( BTree *tree, BTreeNode *z );
		
		/** Parses the tree and executes a user-supplied function */
	void BTree_ParseTree( BTree *tree, BTree_parseFunction *parseFunction, void *args );
	
		/*Parses the tree downwards from a given node in the tree*/
	void BTree_ParseTreeNode( BTreeNode *root, BTree_parseFunction *parseFunction, void *args );

		/* Returns the data stored inside a node in the tree */
	void *BTree_GetData( BTreeNode *node );

	/** Private Functions */
	
		/** Left Rotates a Node in the tree */
	void BTree_LeftRotate ( BTree *tree, BTreeNode *x );

		/** Right Rotates a Node in the tree */
	void BTree_RightRotate ( BTree *tree, BTreeNode *y );

		/** Fixes up the tree to preserve the RedBlack-ness of the tree after node insertion*/
	void BTree_InsertFix (BTree *tree, BTreeNode *x);

		/** Fixes up the tree to preserve the RedBlack-ness of the tree after node deletion*/
	void BTree_DeleteFixUp( BTree *tree, BTreeNode *x );
	
		/** Added ss alias to Stg_Class_Delete for forward declaraion wrappers */
	void BTree_Delete( void* tree );
	
#endif /* __StGermain_Base_Container_BTree_h__ */
