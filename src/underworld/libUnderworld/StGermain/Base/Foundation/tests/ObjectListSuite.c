/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h"
#include "ObjectListSuite.h"

const Type DummyClass_Type = "DummyClass_Type";

typedef struct {
   Stg_ObjectList* ol0;
   Stg_ObjectList  ol1;
   Stg_ObjectList* ol2;
   Stg_ObjectList* ol3;
   Stg_Class*      addPtr0;
   Stg_Class*      addPtr1;
   int*            addPtr2;
   int*            addPtr3;
} ObjectListSuiteData;

Stg_Class* DummyClass_New( void ) {
   return _Stg_Class_New( 
      sizeof(Stg_Class),
      DummyClass_Type,
      _Stg_Class_Delete,
      _Stg_Class_Print,
      _Stg_Class_Copy );
}

int* DummyPointer_New( void ) {
   return Memory_Alloc_Unnamed( int );
}

void DummyPointer_Print( void* ptr, struct Stream* stream ) {
   Journal_Printf( stream, "value: %i\n", *(int*)ptr );
}

void* DummyPointer_Copy( void* ptr, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
   int* newInt;
   
   newInt = Memory_Alloc_Unnamed( int );
   *newInt = *(int*)ptr;
   
   return newInt;
}

void DummyFunc1( void ) {
}

void DummyFunc2( void ) {
}

void DummyFunc3( void ) {
}

void DummyFunc4( void ) {
}

void DummyFunc5( void ) {
}

void DummyFunc6( void ) {
}

void ObjectListSuite_Setup( ObjectListSuiteData* data ) {
   data->ol0 = Stg_ObjectList_New();
   Stg_ObjectList_Init( &data->ol1 );
   data->ol2 = Stg_ObjectList_New();
   data->ol3 = Stg_ObjectList_New();
   data->addPtr0 = NULL;
   data->addPtr1 = NULL;
   data->addPtr2 = NULL;
   data->addPtr3 = NULL;
}

void ObjectListSuite_Teardown( ObjectListSuiteData* data ) {
   Stg_Class_Delete( data->ol3 );
   Stg_Class_Delete( data->ol2 );
   Stg_Class_Delete( &data->ol1 );
   Stg_Class_Delete( data->ol0 );
}

/* Test 2: Can we append the first entry? */
void ObjectListSuite_TestAppend( ObjectListSuiteData* data ) {

   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"a"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 0 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 1 &&
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"a"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 0 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 1 &&
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"a"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 0 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 1 &&
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"a"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 0 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 1 );
}

/* Test 3: Can we prepend the second entry? */
void ObjectListSuite_TestPrepend( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_ClassPrepend(  data->ol0, (data->addPtr0 = DummyClass_New()), "b" );
   Stg_ObjectList_ClassPrepend( &data->ol1, (data->addPtr1 = DummyClass_New()), "b" );
   Stg_ObjectList_PointerPrepend(  data->ol2, (data->addPtr2 = DummyPointer_New()), "b", 0, DummyPointer_Print, DummyPointer_Copy );
   Stg_ObjectList_GlobalPointerPrepend(  data->ol3, (data->addPtr3 = (int*)DummyFunc2), "b" );

   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"b"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 0 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 2 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 1 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"b"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 0 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 2 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 1 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"b"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 0 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 2 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 1 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"b"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 0 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 2 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 1 ) ), "a" ) == 0 );
}

/* Test 4: Can we insert before "a" the third entry? */
void ObjectListSuite_TestInsertBefore( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"b", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"b"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_ClassInsertBefore(  data->ol0, "a", (data->addPtr0 = DummyClass_New()), "c" );
   Stg_ObjectList_ClassInsertBefore( &data->ol1, "a", (data->addPtr1 = DummyClass_New()), "c" );
   Stg_ObjectList_PointerInsertBefore(  
      data->ol2, 
      "a", 
      (data->addPtr2 = DummyPointer_New()), 
      "c", 
      0, 
      DummyPointer_Print, 
      DummyPointer_Copy );
   Stg_ObjectList_GlobalPointerInsertBefore(  data->ol3, "a", (data->addPtr3 = (int*)DummyFunc3), "c" );

   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"c"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 1 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"c"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 1 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"c"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 1 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"c"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 1 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 2 ) ), "a" ) == 0 );
}
   

