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
#include "PtrMapSuite.h"

typedef struct {
   PtrMap*    map;
} PtrMapSuiteData;


void PtrMapSuite_Setup( PtrMapSuiteData* data ) {
   data->map = PtrMap_New( 10 );
}


void PtrMapSuite_Teardown( PtrMapSuiteData* data ) {
   Stg_Class_Delete( data->map );
}


void PtrMapSuite_TestAppendFind( PtrMapSuiteData* data ) {
   ArithPointer		idx;
   
   for( idx = 0; idx < 100; idx++ ) {
      PtrMap_Append( data->map, (void*)(idx + 1), (void*)(100 - idx) );
   }
   
   for( idx = 0; idx < 100; idx++ ) {
      pcu_check_true( (ArithPointer)PtrMap_Find( data->map, (void*)(idx + 1) )
         == (100 - idx) );
   }
}


void PtrMapSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, PtrMapSuiteData );
   pcu_suite_setFixtures( suite, PtrMapSuite_Setup, PtrMapSuite_Teardown );
   pcu_suite_addTest( suite, PtrMapSuite_TestAppendFind );
}


