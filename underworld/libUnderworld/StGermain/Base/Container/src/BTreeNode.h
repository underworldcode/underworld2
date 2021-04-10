/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_BTreeNode_h__
#define __StGermain_Base_Container_BTreeNode_h__

	/** Textual name for BTreeNode class. */
	extern const Type BTreeNode_Type;

	/** \def __BTreeNode See BTreeNode */
	#define __BTreeNode \
		/* General info */ \
		\
		/* Virtual info */ \
		\
		/* BTree info */ \
		Color				color; \
		void 				*data; \
		SizeT				sizeOfData; \
		unsigned int			id; \
		struct BTreeNode	*left; \
		struct BTreeNode	*right; \
		struct BTreeNode	*parent; 

	struct BTreeNode { __BTreeNode };


	extern BTreeNode terminal;
	#define NIL &terminal


	/** Constructor interface. */
	BTreeNode *BTreeNode_New ( ChunkArray *pool );
	
	/** Init interface. */
	void _BTreeNode_Init( BTreeNode *self );
	
	/** Stg_Class_Delete interface. */
	
	/** Print interface. */

#endif /* __StGermain_Base_Container_BTreeNode_h__ */