/* Test 5: Can we insert after "c" the fourth entry? */
void ObjectListSuite_TestInsertAfter( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"b", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"b"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"c", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"c"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_ClassInsertAfter(  data->ol0, "c", (data->addPtr0 = DummyClass_New()), "d" );
   Stg_ObjectList_ClassInsertAfter( &data->ol1, "c", (data->addPtr1 = DummyClass_New()), "d" );

   Stg_ObjectList_PointerInsertAfter(  
      data->ol2, 
      "c", 
      (data->addPtr2 = DummyPointer_New()), 
      "d", 
      0, 
      DummyPointer_Print, 
      DummyPointer_Copy );
   Stg_ObjectList_GlobalPointerInsertAfter(  data->ol3, "c", (data->addPtr3 = (int*)DummyFunc4), "d" );

   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"d"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 2 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"d"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 2 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"d"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 2 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"d"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 2 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 3 ) ), "a" ) == 0 );
}

   
/* Test 6: Can we replace "d" with the fifth entry? */
void ObjectListSuite_TestReplace( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"b", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"b"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"c", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"c"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"d"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"d"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"d", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"d"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_ClassReplace(  data->ol0, "d", DELETE, (data->addPtr0 = DummyClass_New()), "e" );
   Stg_ObjectList_ClassReplace( &data->ol1, "d", DELETE, (data->addPtr1 = DummyClass_New()), "e" );
   Stg_ObjectList_PointerReplace( data->ol2, "d", DELETE, (data->addPtr2 = DummyPointer_New()), (Name)"e", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerReplace( data->ol3, (Name)"d", DELETE, (data->addPtr3 = (int*)DummyFunc5), (Name)"e"  );
   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"e"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 2 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"e"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 2 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"e"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 2 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 3 ) ), "a" ) == 0 );
   pcu_check_true(
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"e"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 2 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 4 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 1 ) ), "c" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 3 ) ), "a" ) == 0 );
}
   
/* Test 7: Can we remove the "c" entry? */
void ObjectListSuite_TestRemove( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"b", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"b"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"c"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"c", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"c"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"e"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"e"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"e", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"e"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_Remove( data->ol0, (Name)"c", DELETE  );
   Stg_ObjectList_Remove( &data->ol1, (Name)"c", DELETE  );
   Stg_ObjectList_Remove( data->ol2, (Name)"c", DELETE  );
   Stg_ObjectList_Remove( data->ol3, (Name)"c", DELETE );

   pcu_check_true(
      Stg_ObjectList_Count( data->ol0 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object* )Stg_ObjectList_At( data->ol0, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 1 ) ), "e" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol0, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      Stg_ObjectList_Count( &data->ol1 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 1 ) ), "e" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &data->ol1, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      Stg_ObjectList_Count( data->ol2 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 1 ) ), "e" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol2, 2 ) ), "a" ) == 0 );
   pcu_check_true(
      Stg_ObjectList_Count( data->ol3 ) == 3 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 0 ) ), "b" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 1 ) ), "e" ) == 0 &&
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( data->ol3, 2 ) ), "a" ) == 0 );
}

/* Test 8: Can we replace all with the sixth entry? */
void ObjectListSuite_TestReplaceAll( ObjectListSuiteData* data ) {
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"b"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"b", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"b"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"e"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"e"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"e", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"e"  );
   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"a"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"a", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"a"  );

   Stg_ObjectList_ClassReplaceAll( data->ol0, DELETE, (data->addPtr0 = DummyClass_New()), (Name)"f"  );
   Stg_ObjectList_ClassReplaceAll( &data->ol1, DELETE, (data->addPtr1 = DummyClass_New()), (Name)"f"  );
   Stg_ObjectList_PointerReplaceAll(  
      data->ol2, 
      DELETE, 
      (data->addPtr2 = DummyPointer_New()), 
      "f", 
      0, 
      DummyPointer_Print, 
      DummyPointer_Copy );
   Stg_ObjectList_GlobalPointerReplaceAll(  data->ol3, DELETE, (data->addPtr3 = (int*)DummyFunc6), "f" );
   pcu_check_true(
      data->addPtr0 == Stg_ObjectList_Get( data->ol0, (Name)"f"  ) &&
      data->addPtr0 == Stg_ObjectList_ObjectAt( data->ol0, 0 ) &&
      Stg_ObjectList_Count( data->ol0 ) == 1 );
   pcu_check_true(
      data->addPtr1 == Stg_ObjectList_Get( &data->ol1, (Name)"f"  ) &&
      data->addPtr1 == Stg_ObjectList_ObjectAt( &data->ol1, 0 ) &&
      Stg_ObjectList_Count( &data->ol1 ) == 1 );
   pcu_check_true(
      data->addPtr2 == Stg_ObjectList_Get( data->ol2, (Name)"f"  ) &&
      data->addPtr2 == Stg_ObjectList_ObjectAt( data->ol2, 0 ) &&
      Stg_ObjectList_Count( data->ol2 ) == 1 );
   pcu_check_true(
      data->addPtr3 == Stg_ObjectList_Get( data->ol3, (Name)"f"  ) &&
      data->addPtr3 == Stg_ObjectList_ObjectAt( data->ol3, 0 ) &&
      Stg_ObjectList_Count( data->ol3 ) == 1 );
}
   
