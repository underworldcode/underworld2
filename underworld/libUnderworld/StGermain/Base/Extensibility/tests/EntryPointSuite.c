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

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"
#include "EntryPointSuite.h"

Stream* stream;

#define NUM_TEST_FUNCS 10

typedef struct {
   EntryPoint*	ep;
   Bool			testFuncsRan[NUM_TEST_FUNCS];
   int			rank;
} EntryPointSuiteData;

void TestHook0( EntryPointSuiteData* data ) {
   data->testFuncsRan[0] = True;
}

void TestHook1( EntryPointSuiteData* data ) {
   data->testFuncsRan[1] = True;
}

void TestHook2( EntryPointSuiteData* data ) {
   data->testFuncsRan[2] = True;
}

void TestHook3( EntryPointSuiteData* data ) {
   data->testFuncsRan[3] = True;
}

void TestHook4( EntryPointSuiteData* data ) {
   data->testFuncsRan[4] = True;
}

void TestHook5( EntryPointSuiteData* data ) {
   data->testFuncsRan[5] = True;
}

void TestHook6( EntryPointSuiteData* data ) {
   data->testFuncsRan[6] = True;
}

void TestHook7( EntryPointSuiteData* data ) {
   data->testFuncsRan[7] = True;
}

void TestHook8( EntryPointSuiteData* data ) {
   data->testFuncsRan[8] = True;
}

void TestHook9( EntryPointSuiteData* data ) {
   data->testFuncsRan[9] = True;
}


void EntryPointSuite_Setup( EntryPointSuiteData* data ) {
   Index ii;

   data->ep = NULL;
   for (ii=0; ii < NUM_TEST_FUNCS; ii++ ) {
      data->testFuncsRan[ii] = False;
   }
   MPI_Comm_rank( MPI_COMM_WORLD, &data->rank );
}


void EntryPointSuite_Teardown( EntryPointSuiteData* data ) {
   Stg_Class_Delete( data->ep );
}


void EntryPointSuite_TestRunEmpty( EntryPointSuiteData* data ) {
   data->ep = EntryPoint_New( "test", EntryPoint_VoidPtr_CastType );
   pcu_check_true( data->ep->hooks->count == 0 );
   ((EntryPoint_VoidPtr_CallCast*) data->ep->run)( data->ep, NULL );
}


void EntryPointSuite_TestAppendPrepend( EntryPointSuiteData* data ) {
   Index    ii=0;

   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_0_CastType );
   EntryPoint_Append( data->ep, "TestHook0", (void*)TestHook0, "testCode" );
   /* TestHook0 */
   EntryPoint_Prepend( data->ep, (Name)"TestHook1", (void*)TestHook1, "testCode" );
   /* TestHook1, TestHook0 */

   pcu_check_true( data->ep->hooks->count == 2 );
   pcu_check_streq( data->ep->hooks->data[0]->name, "TestHook1" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[0])->funcPtr == TestHook1 );
   pcu_check_streq( data->ep->hooks->data[1]->name, "TestHook0" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[1])->funcPtr == TestHook0 );
   for (ii=0; ii < data->ep->hooks->count; ii++ ) {
      pcu_check_streq( ((Hook*)data->ep->hooks->data[ii])->addedBy, "testCode" );
   }
}


