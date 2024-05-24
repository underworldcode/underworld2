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
#include "IndexMapSuite.h"

typedef struct {
   IndexMap*      map;
} IndexMapSuiteData;


void IndexMapSuite_Setup( IndexMapSuiteData* data ) {
   data->map = IndexMap_New();
}

void IndexMapSuite_Teardown( IndexMapSuiteData* data ) {
   Stg_Class_Delete( data->map );
}


void IndexMapSuite_TestAppendFind( IndexMapSuiteData* data ) {
   Index          idx;

   pcu_docstring( "This test inserts a set of indices to an IndexSet in reverse order, "
      " then checks they can be found at the correct indices." );
   
   for( idx = 0; idx < 100; idx++ ) {
      IndexMap_Append( data->map, idx + 1, 100 - idx );
   }
   
   for( idx = 0; idx < 100; idx++ ) {
      pcu_check_true( IndexMap_Find( data->map, idx + 1 ) == (100 - idx) );
   }
}


void IndexMapSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, IndexMapSuiteData );
   pcu_suite_setFixtures( suite, IndexMapSuite_Setup, IndexMapSuite_Teardown );
   pcu_suite_addTest( suite, IndexMapSuite_TestAppendFind );
}


