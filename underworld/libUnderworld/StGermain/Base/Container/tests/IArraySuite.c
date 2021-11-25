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

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "IArraySuite.h"

#define NUM_ITEMS 5

typedef struct {
   IArray*  iArray;
   int      arrayData[NUM_ITEMS];
} IArraySuiteData;


void IArraySuite_Setup( IArraySuiteData* data ) {
   Index         idx;

   data->iArray = IArray_New();
   for( idx = 0; idx < NUM_ITEMS; idx++ ) {
      data->arrayData[idx] = idx;
   }
}


void IArraySuite_Teardown( IArraySuiteData* data ) {
   Stg_Class_Delete( data->iArray );
}


void IArraySuite_TestSet( IArraySuiteData* data ) {
   const int* ptr;
   int i_i;

   IArray_Set( data->iArray, NUM_ITEMS, data->arrayData );
   pcu_check_true( IArray_GetSize( data->iArray ) == NUM_ITEMS );
   pcu_check_true( (ptr = IArray_GetPtr( data->iArray )) != 0 );
   for( i_i = 0; i_i < NUM_ITEMS; i_i++ ) {
      pcu_check_true( ptr[i_i] == i_i );
   }
}


void IArraySuite_TestAdd( IArraySuiteData* data ) {
   const int* ptr;
   int i_i;

   IArray_Set( data->iArray, 3, data->arrayData );
   IArray_Add( data->iArray, 2, data->arrayData + 3 );
   pcu_check_true( IArray_GetSize( data->iArray ) == NUM_ITEMS );
   pcu_check_true( (ptr = IArray_GetPtr( data->iArray )) != 0 );
   for( i_i = 0; i_i < NUM_ITEMS; i_i++ ) {
      pcu_check_true( ptr[i_i] == i_i );
   }
}


void IArraySuite_TestRemove( IArraySuiteData* data ) {
   const int*  ptr;
   IMap*       map = NULL;

   IArray_Set( data->iArray, NUM_ITEMS, data->arrayData );
   data->arrayData[0] = 1; data->arrayData[1] = 3;
   map = IMap_New();
   IArray_Remove( data->iArray, 2, data->arrayData, map );
   pcu_check_true( IArray_GetSize( data->iArray ) == 3 );
   pcu_check_true( (ptr = IArray_GetPtr( data->iArray )) != 0 );
   pcu_check_true( ptr[0] == 0 && ptr[1] == 4 && ptr[2] == 2 );
   pcu_check_true( IMap_GetSize( map ) == 1 );
   pcu_check_true( IMap_Has( map, 4 ) );
   pcu_check_true( IMap_Map( map, 4 ) == 1 );

   Stg_Class_Delete( map );
}


void IArraySuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, IArraySuiteData );
   pcu_suite_setFixtures( suite, IArraySuite_Setup, IArraySuite_Teardown );
   pcu_suite_addTest( suite, IArraySuite_TestSet );
   pcu_suite_addTest( suite, IArraySuite_TestAdd );
   pcu_suite_addTest( suite, IArraySuite_TestRemove );
}


