/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_LinkedList_h__
#define __StGermain_Base_Container_LinkedList_h__

	/** Textual name for List class. */
	extern const Type LinkedList_Type;
	
	/** Virtual Function Pointers */
		/** This is a function type to be provided by the user for doing comparisons on node data */
	typedef int		(LinkedList_compareFunction)		( void *, void * );
		
		/** This is a function type to be provided by the user for copying node data, when a new node is being created */
	typedef void	(LinkedList_dataCopyFunction)		( void **, void *, SizeT );

		/** This is a function type to be provided by the user for printing the contents of a Node's data */
	typedef void	(LinkedList_dataPrintFunction)		( void *, void * );
		
		/** This is a function type to be provided by the user for deleting node data, when a new node is being deleted */
	typedef void	(LinkedList_dataDeleteFunction)		( void * );
	
		/** This is a function type to be provided by the user, which can be applied to each node on the tree later on.
		 * The first argument is the data stored inside a node and the second argument can be a single argument or a 
		 * struct of arguments that need to be passed to the user-supplied function */
	typedef void	(LinkedList_parseFunction)			( void *, void * );
	
	/** \def __List See __List */
	#define __LinkedList \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* List info */ \
		LinkedListNode					*head; \
		LinkedList_compareFunction*		compareFunction; \
		LinkedList_dataCopyFunction*	dataCopyFunction; \
		LinkedList_dataPrintFunction*	dataPrintFunction; \
		LinkedList_dataDeleteFunction*	dataDeleteFunction; \
		Order							listOrder; \
		Index						nodeCount;

	struct LinkedList { __LinkedList };

	/** Constructor interface. */
	LinkedList* LinkedList_New( 
				LinkedList_compareFunction*		compareFunction,
				LinkedList_dataCopyFunction*	dataCopyFunction,
				LinkedList_dataPrintFunction*	dataPrintFunction,
				LinkedList_dataDeleteFunction*	dataDeleteFunction,
				Order listOrder);

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LINKEDLIST_DEFARGS \
                STG_CLASS_DEFARGS

	#define LINKEDLIST_PASSARGS \
                STG_CLASS_PASSARGS

	LinkedList* _LinkedList_New(  LINKEDLIST_DEFARGS  );

	
	/** Init interface. */
	void LinkedList_Init( LinkedList* self );
	
	void _LinkedList_Init( LinkedList* self );
	
	/** Stg_Class_Delete interface. */
		/** Stg_Class delete function */
	void _LinkedList_DeleteFunc ( void *list );
	
	/** Print interface. */
		/** Stg_Class print function */
	void _LinkedList_PrintFunc ( void *list, Stream* stream );

	/** Public functions */
		/** Inserts a new node into the tree conserving the list's (Ascending) order */
	void LinkedList_InsertNode ( LinkedList *list, void *newNodeData, SizeT );
		
		/** Sets the compare function to be used by the tree */
	void LinkedList_SetCompareFunction ( LinkedList *list, LinkedList_compareFunction *compareFunction );
	
		/** Attempts to find a node in the list and returns the node if found*/
	LinkedListNode *LinkedList_FindNode( LinkedList *list, void *data );
		
	/** Attempts to find a node data in the list and returns the node data if found*/
	void *LinkedList_FindNodeDataFunc( LinkedList *list, void *data );
#define LinkedList_FindNodeData( list, data, type ) \
		(type*) LinkedList_FindNodeDataFunc( list, data )
	
	/** Attempts to find a node data in the list at a particular position and returns the node data if found*/
	void *LinkedList_ReturnNodeDataAtFunc( LinkedList *list, int index );
#define LinkedList_ReturnNodeDataAt( list, index, type ) \
		(type*) LinkedList_ReturnNodeDataAtFunc( list, index )
	
	/** Deletes a node from the list with the node-data being passed in as a parameter */
	int LinkedList_DeleteNode( LinkedList *list, void *nodeData );
	
	int LinkedList_DeleteAllNodes( LinkedList *list );
		
		/** Parses the list and executes a user-supplied function */
	void LinkedList_ParseList( LinkedList *list, LinkedList_parseFunction *parseFunction, void *args );
	
		/** Parses the list downwards from a given node in the list */
	void LinkedList_ParseListNode( LinkedListNode *root, LinkedList_parseFunction *parseFunction, void *args );

		/** Creates an array of all the nodes' data and returns it */
	char* LinkedList_ReturnArrayFunc( LinkedList *list, SizeT dataSize );

#define LinkedList_ReturnArray( list, type ) \
		(type*) LinkedList_ReturnArrayFunc( list, sizeof(type) )

	/** Private Functions */
	
	
#endif /* __StGermain_Base_Container_LinkedList_h__ */


