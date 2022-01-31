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
#include "UIntMapSuite.h"

typedef struct {
   UIntMap*    map;
   unsigned	   size;
} UIntMapSuiteData;


void UIntMapSuite_Setup( UIntMapSuiteData* data ) {
   data->map = UIntMap_New();
   data->size = 5;
}


void UIntMapSuite_Teardown( UIntMapSuiteData* data ) {
   FreeObject( data->map );
}


void UIntMapSuite_FillMap( UIntMapSuiteData* data ) {
	unsigned	i;

	for( i = 0; i < data->size; i++ ) {
		UIntMap_Insert( data->map, i, data->size + i );
   }
}


void UIntMapSuite_TestInsert( UIntMapSuiteData* data ) {
   UIntMapSuite_FillMap( data );
   pcu_check_true( data->map->size == data->size );
}


void UIntMapSuite_TestMap( UIntMapSuiteData* data ) {
   unsigned	val;
   unsigned	i;

   UIntMapSuite_FillMap( data );

   for( i = 0; i < data->size; i++ ) {
      pcu_check_true( UIntMap_Map( data->map, i, &val ) );
      pcu_check_true( val == data->size + i );
   }
}


void UIntMapSuite_TestMemoryMap( UIntMapSuiteData* data ) {
   unsigned	nReps = 10;
   unsigned	val;
   unsigned	r_i;

   for( r_i = 0; r_i < nReps; r_i++ ) {
      unsigned	i;

      UIntMapSuite_FillMap( data );
      for( i = 0; i < data->size; i++ ) {
         pcu_check_true( UIntMap_Map( data->map, i, &val ) );
         pcu_check_true( val == data->size + i );
      }
      FreeObject( data->map );
      data->map = UIntMap_New();
   }
}


void UIntMapSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, UIntMapSuiteData );
   pcu_suite_setFixtures( suite, UIntMapSuite_Setup, UIntMapSuite_Teardown );
   pcu_suite_addTest( suite, UIntMapSuite_TestInsert );
   pcu_suite_addTest( suite, UIntMapSuite_TestMap );
   pcu_suite_addTest( suite, UIntMapSuite_TestMemoryMap );
}


