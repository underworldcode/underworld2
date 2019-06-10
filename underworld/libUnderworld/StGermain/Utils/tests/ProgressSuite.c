/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <stdlib.h>
#include <string.h>
#include "pcu/pcu.h"
#include "StGermain/Base/Base.h"
#include "StGermain/Utils/Utils.h"
#include "ProgressSuite.h"


typedef struct {
      Progress* prog;
} ProgressSuiteData;


void ProgressSuite_TestSetStream( ProgressSuiteData* data ) {
   Progress_SetStream( data->prog, NULL );
   pcu_check_true( data->prog->strm == NULL );
   Progress_SetStream( data->prog, (Stream*)1 );
   pcu_check_true( data->prog->strm == (void*)1 );
   Progress_SetStream( data->prog, NULL );
}

void ProgressSuite_TestSetTitle( ProgressSuiteData* data ) {
   Progress_SetTitle( data->prog, NULL );
   pcu_check_true( data->prog->title == NULL );
   Progress_SetTitle( data->prog, "foo" );
   pcu_check_streq( data->prog->title, "foo" );
   Progress_SetTitle( data->prog, NULL );
   pcu_check_true( data->prog->title == NULL );
}

void ProgressSuite_TestSetPrefix( ProgressSuiteData* data ) {
   Progress_SetPrefix( data->prog, NULL );
   pcu_check_true( data->prog->preStr == NULL );
   Progress_SetPrefix( data->prog, "foo" );
   pcu_check_streq( data->prog->preStr, "foo" );
   Progress_SetPrefix( data->prog, NULL );
   pcu_check_true( data->prog->preStr == NULL );
}

void ProgressSuite_TestSetRange( ProgressSuiteData* data ) {
   Progress_SetRange( data->prog, 5, 10 );
   pcu_check_true( data->prog->start == 5 );
   pcu_check_true( data->prog->end == 10 );
   Progress_SetPrefix( data->prog, "foo" );
   pcu_check_streq( data->prog->preStr, "foo" );
   Progress_SetPrefix( data->prog, NULL );
   pcu_check_true( data->prog->preStr == NULL );
}

void ProgressSuite_TestCalcStatus( ProgressSuiteData* data ) {
   int ii;

   Progress_SetRange( data->prog, 0, 100 );
   pcu_check_true( data->prog->perc == 0 );
   for( ii = 0; ii < 99; ii++ ) {
      Progress_Increment( data->prog );
      pcu_check_true( data->prog->perc == ii );
   }
}

void ProgressSuite_Setup( ProgressSuiteData* data ) {
   data->prog = Progress_New();
}

void ProgressSuite_Teardown( ProgressSuiteData* data ) {
   Stg_Class_Delete( data->prog );
}

void ProgressSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ProgressSuiteData );
   pcu_suite_setFixtures( suite, ProgressSuite_Setup, ProgressSuite_Teardown );
   pcu_suite_addTest( suite, ProgressSuite_TestSetStream );
   pcu_suite_addTest( suite, ProgressSuite_TestSetTitle );
   pcu_suite_addTest( suite, ProgressSuite_TestSetPrefix );
   pcu_suite_addTest( suite, ProgressSuite_TestSetRange );
}