void EntryPointSuite_TestInsertBeforeAfterReplace( EntryPointSuiteData* data ) {
   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_0_CastType );
   EntryPoint_Append( data->ep, "TestHook2", (void*)TestHook2, "testCode" );
   EntryPoint_Append( data->ep, "TestHook3", (void*)TestHook3, "testCode" );
   /* TestHook2, TestHook3 */
   EntryPoint_Prepend( data->ep, (Name)"TestHook4", (void*)TestHook4, "testCode" );
   /* TestHook4, TestHook2, TestHook3 */
   EntryPoint_InsertBefore( data->ep, "TestHook3", "TestHook5", (void*)TestHook5, "testCode" );
   /* TestHook4, TestHook2, TestHook5, TestHook3 */
   EntryPoint_InsertAfter( data->ep, "TestHook4", "TestHook6", (void*)TestHook6, "testCode" );
   /* TestHook4, TestHook6, TestHook2, TestHook5, TestHook3 */
   EntryPoint_Replace( data->ep, "TestHook5", "TestHook7", (void*)TestHook7, "testCode" );
   /* TestHook4, TestHook6, TestHook2, TestHook7, TestHook3 */

   pcu_check_true( data->ep->hooks->count == 5 );
   pcu_check_streq( data->ep->hooks->data[0]->name, "TestHook4" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[0])->funcPtr == TestHook4 );
   pcu_check_streq( data->ep->hooks->data[1]->name, "TestHook6" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[1])->funcPtr == TestHook6 );
   pcu_check_streq( data->ep->hooks->data[2]->name, "TestHook2" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[2])->funcPtr == TestHook2 );
   pcu_check_streq( data->ep->hooks->data[3]->name, "TestHook7" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[3])->funcPtr == TestHook7 );
   pcu_check_streq( data->ep->hooks->data[4]->name, "TestHook3" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[4])->funcPtr == TestHook3 );
}


void EntryPointSuite_TestAlwaysFirstLast( EntryPointSuiteData* data ) {
   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_0_CastType );
   EntryPoint_Append_AlwaysLast( data->ep, "TestHook8", (void*)TestHook8, "testCode" );
   /* - TestHook8 */
   EntryPoint_Append( data->ep, "TestHook9", (void*)TestHook9, "testCode" );
   /* TestHook9 - TestHook8 */
   EntryPoint_Prepend_AlwaysFirst( data->ep, "TestHook0", (void*)TestHook0, "testCode" );
   /* TestHook0 - TestHook9 - TestHook8 */
   EntryPoint_Prepend( data->ep, (Name)"TestHook1", (void*)TestHook1, "testCode" );
   /* TestHook0 - TestHook1, TestHook9 - TestHook8 */

   pcu_check_true( data->ep->hooks->count == 4 );
   pcu_check_streq( data->ep->hooks->data[0]->name, "TestHook0" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[0])->funcPtr == TestHook0 );
   pcu_check_streq( data->ep->hooks->data[1]->name, "TestHook1" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[1])->funcPtr == TestHook1 );
   pcu_check_streq( data->ep->hooks->data[2]->name, "TestHook9" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[2])->funcPtr == TestHook9 );
   pcu_check_streq( data->ep->hooks->data[3]->name, "TestHook8" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[3])->funcPtr == TestHook8 );
   pcu_check_streq( data->ep->alwaysFirstHook->name, "TestHook0" );
   pcu_check_true( ((Hook*)data->ep->alwaysFirstHook)->funcPtr == TestHook0 );
   pcu_check_streq( data->ep->alwaysLastHook->name, "TestHook8" );
   pcu_check_true( ((Hook*)data->ep->alwaysLastHook)->funcPtr == TestHook8 );
}


void EntryPointSuite_TestReplaceAll( EntryPointSuiteData* data ) {
   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_0_CastType );
   EntryPoint_Append( data->ep, "TestHook0", (void*)TestHook0, "testCode" );
   EntryPoint_Append( data->ep, "TestHook1", (void*)TestHook0, "testCode" );
   /* TestHook0, TestHook1 */
   EntryPoint_ReplaceAll( data->ep, "TestHook2", (void*)TestHook2, "testCode" );
   /* TestHook2 */
   pcu_check_true( data->ep->hooks->count == 1 );
   pcu_check_streq( data->ep->hooks->data[0]->name, "TestHook2" );
   pcu_check_true( ((Hook*)data->ep->hooks->data[0])->funcPtr == TestHook2 );
}


void EntryPointSuite_TestPurge( EntryPointSuiteData* data ) {
   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_0_CastType );
   EntryPoint_Append( data->ep, "TestHook2", (void*)TestHook2, "testCode" );
   EntryPoint_Append( data->ep, "TestHook3", (void*)TestHook3, "testCode" );
   /* TestHook2, TestHook3 */
   EntryPoint_Purge( data->ep );
   /* */
   pcu_check_true( data->ep->hooks->count == 0 );
}


