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
#include <math.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"
#include "StGermain/Base/Context/src/Context.h"
#include "VariableSuite.h"

typedef double VectorD[3];
typedef double VectorF[3];

typedef struct {
   int      mass;
   VectorF  force;
   short    num;
   char*    info;
} Particle;

typedef struct {
   Variable_Register*      vr;
   Index                   aSize[3];
   double*                 temperature;
   VectorD*                velocity;
   Particle*               particle;
} VariableSuiteData;



void VariableSuite_Setup( VariableSuiteData* data ) {
   Particle                tmpParticle;
   Name                    pNames[] = { "mass", "force", "info" };
   SizeT                   pOffsets[] = { 0, 0, 0 };   /* Init later... */
   StgVariable_DataType       pDataTypes[] = {
                              StgVariable_DataType_Int,
                              StgVariable_DataType_Float,
                              StgVariable_DataType_Pointer, };
   Index                   pDtCounts[] = { 1, 3, 1 };
   static SizeT            pSize = sizeof(Particle);
   
   data->aSize[0] = 16;
   data->aSize[1] = 16;
   data->aSize[2] = 16;

   pOffsets[0] = (ArithPointer)&tmpParticle.mass - (ArithPointer)&tmpParticle;
   pOffsets[1] = (ArithPointer)&tmpParticle.force - (ArithPointer)&tmpParticle;
   pOffsets[2] = (ArithPointer)&tmpParticle.info - (ArithPointer)&tmpParticle;

   /* Construction phase --------------------------------------------------------------------------------------------*/
   data->vr = Variable_Register_New();
   StgVariable_NewScalar( "temperature", NULL, StgVariable_DataType_Double, &data->aSize[0], NULL, (void**)&data->temperature, data->vr );
   StgVariable_NewVector( "velocity", NULL, StgVariable_DataType_Double, 3, &data->aSize[1], NULL, (void**)&data->velocity, data->vr, "vx", "vy", "vz" );
   StgVariable_New( "particle", NULL, 3, pOffsets, pDataTypes, pDtCounts, pNames, &pSize, &data->aSize[2], NULL, (void**)&data->particle, data->vr );
   
   /* Build phase ---------------------------------------------------------------------------------------------------*/
   data->temperature = Memory_Alloc_Array( double, data->aSize[0], "temperature" );
   data->velocity = Memory_Alloc_Array( VectorD, data->aSize[1], "velocity" );
   data->particle = Memory_Alloc_Array( Particle, data->aSize[2], "array" );
   
   Variable_Register_BuildAll( data->vr );
}


void VariableSuite_Teardown( VariableSuiteData* data ) {
   StgVariable_Index          var_I;

   /* manually delete all the created Variables */
   for( var_I = 0; var_I < data->vr->count; var_I++ ) {
      Stg_Class_Delete( data->vr->_variable[var_I] );
   }
   
   Memory_Free( data->particle );
   Memory_Free( data->velocity );
   Memory_Free( data->temperature );
}


void VariableSuite_TestGetValueDouble( VariableSuiteData* data ) {
   Index                   ii;
   double tmp;
   StgVariable*      var = Variable_Register_GetByName( data->vr, "temperature" );

   /* Test the Get and Set of a scalar double....................................................................... */
   /* Fill the temperature array with a known pattern of kinda random (bit filling) numbers. */
   for( ii = 0; ii < data->aSize[0]; ii++ ) {
      data->temperature[ii] = 1.0f / (data->aSize[0]+2) * (ii+1); 
   }
   
   /* Check that StgVariable_GetValueDouble on the temperature Variable returns the right numbers */
   for( ii = 0; ii < data->aSize[0]; ii++ ) {
      tmp = 1.0f / (data->aSize[0]+2) * (ii+1);

      pcu_check_true( fabs(StgVariable_GetValueDouble( var, ii ) - tmp ) < 1e-12);
   }
}
   

void VariableSuite_TestSetValueDouble( VariableSuiteData* data ) {
   Index                   ii;
   double tmp;
   StgVariable*      var = Variable_Register_GetByName( data->vr, "temperature" );

   /* Fill the temperature Variable with another known pattern of kinda random (bit filling) numbers */
   for( ii = 0; ii < data->aSize[0]; ii++ ) {
      
      StgVariable_SetValueDouble( var, ii, 1.0f - ( 1.0f / (data->aSize[0]+2) * (ii+1) ) );
   }
   
   /* Check that StgVariable_SetValueDouble on the temperature Variable set the right numbers */
   for( ii = 0; ii < data->aSize[0]; ii++ ) {
      tmp = 1.0f - 1.0f / (data->aSize[0]+2) * (ii+1);
      
      pcu_check_true( fabs(data->temperature[ii] - tmp ) < 1e-12);
   }
}
   

