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
#include "StGermain/Base/Context/src/Context.h"
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

   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];
   StgVariable* tempVar = NULL;
   Index length = 10;

   Variable_Register* reg;

   array = Memory_Alloc_Array( float, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   reg = Variable_Register_New();

   var = StgVariable_NewScalar(
      "Scalar",
		NULL,
      StgVariable_DataType_Float,
      &length,
      NULL,
      (void**)&array,
      reg );

   vec = StgVariable_NewVector(
      "Three",
		NULL,
      StgVariable_DataType_Float,
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

   tempVar = (StgVariable*) LiveComponentRegister_Get( data->lcRegister, (Name)"Scalar" );
   pcu_check_true( tempVar == var );

   tempVar = (StgVariable* ) LiveComponentRegister_Get( LiveComponentRegister_GetLiveComponentRegister(), (Name)"Three" );
   pcu_check_true( tempVar == vec );

   tempVar = (StgVariable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"a" );
   pcu_check_true( tempVar == vecVar[0] );

   tempVar = (StgVariable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"b" );
   pcu_check_true( tempVar == vecVar[1] );

   tempVar = (StgVariable* ) LiveComponentRegister_Get( data->lcRegister, (Name)"c" );
   pcu_check_true( tempVar == vecVar[2] );
}


void LiveComponentRegisterSuite( pcu_suite_t* suite  ) {
   pcu_suite_setData( suite, LiveComponentRegisterSuiteData );
   pcu_suite_setFixtures( suite, LiveComponentRegisterSuite_Setup, LiveComponentRegisterSuite_Teardown );
   pcu_suite_addTest( suite, LiveComponentRegisterSuite_TestGet );
}


