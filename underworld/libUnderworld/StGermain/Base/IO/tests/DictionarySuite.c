/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "DictionarySuite.h"

typedef struct {
   Dictionary*                   dict;
   DictionarySuite_TestDictData* testDD;
} DictionarySuiteData;


void DictionarySuite_SetupTestDictData( DictionarySuite_TestDictData* testDD ) {
   Index                   ii=0;
   Index                   iter=0;
   Dictionary_Entry_Value* testStruct; 
   Dictionary_Entry_Value* testStruct2; 
   Dictionary_Entry_Value* testList; 

   testDD->testEntriesCount = 9;
   testDD->testKeys = Memory_Alloc_Array_Unnamed( char*, testDD->testEntriesCount );
   testDD->testValues = Memory_Alloc_Array_Unnamed( Dictionary_Entry_Value*,
      testDD->testEntriesCount );

   for ( ii=0; ii< testDD->testEntriesCount; ii++ ) {
      testDD->testKeys[ii] = NULL;
      testDD->testValues[ii] = NULL;
   }

   Stg_asprintf( &testDD->testString, "hello" );
   Stg_asprintf( &testDD->testPlaceHolder, "test_double" );
   testDD->testDouble=45.567;
   testDD->testUint = 5;
   testDD->testInt = -5;
   testDD->testUnsignedlong = 52342423;
   testDD->testBool = True;
   iter = 0;
   Stg_asprintf( &testDD->testKeys[iter], "test_cstring" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromString( testDD->testString );
   Stg_asprintf( &testDD->testKeys[++iter], "test_double" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromDouble( testDD->testDouble );
   Stg_asprintf( &testDD->testKeys[++iter], "test_uint" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromUnsignedInt( testDD->testUint );
   Stg_asprintf( &testDD->testKeys[++iter], "test_int" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromInt( testDD->testInt );
   Stg_asprintf( &testDD->testKeys[++iter], "test_unsignedlong" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromUnsignedLong( testDD->testUnsignedlong );
   Stg_asprintf( &testDD->testKeys[++iter], "test_bool" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromUnsignedInt( testDD->testBool );
   Stg_asprintf( &testDD->testKeys[++iter], "test_placeholder" );
   testDD->testValues[iter] = Dictionary_Entry_Value_FromString( testDD->testPlaceHolder );

   /* adding a list */
   testDD->testListCount = 5;
   testDD->testList = Memory_Alloc_Array_Unnamed( double, testDD->testListCount );
   for (ii=0; ii<testDD->testListCount; ii++ ) {
      testDD->testList[ii] = 10.0 * ii;
   }

   testList = Dictionary_Entry_Value_NewList();
   Stg_asprintf( &testDD->testKeys[++iter], "test_list" );
   testDD->testValues[iter] = testList;
   for (ii=0; ii < testDD->testListCount; ii++ ) {
      Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(testDD->testList[ii]) );
   }

   /* Adding members to a struct */
   testDD->testStruct = Memory_Alloc_Unnamed( TestStruct );
   testDD->testStruct->height = 37;
   testDD->testStruct->anisotropic = True;
   Stg_asprintf( &testDD->testStruct->person, "Patrick" );
   testDD->testStruct->geom.startx = 45;
   testDD->testStruct->geom.starty = 60;
   testDD->testStruct->geom.startz = 70;

   testStruct = Dictionary_Entry_Value_NewStruct();
   Stg_asprintf( &testDD->testKeys[++iter], "test_struct" );
   testDD->testValues[iter] = testStruct;
   Dictionary_Entry_Value_AddMember( testStruct, (Dictionary_Entry_Key)"height", Dictionary_Entry_Value_FromDouble( testDD->testStruct->height )  );
   Dictionary_Entry_Value_AddMember( testStruct, (Dictionary_Entry_Key)"anisotropic", Dictionary_Entry_Value_FromBool( testDD->testStruct->anisotropic )  );
   Dictionary_Entry_Value_AddMember( testStruct, (Dictionary_Entry_Key)"person", Dictionary_Entry_Value_FromString( testDD->testStruct->person ) );

   /* Adding a 2nd struct within the first struct */
   testStruct2 = Dictionary_Entry_Value_NewStruct( );
   Dictionary_Entry_Value_AddMember( testStruct, (Dictionary_Entry_Key)"geom", testStruct2  );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"startx", Dictionary_Entry_Value_FromUnsignedInt( testDD->testStruct->geom.startx )  );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"starty", Dictionary_Entry_Value_FromUnsignedInt( testDD->testStruct->geom.starty )  );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"startz", Dictionary_Entry_Value_FromUnsignedInt( testDD->testStruct->geom.startz ) );
}

void DictionarySuite_Setup( DictionarySuiteData* data ) {
   data->dict     = Dictionary_New();
   data->testDD   = Memory_Alloc_Unnamed( DictionarySuite_TestDictData );
   DictionarySuite_SetupTestDictData( data->testDD );
}

void DictionarySuite_Teardown( DictionarySuiteData* data ) {
   Dictionary_Empty( data->dict );
   Stg_Class_Delete( data->dict );
   DictionarySuite_DictionaryData_Free( data->testDD );
   Memory_Free( data->testDD );
}

void DictionarySuite_DictionaryData_Free( DictionarySuite_TestDictData* testDD ) {
   Index ii;
   for ( ii=0; ii< testDD->testEntriesCount; ii++ ) {
      Memory_Free( testDD->testKeys[ii]  );
      /* Note: we don't free the testValues here, as expect that deleting the
       * dictionary has already done this */
   }
   Memory_Free( testDD->testKeys );
   Memory_Free( testDD->testValues );
   Memory_Free( testDD->testStruct->person );
   Memory_Free( testDD->testStruct );
   Memory_Free( testDD->testList );
}


void DictionarySuite_TestCreateValues( DictionarySuiteData* data ) {
   Dictionary_Entry_Value*   dev;

   /* Don't use the pre-created test values. Want to do a very fundamental test here */
   dev = Dictionary_Entry_Value_FromString( "hello" );
   pcu_check_true( Dictionary_Entry_Value_Type_String == dev->type );
   pcu_check_streq( "hello", dev->as.typeString );
   Dictionary_Entry_Value_Delete( dev );
   dev = Dictionary_Entry_Value_FromDouble( 45.567 );
   pcu_check_true( Dictionary_Entry_Value_Type_Double == dev->type );
   pcu_check_true( 45.567 == dev->as.typeDouble );
   Dictionary_Entry_Value_Delete( dev );
   dev = Dictionary_Entry_Value_FromUnsignedInt( 5 );
   pcu_check_true( Dictionary_Entry_Value_Type_UnsignedInt == dev->type );
   pcu_check_true( 5 == dev->as.typeUnsignedInt );
   Dictionary_Entry_Value_Delete( dev );
   dev = Dictionary_Entry_Value_FromInt( -5 );
   pcu_check_true( Dictionary_Entry_Value_Type_Int == dev->type );
   pcu_check_true( -5 == dev->as.typeInt );
   Dictionary_Entry_Value_Delete( dev );
   dev = Dictionary_Entry_Value_FromUnsignedLong( 52342423 );
   pcu_check_true( Dictionary_Entry_Value_Type_UnsignedLong == dev->type );
   pcu_check_true( 52342423 == dev->as.typeUnsignedLong );
   Dictionary_Entry_Value_Delete( dev );
   dev = Dictionary_Entry_Value_FromBool( True );
   pcu_check_true( Dictionary_Entry_Value_Type_Bool == dev->type );
   pcu_check_true( True == dev->as.typeBool );
   Dictionary_Entry_Value_Delete( dev );

   /* Since we know the DEV Struct is basically a Dictionary, won't test that one
    *  until after we've tested Dictionary_Add works */   
}


void DictionarySuite_PopulateDictWithTestValues( Dictionary* dict, DictionarySuite_TestDictData* testDD ) {
   Index ii;

   for ( ii=0; ii< testDD->testEntriesCount; ii++ ) {
      Dictionary_Add( dict, testDD->testKeys[ii],\
         Dictionary_Entry_Value_Copy( testDD->testValues[ii], True ) );
   }
}


void DictionarySuite_TestCopyCompare( DictionarySuiteData* data ) {
   Index                    ii=0, jj=0;
   Dictionary_Entry_Value*  copiedDev;
   
   for( ii = 0; ii < data->dict->count; ii++ ) {
      copiedDev = Dictionary_Entry_Value_Copy( data->testDD->testValues[ii], True );
      pcu_check_true( Dictionary_Entry_Value_Compare( data->testDD->testValues[ii], copiedDev ) ); 
      Dictionary_Entry_Value_Delete( copiedDev );

      for( jj = 0; jj < data->dict->count; jj++ ) {
         if ( ii == jj ) continue;
         pcu_check_true( False == Dictionary_Entry_Value_Compare( data->testDD->testValues[ii], data->testDD->testValues[jj] ) ); 
      }
   }
}

/* Add a set of values to a dictionary, and check they were added as desired
 * using the Compare functions.
 */
void DictionarySuite_TestAddEmpty( DictionarySuiteData* data ) {
   Dictionary_Index        ii;
   Dictionary_Entry*       entryPtr;

   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   pcu_check_true( data->testDD->testEntriesCount == data->dict->count );

   for( ii = 0; ii < data->dict->count; ii++ ) {
      entryPtr = data->dict->entryPtr[ii];
      pcu_check_true( Dictionary_Entry_Compare( entryPtr, data->testDD->testKeys[ii] ) ); 
      pcu_check_true( Dictionary_Entry_Value_Compare( entryPtr->value, data->testDD->testValues[ii] ) ); 
   }

   Dictionary_Empty( data->dict );
   pcu_check_true( 0 == data->dict->count );
}


void DictionarySuite_TestGet( DictionarySuiteData* data ) {
   Dictionary_Entry_Value* yValue;
   Dictionary_Entry_Value* testStruct;

   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   testStruct = Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_struct"  );
   yValue = Dictionary_Entry_Value_GetMember( Dictionary_Entry_Value_GetMember(testStruct, (Dictionary_Entry_Key)"geom" ), "starty");
   pcu_check_true( data->testDD->testStruct->geom.starty == Dictionary_Entry_Value_AsDouble( yValue ) );
}


void DictionarySuite_TestSet( DictionarySuiteData* data ) {
   Dictionary_Entry_Value* currValue;
   Dictionary_Entry_Value* listValue;
   double                  newVal1 = 34.3, newVal2 = 38.9;
   Index                   ii=0;

   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   listValue = Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_list" );
   /* getting dictionary out of a list */
   currValue = Dictionary_Entry_Value_GetFirstElement( listValue  );
   /* do something to this value */
   Dictionary_Entry_Value_SetFromDouble( currValue, newVal1 );
   currValue = currValue->next;
   /* do something to this value */
   Dictionary_Entry_Value_SetFromDouble( currValue, newVal2 );
   
   pcu_check_true( 5 == Dictionary_Entry_Value_GetCount( listValue ) );
   currValue = Dictionary_Entry_Value_GetFirstElement( listValue );
   pcu_check_le( fabs(newVal1 - Dictionary_Entry_Value_AsDouble( currValue )), 0.01 );
   currValue = currValue->next;
   pcu_check_le( fabs(newVal2 - Dictionary_Entry_Value_AsDouble( currValue )), 0.01 );
   currValue = currValue->next;
   for ( ii=2; ii<data->testDD->testListCount; ii++ ) {
      pcu_check_le( fabs(data->testDD->testList[ii]
         - Dictionary_Entry_Value_AsDouble( currValue )), 0.01 );
      currValue = currValue->next;
   }
}


void DictionarySuite_TestAddElement( DictionarySuiteData* data ) {
   Dictionary_Entry_Value* yValue;
   Dictionary_Entry_Value* testStruct;
   Dictionary_Entry_Value* currValue;
   double                  newVal = -45.0;

   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   /* turning the starty value into a list using add element */
   testStruct = Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_struct"  );
   yValue = Dictionary_Entry_Value_GetMember( Dictionary_Entry_Value_GetMember(testStruct, (Dictionary_Entry_Key)"geom" ), "starty");
   Dictionary_Entry_Value_AddElement( yValue, Dictionary_Entry_Value_FromDouble(newVal) );

   pcu_check_true( Dictionary_Entry_Value_Type_List == yValue->type );
   pcu_check_true( 2 == Dictionary_Entry_Value_GetCount( yValue ) );

   currValue = Dictionary_Entry_Value_GetFirstElement( yValue );
   pcu_check_le( fabs( data->testDD->testStruct->geom.starty - Dictionary_Entry_Value_AsDouble( currValue )), 0.01 );
   currValue = currValue->next;
   pcu_check_le( fabs( newVal - Dictionary_Entry_Value_AsDouble( currValue )), 0.01 );
}


void DictionarySuite_TestShortcuts( DictionarySuiteData* data ) {

   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   /* Testing GetString_WithDefault. If an entry with that key already exists, then
    *  the value of the existing key should be returned, and the default passed in
    *  ignored. However if the given key _doesn't_ exist, the default should be 
    *  returned, and a new entry with the given key added to the dict. */
   pcu_check_streq( data->testDD->testString, Dictionary_GetString_WithDefault( data->dict, "test_cstring", "heya" ) );
   pcu_check_streq( "heya", Dictionary_GetString_WithDefault( data->dict, "test_cstring2", "heya" ) );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_cstring2" )  );
	data->testDD->testDouble = Dictionary_GetDouble_WithDefault( data->dict, (Dictionary_Entry_Key)"test_double", 2.8 );
   pcu_check_true( data->testDD->testDouble );
   pcu_check_true( 2.8 == Dictionary_GetDouble_WithDefault( data->dict, (Dictionary_Entry_Key)"test_double2", 2.8 )  );

   /* test_placeholder refers to test_double which equals 45.567 */
   pcu_check_true( 45.567 == Dictionary_GetDouble_WithScopeDefault( data->dict, (Dictionary_Entry_Key)"test_placeholder", -1 )  );
   
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_double2" )  );
   data->testDD->testUint = Dictionary_GetUnsignedInt_WithDefault( data->dict, "test_uint", 33 );
	pcu_check_true( data->testDD->testUint );
   pcu_check_true( 33 == Dictionary_GetUnsignedInt_WithDefault( data->dict, "test_uint2", 33 ) );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_uint2" )  );
   data->testDD->testInt = Dictionary_GetInt_WithDefault( data->dict, (Dictionary_Entry_Key)"test_int", -24 );
	pcu_check_true( data->testDD->testInt );
   pcu_check_true( -24 == Dictionary_GetInt_WithDefault( data->dict, (Dictionary_Entry_Key)"test_int2", -24 )  );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_int2" )  );
   data->testDD->testUnsignedlong = Dictionary_GetUnsignedLong_WithDefault( data->dict, "test_unsignedlong", 32433 );
	pcu_check_true( data->testDD->testUnsignedlong );
   pcu_check_true( 32433 == Dictionary_GetUnsignedLong_WithDefault( data->dict, "test_unsignedlong2", 32433 ) );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_unsignedlong2" )  );
   data->testDD->testBool = Dictionary_GetBool_WithDefault( data->dict, (Dictionary_Entry_Key)"test_bool", False );
	pcu_check_true( data->testDD->testBool );
   pcu_check_true( False == Dictionary_GetBool_WithDefault( data->dict, (Dictionary_Entry_Key)"test_bool2", False )  );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_bool2" )  );

   pcu_check_streq( data->testDD->testString, Dictionary_GetString_WithPrintfDefault( data->dict, "test_cstring", "heya%s%u", "hey", 3 ) );
   pcu_check_streq( "heyahey3", Dictionary_GetString_WithPrintfDefault( data->dict, "test_cstring3", "heya%s%u", "hey", 3 ) );
   pcu_check_true( NULL != Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_cstring3" ) );
}


void DictionarySuite_TestMerge( DictionarySuiteData* data ) {
   Dictionary_Entry_Value*    testStruct2=NULL;
   Dictionary_Entry_Value*    testGeomStruct2=NULL;
   Dictionary_Entry_Value*    mergedStruct=NULL;
   Dictionary_Entry_Value*    expectedMergedStruct=NULL;

   testStruct2 = Dictionary_Entry_Value_NewStruct( );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"height", Dictionary_Entry_Value_FromDouble( data->testDD->testStruct->height )  );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"anisotropic", Dictionary_Entry_Value_FromBool( False )  );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"new_person", Dictionary_Entry_Value_FromString( "Luke" ) );
   testGeomStruct2 = Dictionary_Entry_Value_NewStruct( );
   Dictionary_Entry_Value_AddMember( testStruct2, (Dictionary_Entry_Key)"geom", testGeomStruct2  );
   Dictionary_Entry_Value_AddMember( testGeomStruct2, (Dictionary_Entry_Key)"startx", Dictionary_Entry_Value_FromUnsignedInt( data->testDD->testStruct->geom.startx )  );
   Dictionary_Entry_Value_AddMember( testGeomStruct2, (Dictionary_Entry_Key)"startz", Dictionary_Entry_Value_FromUnsignedInt( 222 )  );

   /* Testing Merge_Append */
   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );
   /* Do a copy of the DEV during merge, since we don't want it being deleted */
   Dictionary_AddMerge( data->dict, "test_struct", Dictionary_Entry_Value_Copy( testStruct2, True ), Dictionary_MergeType_Append );
   /* OK: since this was an append, we expect _two_ entries called "test_struct",
    * one preceding the other, one with the orig data, one with new data */    
   pcu_check_true( (data->testDD->testEntriesCount+1) == data->dict->count );
   pcu_check_true( Dictionary_Entry_Value_Compare( data->testDD->testValues[8], Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_struct" ) )  );
   pcu_check_true( Dictionary_Entry_Value_Compare( testStruct2, data->dict->entryPtr[9]->value ) );
   Dictionary_Empty( data->dict );

   /* Testing Merge_Merge */
   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );

   /* The nicest way for this test I think is to manually build a merged struct
    *  to compare against */
   expectedMergedStruct = Dictionary_Entry_Value_Copy( data->testDD->testValues[8], True );
   Dictionary_Set( expectedMergedStruct->as.typeStruct, (Dictionary_Entry_Key)"anisotropic", Dictionary_Entry_Value_FromBool( False )  );
   Dictionary_Add( expectedMergedStruct->as.typeStruct, (Dictionary_Entry_Key)"new_person", Dictionary_Entry_Value_FromString( "Luke" )  );
   Dictionary_Set( (Dictionary_Get( expectedMergedStruct->as.typeStruct, (Dictionary_Entry_Key)"geom" ) )->as.typeStruct, "startz", Dictionary_Entry_Value_FromUnsignedInt( 222 ) );

   Dictionary_AddMerge( data->dict, "test_struct", Dictionary_Entry_Value_Copy( testStruct2, True ), Dictionary_MergeType_Merge );
   /* This time, the new struct should be merged into the existing one */
   pcu_check_true( data->testDD->testEntriesCount == data->dict->count );
   mergedStruct = Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_struct"  );
   pcu_check_true( Dictionary_Entry_Value_Compare( mergedStruct, expectedMergedStruct ) );
   Dictionary_Empty( data->dict );
   Dictionary_Entry_Value_Delete( expectedMergedStruct );

   /* Testing Merge_Replace */
   DictionarySuite_PopulateDictWithTestValues( data->dict, data->testDD );
   Dictionary_AddMerge( data->dict, "test_struct", Dictionary_Entry_Value_Copy( testStruct2, True ), Dictionary_MergeType_Replace );
   pcu_check_true( data->testDD->testEntriesCount == data->dict->count );
   pcu_check_true( Dictionary_Entry_Value_Compare( testStruct2, Dictionary_Get( data->dict, (Dictionary_Entry_Key)"test_struct" ) ) );
   Dictionary_Empty( data->dict );

   Dictionary_Entry_Value_Delete( testStruct2 );
}