/* Test the Get and Set of a vector double....................................................................... */
void VariableSuite_TestGetValueAtDouble( VariableSuiteData* data ) {
   Index                   ii;
   double tmp;
   StgVariable*      var = Variable_Register_GetByName( data->vr, "velocity" );

/* Fill the velocity array with a known pattern of kinda random (bit filling) numbers. */
   for( ii = 0; ii < data->aSize[1]; ii++ ) {
      int         d;
      
      for( d = 0; d < 3; d++ ) {
         data->velocity[ii][d] = 1.0f / ((data->aSize[1]*3)+2) * (ii*3+d+1); 
      }
   }
   
   /* Check that StgVariable_GetPtrDouble on the velocity Variable returns the right numbers */
   for( ii = 0; ii < data->aSize[1]; ii++ ) {
      int         d;
      
      for( d = 0; d < 3; d++ ) {
         tmp = 1.0f / ((data->aSize[1]*3)+2) * (ii*3+d+1);
         
         pcu_check_true( fabs(StgVariable_GetValueAtDouble(var, ii, d ) - tmp) < 1e-12);
      }
   }
}


void VariableSuite_TestSetValueAtDouble( VariableSuiteData* data ) {
   Index                   ii;
   double tmp;
   int d;
   StgVariable*      var = Variable_Register_GetByName( data->vr, "velocity" );

   /* Fill the variable Variable with another known pattern of kinda random (bit filling) numbers */
   for( ii = 0; ii < data->aSize[1]; ii++ ) {
      
      for( d = 0; d < 3; d++ ) {
         StgVariable_SetValueAtDouble( var, ii, d, 1.0 - ( 1.0 / ((data->aSize[1]*3)+2) * (ii*3+d+1) ) );
      }
   }
   
   /* Check that StgVariable_SetValueDouble on the velocity Variable set the right numbers */
   for( ii = 0; ii < data->aSize[1]; ii++ ) {
      
      for( d = 0; d < 3; d++ ) {
         tmp = 1.0 - ( 1.0 / ((data->aSize[1]*3)+2) * (ii*3+d+1) );
         
         pcu_check_true( data->velocity[ii][d]-tmp < 1e-12 );
      }
   }
}

   
/* TODO: try out vx, vy, vz, complex tests */

void VariableSuite_TestVariable_Char( VariableSuiteData* data ) {
   typedef char Triple[3];
   char* array;
   Triple* structArray;
   Index length = 10;
   /* List of values to test the variable with.
    * Values to test are hex 5's and a's because they are a series of 0101 and 1010 respectively so they test
    * each bit in memory to read/set.
    */
   long int testValues[] = { 0x55, 0xaa };
   Index testValueCount = 2;
   Index test_I;
   long int testValue;
   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];
   int i, j;

   array = Memory_Alloc_Array( char, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   var = StgVariable_NewScalar( "Char-Scalar", NULL, StgVariable_DataType_Char, &length, NULL, (void**)&array, data->vr );
   vec = StgVariable_NewVector( "Char-Three", NULL, StgVariable_DataType_Char, 3, &length, NULL, (void**)&structArray, data->vr, "a", "b", "c" );

   vecVar[0] = Variable_Register_GetByName( data->vr, "a" );
   vecVar[1] = Variable_Register_GetByName( data->vr, "b" );
   vecVar[2] = Variable_Register_GetByName( data->vr, "c" );

   Variable_Register_BuildAll( data->vr );

   for ( test_I = 0; test_I < testValueCount; ++test_I ) {

      testValue = testValues[test_I];

      for ( i = 0; i < length; ++i ) {
         StgVariable_SetValueChar( var, i, testValue );
         StgVariable_SetValueAtChar( vec, i, 0, testValue );
         StgVariable_SetValueAtChar( vec, i, 1, testValue );
         StgVariable_SetValueAtChar( vec, i, 2, testValue );
      }

      /* ~~~Scalar~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueChar( var, i ) == (char)(char)testValue );
         pcu_check_true( StgVariable_GetValueCharAsShort( var, i ) == (short)(char)testValue );
         pcu_check_true( StgVariable_GetValueCharAsInt( var, i ) == (int)(char)testValue );
         pcu_check_true( StgVariable_GetValueCharAsFloat( var, i ) == (float)(char)testValue );
         pcu_check_true( StgVariable_GetValueCharAsDouble( var, i ) == (double)(char)testValue );
      }

      /*~~~Vector~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueAtChar( vec, i, 0 ) == (char)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsShort( vec, i, 0 ) == (short)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsInt( vec, i, 0 ) == (int)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsFloat( vec, i, 0 ) == (float)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsDouble( vec, i, 0 ) == (double)(char)testValue );

         pcu_check_true( StgVariable_GetPtrAtChar( vec, i, 0 ) == &structArray[i][0] );

         pcu_check_true( StgVariable_GetValueAtChar( vec, i, 1 ) == (char)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsShort( vec, i, 1 ) == (short)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsInt( vec, i, 1 ) == (int)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsFloat( vec, i, 1 ) == (float)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsDouble( vec, i, 1 ) == (double)(char)testValue );
         pcu_check_true( StgVariable_GetPtrAtChar( vec, i, 1 ) == &structArray[i][1] );

         pcu_check_true( StgVariable_GetValueAtChar( vec, i, 2 ) == (char)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsShort( vec, i, 2 ) == (short)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsInt( vec, i, 2 ) == (int)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsFloat( vec, i, 2 ) == (float)(char)testValue );
         pcu_check_true( StgVariable_GetValueAtCharAsDouble( vec, i, 2 ) == (double)(char)testValue );
         pcu_check_true( StgVariable_GetPtrAtChar( vec, i, 2 ) == &structArray[i][2] );
      }

      /*~~~Vector: Sub-Variable~~~*/
      for ( i = 0; i < length; ++i ) {
         for ( j = 0; j < 3; ++j ) {
            pcu_check_true( _StgVariable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] );
         }
      }
   }
}


