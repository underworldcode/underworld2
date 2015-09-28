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
#include "STreeMapSuite.h"

typedef struct {
   STreeMap* sTreeMap;
} STreeMapSuiteData;

void STreeMapSuite_Setup( STreeMapSuiteData* data ) {
   data->sTreeMap = STreeMap_New();
   STreeMap_SetItemSize( data->sTreeMap, sizeof(int), sizeof(int) );
   STree_SetIntCallbacks( data->sTreeMap );
}

void STreeMapSuite_Teardown( STreeMapSuiteData* data ) {
   Stg_Class_Delete( data->sTreeMap );
}

void STreeMapSuite_TestConstruct( STreeMapSuiteData* data ) {
   pcu_check_true( data->sTreeMap );
}

void STreeMapSuite_TestInsert( STreeMapSuiteData* data ) {
   int c_i;

   for ( c_i = 0; c_i < 10; c_i++ ) {
      int tmp = 10 * c_i;
      STreeMap_Insert( data->sTreeMap, &c_i, &tmp );
   }
   pcu_check_true( STree_GetSize( data->sTreeMap ) == 10 );
}

void STreeMapSuite_TestMap( STreeMapSuiteData* data ) {
   int c_i;

   for ( c_i = 0; c_i < 10; c_i++ ) {
      int tmp = 10 * c_i;
      STreeMap_Insert( data->sTreeMap, &c_i, &tmp );
   }
   for( c_i = 0; c_i < 10; c_i++ ) {
      pcu_check_true( *((int*)STreeMap_Map( data->sTreeMap, &c_i )) == 10 * c_i );
   }
}

void STreeMapSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, STreeMapSuiteData );
   pcu_suite_setFixtures( suite, STreeMapSuite_Setup, STreeMapSuite_Teardown );
   pcu_suite_addTest( suite, STreeMapSuite_TestConstruct );
   pcu_suite_addTest( suite, STreeMapSuite_TestInsert );
   pcu_suite_addTest( suite, STreeMapSuite_TestMap );
}