void DictionarySuite_TestReadAllParamFromCommandLine( DictionarySuiteData* data  ) {
   int                        argc;
   char**                     argv;
   char**                     expectedKeys;
   Dictionary_Entry_Value**   expectedVals;
   Index                      ii;
   Dictionary_Entry_Value*    tmpStruct=NULL;
   Dictionary_Entry_Value*    tmpStruct2=NULL;
   Dictionary_Entry_Value*    tmpList=NULL;
   Index                      numExp = 7;

   argc = 16;
   argv = Memory_Alloc_Array_Unnamed( char*, argc );
   expectedVals = Memory_Alloc_Array_Unnamed( Dictionary_Entry_Value*, numExp );
   expectedKeys = Memory_Alloc_Array_Unnamed( char*, numExp );

   /* Now fill in our mock command line, and create the expected values to test
    *  against along the way */   
   Stg_asprintf( &argv[0], "./testStGermain");
   Stg_asprintf( &argv[1], "-param=hey");
   Stg_asprintf( &argv[2], "--option");
   Stg_asprintf( &argv[3], "--output-dir=");
   Stg_asprintf( &expectedKeys[0], "output-dir" );

   expectedVals[0] = Dictionary_Entry_Value_FromString( "" );
   Stg_asprintf( &argv[4], "--Ra=1.0e4");
   Stg_asprintf( &expectedKeys[1], "Ra" );

   expectedVals[1] = Dictionary_Entry_Value_FromDouble( 1.0e4 );
   Stg_asprintf( &argv[5], "--foo.bar=5");
   Stg_asprintf( &expectedKeys[2], "foo" );

   expectedVals[2] = Dictionary_Entry_Value_NewStruct();
   Dictionary_Entry_Value_AddMember( expectedVals[2], (Dictionary_Entry_Key)"bar", Dictionary_Entry_Value_FromDouble( 5 )  );
   Stg_asprintf( &argv[6], "--hot.tub.times=fun");
   Stg_asprintf( &expectedKeys[3], "hot" );

   expectedVals[3] = Dictionary_Entry_Value_NewStruct();
   Dictionary_Entry_Value_AddMember( expectedVals[3], (Dictionary_Entry_Key)"tub", tmpStruct = Dictionary_Entry_Value_NewStruct()  );
   Dictionary_Entry_Value_AddMember( tmpStruct, (Dictionary_Entry_Key)"times", Dictionary_Entry_Value_FromString( "fun" )  );
   Stg_asprintf( &argv[7], "--foo.bot=7");
   Dictionary_Entry_Value_AddMember( expectedVals[2], (Dictionary_Entry_Key)"bot", Dictionary_Entry_Value_FromDouble( 7 )  );
   Stg_asprintf( &argv[8], "--sports[]=hockey");
   Stg_asprintf( &expectedKeys[4], "sports" );
   
   expectedVals[4] = Dictionary_Entry_Value_NewList();
   Dictionary_Entry_Value_AddElement( expectedVals[4], Dictionary_Entry_Value_FromString( "hockey" ) );
   Stg_asprintf( &argv[9], "--sports[]=chess");
   /* This should be overwritten by next entry, so ignore */
   Stg_asprintf( &argv[10], "--sports[1]=tennis");
   Dictionary_Entry_Value_AddElement( expectedVals[4], Dictionary_Entry_Value_FromString( "tennis" ) );
   Stg_asprintf( &argv[11], "--sles[].name=pressure");
   Stg_asprintf( &expectedKeys[5], "sles" );

   expectedVals[5] = Dictionary_Entry_Value_NewList();
   Dictionary_Entry_Value_AddElement( expectedVals[5], tmpStruct = Dictionary_Entry_Value_NewStruct() );
   Dictionary_Entry_Value_AddMember( tmpStruct, (Dictionary_Entry_Key)"name", Dictionary_Entry_Value_FromString( "pressure" )  );
   Stg_asprintf( &argv[12], "--sles[].name=temperature");
   Dictionary_Entry_Value_AddElement( expectedVals[5], tmpStruct2 = Dictionary_Entry_Value_NewStruct() );
   Dictionary_Entry_Value_AddMember( tmpStruct2, (Dictionary_Entry_Key)"name", Dictionary_Entry_Value_FromString( "temperature" )  );
   Stg_asprintf( &argv[13], "--sles[0].solver=mg");
   Dictionary_Entry_Value_AddMember( tmpStruct, (Dictionary_Entry_Key)"solver", Dictionary_Entry_Value_FromString( "mg" )  );
   Stg_asprintf( &argv[14], "--sles[1].solver=direct");
   Dictionary_Entry_Value_AddMember( tmpStruct2, (Dictionary_Entry_Key)"solver", Dictionary_Entry_Value_FromString( "direct" )  );
   Stg_asprintf( &argv[15], "--some.crazy[].shit=here");
   Stg_asprintf( &expectedKeys[6], "some" );

   expectedVals[6] = Dictionary_Entry_Value_NewStruct();
   Dictionary_Entry_Value_AddMember( expectedVals[6], (Dictionary_Entry_Key)"crazy", tmpList = Dictionary_Entry_Value_NewList()  );
   Dictionary_Entry_Value_AddElement( tmpList, tmpStruct = Dictionary_Entry_Value_NewStruct() );
   Dictionary_Entry_Value_AddMember( tmpStruct, (Dictionary_Entry_Key)"shit", Dictionary_Entry_Value_FromString( "here" )  );

   Dictionary_ReadAllParamFromCommandLine( data->dict, argc, argv );

   for (ii=0; ii < numExp; ii++) {
      pcu_check_true( Dictionary_Entry_Compare( data->dict->entryPtr[ii], expectedKeys[ii]));
      pcu_check_true( Dictionary_Entry_Value_Compare( expectedVals[ii], data->dict->entryPtr[ii]->value ));
   }

   for (ii=0; ii < argc; ii++) {
      Memory_Free( argv[ii] );
   }
   Memory_Free( argv );

   for (ii=0; ii < numExp; ii++) {
      Memory_Free( expectedKeys[ii] );
      Memory_Free( expectedVals[ii] );
   }
   Memory_Free( expectedKeys );
   Memory_Free( expectedVals );
}


void DictionarySuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, DictionarySuiteData );
   pcu_suite_setFixtures( suite, DictionarySuite_Setup, DictionarySuite_Teardown );
   pcu_suite_addTest( suite, DictionarySuite_TestCreateValues );
   pcu_suite_addTest( suite, DictionarySuite_TestCopyCompare );
   pcu_suite_addTest( suite, DictionarySuite_TestAddEmpty );
   pcu_suite_addTest( suite, DictionarySuite_TestGet );
   pcu_suite_addTest( suite, DictionarySuite_TestSet );
   pcu_suite_addTest( suite, DictionarySuite_TestAddElement );
   pcu_suite_addTest( suite, DictionarySuite_TestShortcuts );
   pcu_suite_addTest( suite, DictionarySuite_TestMerge );
   pcu_suite_addTest( suite, DictionarySuite_TestReadAllParamFromCommandLine );
}


