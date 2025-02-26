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
#include <time.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "BTreeSuite.h"

typedef struct {
   BTree*       numList;
} BTreeSuiteData;

#define NUM_DATA 100
#define NUM_DATA_LARGE 10000

typedef struct praseFunctionArguments_t{
   double result;
}parseFunctionArguments;

static int compareFunction(void *data1, void *data2) {
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

static void dataPrintFunction( void *nodeData, Stream *printStream ) {
   assert (nodeData);

   Journal_Printf( printStream, "\t%d\n", *(int*)nodeData );
}

static void dataCopyFunction( void **nodeData, void *newData, SizeT dataSize) {
   *nodeData = malloc( dataSize );
   memset(*nodeData, 0, dataSize);

   memcpy(*nodeData, newData, dataSize);
}

static void dataDeleteFunction( void *nodeData ) {
   if( nodeData != NULL ){
      free( nodeData );
   }
}

void treeParseFunction( void *nodeData, void *args ) {
   parseFunctionArguments *arguments = NULL;

   assert( nodeData );
   arguments = (parseFunctionArguments*) args;
   assert( arguments );

   arguments->result += *(int*)nodeData;
}

void BTreeSuite_Setup( BTreeSuiteData* data ) {
   data->numList = BTree_New(
      compareFunction,
      dataCopyFunction,
      dataDeleteFunction,
      dataPrintFunction,
      BTREE_ALLOW_DUPLICATES );
}

void BTreeSuite_Teardown( BTreeSuiteData* data ) {
   Stg_Class_Delete( data->numList );
}

void BTreeSuite_TestInsert( BTreeSuiteData* data ) {
   int*           array[NUM_DATA] = {0};
   BTreeIterator* iterator = NULL;
   void*          treeData = NULL;
   int            ii = 0;

   /* Testing basic Insertion, Deletion and Query functions\n */

   /* Inserting data into the Tree\n */
   for(ii=0; ii<NUM_DATA; ii++){
      array[ii] = (int*)malloc(sizeof(int));
      *array[ii] = NUM_DATA - ii;
      BTree_InsertNode(data->numList, (void*)array[ii], sizeof(int));
   }

   /* Expect the nodes to have been inserted from 0 to NUM_DATA now */
   pcu_check_true(data->numList->nodeCount == NUM_DATA);
   ii = 1;
   iterator = BTreeIterator_New( data->numList );
   treeData = BTreeIterator_First( iterator ); 
   for( ; treeData != NULL; treeData = BTreeIterator_Next( iterator ) ){
      pcu_check_true( *(int*)treeData == ii++ );
   }

   for(ii=0; ii<NUM_DATA; ii++) {
      free( array[ii] );
   }
   Stg_Class_Delete( iterator );
}   

void BTreeSuite_TestDelete( BTreeSuiteData* data ) {
   int*           array[NUM_DATA] = {0};
   BTreeIterator* iterator = NULL;
   void*          treeData = NULL;
   BTreeNode*     node = NULL;
   int            ii = 0;

   /* Inserting data into the Tree\n */
   for(ii=0; ii<NUM_DATA; ii++){
      array[ii] = (int*)malloc(sizeof(int));
      *array[ii] = NUM_DATA - ii;
      BTree_InsertNode(data->numList, (void*)array[ii], sizeof(int));
   }

   /* \nDeleting half the nodes previously inserted into the tree\n */
   for(ii=0; ii<NUM_DATA/2; ii++){
      node = BTree_FindNode( data->numList, (void*)array[ii] );
      
      if( node ){
         BTree_DeleteNode( data->numList, node );
      }
   }
   
   pcu_check_true(data->numList->nodeCount == NUM_DATA/2);
   ii = 1;
   iterator = BTreeIterator_New( data->numList );
   treeData = BTreeIterator_First( iterator ); 
   for( ; treeData != NULL; treeData = BTreeIterator_Next( iterator ) ){
      pcu_check_true( *(int*)treeData == ii++ );
   }

   for(ii=0; ii<NUM_DATA; ii++) {
      free( array[ii] );
   }
   Stg_Class_Delete( iterator );
}
 
void BTreeSuite_TestFindNode( BTreeSuiteData* data ) {
   int*       array[NUM_DATA] = {0};
   BTreeNode* node = NULL;
   int        ii = 0;

   /* Inserting data into the Tree\n */
   for(ii=0; ii<NUM_DATA/2; ii++){
      array[ii] = (int*)malloc(sizeof(int));
      *array[ii] = NUM_DATA - ii;
      BTree_InsertNode(data->numList, (void*)array[ii], sizeof(int));
   }

   /* \nSearching for Node data in the tree\n */
   for(ii=0; ii<NUM_DATA/2; ii++){
      node = BTree_FindNode( data->numList, array[ii] );
      pcu_check_true( node != NULL );
   }
   for(ii=NUM_DATA/2; ii<NUM_DATA; ii++){
      node = BTree_FindNode( data->numList, array[ii] );
      pcu_check_true( node == NULL );
   }

   for(ii=0; ii<NUM_DATA; ii++) {
      free( array[ii] );
   }
}

void BTreeSuite_TestComplexityPerformance( BTreeSuiteData* data ) {
   int        intData = 0;
   int        idealComplexity = 0, hops = 0;
   int        ii = 0;
   BTreeNode* node = NULL;

   srand( (unsigned)time( NULL ) );
   for( ii=0; ii<NUM_DATA_LARGE; ii++ ) {
      intData = (int) NUM_DATA_LARGE * (rand() / (RAND_MAX + 1.0));
      BTree_InsertNode(data->numList, (void*)&intData, sizeof(int));
      
      idealComplexity = (int)ceil((log( (double)data->numList->nodeCount ) / log(2.0)));
      
      /* 
       *          n >= 2^(bh) - 1
       *  where bh is the black height of the tree 
       *  but bh ~ h/2 
       *  
       *  log( n+1 ) >= (bh/2)
       *   bh <= 2log(n+1)
       *
       *  */   
      node = BTree_FindNodeAndHops( data->numList, (void*)&intData, &hops ); 
      pcu_check_true( node != NULL );
      /* Testing BTree_FindNode has a complexity of O(log n):
         This implies similar complexity for Insertion and Deletion. */
      /* Need the if statement to check for special case of 1-node tree, in which case both hops and
       * idealComplexity will be 0 */
      if ( hops > 0 ) {
         pcu_check_true( hops < (idealComplexity * 2)  );
      }
   }
}
   
void BTreeSuite_TestParseTreeForAverage( BTreeSuiteData* data ) {
   int                    intData = 0;
   double                 average = 0, parsedAverage = 0;
   parseFunctionArguments parseArguments;
   double                 epsilon  = 1e-5;
   int                    ii = 0;

   average = 0;
   srand( (unsigned)time( NULL ) );
   for( ii=0; ii<NUM_DATA_LARGE; ii++ ){
      intData = (int) NUM_DATA_LARGE * (rand() / (RAND_MAX + 1.0));
      BTree_InsertNode(data->numList, (void*)&intData, sizeof(int));
      average += intData;
   }
   /* \nCalculating the average of all the numbers stored in the tree\n */
   average /= NUM_DATA_LARGE;
   
   /* Using the parseTreeFunction to parse the entire tree and calculate the average of all
    *  the numnbers stored in the tree.
    * The parseTreeFunction returns the sum of all the numbers in the tree via
    * parseArguments.result, where parseArguments is a user specified struct. */

   parseArguments.result = 0;
   BTree_ParseTree( data->numList, treeParseFunction, (void*) &parseArguments );
   parsedAverage = parseArguments.result / NUM_DATA_LARGE;

   pcu_check_true( fabs(average - parsedAverage) < epsilon );
}

void BTreeSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, BTreeSuiteData );
   pcu_suite_setFixtures( suite, BTreeSuite_Setup, BTreeSuite_Teardown );
   pcu_suite_addTest( suite, BTreeSuite_TestInsert );
   pcu_suite_addTest( suite, BTreeSuite_TestDelete );
   pcu_suite_addTest( suite, BTreeSuite_TestFindNode );
   pcu_suite_addTest( suite, BTreeSuite_TestComplexityPerformance );
   pcu_suite_addTest( suite, BTreeSuite_TestParseTreeForAverage );
}


