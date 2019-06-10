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
#include <StGermain/StGermain.h> 
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h" 
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "TensorMultMathSuite.h"

#define TENSORMULTMATH_TEST_ERROR 1e-05

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} TensorMultMathSuiteData;

void TensorMultMathSuite_Setup( TensorMultMathSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void TensorMultMathSuite_Teardown( TensorMultMathSuiteData* data ) {
}

void TensorMultMathSuite_TestTensorArrayIdentity( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayIdentity" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayIdentity.dat" );
      Journal_Printf( stream, "*******************    Test 1   ************************\n");
      Journal_Printf( stream, "Test TensorArray Identity\n\n");
    
      Journal_Printf( stream, "2-D\n");
      TensorArray_Identity( 2, tensorArray );
      Journal_PrintTensorArray( stream, tensorArray, 2); 
    
      Journal_Printf( stream, "3-D\n");
      TensorArray_Identity( 3, tensorArray );
      Journal_PrintTensorArray( stream, tensorArray, 3); 

      pcu_filename_expected( "testTensorMultMathArrayIdentity.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayIdentity.dat", expected_file );
      remove( "testTensorArrayIdentity.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestSymmetricTensorIdentity( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"SymmetricTensorIdentity" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testSymmetricTensorIdentity.dat" );

      Journal_Printf( stream, "*******************    Test 2   ************************\n");
      Journal_Printf( stream, "Test SymmetricTensor Identity\n\n");
    
      Journal_Printf( stream, "2-D\n");
      SymmetricTensor_Identity( 2, symmTensor );
      Journal_PrintSymmetricTensor( stream, symmTensor, 2); 
    
      Journal_Printf( stream, "3-D\n");
      SymmetricTensor_Identity( 3, symmTensor );
      Journal_PrintSymmetricTensor( stream, symmTensor, 3); 

      pcu_filename_expected( "testTensorMultMathSymmetricTensorIdentity.expected", expected_file );
      pcu_check_fileEq( "testSymmetricTensorIdentity.dat", expected_file );
      remove( "testSymmetricTensorIdentity.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayTranspose( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray, tensorResult;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayTranspose" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayTranspose.dat" );

      Journal_Printf( stream, "*******************    Test 3   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_Transpose \n\n");

      Journal_Printf( stream, "2-D\n");    
      tensorArray[FT2D_00] = 0.7; tensorArray[FT2D_01] = 1;
      tensorArray[FT2D_10] = 2; tensorArray[FT2D_11] = 3;

      Journal_PrintTensorArray( stream, tensorArray, 2); 
      TensorArray_Transpose( tensorArray, 2, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 2); 

      Journal_Printf( stream, "3-D\n");    
      tensorArray[FT3D_00] = 0.5; tensorArray[FT3D_01] = 10; tensorArray[FT3D_02] = 20; 
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 40; tensorArray[FT3D_12] = 50; 
      tensorArray[FT3D_20] = 60; tensorArray[FT3D_21] = 70; tensorArray[FT3D_22] = 80;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      TensorArray_Transpose( tensorArray, 3, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 3);

      pcu_filename_expected( "testTensorMultTranspose.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayTranspose.dat", expected_file );
      remove( "testTensorArrayTranspose.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayAdd( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray, tensorArray2, tensorResult;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayAdd" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayAdd.dat" );

      Journal_Printf( stream, "*******************    Test 4   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_Add \n\n");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 0.7; tensorArray[FT2D_01] = 1;
      tensorArray[FT2D_10] = 2; tensorArray[FT2D_11] = 3;

      tensorArray2[FT2D_00] = 5; tensorArray2[FT2D_01] = 6;
      tensorArray2[FT2D_10] = 7; tensorArray2[FT2D_11] = 8;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      Journal_PrintTensorArray( stream, tensorArray2, 2);
      TensorArray_Add( tensorArray, tensorArray2, 2, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 0.5; tensorArray[FT3D_01] = 10; tensorArray[FT3D_02] = 20; 
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 40; tensorArray[FT3D_12] = 50; 
      tensorArray[FT3D_20] = 60; tensorArray[FT3D_21] = 70; tensorArray[FT3D_22] = 80;

      tensorArray2[FT3D_00] = 5; tensorArray2[FT3D_01] = 1; tensorArray2[FT3D_02] = 2;
      tensorArray2[FT3D_10] = 3; tensorArray2[FT3D_11] = 4; tensorArray2[FT3D_12] = 5;
      tensorArray2[FT3D_20] = 6; tensorArray2[FT3D_21] = 7; tensorArray2[FT3D_22] = 8;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      Journal_PrintTensorArray( stream, tensorArray2, 3);
      TensorArray_Add( tensorArray, tensorArray2, 3, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 3);

      pcu_filename_expected( "testTensorMultAdd.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayAdd.dat", expected_file );
      remove( "testTensorArrayAdd.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArraySubstract( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray, tensorArray2, tensorResult;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArraySubstract" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArraySubstract.dat" );

      Journal_Printf( stream, "*******************    Test 5   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_Subtract \n\n");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 0.7; tensorArray[FT2D_01] = 1;
      tensorArray[FT2D_10] = 2; tensorArray[FT2D_11] = 3;

      tensorArray2[FT2D_00] = 5; tensorArray2[FT2D_01] = 6;
      tensorArray2[FT2D_10] = 7; tensorArray2[FT2D_11] = 8;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      Journal_PrintTensorArray( stream, tensorArray2, 2);
      TensorArray_Subtract( tensorArray, tensorArray2, 2, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 0.5; tensorArray[FT3D_01] = 10; tensorArray[FT3D_02] = 20; 
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 40; tensorArray[FT3D_12] = 50; 
      tensorArray[FT3D_20] = 60; tensorArray[FT3D_21] = 70; tensorArray[FT3D_22] = 80;

      tensorArray2[FT3D_00] = 50; tensorArray2[FT3D_01] = 1; tensorArray2[FT3D_02] = 2;
      tensorArray2[FT3D_10] = 3; tensorArray2[FT3D_11] = 4; tensorArray2[FT3D_12] = 5;
      tensorArray2[FT3D_20] = 6; tensorArray2[FT3D_21] = 7; tensorArray2[FT3D_22] = 8;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      Journal_PrintTensorArray( stream, tensorArray2, 3);
      TensorArray_Subtract( tensorArray, tensorArray2, 3, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 3);

      pcu_filename_expected( "testTensorMultSubstract.expected", expected_file );
      pcu_check_fileEq( "testTensorArraySubstract.dat", expected_file );
      remove( "testTensorArraySubstract.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyByTensorArray( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray, tensorArray2, tensorResult;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyByTensorArray" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayMultiplyByTensorArray.dat" );

      Journal_Printf( stream, "*******************    Test 6   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyByTensorArray \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");
      Journal_Printf( stream, "2-D\n");

      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      tensorArray2[FT2D_00] = 5; tensorArray2[FT2D_01] = 6;
      tensorArray2[FT2D_10] = 7; tensorArray2[FT2D_11] = 8;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      Journal_PrintTensorArray( stream, tensorArray2, 2);
      TensorArray_MultiplyByTensorArray( tensorArray, tensorArray2, 2, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      tensorArray2[FT3D_00] = 10; tensorArray2[FT3D_01] = 11; tensorArray2[FT3D_02] = 12;
      tensorArray2[FT3D_10] = 13; tensorArray2[FT3D_11] = 14; tensorArray2[FT3D_12] = 15;
      tensorArray2[FT3D_20] = 16; tensorArray2[FT3D_21] = 17; tensorArray2[FT3D_22] = 18;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      Journal_PrintTensorArray( stream, tensorArray2, 3);
      TensorArray_MultiplyByTensorArray( tensorArray, tensorArray2, 3, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyByTensorArray.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyByTensorArray.dat", expected_file );
      remove( "testTensorArrayMultiplyByTensorArray.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyByRightTranspose( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensorResult;
      TensorArray     tensorArray;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyByRightTranspose" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayMultiplyByRightTranspose.dat" );

      Journal_Printf( stream, "*******************    Test 7   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyByRightTranspose \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      TensorArray_MultiplyByRightTranspose( tensorArray, 2, symmTensorResult);
      Journal_Printf( stream, "The answer, A * A^T = \n");
      Journal_PrintSymmetricTensor( stream, symmTensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      TensorArray_MultiplyByRightTranspose( tensorArray, 3, symmTensorResult);
      Journal_Printf( stream, "The answer, A * A^T = \n");
      Journal_PrintSymmetricTensor( stream, symmTensorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyByRightTranspose.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyByRightTranspose.dat", expected_file );
      remove( "testTensorArrayMultiplyByRightTranspose.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyByLeftTranspose( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensorResult;
      TensorArray     tensorArray;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyByLeftTranspose" );
      char            expected_file[PCU_PATH_MAX];
 
      Stream_RedirectFile( stream, "testTensorArrayMultiplyByLeftTranspose.dat" );

      Journal_Printf( stream, "*******************    Test 8   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyByLeftTranspose \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      TensorArray_MultiplyByLeftTranspose( tensorArray, 2, symmTensorResult);
      Journal_Printf( stream, "The answer, A^T * A = \n");
      Journal_PrintSymmetricTensor( stream, symmTensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      TensorArray_MultiplyByLeftTranspose( tensorArray, 3, symmTensorResult);
      Journal_Printf( stream, "The answer, A^T * A = \n");
      Journal_PrintSymmetricTensor( stream, symmTensorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyByLeftTranspose.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyByLeftTranspose.dat", expected_file );
      remove( "testTensorArrayMultiplyByLeftTranspose.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyBySymmetricTensor( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      TensorArray     tensorArray, tensorResult;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyBySymmetricTensor" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayMultiplyBySymemtricTensor.dat" );

      Journal_Printf( stream, "*******************    Test 9   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyBySymmetricTensor \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      symmTensor[ST2D_00] = 5; symmTensor[ST2D_01] = 6; symmTensor[ST2D_11] = 7;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      Journal_PrintSymmetricTensor( stream, symmTensor, 2);
      TensorArray_MultiplyBySymmetricTensor( tensorArray, symmTensor, 2, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      symmTensor[ST3D_00] = 10; symmTensor[ST3D_01] = 11; symmTensor[ST3D_02] = 12;
      symmTensor[ST3D_11] = 13; symmTensor[ST3D_12] = 14; symmTensor[ST3D_22] = 15;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      Journal_PrintSymmetricTensor( stream, symmTensor, 3);
      TensorArray_MultiplyBySymmetricTensor( tensorArray, symmTensor, 3, tensorResult);
      Journal_PrintTensorArray( stream, tensorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyBySymmetricTensor.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyBySymemtricTensor.dat", expected_file );
      remove( "testTensorArrayMultiplyBySymemtricTensor.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyByLeftVector( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      XYZ         vector, vectorResult;
      TensorArray tensorArray;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyByLeftVector" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayMultiplyByLeftVector.dat" );

      Journal_Printf( stream, "*******************    Test 10   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyByLeftVector \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      vector[0] = 5; vector[1] = 6;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      StGermain_PrintNamedVector( stream, vector, 2);
      TensorArray_MultiplyByLeftVector( tensorArray, vector, 2, vectorResult);
      StGermain_PrintNamedVector( stream, vectorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      vector[0] = 10; vector[1] = 11; vector[2] = 12;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      StGermain_PrintNamedVector( stream, vector, 3);
      TensorArray_MultiplyByLeftVector( tensorArray, vector, 3, vectorResult);
      StGermain_PrintNamedVector( stream, vectorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyByLeftVector.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyByLeftVector.dat", expected_file );
      remove( "testTensorArrayMultiplyByLeftVector.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayMultiplyByRightVector( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      XYZ         vector, vectorResult;
      TensorArray tensorArray;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayMultiplyByRightVector" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorArrayMultiplyByRightVector.dat" );

      Journal_Printf( stream, "*******************    Test 11   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_MultiplyByRightVector \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      vector[0] = 5; vector[1] = 6;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      StGermain_PrintNamedVector( stream, vector, 2);
      TensorArray_MultiplyByRightVector( tensorArray, vector, 2, vectorResult);
      StGermain_PrintNamedVector( stream, vectorResult, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;

      vector[0] = 10; vector[1] = 11; vector[2] = 12;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      StGermain_PrintNamedVector( stream, vector, 3);
      TensorArray_MultiplyByRightVector( tensorArray, vector, 3, vectorResult);
      StGermain_PrintNamedVector( stream, vectorResult, 3);

      pcu_filename_expected( "testTensorMultMultiplyByRightVector.expected", expected_file );
      pcu_check_fileEq( "testTensorArrayMultiplyByRightVector.dat", expected_file );
      remove( "testTensorArrayMultiplyByRightVector.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestCalcDeterminant( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray;
      double      result;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"CalcDeterminant" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testCalcDeterminant.dat" );

      Journal_Printf( stream, "*******************    Test 12   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_CalcDeterminant \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.bluebit.gr/matrix-calculator/");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      result = TensorArray_CalcDeterminant( tensorArray, 2);
      Journal_Printf( stream, "Determinant = \n");
      Journal_PrintValue( stream, result);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 22; tensorArray[FT3D_12] = 4;
      tensorArray[FT3D_20] = 5; tensorArray[FT3D_21] = 7; tensorArray[FT3D_22] = 9;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      result = TensorArray_CalcDeterminant( tensorArray, 3);
      Journal_Printf( stream, "Determinant = \n");
      Journal_PrintValue( stream, result);

      pcu_filename_expected( "testTensorMultCalcDeterminant.expected", expected_file );
      pcu_check_fileEq( "testCalcDeterminant.dat", expected_file );
      remove( "testCalcDeterminant.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestCalcInverseWithDeterminant( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray     tensorArray, tensorResult, tensorCompare, tensorDiff;
      double          result, errorValue;
      Dimension_Index row, col;
      Bool            tensorMultMathTest_Flag;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"CalcInverseWithDeterminant" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testCalcInverseWithDeterminant.dat" );

      #define STG_TENSORMULTMATHTEST_ERROR 1.0e-14;
      errorValue = STG_TENSORMULTMATHTEST_ERROR;

      Journal_Printf( stream, "*******************    Test 13   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_CalcInverseWithDeterminant \n\n");

      Journal_Printf( stream, "2-D\n");

      tensorMultMathTest_Flag = True;

      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      tensorCompare[FT2D_00] = -2; tensorCompare[FT2D_01] = 1;
      tensorCompare[FT2D_10] = 1.5; tensorCompare[FT2D_11] = -0.5;

      Journal_PrintTensorArray( stream, tensorArray, 2);
      result = TensorArray_CalcDeterminant( tensorArray, 2);
      TensorArray_CalcInverseWithDeterminant( tensorArray, result, 2, tensorResult);

      TensorArray_Subtract( tensorResult, tensorCompare, 2, tensorDiff);

      for (row = 0; row < 2; row++ ){
         for ( col = 0; col < 2; col++ ){
            if (fabs( tensorDiff[TensorArray_TensorMap( row, col, 2)]) > errorValue ) {
               Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n",
                  tensorResult[TensorArray_TensorMap( row, col, 2)],row, col, errorValue);
               tensorMultMathTest_Flag = False;
            }
         }
      }
      if (tensorMultMathTest_Flag == True) {
         Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
      }
      else {
         Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);
      }

      Journal_Printf( stream, "Inverse of tensor:\n");
      Journal_PrintTensorArray( stream, tensorCompare, 2);

      Journal_Printf( stream, "3-D\n");

      tensorMultMathTest_Flag = True;

      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 22; tensorArray[FT3D_12] = 4;
      tensorArray[FT3D_20] = 5; tensorArray[FT3D_21] = 7; tensorArray[FT3D_22] = 9;

      tensorCompare[FT3D_00] = -5.666666666666667; tensorCompare[FT3D_01] = -0.100000000000000;
      tensorCompare[FT3D_02] =  1.933333333333333; tensorCompare[FT3D_10] =  8.333333333333333;
      tensorCompare[FT3D_11] =  0.200000000000000; tensorCompare[FT3D_12] = -2.866666666666667;
      tensorCompare[FT3D_20] = -3.333333333333333; tensorCompare[FT3D_21] = -0.100000000000000;
      tensorCompare[FT3D_22] =  1.266666666666667;

      Journal_PrintTensorArray( stream, tensorArray, 3);
      result = TensorArray_CalcDeterminant( tensorArray, 3);
      TensorArray_CalcInverseWithDeterminant( tensorArray,result, 3, tensorResult);

      TensorArray_Subtract( tensorResult, tensorCompare, 3, tensorDiff);

      for (row = 0; row < 3; row++ ){
         for ( col = 0; col < 3; col++ ){
            if (fabs( tensorDiff[TensorArray_TensorMap( row, col, 3)]) > errorValue ) {
               Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n",
                  tensorResult[TensorArray_TensorMap( row, col, 3)],row, col, errorValue);
               tensorMultMathTest_Flag = False;
            }
         }
      }
      if (tensorMultMathTest_Flag == True) {
         Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
      }
      else {
         Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);
      }

      Journal_Printf( stream, "Inverse of tensor:\n");
      Journal_PrintTensorArray( stream, tensorCompare, 3);

      pcu_filename_expected( "testTensorMultCalcInverseWithDeterminant.expected", expected_file );
      pcu_check_fileEq( "testCalcInverseWithDeterminant.dat", expected_file );
      remove( "testCalcInverseWithDeterminant.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestCalcInverse( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray     tensorArray, tensorResult, tensorCompare, tensorDiff;
      double          errorValue;
      Dimension_Index row, col;
      Bool            tensorMultMathTest_Flag;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"CalcInverse" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testCalcInverse.dat" );

      #define STG_TENSORMULTMATHTEST_ERROR 1.0e-14;
      errorValue = STG_TENSORMULTMATHTEST_ERROR;

      Journal_Printf( stream, "*******************    Test 14   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_CalcInverse \n\n");
      Journal_Printf( stream, "Solutions tested against: http://www.bluebit.gr/matrix-calculator/");      

      Journal_Printf( stream, "2-D\n");

      tensorMultMathTest_Flag = True;      
      
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      tensorCompare[FT2D_00] = -2; tensorCompare[FT2D_01] =  1;
      tensorCompare[FT2D_10] =  1.5; tensorCompare[FT2D_11] = -0.5;
      
      Journal_PrintTensorArray( stream, tensorArray, 2);   
      TensorArray_CalcInverse( tensorArray, 2, tensorResult);
      
      TensorArray_Subtract( tensorResult, tensorCompare, 2, tensorDiff);
      
      for (row = 0; row < 2; row++ ){
         for ( col = 0; col < 2; col++ ){
            if (fabs( tensorDiff[TensorArray_TensorMap( row, col, 2)]) > errorValue ) {
               Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
                     tensorResult[TensorArray_TensorMap( row, col, 2)],row, col, errorValue);
               tensorMultMathTest_Flag = False;
            }
         }
      }
      if (tensorMultMathTest_Flag == True) {
         Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
      }
      else {
         Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);         
      }
      
      Journal_Printf( stream, "Inverse of tensor:\n");
      Journal_PrintTensorArray( stream, tensorCompare, 2);
      
      Journal_Printf( stream, "3-D\n");

      tensorMultMathTest_Flag = True;

      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 30; tensorArray[FT3D_11] = 22; tensorArray[FT3D_12] = 4;
      tensorArray[FT3D_20] = 5; tensorArray[FT3D_21] = 7; tensorArray[FT3D_22] = 9;

      tensorCompare[FT3D_00] = -5.666666666666667; tensorCompare[FT3D_01] = -0.1;
      tensorCompare[FT3D_02] =  1.933333333333333; tensorCompare[FT3D_10] =  8.333333333333333;
      tensorCompare[FT3D_11] =  0.200000000000; tensorCompare[FT3D_12] = -2.866666666666667;
      tensorCompare[FT3D_20] = -3.333333333333333; tensorCompare[FT3D_21] = -0.1;
      tensorCompare[FT3D_22] =  1.266666666666667;
      
      Journal_PrintTensorArray( stream, tensorArray, 3);   
      TensorArray_CalcInverse( tensorArray, 3, tensorResult);
      
      TensorArray_Subtract( tensorResult, tensorCompare, 3, tensorDiff);
      
      for (row = 0; row < 3; row++ ){
         for ( col = 0; col < 3; col++ ){
            if (fabs( tensorDiff[TensorArray_TensorMap( row, col, 3)]) > errorValue ) {
               Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
                     tensorResult[TensorArray_TensorMap( row, col, 3)],row, col, errorValue);
               tensorMultMathTest_Flag = False;
            }
         }
      }
      
      if (tensorMultMathTest_Flag == True) {
         Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
      }
      else {
         Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);         
      }
      
      Journal_Printf( stream, "Inverse of tensor:\n");
      Journal_PrintTensorArray( stream, tensorCompare, 3);

      pcu_filename_expected( "testTensorMultCalcInverse.expected", expected_file );
      pcu_check_fileEq( "testCalcInverse.dat", expected_file );
      remove( "testCalcInverse.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestTensorArrayDoubleContraction( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      TensorArray tensorArray, tensorArray2;
      double      result;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayDoubleContraction" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTADoubleContraction.dat" );

      Journal_Printf( stream, "*******************    Test 15   ************************\n");
      Journal_Printf( stream, "Test function TensorArray_DoubleContraction \n\n");
      Journal_Printf( stream, "Hand verified\n");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1; tensorArray[FT2D_01] = 2;
      tensorArray[FT2D_10] = 3; tensorArray[FT2D_11] = 4;

      tensorArray2[FT2D_00] = 5; tensorArray2[FT2D_01] = 6;
      tensorArray2[FT2D_10] = 7; tensorArray2[FT2D_11] = 8;
      
      Journal_PrintTensorArray( stream, tensorArray, 2);   
      Journal_PrintTensorArray( stream, tensorArray2, 2);   
      result = TensorArray_DoubleContraction( tensorArray, tensorArray2, 2);
      Journal_Printf( stream, "Double Contraction = \n");
      Journal_PrintValue( stream, result);
      
      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 1; tensorArray[FT3D_01] = 2; tensorArray[FT3D_02] = 3;
      tensorArray[FT3D_10] = 4; tensorArray[FT3D_11] = 5; tensorArray[FT3D_12] = 6;
      tensorArray[FT3D_20] = 7; tensorArray[FT3D_21] = 8; tensorArray[FT3D_22] = 9;
      
      tensorArray2[FT3D_00] = 11; tensorArray2[FT3D_01] = 12; tensorArray2[FT3D_02] = 13;
      tensorArray2[FT3D_10] = 14; tensorArray2[FT3D_11] = 15; tensorArray2[FT3D_12] = 16;
      tensorArray2[FT3D_20] = 17; tensorArray2[FT3D_21] = 18; tensorArray2[FT3D_22] = 19;
      
      Journal_PrintTensorArray( stream, tensorArray, 3);   
      Journal_PrintTensorArray( stream, tensorArray2, 3);   
      result = TensorArray_DoubleContraction( tensorArray, tensorArray2, 3);
      Journal_Printf( stream, "Double Contraction = \n");
      Journal_PrintValue( stream, result);

      pcu_filename_expected( "testTensorMultTADoubleContraction.expected", expected_file );
      pcu_check_fileEq( "testTADoubleContraction.dat", expected_file );
      remove( "testTADoubleContraction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestSymmetricTensorDoubleContraction( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor, symmTensor2;
      double          result;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"SymmetricTensorDoubleContraction" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testSTDoubleContraction.dat" );

      Journal_Printf( stream, "*******************    Test 16   ************************\n");
      Journal_Printf( stream, "Test function SymmetricTensor_DoubleContraction \n\n");
      Journal_Printf( stream, "Hand verified\n");
      Journal_Printf( stream, "2-D\n");      
      symmTensor[ST2D_00] = 1; symmTensor[ST2D_01] = 2; symmTensor[ST2D_11] = 4;

      symmTensor2[ST2D_00] = 10; symmTensor2[ST2D_01] = 20; symmTensor2[ST2D_11] = 40;

      Journal_PrintSymmetricTensor( stream, symmTensor, 2);
      Journal_PrintSymmetricTensor( stream, symmTensor2, 2);
      result = SymmetricTensor_DoubleContraction( symmTensor,symmTensor2, 2);
      Journal_Printf( stream, "Double Contraction = \n");
      Journal_PrintValue( stream, result);

      Journal_Printf( stream, "3-D\n");
      symmTensor[ST3D_00] = 1; symmTensor[ST3D_01] = 2; symmTensor[ST3D_02] = 3;
      symmTensor[ST3D_11] = 4; symmTensor[ST3D_12] = 5; symmTensor[ST3D_22] = 6;

      symmTensor2[ST3D_00] = 10; symmTensor2[ST3D_01] = 20; symmTensor2[ST3D_02] = 30;
      symmTensor2[ST3D_11] = 40; symmTensor2[ST3D_12] = 50; symmTensor2[ST3D_22] = 60;
      
      Journal_PrintSymmetricTensor( stream, symmTensor, 3);
      Journal_PrintSymmetricTensor( stream, symmTensor2, 3);
      result = SymmetricTensor_DoubleContraction( symmTensor,symmTensor2, 3);
      Journal_Printf( stream, "Double Contraction = \n");
      Journal_PrintValue( stream, result);

      pcu_filename_expected( "testTensorMultSTDoubleContraction.expected", expected_file );
      pcu_check_fileEq( "testSTDoubleContraction.dat", expected_file );
      remove( "testSTDoubleContraction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestPrintNonSquareMatrix( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double  **nonSquareMatrixA, **nonSquareMatrixB;
      double  **nonSquareMatrixResult, **nonSquareMatrixAT;
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"PrintNonSquareMatrix" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testPrintNonSquareMatrix.dat" );

      Journal_Printf( stream, "*******************    Test 17   ************************\n");
      Journal_Printf( stream, "Test function Journal_PrintNonSquareMatrix \n\n");

      nonSquareMatrixA = Memory_Alloc_2DArray( double, 3, 4, (Name)"NonSquareMatrixA" );
      nonSquareMatrixB = Memory_Alloc_2DArray( double, 4, 2, (Name)"NonSquareMatrixB" );
      nonSquareMatrixAT = Memory_Alloc_2DArray( double, 4, 3, (Name)"NonSquareMatrixAT" );
      nonSquareMatrixResult = Memory_Alloc_2DArray( double, 3, 2, (Name)"NonSquareMatrixResult" );
      /* Matrix A */
      nonSquareMatrixA[0][0] = 0; nonSquareMatrixA[1][0] =  4;
      nonSquareMatrixA[0][1] = 1; nonSquareMatrixA[1][1] =  5;
      nonSquareMatrixA[0][2] = 2; nonSquareMatrixA[1][2] =  6;      
      nonSquareMatrixA[0][3] = 3; nonSquareMatrixA[1][3] =  7;
      
      nonSquareMatrixA[2][0] = 8; nonSquareMatrixA[2][1] = 9;
      nonSquareMatrixA[2][2] = 10; nonSquareMatrixA[2][3] = 11;
      /* Matrix B */
      nonSquareMatrixB[0][0] = 0; nonSquareMatrixB[1][0] =  2;
      nonSquareMatrixB[0][1] = 1; nonSquareMatrixB[1][1] =  3;
      
      nonSquareMatrixB[2][0] = 4; nonSquareMatrixB[3][0] =  6;
      nonSquareMatrixB[2][1] = 5; nonSquareMatrixB[3][1] =  7;
      
      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixA, 3, 4);
      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixB, 4, 2);

      pcu_filename_expected( "testTensorMultPrintNonSquareMatrix.expected", expected_file );
      pcu_check_fileEq( "testPrintNonSquareMatrix.dat", expected_file );
      remove( "testPrintNonSquareMatrix.dat" );

      Memory_Free( nonSquareMatrixA);
      Memory_Free( nonSquareMatrixB);
      Memory_Free( nonSquareMatrixAT);
      Memory_Free( nonSquareMatrixResult);

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestNonSquareMatrixTranspose( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double  **nonSquareMatrixA, **nonSquareMatrixB;
      double  **nonSquareMatrixResult, **nonSquareMatrixAT;
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"NonSquareMatrixTranspose" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testNonSquareMatrixTranspose.dat" );

      Journal_Printf( stream, "*******************    Test 18   ************************\n");
      Journal_Printf( stream, "Test function NonSquareMatrix_Transpose \n\n");

      nonSquareMatrixA = Memory_Alloc_2DArray( double, 3, 4, (Name)"NonSquareMatrixA" );
      nonSquareMatrixB = Memory_Alloc_2DArray( double, 4, 2, (Name)"NonSquareMatrixB" );
      nonSquareMatrixAT = Memory_Alloc_2DArray( double, 4, 3, (Name)"NonSquareMatrixAT" );
      nonSquareMatrixResult = Memory_Alloc_2DArray( double, 3, 2, (Name)"NonSquareMatrixResult" );
      /* Matrix A */
      nonSquareMatrixA[0][0] = 0; nonSquareMatrixA[1][0] =  4;
      nonSquareMatrixA[0][1] = 1; nonSquareMatrixA[1][1] =  5;
      nonSquareMatrixA[0][2] = 2; nonSquareMatrixA[1][2] =  6;      
      nonSquareMatrixA[0][3] = 3; nonSquareMatrixA[1][3] =  7;
      
      nonSquareMatrixA[2][0] = 8; nonSquareMatrixA[2][1] = 9;
      nonSquareMatrixA[2][2] = 10; nonSquareMatrixA[2][3] = 11;
      /* Matrix B */
      nonSquareMatrixB[0][0] = 0; nonSquareMatrixB[1][0] =  2;
      nonSquareMatrixB[0][1] = 1; nonSquareMatrixB[1][1] =  3;
      
      nonSquareMatrixB[2][0] = 4; nonSquareMatrixB[3][0] =  6;
      nonSquareMatrixB[2][1] = 5; nonSquareMatrixB[3][1] =  7;

      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixA, 3, 4);
      NonSquareMatrix_Transpose( nonSquareMatrixA, 3, 4, nonSquareMatrixAT);
      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixAT, 4, 3);

      pcu_filename_expected( "testTensorMultNonSquareMatrixTranspose.expected", expected_file );
      pcu_check_fileEq( "testNonSquareMatrixTranspose.dat", expected_file );
      remove( "testNonSquareMatrixTranspose.dat" );

      Memory_Free( nonSquareMatrixA);
      Memory_Free( nonSquareMatrixB);
      Memory_Free( nonSquareMatrixAT);
      Memory_Free( nonSquareMatrixResult);

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestMultiplicationByNonSquareMatrix( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double  **nonSquareMatrixA, **nonSquareMatrixB;
      double  **nonSquareMatrixResult, **nonSquareMatrixAT;
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"MultiplicationByNonSquareMatrix" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testMultiplicationByNonSquareMatrix.dat" );

      Journal_Printf( stream, "*******************    Test 19   ************************\n");
      Journal_Printf( stream, "Test function NonSquareMatrix_MultiplicationByNonSquareMatrix \n\n");
      
      nonSquareMatrixA = Memory_Alloc_2DArray( double, 3, 4, (Name)"NonSquareMatrixA" );
      nonSquareMatrixB = Memory_Alloc_2DArray( double, 4, 2, (Name)"NonSquareMatrixB" );
      nonSquareMatrixAT = Memory_Alloc_2DArray( double, 4, 3, (Name)"NonSquareMatrixAT" );
      nonSquareMatrixResult = Memory_Alloc_2DArray( double, 3, 2, (Name)"NonSquareMatrixResult" );
      /* Matrix A */
      nonSquareMatrixA[0][0] = 0; nonSquareMatrixA[1][0] =  4;
      nonSquareMatrixA[0][1] = 1; nonSquareMatrixA[1][1] =  5;
      nonSquareMatrixA[0][2] = 2; nonSquareMatrixA[1][2] =  6;      
      nonSquareMatrixA[0][3] = 3; nonSquareMatrixA[1][3] =  7;
      
      nonSquareMatrixA[2][0] = 8; nonSquareMatrixA[2][1] = 9;
      nonSquareMatrixA[2][2] = 10; nonSquareMatrixA[2][3] = 11;
      /* Matrix B */
      nonSquareMatrixB[0][0] = 0; nonSquareMatrixB[1][0] =  2;
      nonSquareMatrixB[0][1] = 1; nonSquareMatrixB[1][1] =  3;
      
      nonSquareMatrixB[2][0] = 4; nonSquareMatrixB[3][0] =  6;
      nonSquareMatrixB[2][1] = 5; nonSquareMatrixB[3][1] =  7;

      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixA, 3, 4);
      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixB, 4, 2);

      NonSquareMatrix_MultiplicationByNonSquareMatrix( nonSquareMatrixA, 3, 4, nonSquareMatrixB, 4,2, nonSquareMatrixResult );

      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixResult, 3, 2);

      pcu_filename_expected( "testTensorMultMultiplicationByNonSquareMatrix.expected", expected_file );
      pcu_check_fileEq( "testMultiplicationByNonSquareMatrix.dat", expected_file );
      remove( "testMultiplicationByNonSquareMatrix.dat" );

      Memory_Free( nonSquareMatrixA);
      Memory_Free( nonSquareMatrixB);
      Memory_Free( nonSquareMatrixAT);
      Memory_Free( nonSquareMatrixResult);

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite_TestMatrixVectorMultiplication( TensorMultMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      XYZ             vector;
      Bool            error_flag;
      double          **nonSquareMatrixA, **nonSquareMatrixAT;
      double          solutionVector[4], solutionVectorCompare[4];
      Dimension_Index row;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"MatrixVectorMultiplication" );
      char            expected_file[PCU_PATH_MAX];

      #define STG_TENSORMULTMATHTEST_ERROR 1.0e-14;
      Stream_RedirectFile( stream, "testMatrixVectorMultiplication.dat" );

      Journal_Printf( stream, "*******************    Test 20   ************************\n");
      Journal_Printf( stream, "Test function NonSquareMatrix_MatrixVectorMultiplication \n\n");
      Journal_Printf( stream, "Tested against solutions at http://www.uni-bonn.de/~manfear/solve_lineq.php\n\n");

      nonSquareMatrixA = Memory_Alloc_2DArray( double, 3, 4, (Name)"NonSquareMatrixA" );
      nonSquareMatrixAT = Memory_Alloc_2DArray( double, 4, 3, (Name)"NonSquareMatrixAT" );
      /* Matrix A */
      nonSquareMatrixA[0][0] = 0; nonSquareMatrixA[1][0] =  4;
      nonSquareMatrixA[0][1] = 1; nonSquareMatrixA[1][1] =  5;
      nonSquareMatrixA[0][2] = 2; nonSquareMatrixA[1][2] =  6;      
      nonSquareMatrixA[0][3] = 3; nonSquareMatrixA[1][3] =  7;
      
      nonSquareMatrixA[2][0] = 8; nonSquareMatrixA[2][1] = 9;
      nonSquareMatrixA[2][2] = 10; nonSquareMatrixA[2][3] = 11;

      NonSquareMatrix_Transpose( nonSquareMatrixA, 3, 4, nonSquareMatrixAT);
      
      vector[0] = 1; vector[1] = 2; vector[2] = 3;

      solutionVectorCompare[0] = 32; solutionVectorCompare[1] = 38;
      solutionVectorCompare[2] = 44; solutionVectorCompare[3] = 50;

      Journal_PrintNonSquareMatrix( stream, nonSquareMatrixAT, 4, 3);
      StGermain_PrintNamedVector( stream, vector, 3);
      
      NonSquareMatrix_MatrixVectorMultiplication( nonSquareMatrixAT, 4, 3, vector, 3, solutionVector );
      error_flag = False;

      for ( row = 0; row < 4; row++ ) {
         if (fabs( solutionVector[row] - solutionVectorCompare[row]) > TENSORMULTMATH_TEST_ERROR ) {
            error_flag = True;
            Journal_Printf( stream, "solutionVector[%d]=%f not within tolerance of solution, %f\n", row, solutionVector[row], solutionVectorCompare[row]);
         }
      }
      if (error_flag == False) {
         Journal_Printf( stream, "SolutionVector within tolerance %g of solution:\n", TENSORMULTMATH_TEST_ERROR);
         StGermain_PrintNamedVector( stream, solutionVectorCompare, 4);
      }
      else {
         Journal_Printf( stream, "SolutionVector not within tolerance %g of solution:\n", TENSORMULTMATH_TEST_ERROR);
         StGermain_PrintNamedVector( stream, solutionVectorCompare, 4);
      }

      pcu_filename_expected( "testTensorMultMatrixVectorMultiplication.expected", expected_file );
      pcu_check_fileEq( "testMatrixVectorMultiplication.dat", expected_file );
      remove( "testMatrixVectorMultiplication.dat" );

      Memory_Free( nonSquareMatrixA);
      Memory_Free( nonSquareMatrixAT);

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMultMathSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, TensorMultMathSuiteData );
   pcu_suite_setFixtures( suite, TensorMultMathSuite_Setup, TensorMultMathSuite_Teardown );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayIdentity );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestSymmetricTensorIdentity );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayTranspose );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayAdd );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArraySubstract );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyByTensorArray );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyByRightTranspose );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyByLeftTranspose );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyBySymmetricTensor );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyByRightVector );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayMultiplyByLeftVector );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestCalcDeterminant );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestCalcInverseWithDeterminant );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestCalcInverse );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestTensorArrayDoubleContraction );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestSymmetricTensorDoubleContraction );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestPrintNonSquareMatrix );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestNonSquareMatrixTranspose );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestMultiplicationByNonSquareMatrix );
   pcu_suite_addTest( suite, TensorMultMathSuite_TestMatrixVectorMultiplication );
}