/* Test 9: Copying */
/* Shallow copying not yet implemented */
/* Deep copying "OfPointer" (i.e. non-StGermain class) not yet implemented */
void ObjectListSuite_TestCopy( ObjectListSuiteData* data ) {
   Stg_ObjectList* ol0deep;
   Stg_ObjectList* ol1deep;
   Stg_ObjectList* ol2deep;
   Stg_ObjectList* ol3deep;

   Stg_ObjectList_ClassAppend( data->ol0, (data->addPtr0 = DummyClass_New()), (Name)"f"  );
   Stg_ObjectList_ClassAppend( &data->ol1, (data->addPtr1 = DummyClass_New()), (Name)"f"  );
   Stg_ObjectList_PointerAppend( data->ol2, (data->addPtr2 = DummyPointer_New()), (Name)"f", 0, DummyPointer_Print, DummyPointer_Copy  );
   Stg_ObjectList_GlobalPointerAppend( data->ol3, (data->addPtr3 = (int*)DummyFunc1), (Name)"f"  );

   ol0deep = (Stg_ObjectList*)Stg_Class_Copy( data->ol0, 0, True, 0, 0 );
   ol1deep = (Stg_ObjectList*)Stg_Class_Copy( &data->ol1, 0, True, 0, 0 );
   ol2deep = (Stg_ObjectList*)Stg_Class_Copy( data->ol2, 0, True, 0, 0 );
   ol3deep = (Stg_ObjectList*)Stg_Class_Copy( data->ol3, 0, True, 0, 0 );

   pcu_check_true(
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0deep, 0 ) ), "f" ) == 0 &&
      Stg_ObjectList_Count( ol0deep ) == 1 &&
      Stg_ObjectList_ObjectAt( data->ol0, 0 ) != Stg_ObjectList_ObjectAt( ol0deep, 0 ) &&
      Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( data->ol0, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol0deep, 0 ) ) );
   pcu_check_true(
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol1deep, 0 ) ), "f" ) == 0 &&
      Stg_ObjectList_Count( ol1deep ) == 1 &&
      Stg_ObjectList_ObjectAt( &data->ol1, 0 ) != Stg_ObjectList_ObjectAt( ol1deep, 0 ) &&
      Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( &data->ol1, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol1deep, 0 ) ) );
   pcu_check_true(
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2deep, 0 ) ), "f" ) == 0 &&
      Stg_ObjectList_Count( ol2deep ) == 1 &&
      Stg_ObjectList_ObjectAt( data->ol2, 0 ) != Stg_ObjectList_ObjectAt( ol2deep, 0 ) );
   pcu_check_true(
      strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3deep, 0 ) ), "f" ) == 0 &&
      Stg_ObjectList_Count( ol3deep ) == 1 &&
      Stg_ObjectList_ObjectAt( data->ol3, 0 ) == Stg_ObjectList_ObjectAt( ol3deep, 0 ) && /* different to others */
      Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( data->ol3, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol3deep, 0 ) ) );

      Stg_Class_Delete(  ol3deep );
      Stg_Class_Delete(  ol2deep );
      Stg_Class_Delete(  ol1deep );
      Stg_Class_Delete(  ol0deep );
}

void ObjectListSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ObjectListSuiteData );
   pcu_suite_setFixtures( suite, ObjectListSuite_Setup, ObjectListSuite_Teardown );
   pcu_suite_addTest( suite, ObjectListSuite_TestAppend );
   pcu_suite_addTest( suite, ObjectListSuite_TestPrepend );
   pcu_suite_addTest( suite, ObjectListSuite_TestInsertBefore );
   pcu_suite_addTest( suite, ObjectListSuite_TestInsertAfter );
   pcu_suite_addTest( suite, ObjectListSuite_TestReplace );
   pcu_suite_addTest( suite, ObjectListSuite_TestRemove );
   pcu_suite_addTest( suite, ObjectListSuite_TestReplaceAll );
   pcu_suite_addTest( suite, ObjectListSuite_TestCopy );
}