void VariableSuite_TestVariable_Double( VariableSuiteData* data ) {
   typedef double Triple[3];

   double* array;
   Triple* structArray;
   Index length = 10;

   double testValues[] = { 123456789.0, 0.987654321 };
   Index testValueCount = 2;
   Index test_I;
   double testValue;

   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];

   int i, j;

   array = Memory_Alloc_Array( double, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   var = StgVariable_NewScalar( "Double-Scalar", NULL, StgVariable_DataType_Double, &length, NULL, (void**)&array, data->vr ); 
   vec = StgVariable_NewVector( "Double-Three", NULL, StgVariable_DataType_Double, 3, &length, NULL, (void**)&structArray, data->vr, "a", "b", "c" );

   vecVar[0] = Variable_Register_GetByName( data->vr, "a" );
   vecVar[1] = Variable_Register_GetByName( data->vr, "b" );
   vecVar[2] = Variable_Register_GetByName( data->vr, "c" );

   Variable_Register_BuildAll( data->vr );


   for ( test_I = 0; test_I < testValueCount; ++test_I ) {	

      testValue = testValues[test_I];

      for ( i = 0; i < length; ++i ) {
         StgVariable_SetValueDouble( var, i, testValue );

         StgVariable_SetValueAtDouble( vec, i, 0, testValue );
         StgVariable_SetValueAtDouble( vec, i, 1, testValue );
         StgVariable_SetValueAtDouble( vec, i, 2, testValue );
      }

      /* "~~~Scalar~~~\n" */
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueDouble( var, i ) == (double)(double)testValue );
         pcu_check_true( StgVariable_GetValueDoubleAsChar( var, i ) == (char)(double)testValue );
         pcu_check_true( StgVariable_GetValueDoubleAsShort( var, i ) == (short)(double)testValue );
         pcu_check_true( StgVariable_GetValueDoubleAsInt( var, i ) == (int)(double)testValue );
         pcu_check_true( StgVariable_GetValueDoubleAsFloat( var, i ) == (float)(double)testValue );
      }

      /*~~~Vector~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueAtDouble( vec, i, 0 ) == (double)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsChar( vec, i, 0 ) == (char)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsShort( vec, i, 0 ) == (short)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsInt( vec, i, 0 ) == (int)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsFloat( vec, i, 0 ) == (float)(double)testValue );
         pcu_check_true( StgVariable_GetPtrAtDouble( vec, i, 0 ) == &structArray[i][0] );

         pcu_check_true( StgVariable_GetValueAtDouble( vec, i, 1 ) == (double)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsChar( vec, i, 1 ) == (char)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsShort( vec, i, 1 ) == (short)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsInt( vec, i, 1 ) == (int)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsFloat( vec, i, 1 ) == (float)(double)testValue );
         pcu_check_true( StgVariable_GetPtrAtDouble( vec, i, 1 ) == &structArray[i][1] );

         pcu_check_true( StgVariable_GetValueAtDouble( vec, i, 2 ) == (double)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsChar( vec, i, 2 ) == (char)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsShort( vec, i, 2 ) == (short)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsInt( vec, i, 2 ) == (int)(double)testValue );
         pcu_check_true( StgVariable_GetValueAtDoubleAsFloat( vec, i, 2 ) == (float)(double)testValue );
         pcu_check_true( StgVariable_GetPtrAtDouble( vec, i, 2 ) == &structArray[i][2] );
      }

      /*~~~Vector: Sub-Variable~~~*/
      for ( i = 0; i < length; ++i ) {
         for ( j = 0; j < 3; ++j ) {
            pcu_check_true( StgVariable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] );
         }
      }
   }
}


