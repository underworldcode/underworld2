/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h" /* For Journal stuff */
#include "NamedObject_RegisterSuite.h"

typedef struct {
	__Stg_Object
} TestObject;

Stg_Object* TestObject_New( Name name ) {
	/* Variables set in this function */
	SizeT                             _sizeOfSelf = sizeof( TestObject );
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
} NamedObject_RegisterSuiteData;

void NamedObject_RegisterSuite_Setup( NamedObject_RegisterSuiteData* data ) {
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
      data->testObjects[ii] = TestObject_New( data->testObjectNames[ii] );
   }
}

void NamedObject_RegisterSuite_Teardown( NamedObject_RegisterSuiteData* data ) {
   Index ii;

   _NamedObject_Register_Delete( data->reg );
   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      _Stg_Object_Delete( data->testObjects[ii] );
      free( data->testObjectNames[ii] );
   }
   free( data->testObjects );
   free( data->testObjectNames );
}

void NamedObject_RegisterSuite_TestAdd( NamedObject_RegisterSuiteData* data ) {
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
}


void NamedObject_RegisterSuite_TestGetFunctions( NamedObject_RegisterSuiteData* data ) {
   Index ii;

   pcu_docstring( "Tests objects can be searched and got from the register, by both name and index." );

   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      NamedObject_Register_Add( data->reg, data->testObjects[ii] );
   }

   for (ii=0; ii < data->testObjectsCount; ii++ ) {
      pcu_check_true( ii == NamedObject_Register_GetIndex( data->reg,
         data->testObjectNames[ii] ) );
      pcu_check_true( data->testObjects[ii] == NamedObject_Register_GetByIndex( data->reg, ii ) );
      pcu_check_true( data->testObjects[ii] == NamedObject_Register_GetByName( data->reg,
         data->testObjectNames[ii] ) );
   }
}

void NamedObject_RegisterSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, NamedObject_RegisterSuiteData );
   pcu_suite_setFixtures( suite, NamedObject_RegisterSuite_Setup, NamedObject_RegisterSuite_Teardown );
   pcu_suite_addTest( suite, NamedObject_RegisterSuite_TestAdd );
   pcu_suite_addTest( suite, NamedObject_RegisterSuite_TestGetFunctions );
}


