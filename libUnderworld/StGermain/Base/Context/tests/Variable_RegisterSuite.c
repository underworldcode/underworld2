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
#include "Variable_RegisterSuite.h"

typedef struct {
   Variable_Register*	reg;
} Variable_RegisterSuiteData;


void Variable_RegisterSuite_Setup( Variable_RegisterSuiteData* data ) {
   data->reg = Variable_Register_New();
}

void Variable_RegisterSuite_Teardown( Variable_RegisterSuiteData* data ) {
   Stg_Class_Delete( data->reg );
}
   

void Variable_RegisterSuite_TestAddGet( Variable_RegisterSuiteData* data ) {
   Variable*		var[10];
   #define ARRAY_SIZE	4
   #define STRUCT_SIZE	4
   double			array[ARRAY_SIZE];
   Index			   arraySize = ARRAY_SIZE;
   char*			   name[10] = {"testVar0", "testVar1", "testVar2", "testVar3",
                  "testVar4", "testVar5", "testVar6", "testVar7",
                  "testVar8", "testVar9"};
   Index		   	i;

   for (i = 0; i < 10; i++) {
      var[i] = Variable_NewVector( name[i], NULL, Variable_DataType_Double, 4, &arraySize, NULL, (void**)&array, 0 );
   }

   for (i = 0; i < 10; i++)
   {
      Variable_Register_Add(data->reg, var[i]);
   }

   for (i = 0; i < 10; i++) {
      pcu_check_true( i == Variable_Register_GetIndex(data->reg, name[i]));
   }

   for (i = 0; i < 10; i++) {
      pcu_check_true( var[i] == Variable_Register_GetByName(data->reg, name[i]));
   }

   for (i = 0; i < 10; i++) {
      Stg_Class_Delete(var[i]);
   }
}


void Variable_RegisterSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, Variable_RegisterSuiteData );
   pcu_suite_setFixtures( suite, Variable_RegisterSuite_Setup, Variable_RegisterSuite_Teardown );
   pcu_suite_addTest( suite, Variable_RegisterSuite_TestAddGet );
}