void VariableSuite_TestVariable_Float( VariableSuiteData* data ) {
   typedef float Triple[3];

   float* array;
   Triple* structArray;
   Index length = 10;

   float testValues[] = { 123456789.0, 0.987654321 };
   Index testValueCount = 2;
   Index test_I;
   float testValue;

   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];

   int i, j;

   array = Memory_Alloc_Array( float, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   var = StgVariable_NewScalar( "Float-Scalar", NULL, StgVariable_DataType_Float, &length, NULL, (void**)&array, data->vr );
   vec = StgVariable_NewVector( "Float-Three", NULL, StgVariable_DataType_Float, 3, &length, NULL, (void**)&structArray, data->vr, "a", "b", "c" );

   vecVar[0] = Variable_Register_GetByName( data->vr, "a" );
   vecVar[1] = Variable_Register_GetByName( data->vr, "b" );
   vecVar[2] = Variable_Register_GetByName( data->vr, "c" );

   Variable_Register_BuildAll( data->vr );

   for ( test_I = 0; test_I < testValueCount; ++test_I ) {	
      testValue = testValues[test_I];

      for ( i = 0; i < length; ++i ) {
         StgVariable_SetValueFloat( var, i, testValue );

         StgVariable_SetValueAtFloat( vec, i, 0, testValue );
         StgVariable_SetValueAtFloat( vec, i, 1, testValue );
         StgVariable_SetValueAtFloat( vec, i, 2, testValue );
      }
      
      /* "~~~Scalar~~~\n" */
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueFloat( var, i ) == (float)(float)testValue );
         pcu_check_true( StgVariable_GetValueFloatAsChar( var, i ) == (float)(char)testValue );
         pcu_check_true( StgVariable_GetValueFloatAsShort( var, i ) == (float)(short)testValue );
         pcu_check_true( StgVariable_GetValueFloatAsInt( var, i ) == (float)(int)testValue );
         pcu_check_true( StgVariable_GetValueFloatAsDouble( var, i ) == (double)(float)testValue );
      }

      /*~~~Vector~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueAtFloat( vec, i, 0 ) == (float)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsChar( vec, i, 0 ) == (char)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsShort( vec, i, 0 ) == (short)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsInt( vec, i, 0 ) == (int)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsDouble( vec, i, 0 ) == (double)(float)testValue );
         pcu_check_true( StgVariable_GetPtrAtFloat( vec, i, 0 ) == &structArray[i][0] );

         pcu_check_true( StgVariable_GetValueAtFloat( vec, i, 1 ) == (float)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsChar( vec, i, 1 ) == (char)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsShort( vec, i, 1 ) == (short)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsInt( vec, i, 1 ) == (int)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsDouble( vec, i, 1 ) == (double)(float)testValue );
         pcu_check_true( StgVariable_GetPtrAtFloat( vec, i, 1 ) == &structArray[i][1] );

         pcu_check_true( StgVariable_GetValueAtFloat( vec, i, 2 ) == (float)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsChar( vec, i, 2 ) == (char)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsShort( vec, i, 2 ) == (short)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsInt( vec, i, 2 ) == (int)(float)testValue );
         pcu_check_true( StgVariable_GetValueAtFloatAsDouble( vec, i, 2 ) == (double)(float)testValue );
         pcu_check_true( StgVariable_GetPtrAtFloat( vec, i, 2 ) == &structArray[i][2] );
      }

      /*~~~Vector: Sub-Variable~~~*/
      for ( i = 0; i < length; ++i ) {
         for ( j = 0; j < 3; ++j ) {
            pcu_check_true( StgVariable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] );
         }
      }
   }
}


