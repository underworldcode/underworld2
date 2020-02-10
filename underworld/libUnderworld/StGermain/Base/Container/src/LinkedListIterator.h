/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_LinkedListIterator_h__
#define __StGermain_Base_Container_LinkedListIterator_h__

	/** Textual name for LinkedListIterator class. */
	extern const Type LinkedListIterator_Type;
	
	/** Virtual Function Pointers */
	
	/** \def __LinkedListIterator_Iterator See __LinkedList_Iterator */
	#define __LinkedListIterator \
		__Stg_Class \
		LinkedList		*list; \
		LinkedListNode	*curr;
	
	struct LinkedListIterator { __LinkedListIterator };
	

	/** Constructor interface. */
	LinkedListIterator* LinkedListIterator_New( LinkedList *list );
	
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LINKEDLISTITERATOR_DEFARGS \
                STG_CLASS_DEFARGS

	#define LINKEDLISTITERATOR_PASSARGS \
                STG_CLASS_PASSARGS

	LinkedListIterator *_LinkedListIterator_New(  LINKEDLISTITERATOR_DEFARGS  );
	
	/** Init interface. */
	void _LinkedListIterator_Init( LinkedListIterator *self );
	
	/** Stg_Class_Delete Interface */
	void _LinkedListIterator_DeleteFunc( void *self );

	/** Print Interface */
	void _LinkedListIterator_PrintFunc( void *, Stream * );
	
	#define LinkedListIterator_First( it ) \
                ((it==NULL)?NULL:(it->list == NULL)?NULL:((it->curr = it->list->head)==NULL)?NULL:it->curr->data)

	#define LinkedListIterator_Next( it ) \
                ((it==NULL)?NULL:(it->curr == NULL)?NULL:((it->curr = it->curr->next)==NULL)?NULL:it->curr->data)


#endif /* __StGermain_Base_Container_LinkedListIterator_h__ */