void EntryPointSuite_TestRun( EntryPointSuiteData* data ) {
   Hook_Index hookIndex;

   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_VoidPtr_CastType );
   EntryPoint_Append( data->ep, "TestHook0", (void*)TestHook0, "testCode" );
   EntryPoint_Append( data->ep, "TestHook1", (void*)TestHook1, "testCode" );
   EntryPoint_Append( data->ep, "TestHook2", (void*)TestHook2, "testCode" );
   EntryPoint_Append( data->ep, "TestHook3", (void*)TestHook3, "testCode" );
   EntryPoint_Append( data->ep, "TestHook4", (void*)TestHook4, "testCode" );
   
   pcu_check_true( data->ep->hooks->count == 5 );

   ((EntryPoint_VoidPtr_CallCast*) data->ep->run)( data->ep, data );

   for (hookIndex = 0; hookIndex < data->ep->hooks->count; hookIndex++ ) {
      pcu_check_true( data->testFuncsRan[hookIndex] == True );
   }
   for (hookIndex = data->ep->hooks->count; hookIndex < NUM_TEST_FUNCS; hookIndex++ ) {
      pcu_check_true( data->testFuncsRan[hookIndex] == False );
   }
}


void EntryPointSuite_TestPrintConcise( EntryPointSuiteData* data ) {
   Stream*        stream = NULL;
   const char*    testFilename = "testEP-PrintConcise.txt";

   data->ep = EntryPoint_New( "testEntryPoint", EntryPoint_VoidPtr_CastType );
   EntryPoint_Append( data->ep, "TestHook0", (void*)TestHook0, "testCode" );
   EntryPoint_Append( data->ep, "TestHook1", (void*)TestHook1, "testCode" );
   EntryPoint_Append( data->ep, "TestHook2", (void*)TestHook2, "testCode" );
   EntryPoint_Append( data->ep, "TestHook3", (void*)TestHook3, "testCode" );
   EntryPoint_Append( data->ep, "TestHook4", (void*)TestHook4, "testCode" );
   
   pcu_check_true( data->ep->hooks->count == 5 );

   stream = Journal_Register( InfoStream_Type, (Name)EntryPoint_Type  );
   Stream_RedirectFile( stream, testFilename );
   EntryPoint_PrintConcise( data->ep, stream );

   if (data->rank==0) {
      char        expectedFilename[PCU_PATH_MAX];

      pcu_filename_expected( testFilename, expectedFilename );
      pcu_check_fileEq( testFilename, expectedFilename );
      remove( testFilename );
   }
}

/***** For the ClassHook test ************************/

#define __Listener \
   __Stg_Class \
   int   number; \
   Bool  hasRun_0_func; \
   Bool  hasRun_VoidPtr_func; \
   int   calcVal;
struct Listener { __Listener };
typedef struct Listener Listener;

Listener* Listener_New( int number ) {
   Listener* result = (Listener*)_Stg_Class_New(
      sizeof( Listener ),
      "Listener",
      _Stg_Class_Delete,
      NULL,
      NULL );
   result->number = number;
   result->hasRun_0_func = False;
   result->hasRun_VoidPtr_func = False;
   result->calcVal = -1;
   return result;
}

void Listener_0_Func( void* ref ) {
   Listener* self = (Listener*) ref;
   self->hasRun_0_func = True;
}
void Listener_VoidPtr_Func( void* ref, void* data0 ) {
   Listener* self = (Listener*) ref;
   int* data = (int*)data0;
   self->hasRun_VoidPtr_func = True;
   self->calcVal = self->number * (*data);
}