void VariableSuite_TestVariable_Int( VariableSuiteData* data ) {
   typedef int Triple[3];

   int* array;
   Triple* structArray;
   Index length = 10;

   /* List of values to test the variable with.
    * Values to test are hex 5's and a's because they are a series of 0101 and 1010 respectively so they test
    * each bit in memory to read/set.
    */
   long int testValues[] = { 0x55555555, 0xaaaaaaaa };
   Index testValueCount = 2;
   Index test_I;
   long int testValue;

   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];

   int i, j;

   array = Memory_Alloc_Array( int, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   var = StgVariable_NewScalar( "Int-Scalar", NULL, StgVariable_DataType_Int, &length, NULL, (void**)&array, data->vr );
   vec = StgVariable_NewVector( "Int-Three", NULL, StgVariable_DataType_Int, 3, &length, NULL, (void**)&structArray, data->vr, "a", "b", "c" );

   vecVar[0] = Variable_Register_GetByName( data->vr, "a" );
   vecVar[1] = Variable_Register_GetByName( data->vr, "b" );
   vecVar[2] = Variable_Register_GetByName( data->vr, "c" );

   Variable_Register_BuildAll( data->vr );

   for ( test_I = 0; test_I < testValueCount; ++test_I ) {	

      testValue = testValues[test_I];

      for ( i = 0; i < length; ++i ) {
         StgVariable_SetValueInt( var, i, testValue );

         StgVariable_SetValueAtInt( vec, i, 0, testValue );
         StgVariable_SetValueAtInt( vec, i, 1, testValue );
         StgVariable_SetValueAtInt( vec, i, 2, testValue );
      }

      /*~~~Scalar~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueInt( var, i ) == (int)(int)testValue );
         pcu_check_true( StgVariable_GetValueIntAsChar( var, i ) == (char)(int)testValue );
         pcu_check_true( StgVariable_GetValueIntAsShort( var, i ) == (short)(int)testValue );
         pcu_check_true( fabsf(StgVariable_GetValueIntAsFloat( var, i ) - (float)(int)testValue) < fabsf(0.00001*testValue) );
         pcu_check_true( fabs(StgVariable_GetValueIntAsDouble( var, i ) - (double)(int)testValue) < fabs(0.00001*testValue) );
      }

      /*~~~Vector~~~*/
      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueAtInt( vec, i, 0 ) == (int)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsChar( vec, i, 0 ) == (char)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsShort( vec, i, 0 ) == (short)(int)testValue );
         pcu_check_true( fabsf(StgVariable_GetValueAtIntAsFloat( vec, i, 0 ) - (float)(int)testValue) < fabsf(0.00001*testValue) );
         pcu_check_true( fabs(StgVariable_GetValueAtIntAsDouble( vec, i, 0 ) - (double)(int)testValue) < fabs(0.00001*testValue));
         pcu_check_true( StgVariable_GetPtrAtInt( vec, i, 0 ) == &structArray[i][0] );

         pcu_check_true( StgVariable_GetValueAtInt( vec, i, 1 ) == (int)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsChar( vec, i, 1 ) == (char)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsShort( vec, i, 1 ) == (short)(int)testValue );
         pcu_check_true( fabsf(StgVariable_GetValueAtIntAsFloat( vec, i, 1 ) - (float)(int)testValue) < fabsf(0.00001*testValue) );
         pcu_check_true( fabs(StgVariable_GetValueAtIntAsDouble( vec, i, 1 ) - (double)(int)testValue) < fabs(0.00001*testValue));
         pcu_check_true( StgVariable_GetPtrAtInt( vec, i, 1 ) == &structArray[i][1] );

         pcu_check_true( StgVariable_GetValueAtInt( vec, i, 2 ) == (int)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsChar( vec, i, 2 ) == (char)(int)testValue );
         pcu_check_true( StgVariable_GetValueAtIntAsShort( vec, i, 2 ) == (short)(int)testValue );
         pcu_check_true( fabsf(StgVariable_GetValueAtIntAsFloat( vec, i, 2 ) - (float)(int)testValue) < fabsf(0.00001*testValue));
         pcu_check_true( fabs(StgVariable_GetValueAtIntAsDouble( vec, i, 2 ) - (double)(int)testValue) < fabs(0.0001*testValue));
         pcu_check_true( StgVariable_GetPtrAtInt( vec, i, 2 ) == &structArray[i][2] );
      }

      /*~~~Vector: Sub-Variable~~~*/
      for ( i = 0; i < length; ++i ) {
         for ( j = 0; j < 3; ++j ) {
            pcu_check_true( StgVariable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] );
         }
      }
   }
}


