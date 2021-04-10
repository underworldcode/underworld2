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

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "StGermain/Base/Automation/Automation.h"
#include "StGermain/Base/Extensibility/Extensibility.h"
#include "StGermain/Base/Context/Context.h"
#include "ConditionFunctionSuite.h"

typedef struct {
} ConditionFunctionSuiteData;


#define TEST_CF_RESULT 10

void func(Index index, StgVariable_Index var_I, void* context, void* data, void* result)
{
   *((double*)result) = TEST_CF_RESULT;
}


void ConditionFunctionSuite_Setup( ConditionFunctionSuiteData* data ) {
}

void ConditionFunctionSuite_Teardown( ConditionFunctionSuiteData* data ) {
}
   

void ConditionFunctionSuite_TestApply( ConditionFunctionSuiteData* data ) {
   ConditionFunction*	cf;
   double               result;

   cf = ConditionFunction_New( func, (Name)"quadratic", NULL );

   ConditionFunction_Apply(cf, 4, 2, NULL, &result);
   pcu_check_true( TEST_CF_RESULT == result );

   Stg_Class_Delete(cf);
}


void ConditionFunctionSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ConditionFunctionSuiteData );
   pcu_suite_setFixtures( suite, ConditionFunctionSuite_Setup, ConditionFunctionSuite_Teardown );
   pcu_suite_addTest( suite, ConditionFunctionSuite_TestApply );
}