void EntryPointSuite_TestClassHook( EntryPointSuiteData* data ) {
   EntryPoint*    classVoidPtr;
   #define        NUM_LISTENERS 3
   Listener*      listeners[NUM_LISTENERS];
   char           hookName[100];
   int            ii;
   int            inputData = 5;

   data->ep = EntryPoint_New( "Class0", EntryPoint_Class_0_CastType );
   classVoidPtr = EntryPoint_New( "Class_VoidPtr", EntryPoint_Class_VoidPtr_CastType );

   for ( ii = 0; ii < NUM_LISTENERS; ++ii ) {
      listeners[ii] = Listener_New( ii );
      sprintf( hookName, "hook%d", ii );
      EntryPoint_AppendClassHook( data->ep, hookName, (void*)Listener_0_Func,
          __FILE__, listeners[ii] );
      EntryPoint_AppendClassHook( classVoidPtr, hookName, (void*)Listener_VoidPtr_Func,
          __FILE__, listeners[ii] );
   }

   pcu_check_true(
      data->ep->hooks->count == NUM_LISTENERS &&
      classVoidPtr->hooks->count ==  NUM_LISTENERS );

   /* Run the entry points */
   ((EntryPoint_Class_0_CallCast*) data->ep->run)( data->ep );
   ((EntryPoint_Class_VoidPtr_CallCast*) classVoidPtr->run)( classVoidPtr, &inputData );

   for ( ii = 0; ii < NUM_LISTENERS; ++ii ) {
      pcu_check_true( listeners[ii]->hasRun_0_func == True );
      pcu_check_true( listeners[ii]->hasRun_VoidPtr_func == True );
      pcu_check_true( listeners[ii]->calcVal == ii*inputData );
   }

   for ( ii = 0; ii < NUM_LISTENERS; ++ii ) {
      Stg_Class_Delete( listeners[ii] );
   }

   /* the second EntryPoint won't be automatically deleted by the tearDown func, so delete here */
   Stg_Class_Delete( classVoidPtr );
}

/******** For the MinMax test *****************/
double Return1( Stream* stream ) {
   Journal_Printf( stream, "In func %s\n", __func__ );
   return 1.0;
}
   
double Return89( Stream* stream ) {
   Journal_Printf( stream, "In func %s\n", __func__ );
   return 89.0;
}

double ReturnNeg43( Stream* stream ) {
   Journal_Printf( stream, "In func %s\n", __func__ );
   return -43;
}
double ReturnZero( Stream* stream ) {
   Journal_Printf( stream, "In func %s\n", __func__ );
   return 0.0;
}


void EntryPointSuite_TestMinMax( EntryPointSuiteData* data ) {
   const Name  testEpName = "testEntryPoint";
   double      result;
   Stream*     stream;

   stream = Journal_Register( InfoStream_Type, (Name)"myStream"  );
   Stream_Enable( stream, False );

   data->ep = EntryPoint_New( testEpName, EntryPoint_Maximum_VoidPtr_CastType );
   EntryPoint_Append( data->ep, "TestHook0", Return1, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook1", Return89, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook2", ReturnNeg43, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook3", ReturnZero, "testMinMaxFunc" );
   result = ((EntryPoint_Maximum_VoidPtr_CallCast*) data->ep->run)( data->ep, stream );
   pcu_check_true( result == 89.0 );
   Stg_Class_Delete( data->ep );

   /* Get Minimum of Values */
   data->ep = EntryPoint_New( testEpName, EntryPoint_Minimum_VoidPtr_CastType );
   EntryPoint_Append( data->ep, "TestHook0", Return1, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook1", Return89, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook2", ReturnNeg43, "testMinMaxFunc" );
   EntryPoint_Append( data->ep, "TestHook3", ReturnZero, "testMinMaxFunc" );
   result = ((EntryPoint_Minimum_VoidPtr_CallCast*) data->ep->run)( data->ep, stream );
   pcu_check_true( result == -43 );
}


void EntryPointSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, EntryPointSuiteData );
   pcu_suite_setFixtures( suite, EntryPointSuite_Setup, EntryPointSuite_Teardown );
   pcu_suite_addTest( suite, EntryPointSuite_TestRunEmpty );
   pcu_suite_addTest( suite, EntryPointSuite_TestAppendPrepend );
   pcu_suite_addTest( suite, EntryPointSuite_TestInsertBeforeAfterReplace );
   pcu_suite_addTest( suite, EntryPointSuite_TestAlwaysFirstLast );
   pcu_suite_addTest( suite, EntryPointSuite_TestReplaceAll );
   pcu_suite_addTest( suite, EntryPointSuite_TestPurge );
   pcu_suite_addTest( suite, EntryPointSuite_TestRun );
   pcu_suite_addTest( suite, EntryPointSuite_TestPrintConcise );
   pcu_suite_addTest( suite, EntryPointSuite_TestClassHook );
   pcu_suite_addTest( suite, EntryPointSuite_TestMinMax );
}


