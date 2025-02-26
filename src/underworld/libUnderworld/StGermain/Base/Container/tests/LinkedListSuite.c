/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "LinkedListSuite.h"

#define NUM_DATA 100

/*
 * Define the context data structure to be usesd for each test-case.
 */
typedef struct {
   LinkedList*       numList;
   int*              array[NUM_DATA];
} LinkedListSuiteData;


int LinkedListSuite_CompareFunction(void *data1, void *data2)
{
	int *d1 = NULL, *d2 = NULL;

	d1 = (int*)data1;
	d2 = (int*)data2;

	if (d1 == NULL || d2 == NULL){
		return 0;	
	}
	
	if (*d1 > *d2){
		return  1;
	}
	else if (*d1 == *d2){
		return 0;
	}
	else{
		return -1;
	}
}

void LinkedListSuite_DataPrintFunction( void *nodeData, void *args )
{
	Stream *myStream = NULL;
	
	assert (nodeData);
	myStream = Journal_Register( InfoStream_Type, (Name)"LinkedListStream"  );

	Journal_Printf( myStream, "\t%d\n", *(int*)nodeData );
}

void LinkedListSuite_DataCopyFunction( void **nodeData, void *newData, SizeT dataSize)
{
	*nodeData = Memory_Alloc_Bytes_Unnamed(dataSize, "char");
	memset(*nodeData, 0, dataSize);

	memcpy(*nodeData, newData, dataSize);
}

void LinkedListSuite_Setup( LinkedListSuiteData* data ) {
   Index          ii = 0;

   data->numList = LinkedList_New(
            LinkedListSuite_CompareFunction,
            LinkedListSuite_DataCopyFunction,
            LinkedListSuite_DataPrintFunction,
            NULL,
            LINKEDLIST_UNSORTED);

   for(ii=0; ii<NUM_DATA; ii++){
      data->array[ii] = Memory_Alloc(int, "TestLinkedList_ArrayEntry");
      *data->array[ii] = ii;
   }
}

void LinkedListSuite_Teardown( LinkedListSuiteData* data ) {
   Index          ii = 0;

   Stg_Class_Delete( data->numList );
   for(ii=0; ii < NUM_DATA; ii++){
      Memory_Free( data->array[ii] );
   }
}


void LinkedListSuite_TestInsert( LinkedListSuiteData* data ) {
   Index             ii = 0;
   LinkedListNode*   currNode=NULL;   

   /* Inserting data into the List\n */
   for(ii=0; ii<NUM_DATA; ii++){
      LinkedList_InsertNode(data->numList, data->array[ii], sizeof(int));
   }

   pcu_check_true( data->numList->nodeCount == NUM_DATA );
   currNode = data->numList->head;

   /* Note: since current implementation inserts new linked list nodes at the head, rather than the tail, the
    * list nodes will be in reverse order. This probably should be changed to insert at the tail, but I won't
    * do this just in case some other code is relying on this reversed order. --PatrickSunter, 31 May 2009 */ 
   for(ii=0; ii<NUM_DATA; ii++){
      pcu_check_true( *((int*)currNode->data) == (NUM_DATA-1 - *data->array[ii]) );
      currNode = currNode->next;
   }
}


void LinkedListSuite_TestDelete( LinkedListSuiteData* data ) {
   Index          ii = 0;
   LinkedListNode*   currNode=NULL;   

   for(ii=0; ii<NUM_DATA; ii++){
      LinkedList_InsertNode(data->numList, data->array[ii], sizeof(int));
   }
   /* \nDeleting half the nodes previously inserted into the list\n */
   for(ii=0; ii<NUM_DATA/2; ii++){
      LinkedList_DeleteNode(data->numList, data->array[ii]);
   }
   
   pcu_check_true( data->numList->nodeCount == NUM_DATA/2 );
   currNode = data->numList->head;
   /* Since they end up in reverse order, the deleted notes should be the 2nd half of the list, so the first half
    * should have remained unchanged */   
   for(ii=0; ii<NUM_DATA/2; ii++){
      pcu_check_true( *((int*)currNode->data) == (NUM_DATA-1 - *data->array[ii]) );
      currNode = currNode->next;
   }
}


void LinkedListSuite_TestReturnArray( LinkedListSuiteData* data ) {
   int*           numArray;
   Index          ii = 0;

   for(ii=0; ii<NUM_DATA; ii++){
      LinkedList_InsertNode(data->numList, data->array[ii], sizeof(int));
   }
   /* \nRetrieving the data held in the list as an array\n */
   numArray = LinkedList_ReturnArray(data->numList, int);

   for(ii=0; ii<data->numList->nodeCount; ii++){
      pcu_check_true(numArray[ii] == (NUM_DATA-1 - *data->array[ii]));
   }
   Memory_Free( numArray );
}


void LinkedListSuite_TestFindNodeData( LinkedListSuiteData* data ) {
   int*           result = NULL;
   Index          ii = 0;
   int            secondArray[NUM_DATA];

   for(ii=0; ii<NUM_DATA; ii++){
      LinkedList_InsertNode(data->numList, data->array[ii], sizeof(int));
      secondArray[ii] = *data->array[ii];
   }
   /* \nSearching for Node data in the list\n */
   /* Deliberately search with ptrs from a different array: want the ptrs to be different, thus checking using the
    *  compare function if the data itself is the same */
   for(ii=0; ii<NUM_DATA/4; ii++) {
      result = LinkedList_FindNodeData(data->numList, &secondArray[ii], int);
      pcu_check_true( result != NULL );
      pcu_check_true( *result == *data->array[ii] );
   }
}


void LinkedListSuite( pcu_suite_t* suite ) {
    /* Tell PCU the context data type to pass to each test-case in this suite. */
   pcu_suite_setData( suite, LinkedListSuiteData );

   /* Set the fixtures to be run before and after each test-case. */
   pcu_suite_setFixtures( suite, LinkedListSuite_Setup, LinkedListSuite_Teardown );

   /* Add all the test-cases. */
   pcu_suite_addTest( suite, LinkedListSuite_TestInsert );
   pcu_suite_addTest( suite, LinkedListSuite_TestDelete );
   pcu_suite_addTest( suite, LinkedListSuite_TestReturnArray );
   pcu_suite_addTest( suite, LinkedListSuite_TestFindNodeData );
}


