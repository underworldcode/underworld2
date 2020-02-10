/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h" /* For Journal stuff */
#include "NamedObject_RegisterSuite.h"
#include "NamedObject_Register2Suite.h"

typedef struct {
	__Stg_Object
} TestObject2;

Stg_Object* TestObject2_New( Name name ) {
	/* Variables set in this function */
	SizeT                             _sizeOfSelf = sizeof( TestObject2 );
	Type                                     type = "TestObject";
	Stg_Class_DeleteFunction*             _delete = _Stg_Object_Delete;
	Stg_Class_PrintFunction*               _print = _Stg_Object_Print;
	Stg_Class_CopyFunction*                 _copy = _Stg_Object_Copy;
	AllocationType             nameAllocationType = NON_GLOBAL;

	return _Stg_Object_New(  STG_OBJECT_PASSARGS  );
}

typedef struct {
   NamedObject_Register* reg;
   Stg_Object**          testObjects;
   char**                testObjectNames;
   Index                 testObjectsCount;
} NamedObject_RegisterSuite2Data;

void NamedObject_RegisterSuite2_Setup( NamedObject_RegisterSuite2Data* data ) {
   Index  ii=0;
   char   letter='0';

   data->reg = NamedObject_Register_New();
   data->testObjectsCount = 5;
   data->testObjectNames = (char**)malloc(sizeof(char*) * data->testObjectsCount);
   data->testObjects = (Stg_Object**)malloc(sizeof(Stg_Object*) * data->testObjectsCount);

   letter='a';
   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      data->testObjectNames[ii] = (char*)malloc(sizeof(char) * 2 );
      sprintf( data->testObjectNames[ii], "%c", letter );
      letter++;
      data->testObjects[ii] = TestObject2_New( data->testObjectNames[ii] );
   }
}

void NamedObject_RegisterSuite2_Teardown( NamedObject_RegisterSuite2Data* data ) {
   Index ii;

   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      free( data->testObjectNames[ii] );
   }
   free( data->testObjects );
   free( data->testObjectNames );
}

void NamedObject_RegisterSuite2_TestDeleteAll( NamedObject_RegisterSuite2Data* data ) {
   Index ii;

   pcu_docstring( "Tests a series of new objects can be added to a register, and the register's data "
      "fields are updated correctly" );

   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      NamedObject_Register_Add( data->reg, data->testObjects[ii] );
   }

   pcu_check_true( data->testObjectsCount == data->reg->objects->count );
   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      pcu_check_true( data->testObjects[ii] == data->reg->objects->data[ii] );
   }

   NamedObject_Register_DeleteAll( data->reg );
}


void NamedObject_Register2Suite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, NamedObject_RegisterSuite2Data );
   pcu_suite_setFixtures( suite, NamedObject_RegisterSuite2_Setup, NamedObject_RegisterSuite2_Teardown );
   pcu_suite_addTest( suite, NamedObject_RegisterSuite2_TestDeleteAll );
}


