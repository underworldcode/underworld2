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
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h" /* For Journal stuff */
#include "CommonRoutinesSuite.h"

#define N_TEST_VALUES 7

typedef struct {
   Index  nTestValues;
   double testValues[N_TEST_VALUES];
   double tolerances[N_TEST_VALUES];
} CommonRoutinesSuiteData;


void CommonRoutinesSuite_Setup( CommonRoutinesSuiteData* data ) {
   data->nTestValues = 7;
   data->testValues[0] = 0.0;
   data->testValues[1] = 3.4562e-30;
   data->testValues[2] = 9.7324;
   data->testValues[3] = 97.654;
   data->testValues[4] = 104.321;
   data->testValues[5] = -13762.1;
   data->testValues[6] = 0.0043253;
   data->tolerances[0] = 1e-40;
   data->tolerances[1] = 1e-40;
   data->tolerances[2] = 1e-12;
   data->tolerances[3] = 1e-12;
   data->tolerances[4] = 1e-12;
   data->tolerances[5] = 1e-12;
   data->tolerances[6] = 1e-12;
}


void CommonRoutinesSuite_Teardown( CommonRoutinesSuiteData* data ) {
}


void CommonRoutinesSuite_TestLMS( CommonRoutinesSuiteData* data ) {
   pcu_check_true( 1 == Stg_LongestMatchingSubsequenceLength( (char *)"Acrobat", (char *)"BOAT", True )  );
   pcu_check_true( 6 == Stg_LongestMatchingSubsequenceLength( (char *)"Abracadabra", (char *)"Yabbadabbadoo", True )  );
   pcu_check_true( 7 == Stg_LongestMatchingSubsequenceLength( (char *)"Abracadabra", (char *)"Yabbadabbadoo", False )  );
   pcu_check_true( 6 == Stg_LongestMatchingSubsequenceLength( (char *)"Python", (char *)"PythonShape", False ) );
}


void CommonRoutinesSuite_TestStringIsNumeric( CommonRoutinesSuiteData* data ) {
   pcu_check_true( False == Stg_StringIsNumeric( (char *)"nan" ) );
   pcu_check_true( True == Stg_StringIsNumeric( "567" ) );
   pcu_check_true( True == Stg_StringIsNumeric( "1.0e90" ) );
   pcu_check_true( False ==  Stg_StringIsNumeric( "1e90e80" ) );
   pcu_check_true( False == Stg_StringIsNumeric(".asdfasdf" ) );
   pcu_check_true( True == Stg_StringIsNumeric(".0032" ) );
   pcu_check_true( False == Stg_StringIsNumeric(".0032.00" ) );
}
 
  
void CommonRoutinesSuite_TestStringIsEmpty( CommonRoutinesSuiteData* data ) {
   pcu_check_true( True  == Stg_StringIsEmpty( (char *)"\t \n" ) );
   pcu_check_true( False == Stg_StringIsEmpty( "asdf" ) );
   pcu_check_true( True  == Stg_StringIsEmpty( "    " ) );
   pcu_check_true( True  == Stg_StringIsEmpty( "    \n" ) );
   pcu_check_true( False == Stg_StringIsEmpty( "  sdf  \n" ) );
}


void CommonRoutinesSuite_TestStG_RoundDoubleToNDecimalPlaces( CommonRoutinesSuiteData* data    ) {
   double roundedValue;
   double errorMargin;
   Index testValue_I;
   Index nDecPlaces;
   double expectedRoundedToDecPlaces[7][4] = {
      { 0.0, 0.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0, 0.0 },
      { 10., 9.7, 9.73, 9.732 },
      { 98., 97.7, 97.65, 97.654 },
      { 104., 104.3, 104.32, 104.321 },
      { -13762., -13762.1, -13762.10, -13762.100 },
      { 0.,    0.0, 0.00, 0.004 } };

   for ( testValue_I = 0; testValue_I < data->nTestValues; testValue_I++ ) {
      for ( nDecPlaces = 0; nDecPlaces <=3; nDecPlaces++ ) {
         roundedValue = StG_RoundDoubleToNDecimalPlaces(
            data->testValues[testValue_I], nDecPlaces );
         errorMargin = fabs( roundedValue -
            expectedRoundedToDecPlaces[testValue_I][nDecPlaces] );

         pcu_check_le( errorMargin, data->tolerances[testValue_I] );
      }
   }
}


void CommonRoutinesSuite_TestStG_RoundDoubleToNSigFigs( CommonRoutinesSuiteData* data ) {
   double roundedValue;
   double errorMargin;
   Index testValue_I;
   Index nSigFigs;
   double expectedRoundedToSigFigs[7][4] = {
      { 0.0, 0.0, 0.0, 0.0 },
      { 0.0, 3e-30, 3.5e-30, 3.46e-30 },
      { 0., 10, 9.7, 9.73 },
      { 0., 100, 98, 97.7 },
      { 0., 100, 100, 104 },
      { 0., -10000, -14000, -13800 },
      { 0., 0.004, 0.0043, 0.00433 } };

   for ( testValue_I = 0; testValue_I < data->nTestValues; testValue_I++ ) {
      for ( nSigFigs = 1; nSigFigs <=3; nSigFigs++ ) {
         roundedValue = StG_RoundDoubleToNSigFigs(
            data->testValues[testValue_I], nSigFigs );
         errorMargin = fabs( roundedValue -
            expectedRoundedToSigFigs[testValue_I][nSigFigs] );
         pcu_check_le( errorMargin, data->tolerances[testValue_I] );
      }
   }
}


void CommonRoutinesSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, CommonRoutinesSuiteData );
   pcu_suite_setFixtures( suite, CommonRoutinesSuite_Setup, CommonRoutinesSuite_Teardown );
   pcu_suite_addTest( suite, CommonRoutinesSuite_TestLMS );
   pcu_suite_addTest( suite, CommonRoutinesSuite_TestStringIsNumeric );
   pcu_suite_addTest( suite, CommonRoutinesSuite_TestStringIsEmpty );
   pcu_suite_addTest( suite, CommonRoutinesSuite_TestStG_RoundDoubleToNDecimalPlaces );
   pcu_suite_addTest( suite, CommonRoutinesSuite_TestStG_RoundDoubleToNSigFigs );
}