void VariableSuite_TestVariable_Short( VariableSuiteData* data ) {
   typedef short Triple[3];

   short* array;
   Triple* structArray;
   Index length = 10;

   /* List of values to test the variable with.
    * Values to test are hex 5's and a's because they are a series of 0101 and 1010 respectively so they test
    * each bit in memory to read/set.
    */
   long int testValues[] = { 0x5555, 0xaaaa };
   Index testValueCount = 2;
   Index test_I;
   long int testValue;

   StgVariable* var;
   StgVariable* vec;
   StgVariable* vecVar[3];

   int i, j;

   array = Memory_Alloc_Array( short, length, "test" );
   structArray = Memory_Alloc_Array( Triple, length, "test" );

   var = StgVariable_NewScalar( "Short-Scalar", NULL, StgVariable_DataType_Short, &length, NULL, (void**)&array, data->vr );
   vec = StgVariable_NewVector( "Short-Three", NULL, StgVariable_DataType_Short, 3, &length, NULL, (void**)&structArray, data->vr, "a", "b", "c" );

   vecVar[0] = Variable_Register_GetByName( data->vr, "a" );
   vecVar[1] = Variable_Register_GetByName( data->vr, "b" );
   vecVar[2] = Variable_Register_GetByName( data->vr, "c" );

   Variable_Register_BuildAll( data->vr );


   for ( test_I = 0; test_I < testValueCount; ++test_I ) {	

      testValue = testValues[test_I];


      for ( i = 0; i < length; ++i ) {
         StgVariable_SetValueShort( var, i, testValue );

         StgVariable_SetValueAtShort( vec, i, 0, testValue );
         StgVariable_SetValueAtShort( vec, i, 1, testValue );
         StgVariable_SetValueAtShort( vec, i, 2, testValue );
      }

      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueShort( var, i ) == (short)(short)testValue );
         pcu_check_true( StgVariable_GetValueShortAsChar( var, i ) == (char)(short)testValue );
         pcu_check_true( StgVariable_GetValueShortAsInt( var, i ) == (int)(short)testValue );
         pcu_check_true( StgVariable_GetValueShortAsFloat( var, i ) == (float)(short)testValue );
         pcu_check_true( StgVariable_GetValueShortAsDouble( var, i ) == (double)(short)testValue );
      }

      for ( i = 0; i < length; ++i ) {
         pcu_check_true( StgVariable_GetValueAtShort( vec, i, 0 ) == (short)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsChar( vec, i, 0 ) == (char)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsInt( vec, i, 0 ) == (int)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsFloat( vec, i, 0 ) == (float)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsDouble( vec, i, 0 ) == (double)(short)testValue );
         pcu_check_true( StgVariable_GetPtrAtShort( vec, i, 0 ) == &structArray[i][0] );

         pcu_check_true( StgVariable_GetValueAtShort( vec, i, 1 ) == (short)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsChar( vec, i, 1 ) == (char)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsInt( vec, i, 1 ) == (int)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsFloat( vec, i, 1 ) == (float)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsDouble( vec, i, 1 ) == (double)(short)testValue );
         pcu_check_true( StgVariable_GetPtrAtShort( vec, i, 1 ) == &structArray[i][1] );

         pcu_check_true( StgVariable_GetValueAtShort( vec, i, 2 ) == (short)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsChar( vec, i, 2 ) == (char)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsInt( vec, i, 2 ) == (int)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsFloat( vec, i, 2 ) == (float)(short)testValue );
         pcu_check_true( StgVariable_GetValueAtShortAsDouble( vec, i, 2 ) == (double)(short)testValue );
         pcu_check_true( StgVariable_GetPtrAtShort( vec, i, 2 ) == &structArray[i][2] );
      }

      for ( i = 0; i < length; ++i ) {
         for ( j = 0; j < 3; ++j ) {
            pcu_check_true( StgVariable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] );
         }
      }
   }
}


/* A few typedefs needed for the next test */
#define VECTOR_DATA_COUNT 3
typedef double Triple[VECTOR_DATA_COUNT];

struct ComplexStuff {
	int x;
	float y;
	char z; /* variablise the y and z member */
};
typedef struct ComplexStuff ComplexStuff;

struct MockContext {
	float*               scalars;
	Index                scalarCount;
	Triple*              vectors;
	Index                vectorCount;
	ComplexStuff*        stuff;
	Index                stuffCount;
	SizeT                complexStuffSize;
   Variable_Register*   vr;
};
typedef struct MockContext MockContext;

