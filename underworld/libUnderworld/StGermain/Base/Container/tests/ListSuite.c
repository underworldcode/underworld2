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

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "ListSuite.h"

#define NUM_ITEMS  100

typedef struct {
   List* list;
   int   arrayData[NUM_ITEMS];
} ListSuiteData;

void ListSuite_Setup( ListSuiteData* data ) {
   Index idx;

   data->list = List_New();
   List_SetItemSize( data->list, sizeof(int) );
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      data->arrayData[idx] = idx;
   }
}

void ListSuite_Teardown( ListSuiteData* data ) {
   Stg_Class_Delete( data->list );
}

void ListSuite_TestAppend( ListSuiteData* data ) {
   Index idx;

   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      List_Append( data->list, &data->arrayData[idx] );
   }

   pcu_check_true( data->list->nItems == NUM_ITEMS );
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      pcu_check_true( *(int*)List_GetItem( data->list, idx ) == idx );
   }
}

void ListSuite_TestPrepend( ListSuiteData* data ) {
   Index idx;

   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      List_Prepend( data->list, &data->arrayData[idx] );
   }

   pcu_check_true( data->list->nItems == NUM_ITEMS );
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      pcu_check_true( *(int*)List_GetItem( data->list, idx ) == ((NUM_ITEMS-1) - idx) );
   }
}

void ListSuite_TestInsert( ListSuiteData* data ) {
   Index idx;

   /* Initially, insert at the end */
   for( idx = 0; idx < NUM_ITEMS/2; idx++ ) {
      List_Insert( data->list, idx, &data->arrayData[idx] );
   }
   /* Then insert the rest from the half-way point */
   for( idx = NUM_ITEMS/2; idx < NUM_ITEMS; idx++ ) {
      List_Insert( data->list, NUM_ITEMS/2, &data->arrayData[idx] );
   }

   pcu_check_true( data->list->nItems == NUM_ITEMS );

   for( idx = 0; idx < NUM_ITEMS/2; idx++ ) {
      pcu_check_true( *(int*)List_GetItem( data->list, idx ) == idx );
   }
   for( idx = NUM_ITEMS/2; idx < NUM_ITEMS; idx++ ) {
      pcu_check_true( *(int*)List_GetItem( data->list, idx ) == ((NUM_ITEMS-1) - (idx - NUM_ITEMS/2)) );
   }
}

void ListSuite_TestClear( ListSuiteData* data ) {
   Index idx;

   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      List_Append( data->list, &data->arrayData[idx] );
   }
   List_Clear( data->list );
   pcu_check_true( data->list->nItems == 0 );
   List_Clear( data->list );
   pcu_check_true( data->list->nItems == 0 );
}

void ListSuite_TestRemove( ListSuiteData* data ) {
   Index idx;
   Index listIndex=0;

   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      List_Append( data->list, &data->arrayData[idx] );
   }
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      if ( idx % 2 == 0 ) {
         List_Remove( data->list, &data->arrayData[idx] );
      }
   }

   pcu_check_true( data->list->nItems == NUM_ITEMS/2 );
   listIndex=0;
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      if ( idx % 2 == 1 ) {
         pcu_check_true( *(int*)List_GetItem( data->list, listIndex ) == idx );
         listIndex++;
      }
   }
}

void ListSuite_TestExists( ListSuiteData* data ) {
   Index idx;
   int   inArray=5;
   int   notInArray=34352;
 
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      List_Append( data->list, &data->arrayData[idx] );
   }

   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      pcu_check_true( List_Exists( data->list, &data->arrayData[idx] ) == True );
   }
   pcu_check_true( List_Exists( data->list, &inArray ) == True );
   pcu_check_true( List_Exists( data->list, &notInArray ) == False );
}

void ListSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ListSuiteData );
   pcu_suite_setFixtures( suite, ListSuite_Setup, ListSuite_Teardown );
   pcu_suite_addTest( suite, ListSuite_TestAppend );
   pcu_suite_addTest( suite, ListSuite_TestPrepend );
   pcu_suite_addTest( suite, ListSuite_TestInsert );
   pcu_suite_addTest( suite, ListSuite_TestClear );
   pcu_suite_addTest( suite, ListSuite_TestRemove );
   pcu_suite_addTest( suite, ListSuite_TestExists );
}


