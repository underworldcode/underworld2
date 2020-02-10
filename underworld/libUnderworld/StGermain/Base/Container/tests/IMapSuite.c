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
#include "IMapSuite.h"

#define NUM_ITEMS 5

typedef struct {
   IMap*    iMap;
} IMapSuiteData;


void IMapSuite_Setup( IMapSuiteData* data ) {
   data->iMap = IMap_New();
}


void IMapSuite_Teardown( IMapSuiteData* data ) {
   Stg_Class_Delete( data->iMap );
}


void IMapSuite_TestConstruct( IMapSuiteData* data ) {

   pcu_check_true( data->iMap );
   pcu_check_true( data->iMap->maxSize == 0 );
   pcu_check_true( data->iMap->curSize == 0 );
}


void IMapSuite_TestSetMaxSize( IMapSuiteData* data ) {
   IMap_SetMaxSize( data->iMap, 10 );
   pcu_check_true( data->iMap->maxSize == 10 );
   pcu_check_true( data->iMap->tblSize >= 10 );
   IMap_SetMaxSize( data->iMap, 20 );
   pcu_check_true( data->iMap->maxSize == 20 );
   pcu_check_true( data->iMap->tblSize >= 20 );
}


void IMapSuite_TestInsert( IMapSuiteData* data ) {
   int i_i;

   IMap_SetMaxSize( data->iMap, 10 );
   for( i_i = 0; i_i < 20; i_i += 2 ) {
      IMap_Insert( data->iMap, i_i, i_i + 100 );
   }
   pcu_check_assert( IMap_Insert( data->iMap, 0, 100 ) );
   pcu_check_true( IMap_GetSize( data->iMap ) == 10 );
   for( i_i = 0; i_i < 20; i_i += 2 ) {
      pcu_check_true( IMap_Has( data->iMap, i_i ) );
   }
}


void IMapSuite_TestMap( IMapSuiteData* data ) {
   int i_i;

   IMap_SetMaxSize( data->iMap, 10 );
   for( i_i = 0; i_i < 20; i_i += 2 ) {
      IMap_Insert( data->iMap, i_i, i_i + 100 );
   }
   for( i_i = 0; i_i < 20; i_i += 2 ) {
      pcu_check_true( IMap_Map( data->iMap, i_i ) == i_i + 100 );
   }
}


void IMapSuite_TestRemove( IMapSuiteData* data ) {
   int i_i;

   IMap_SetMaxSize( data->iMap, 10 );
   for( i_i = 0; i_i < 20; i_i += 2 ) {
      IMap_Insert( data->iMap, i_i, i_i + 100 );
   }
   for( i_i = 0; i_i < 20; i_i += 4 ) {
      IMap_Remove( data->iMap, i_i );
   }
   pcu_check_assert( IMap_Remove( data->iMap, 0 ) );
   pcu_check_true( IMap_GetSize( data->iMap ) == 5 );
   for( i_i = 0; i_i < 20; i_i += 4 ) {
      if( i_i % 4 ) {
	      pcu_check_true( IMap_Has( data->iMap, i_i ) );
      }
      else {
	      pcu_check_true( !IMap_Has( data->iMap, i_i ) );
      }
   }
}


void IMapSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, IMapSuiteData );
   pcu_suite_setFixtures( suite, IMapSuite_Setup, IMapSuite_Teardown );
   pcu_suite_addTest( suite, IMapSuite_TestConstruct );
   pcu_suite_addTest( suite, IMapSuite_TestSetMaxSize );
   pcu_suite_addTest( suite, IMapSuite_TestInsert );
   pcu_suite_addTest( suite, IMapSuite_TestMap );
   pcu_suite_addTest( suite, IMapSuite_TestRemove );
}