void VariableSuite_TestVariableCopy( VariableSuiteData* data ) {
   MockContext*         ctx1 = NULL;
   MockContext*         ctx2 = NULL;
	StgVariable*            scalar = NULL;
	StgVariable*            vector = NULL;
	StgVariable*            complexStuff = NULL;
   PtrMap*              ptrMap = PtrMap_New( 10 );
   Index                ii=0;
   Index                jj=0;
   Index                var_I=0;


   ctx1 = Memory_Alloc( MockContext, "ctx1" );
   ctx2 = Memory_Alloc( MockContext, "ctx2" );

	ctx1->scalarCount = 10;
	ctx1->vectorCount = 10;
	ctx1->stuffCount = 10;
	ctx1->complexStuffSize = sizeof( ComplexStuff );

	ctx1->scalars = Memory_Alloc_Array( float, ctx1->scalarCount, "ctx1->scalars" );
	ctx1->vectors = Memory_Alloc_Array( Triple, ctx1->vectorCount, "ctx1->vectors" );
	ctx1->stuff = Memory_Alloc_Array( ComplexStuff, ctx1->stuffCount, "ctx1->stuff" );

   ctx1->vr = Variable_Register_New();

	StgVariable_NewScalar( "Scalar", NULL, StgVariable_DataType_Float, &(ctx1->scalarCount), NULL, (void**)&(ctx1->scalars), ctx1->vr );
	StgVariable_NewVector( "Vector", NULL, StgVariable_DataType_Double, VECTOR_DATA_COUNT, &(ctx1->vectorCount), NULL, (void**)&(ctx1->vectors), ctx1->vr, "x", "y", "z" );

	{
		ComplexStuff tmp;
		SizeT dataOffsets[] = { 0, 0 };
		StgVariable_DataType dataTypes[] = { StgVariable_DataType_Float, StgVariable_DataType_Char };
		Index dataTypeCounts[] = { 1, 1 };
		Name dataNames[] = { "complexY", "complexZ" };
		
		dataOffsets[0] = (ArithPointer)&tmp.y - (ArithPointer)&tmp;
		dataOffsets[1] = (ArithPointer)&tmp.z - (ArithPointer)&tmp;

		StgVariable_New( "Complex", NULL, 2, dataOffsets, dataTypes, dataTypeCounts, dataNames, &(ctx1->complexStuffSize), &(ctx1->stuffCount), NULL, (void**)&(ctx1->stuff), ctx1->vr );
	}

	Variable_Register_BuildAll( ctx1->vr );

   scalar = Variable_Register_GetByName( ctx1->vr, "Scalar" );
	vector = Variable_Register_GetByName( ctx1->vr, "Vector" );
	complexStuff = Variable_Register_GetByName( ctx1->vr, "Complex" );

   for ( ii = 0; ii < ctx1->scalarCount; ++ii ) {
      StgVariable_SetValueFloat( scalar, ii, (float)ii );
   }
   for ( ii = 0; ii < ctx1->vectorCount; ++ii ) {
      StgVariable_SetValueAtDouble( vector, ii, 0, (double)ii );
      StgVariable_SetValueAtDouble( vector, ii, 1, (double)ii );
      StgVariable_SetValueAtDouble( vector, ii, 2, (double)ii );
   }
   for ( ii = 0; ii < ctx1->stuffCount; ++ii ) {
      ComplexStuff* stuff = (ComplexStuff*)StgVariable_GetStructPtr( complexStuff, ii );
      stuff->y = (float)ii;
      stuff->z = '0' + ii;
   }

	/* Indicate the area of memory which is given for data so that Variables can attach to it */
	PtrMap_Append( ptrMap, &(ctx1->scalars), &(ctx2->scalars) );
	PtrMap_Append( ptrMap, &(ctx1->vectors), &(ctx2->vectors) );
	PtrMap_Append( ptrMap, &(ctx1->stuff), &(ctx2->stuff) );
	
	PtrMap_Append( ptrMap, &(ctx1->scalarCount), &(ctx2->scalarCount) );
	PtrMap_Append( ptrMap, &(ctx1->vectorCount), &(ctx2->vectorCount) );
	PtrMap_Append( ptrMap, &(ctx1->stuffCount), &(ctx2->stuffCount) );
	PtrMap_Append( ptrMap, &(ctx1->complexStuffSize), &(ctx2->complexStuffSize) );

	ctx2->scalars = Memory_Alloc_Array( float, ctx1->scalarCount, "scalars" );
	ctx2->vectors = Memory_Alloc_Array( Triple, ctx1->vectorCount, "vectors" );
	ctx2->stuff = Memory_Alloc_Array( ComplexStuff, ctx1->stuffCount, "stuff" );
	PtrMap_Append( ptrMap, ctx1->scalars, ctx2->scalars );
	PtrMap_Append( ptrMap, ctx1->vectors, ctx2->vectors );
	PtrMap_Append( ptrMap, ctx1->stuff, ctx2->stuff );

   /* Doing a copy of the whole Variable Register, should trigger a copy of the values in all the variables */
	ctx2->vr = (Variable_Register*)Stg_Class_Copy( ctx1->vr, NULL, True, NULL, ptrMap );

   /* test equality of copy */
   for ( ii = 0; ii < ctx1->scalarCount; ++ii ) {
      pcu_check_true( ctx1->scalars[ii] == ctx2->scalars[ii] );
   }
   for ( ii = 0; ii < ctx1->vectorCount; ++ii ) {
      for ( jj = 0; jj < VECTOR_DATA_COUNT; ++jj ) {
         pcu_check_true( ctx1->vectors[ii][jj] == ctx2->vectors[ii][jj] );
      }
   }
   for ( ii = 0; ii < ctx1->stuffCount; ++ii ) {
      pcu_check_true( ctx1->stuff->y == ctx2->stuff->y );
      pcu_check_true( ctx1->stuff->z == ctx2->stuff->z );
   }

   /* Clean up */
	for ( var_I = 0; var_I < ctx1->vr->count; ++var_I ) {
		Stg_Class_Delete( ctx1->vr->_variable[var_I] );
	}
	Memory_Free( ctx1->scalars );
	Memory_Free( ctx1->vectors );
	Memory_Free( ctx1->stuff );
	Stg_Class_Delete( ctx1->vr );
	Memory_Free( ctx1 );
	for ( var_I = 0; var_I < ctx2->vr->count; ++var_I ) {
		Stg_Class_Delete( ctx2->vr->_variable[var_I] );
	}
	Memory_Free( ctx2->scalars );
	Memory_Free( ctx2->vectors );
	Memory_Free( ctx2->stuff );
	Stg_Class_Delete( ctx2->vr );
	Memory_Free( ctx2 );
}


