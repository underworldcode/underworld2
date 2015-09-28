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
#include "LiveComponentRegisterSuite.h"

typedef struct {
   LiveComponentRegister*  lcRegister;
} LiveComponentRegisterSuiteData;


void LiveComponentRegisterSuite_Setup( LiveComponentRegisterSuiteData* data ) {
   data->lcRegister = LiveComponentRegister_New( );
}

void LiveComponentRegisterSuite_Teardown( LiveComponentRegisterSuiteData* data ) {
   Stg_Class_Delete( data->lcRegister );
}
   

void LiveComponentRegisterSuite_TestGet( LiveComponentRegisterSuiteData* data ) {
   typedef float Triple[3];

   float* array;
   Triple* structArray;

   Variable* var;
   Variable* vec;
   Variable* vecVar[3];
   Variable* tempVar = NULL;
   Index length = 10;

   Variable_Register* reg;

   array = Memory_Alloc_Array( float, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   reg = Variable_Register_New();

   var = Variable_NewScalar(
      "Scalar",
		NULL,
      Variable_DataType_Float,
      &length,
      NULL,
      (void**)&array,
      reg );

   vec = Variable_NewVector(
      "Three",
		NULL,
      Variable_DataType_Float,
      3,
      &length,
      NULL,
      (void**)&structArray,
      reg,
      "a",
      "b",
      "c" );

   vecVar[0] = Variable_Register_GetByName( reg, "a" );
   vecVar[1] = Variable_Register_GetByName( reg, "b" );
   vecVar[2] = Variable_Register_GetByName( reg, "c" );

   Variable_Register_BuildAll( reg );

   LiveComponentRegister_Add( data->lcRegister, (Stg_Component*) var );
   pcu_check_true( LiveComponentRegister_IfRegThenAdd( (Stg_Component*) vec ) );
   LiveComponentRegister_Add( data->lcRegister, (Stg_Component*) vecVar[0] );
   LiveComponentRegister_Add( data->lcRegister, (Stg_Component*) vecVar[1] );
   LiveComponentRegister_Add( data->lcRegister, (Stg_Component*) vecVar[2] );

   tempVar = (Variable*) LiveComponentRegister_Get( data->lcRegister, (Name)"Scalar" );
   pcu_check_true( tempVar == var );

   tempVar = (Variable* ) LiveComponentRegister_Get( LiveComponentRegister_GetLiveComponentRegister(), (Name)"Three" );
   pcu_check_true( tempVar == vec );

   tempVar = (Variable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"a" );
   pcu_check_true( tempVar == vecVar[0] );

   tempVar = (Variable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"b" );
   pcu_check_true( tempVar == vecVar[1] );

   tempVar = (Variable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"c" );
   pcu_check_true( tempVar == vecVar[2] );
}


void LiveComponentRegisterSuite( pcu_suite_t* suite  ) {
   pcu_suite_setData( suite, LiveComponentRegisterSuiteData );
   pcu_suite_setFixtures( suite, LiveComponentRegisterSuite_Setup, LiveComponentRegisterSuite_Teardown );
   pcu_suite_addTest( suite, LiveComponentRegisterSuite_TestGet );
}