void VariableSuite_TestVariableValueCompare( VariableSuiteData* data ) {
   StgVariable*           orig;
   StgVariable*           compare;
   double*             dataArray;
   double*             dataArray2;
   Index               arrayCount      = 150;
   Index               componentCount  = 4;
   Index               index;
   double              amp             = 0.1;
   double              tolerance       = 0.04;

   dataArray = Memory_Alloc_Array( double, arrayCount * componentCount, "test" );
   dataArray2 = Memory_Alloc_Array( double, arrayCount * componentCount, "test" );
   for( index = 0; index < arrayCount * componentCount; index++ ) {
      dataArray[index] = 1.0f / (arrayCount+2) * (index+1); 
      dataArray2[ index ] = dataArray[ index ] + amp * cos( index );
   }		
   
   orig = StgVariable_NewVector( "orig", NULL, StgVariable_DataType_Double, componentCount,
		&arrayCount, NULL, (void**)&dataArray, data->vr, "orig1", "orig2", "orig3", "orig4" );
   compare = StgVariable_NewVector( "compare", NULL, StgVariable_DataType_Double, componentCount,
		&arrayCount, NULL, (void**)&dataArray2, data->vr, "compare1", "compare2", "compare3", "compare4" );

   Stg_Component_Build( orig, 0, False );
   Stg_Component_Build( compare, 0, False );

   pcu_check_true( abs( 0.030987 - StgVariable_ValueCompare( orig, compare ) ) < 0.00001 );
   pcu_check_true( True == StgVariable_ValueCompareWithinTolerance( orig, compare, tolerance ) );

   Memory_Free( dataArray );
   Memory_Free( dataArray2 );
}


void VariableSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, VariableSuiteData );
   pcu_suite_setFixtures( suite, VariableSuite_Setup, VariableSuite_Teardown );
   pcu_suite_addTest( suite, VariableSuite_TestGetValueDouble );
   pcu_suite_addTest( suite, VariableSuite_TestSetValueDouble );
   pcu_suite_addTest( suite, VariableSuite_TestGetValueAtDouble );
   pcu_suite_addTest( suite, VariableSuite_TestSetValueAtDouble );
   pcu_suite_addTest( suite, VariableSuite_TestVariable_Char );
   pcu_suite_addTest( suite, VariableSuite_TestVariable_Double );
   pcu_suite_addTest( suite, VariableSuite_TestVariable_Float );
   pcu_suite_addTest( suite, VariableSuite_TestVariable_Int );
   pcu_suite_addTest( suite, VariableSuite_TestVariable_Short );
   pcu_suite_addTest( suite, VariableSuite_TestVariableCopy );
   pcu_suite_addTest( suite, VariableSuite_TestVariableValueCompare );
}


