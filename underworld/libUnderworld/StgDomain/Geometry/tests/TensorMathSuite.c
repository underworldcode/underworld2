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
#include <StGermain/libStGermain/src/StGermain.h> 
#include "StgDomain/Geometry/src/Geometry.h"
#include "StgDomain/Shape/src/Shape.h"
#include "StgDomain/Mesh/src/Mesh.h" 
#include "StgDomain/Utils/src/Utils.h"
#include "StgDomain/Swarm/src/Swarm.h"

#include "TensorMathSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} TensorMathSuiteData;

void TensorMathSuite_testCubic( double a3, double a2, double a1, double a0, Stream* stream ) { 
   double rootList[3];
   Index  root_I;

   Journal_Printf( stream, "\nTrying to solve %.4f x^3 + %.4f x^2 + %.4f x + %.4f = 0\n", a3, a2, a1, a0 );

   CubicSolver_OnlyRealRoots( a2/a3, a1/a3, a0/a3, rootList );

   for ( root_I = 0; root_I < 3; root_I++ ) { 
      double root =  rootList[ root_I ];
      Journal_Printf( stream, "Found root %.4f: ", root );

      if (  fabs( a3*root*root*root + a2*root*root + a1 * root + a0 ) > 1.0e-5 )
         Journal_Printf( stream, "Failed root test - %.4f x^3 + %.4f x^2 + %.4f x + %.4f = %.4f\n", a3, a2, a1, a0, a3*root*root*root + a2*root*root + a1 * root + a0 );
      else 
         Journal_Printf( stream, "Passed.\n" );
   }   
}

void TensorMathSuite_Setup( TensorMathSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void TensorMathSuite_Teardown( TensorMathSuiteData* data ) {
}

void TensorMathSuite_TestSymmetricTensorVectorComponents( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      int     dim;
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"SymmetricTensorVectorComponents" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathSymmetricTensorVectorComponents.dat" );

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Check StGermain_nSymmetricTensorVectorComponents function\n");
      dim = 2;
      Journal_Printf( stream, "Number of unique components of symmetric tensor of dimension %d is %d\n", dim, StGermain_nSymmetricTensorVectorComponents( dim ) );
      dim = 3;
      Journal_Printf( stream, "Number of unique components of symmetric tensor of dimension %d is %d\n", dim, StGermain_nSymmetricTensorVectorComponents( dim ) );

      pcu_filename_expected( "testTensorMathSymmetricTensorVectorComponents.expected", expected_file );
      pcu_check_fileEq( "testTensorMathSymmetricTensorVectorComponents.dat", expected_file );
      remove( "testTensorMathSymmetricTensorVectorComponents.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestJournalPrintTensorArray( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double      **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      TensorArray tensorArray;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"JournalPrintTensorArray" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathJournalPrintTensorArray.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12; 

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing Journal_PrintTensorArray\n");
      TensorArray_SetFromDoubleArray( tensorArray, tensor, 3 );
      Journal_PrintTensorArray( stream, tensorArray, 3 );

      pcu_filename_expected( "testTensorMathJournalPrintTensorArray.expected", expected_file );
      pcu_check_fileEq( "testTensorMathJournalPrintTensorArray.dat", expected_file );
      remove( "testTensorMathJournalPrintTensorArray.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestTensorToTensorArrayFunction( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      TensorArray     tensorArray;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"TensorToTensorArrayFunction" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathArrayFunction.dat" );

      Journal_Printf( stream, "/*******************************************/\n");
      Journal_Printf( stream, "Test Symmetric Tensor to Tensor Array function\n\n");

      Journal_Printf( stream, "2-D\n");
      Journal_Printf( stream, "Symmetric Tensor\n");
      symmTensor[ST2D_00] = 1; symmTensor[ST2D_11] = 8;
      symmTensor[ST2D_01] = 7;
      Journal_PrintSymmetricTensor_Unnamed(stream, symmTensor, 2); 
    
      StGermain_SymmetricTensor_ToTensorArray2D(symmTensor, tensorArray);
      Journal_Printf( stream, "Tensor Array - 2D\n");
      Journal_PrintTensorArray_Unnamed(stream, tensorArray, 2); 
    
      Journal_Printf( stream, "Tensor Array \n");
      StGermain_SymmetricTensor_ToTensorArray(symmTensor, 2, tensorArray);
      Journal_PrintTensorArray_Unnamed(stream, tensorArray, 2); 
    
      Journal_Printf( stream, "3-D\n");
      Journal_Printf( stream, "Symmetric Tensor\n");
      symmTensor[ST3D_00] = 1; symmTensor[ST3D_11] = 8; symmTensor[ST3D_22] = 5;
      symmTensor[ST3D_01] = 7; symmTensor[ST3D_02] = 0; symmTensor[ST3D_12] = 3;
    
      Journal_PrintSymmetricTensor_Unnamed(stream, symmTensor, 3); 
    
      StGermain_SymmetricTensor_ToTensorArray3D(symmTensor, tensorArray);
      Journal_Printf( stream, "Tensor Array - 3D\n");
      Journal_PrintTensorArray_Unnamed(stream, tensorArray, 3);

      Journal_Printf( stream, "Tensor Array \n");
      StGermain_SymmetricTensor_ToTensorArray(symmTensor, 3, tensorArray);
      Journal_PrintTensorArray_Unnamed(stream, tensorArray, 3);

      pcu_filename_expected( "testTensorMathArrayFunction.expected", expected_file );
      pcu_check_fileEq( "testTensorMathArrayFunction.dat", expected_file );
      remove( "testTensorMathArrayFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestMathArrayToMatrix( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double      **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      TensorArray tensorArray;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"TensorArrayToMatrix" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathArrayToMatrix.dat" );

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing TensorArray_ToMatrix\n");

      Journal_Printf( stream, "2-D\n");
      tensorArray[FT2D_00] = 1;  tensorArray[FT2D_01] = 3;
      tensorArray[FT2D_10] = 2;  tensorArray[FT2D_11] = 6.8;

      Journal_PrintTensorArray( stream, tensorArray,2);
      TensorArray_ToMatrix(tensorArray, 2, tensor );
      Journal_PrintSquareArray( stream, tensor, 2);

      Journal_Printf( stream, "3-D\n");
      tensorArray[FT3D_00] = 30;  tensorArray[FT3D_01] = 27;  tensorArray[FT3D_02] = -24;
      tensorArray[FT3D_10] = 29;  tensorArray[FT3D_11] = 26.8; tensorArray[FT3D_12] = 23;
      tensorArray[FT3D_20] = 28;  tensorArray[FT3D_21] = 25;   tensorArray[FT3D_22] = -0.22;

      Journal_PrintTensorArray( stream, tensorArray,3);
      TensorArray_ToMatrix(tensorArray, 3, tensor );
      Journal_PrintSquareArray( stream, tensor, 3);

      pcu_filename_expected( "testTensorMathArrayToMatrix.expected", expected_file );
      pcu_check_fileEq( "testTensorMathArrayToMatrix.dat", expected_file );
      remove( "testTensorMathArrayToMatrix.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestSymmetricTensorToMatrix( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double          **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      SymmetricTensor symmTensor;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"SymmetricTensorToMatrix" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathSymmetricTensorToMatrix.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12;

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing SymmetricTensor_ToMatrix\n");

      Journal_Printf( stream, "2-D\n");
      symmTensor[ST2D_00] = 7;  symmTensor[ST2D_01] = 11;
      symmTensor[ST2D_11] = 9.8;

      Journal_PrintSymmetricTensor( stream, symmTensor,2);
      SymmetricTensor_ToMatrix(symmTensor, 2, tensor );
      Journal_PrintSquareArray( stream, tensor, 2);

      Journal_Printf( stream, "3-D\n");
      symmTensor[ST3D_00] = 0;   symmTensor[ST3D_01] = 12;  symmTensor[ST3D_02] = -100;
      symmTensor[ST3D_11] = 0.8; symmTensor[ST3D_12] = 20.3;
      symmTensor[ST3D_22] = -7.5;

      Journal_PrintSymmetricTensor( stream, symmTensor,3);
      SymmetricTensor_ToMatrix(symmTensor, 3, tensor );
      Journal_PrintSquareArray( stream, tensor, 3);
      
      pcu_filename_expected( "testTensorMathSymmetricTensorToMatrix.expected", expected_file );
      pcu_check_fileEq( "testTensorMathSymmetricTensorToMatrix.dat", expected_file );
      remove( "testTensorMathSymmetricTensorToMatrix.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestGetAntisymmetricPart( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double      **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      TensorArray tensorArray;
      TensorArray tensor2;
      int         dim;
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"GetAntisymmetricPart" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathGetAntisymmetricPart.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12;

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing GetAntisymmetricPart\n");
      dim = 2;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      TensorArray_GetAntisymmetricPart( tensorArray, dim, tensor2 );
      Journal_PrintTensorArray( stream, tensor2, dim );

      dim = 3;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      TensorArray_GetAntisymmetricPart( tensorArray, dim, tensor2 );
      Journal_PrintTensorArray( stream, tensor2, dim );
         
      pcu_filename_expected( "testTensorMathGetAntisymmetricPart.expected", expected_file );
      pcu_check_fileEq( "testTensorMathGetAntisymmetricPart.dat", expected_file );
      remove( "testTensorMathGetAntisymmetricPart.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestGetSymmetricPart( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double          **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      SymmetricTensor symmTensor;
      TensorArray     tensorArray;
      int             dim;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"GetSymmetricPart" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathGetSymmetricPart.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12;

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing GetSymmetricPart\n");
      dim = 2;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      TensorArray_GetSymmetricPart( tensorArray, dim, symmTensor );
      Journal_PrintSymmetricTensor( stream, symmTensor, dim );

      dim = 3;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      TensorArray_GetSymmetricPart( tensorArray, dim, symmTensor );
      Journal_PrintSymmetricTensor( stream, symmTensor, dim );
         
      pcu_filename_expected( "testTensorMathGetSymmetricPart.expected", expected_file );
      pcu_check_fileEq( "testTensorMathGetSymmetricPart.dat", expected_file );
      remove( "testTensorMathGetSymmetricPart.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_Test2ndInvariant( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double          **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      SymmetricTensor symmTensor;
      TensorArray     tensorArray;
      int             dim;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"2ndInvariant" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMath2ndInvariant.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12;

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing 2ndInvariant\n");
      dim = 2;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      Journal_Printf( stream, "inv = %0.5g\n", TensorArray_2ndInvariant( tensorArray, dim ));
      TensorArray_GetSymmetricPart( tensorArray, dim, symmTensor );
      Journal_Printf( stream, "inv = %0.5g\n", SymmetricTensor_2ndInvariant( symmTensor, dim ));

      dim = 3;
      Journal_Printf( stream, "dim = %d\n", dim);
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      Journal_Printf( stream, "inv = %0.5g\n", TensorArray_2ndInvariant( tensorArray, dim ));
      TensorArray_GetSymmetricPart( tensorArray, dim, symmTensor );
      Journal_Printf( stream, "inv = %0.5g\n", SymmetricTensor_2ndInvariant( symmTensor, dim ));
         
      pcu_filename_expected( "testTensorMath2ndInvariant.expected", expected_file );
      pcu_check_fileEq( "testTensorMath2ndInvariant.dat", expected_file );
      remove( "testTensorMath2ndInvariant.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestVectorTensorVector( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double      **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      TensorArray tensorArray;
      int         dim;
      double      a[] = {2,3,6,-2,9.1};
      double      b[] = {1,-3,2.6,-2.2,-1.91};
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"VectorTensorVector" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathVectorTensorVector.dat" );

      tensor[0][0] = 3;  tensor[0][1] = 21;  tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
      tensor[1][0] = 6;  tensor[1][1] = 5.8; tensor[1][2] = 32; tensor[1][3] = 3;   tensor[1][4] = -2.5;
      tensor[2][0] = 2;  tensor[2][1] = 2;   tensor[2][2] = -7; tensor[2][3] = 2;   tensor[2][4] = 3.1;
      tensor[3][0] = -4; tensor[3][1] = 9;   tensor[3][2] = 3;  tensor[3][3] = 8;   tensor[3][4] = 6;
      tensor[4][0] = 3;  tensor[4][1] = 1;   tensor[4][2] = 9;  tensor[4][3] = 2;   tensor[4][4] = 12;

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing StGermain_VectorTensorVector\n");
      dim = 2;
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      Journal_Printf( stream, "dim = %d - a_i u_ij b_j = %2.4lf\n", dim, TensorArray_MultiplyByVectors( tensorArray, a, b,dim));
      dim = 3;
      TensorArray_SetFromDoubleArray( tensorArray, tensor, dim );
      Journal_Printf( stream, "dim = %d - a_i u_ij b_j = %2.4lf\n", dim, TensorArray_MultiplyByVectors( tensorArray, a, b,dim));
         
      pcu_filename_expected( "testTensorMathVectorTensorVector.expected", expected_file );
      pcu_check_fileEq( "testTensorMathVectorTensorVector.dat", expected_file );
      remove( "testTensorMathVectorTensorVector.dat" );

      Memory_Free( tensor );
      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestZeroTensor( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      TensorArray     tensor2;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"ZeroTensor" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathZeroTensor.dat" );

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing ZeroTensor\n");
      TensorArray_Zero( tensor2 );
      Journal_PrintTensorArray( stream, tensor2, 2 );
      Journal_PrintTensorArray( stream, tensor2, 3 );
      SymmetricTensor_Zero( symmTensor );
      Journal_PrintSymmetricTensor( stream, symmTensor, 2 );
      Journal_PrintSymmetricTensor( stream, symmTensor, 3 );
      
      pcu_filename_expected( "testTensorMathZeroTensor.expected", expected_file );
      pcu_check_fileEq( "testTensorMathZeroTensor.dat", expected_file );
      remove( "testTensorMathZeroTensor.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestMatrixDeterminant( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      double  **tensor = Memory_Alloc_2DArray( double , 5, 5, (Name)"Tensor" );
      double  determinant;
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"MatrixDeterminant" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathMatrixDeterminant.dat" );

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing StGermain_MatrixDeterminant\n");
      tensor[0][0] = 1.0; tensor[0][1] = 2.0; tensor[0][2] = 3.0;
      tensor[1][0] = 4.0; tensor[1][1] = 5.0; tensor[1][2] = 6.0;
      tensor[2][0] = 7.0; tensor[2][1] = 8.0; tensor[2][2] = 11.0;

      determinant = StGermain_MatrixDeterminant( tensor, 1 );
      Journal_Printf( stream, "%2.4g is determinant of matrix : " , determinant );
      Journal_PrintSquareArray( stream, tensor, 1 );

      determinant = StGermain_MatrixDeterminant( tensor, 2 );
      Journal_Printf( stream, "%2.4g is determinant of matrix : " , determinant );
      Journal_PrintSquareArray( stream, tensor, 2 );

      determinant = StGermain_MatrixDeterminant( tensor, 3 );
      Journal_Printf( stream, "%2.4g is determinant of matrix : " , determinant );
      Journal_PrintSquareArray( stream, tensor, 3 );

      determinant = StGermain_MatrixDeterminant( tensor, 3 );
      Journal_Printf( stream, "%2.4g is determinant of matrix : " , determinant );
      Journal_PrintSquareArray( stream, tensor, 3 );

      determinant = StGermain_MatrixDeterminant_AxisIndependent( tensor, 2, I_AXIS, K_AXIS, J_AXIS );
      Journal_Printf( stream, "%2.4g is determinant of matrix :\n" , determinant );
      Journal_Printf( stream, "%2.4g \t %2.4g\n", tensor[I_AXIS][I_AXIS], tensor[I_AXIS][K_AXIS] );
      Journal_Printf( stream, "%2.4g \t %2.4g\n", tensor[K_AXIS][I_AXIS], tensor[K_AXIS][K_AXIS] );
      
      pcu_filename_expected( "testTensorMathMatrixDeterminant.expected", expected_file );
      pcu_check_fileEq( "testTensorMathMatrixDeterminant.dat", expected_file );
      remove( "testTensorMathMatrixDeterminant.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestCubicSolver( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      Stream* stream = Journal_Register( InfoStream_Type, (Name)"CubicSolver" );
      char    expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathCubicSolver.dat" );

      Journal_Printf( stream, "****************************************\n");
      Journal_Printf( stream, "Testing Cubic Solver\n");
      TensorMathSuite_testCubic( 2.0, 0.0, -2.0,0.0, stream );
      TensorMathSuite_testCubic( 1.0, -4.0, 5.0, -2.0, stream );
      TensorMathSuite_testCubic( 1.0, 4.0, -11.0, -30.0, stream );
      
      pcu_filename_expected( "testTensorMathCubicSolver.expected", expected_file );
      pcu_check_fileEq( "testTensorMathCubicSolver.dat", expected_file );
      remove( "testTensorMathCubicSolver.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestMappingFunctions( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      TensorArray     tensorArray;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"MappingFunctions" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathMappingFunctions.dat" );

      Journal_Printf(stream, "****************************************\n");
      Journal_Printf( stream, "Test Mapping functions\n\n");
      Journal_Printf( stream, "2-D\n");

      tensorArray[0] = 0.5; tensorArray[1] = 10; tensorArray[2] = 20; tensorArray[3] = 30;
      symmTensor[0] = 40; symmTensor[1] = 50; symmTensor[2] = 60;

      Journal_Printf( stream, "The full tensor:\n");
      Journal_PrintTensorArray( stream, tensorArray, 2 );

      Journal_Printf( stream, "The tensor Mapping using enumerated types:\n");
      Journal_PrintValue( stream, tensorArray[FT2D_00]);
      Journal_PrintValue( stream, tensorArray[FT2D_01]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, tensorArray[FT2D_10]);
      Journal_PrintValue( stream, tensorArray[FT2D_11]);

      Journal_Printf( stream, "The symmetric tensor:\n");
      Journal_PrintSymmetricTensor( stream, symmTensor, 2 );

      Journal_Printf( stream, "The symmetric tensor Mapping using enumerated types:\n");
      Journal_PrintValue( stream, symmTensor[ST2D_00]);
      Journal_PrintValue( stream, symmTensor[ST2D_01]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, symmTensor[ST2D_11]);

      Journal_Printf( stream, "Test Mapping functions for Full Tensor ");
      Journal_Printf( stream, "using TensorArray_TensorMap\n\n");
      Journal_Printf( stream, "(0,0): %d  = %d  ", FT2D_00, TensorArray_TensorMap(0,0,2));
      Journal_Printf( stream, "(0,1): %d  = %d\n", FT2D_01, TensorArray_TensorMap(0,1,2));
      Journal_Printf( stream, "(1,0): %d  = %d  ", FT2D_10, TensorArray_TensorMap(1,0,2));
      Journal_Printf( stream, "(1,1): %d  = %d\n", FT2D_11, TensorArray_TensorMap(1,1,2));

      Journal_Printf( stream, "Test Mapping functions for Symmetric Tensor ");
      Journal_Printf( stream, "using SymmetricTensor_TensorMap\n\n");
      Journal_Printf( stream, "(0,0): %d  = %d  ", ST2D_00, SymmetricTensor_TensorMap(0,0,2));
      Journal_Printf( stream, "(0,1): %d  = %d\n", ST2D_01, SymmetricTensor_TensorMap(0,1,2));
      Journal_Printf( stream, "(1,0): %d  = %d  ", ST2D_01, SymmetricTensor_TensorMap(1,0,2));
      Journal_Printf( stream, "(1,1): %d  = %d\n", ST2D_11, SymmetricTensor_TensorMap(1,1,2));

      Journal_Printf( stream, "3-D\n");
      tensorArray[0] = 0; tensorArray[1] = 1; tensorArray[2] = 2;
      tensorArray[3] = 3; tensorArray[4] = 4; tensorArray[5] = 5;
      tensorArray[6] = 6; tensorArray[7] = 7; tensorArray[8] = 8;

      symmTensor[0] = 90; symmTensor[1] = 100; symmTensor[2] = 110;
      symmTensor[3] = 120; symmTensor[4] = 130; symmTensor[5] = 140;

      Journal_Printf( stream, "The full tensor:\n");
      Journal_PrintTensorArray( stream, tensorArray, 3 );

      Journal_Printf( stream, "The tensor Mapping using enumerated types:\n");
      Journal_PrintValue( stream, tensorArray[FT3D_00]);
      Journal_PrintValue( stream, tensorArray[FT3D_01]);
      Journal_PrintValue( stream, tensorArray[FT3D_02]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, tensorArray[FT3D_10]);
      Journal_PrintValue( stream, tensorArray[FT3D_11]);
      Journal_PrintValue( stream, tensorArray[FT3D_12]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, tensorArray[FT3D_20]);
      Journal_PrintValue( stream, tensorArray[FT3D_21]);
      Journal_PrintValue( stream, tensorArray[FT3D_22]);

      Journal_Printf( stream, "The symmetric tensor:\n");
      Journal_PrintSymmetricTensor( stream, symmTensor, 3 );

      Journal_Printf( stream, "The symmetric tensor Mapping using enumerated types:\n");
      Journal_PrintValue( stream, symmTensor[ST3D_00]);
      Journal_PrintValue( stream, symmTensor[ST3D_01]);
      Journal_PrintValue( stream, symmTensor[ST3D_02]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, symmTensor[ST3D_11]);
      Journal_PrintValue( stream, symmTensor[ST3D_12]);
      Journal_Printf( stream, " ---\n");
      Journal_PrintValue( stream, symmTensor[ST3D_22]);

      Journal_Printf( stream, "Test Mapping functions for Full Tensor ");
      Journal_Printf( stream, "using TensorArray_TensorMap\n\n");
      Journal_Printf( stream, "(0,0): %d  = %d  ", FT3D_00, TensorArray_TensorMap(0,0,3));
      Journal_Printf( stream, "(0,1): %d  = %d  ", FT3D_01, TensorArray_TensorMap(0,1,3));
      Journal_Printf( stream, "(0,2): %d  = %d\n", FT3D_02, TensorArray_TensorMap(0,2,3));

      Journal_Printf( stream, "(1,0): %d  = %d  ", FT3D_10, TensorArray_TensorMap(1,0,3));
      Journal_Printf( stream, "(1,1): %d  = %d  ", FT3D_11, TensorArray_TensorMap(1,1,3));
      Journal_Printf( stream, "(1,2): %d  = %d\n", FT3D_12, TensorArray_TensorMap(1,2,3));

      Journal_Printf( stream, "(2,0): %d  = %d  ", FT3D_20, TensorArray_TensorMap(2,0,3));
      Journal_Printf( stream, "(2,1): %d  = %d  ", FT3D_21, TensorArray_TensorMap(2,1,3));
      Journal_Printf( stream, "(2,2): %d  = %d\n", FT3D_22, TensorArray_TensorMap(2,2,3));

      Journal_Printf( stream, "Test Mapping functions for Symmetric Tensor ");
      Journal_Printf( stream, "using SymmetricTensor_TensorMap\n\n");
      Journal_Printf( stream, "(0,0): %d  = %d  ", ST3D_00, SymmetricTensor_TensorMap(0,0,3));
      Journal_Printf( stream, "(0,1): %d  = %d  ", ST3D_01, SymmetricTensor_TensorMap(0,1,3));
      Journal_Printf( stream, "(0,2): %d  = %d\n", ST3D_02, SymmetricTensor_TensorMap(0,2,3));

      Journal_Printf( stream, "(1,0): %d  = %d  ", ST3D_01, SymmetricTensor_TensorMap(1,0,3));
      Journal_Printf( stream, "(1,1): %d  = %d  ", ST3D_11, SymmetricTensor_TensorMap(1,1,3));
      Journal_Printf( stream, "(1,2): %d  = %d\n", ST3D_12, SymmetricTensor_TensorMap(1,2,3));

      Journal_Printf( stream, "(2,0): %d  = %d  ", ST3D_02, SymmetricTensor_TensorMap(2,0,3));
      Journal_Printf( stream, "(2,1): %d  = %d  ", ST3D_12, SymmetricTensor_TensorMap(2,1,3));
      Journal_Printf( stream, "(2,2): %d  = %d\n", ST3D_22, SymmetricTensor_TensorMap(2,2,3));

      pcu_filename_expected( "testTensorMathMappingFunctions.expected", expected_file );
      pcu_check_fileEq( "testTensorMathMappingFunctions.dat", expected_file );
      remove( "testTensorMathMappingFunctions.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestEigenVectorListSort( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      Eigenvector eigenvectorList[3];
      Stream*     stream = Journal_Register( InfoStream_Type, (Name)"EigenVectorListSort" );
      char        expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathEigenVectorListSort.dat" );

      Journal_Printf( stream, "*************************************\n" );
      Journal_Printf( stream, "Test function EigenvectorList_Sort\n" );
      /*  Matrix that gives eigenvalues and vectors  */
      /* | 1.000 1.000 2.000 | */
      /* | 1.000 2.000 1.000 | */
      /* | 2.000 1.000 1.000 | */
      Journal_Printf( stream, " 3-D\n");
      eigenvectorList[0].eigenvalue = 4; eigenvectorList[1].eigenvalue = -1; eigenvectorList[2].eigenvalue = 1;
      eigenvectorList[0].vector[0] = 0.577; eigenvectorList[0].vector[1] = 0.577; eigenvectorList[0].vector[2] = 0.577;
      eigenvectorList[1].vector[0] = -0.707; eigenvectorList[1].vector[1] = 0; eigenvectorList[1].vector[2] = 0.707;
      eigenvectorList[2].vector[0] = 0.408; eigenvectorList[2].vector[1] = -0.816; eigenvectorList[2].vector[2] = 0.408;

      Journal_Printf( stream, "Unsorted eigenvectorList\n");
      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[2].eigenvalue );

      Journal_PrintArray( stream, eigenvectorList[0].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[2].vector, 3 );

      EigenvectorList_Sort( eigenvectorList, 3);

      Journal_Printf( stream, "Sorted eigenvectorList\n");

      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[2].eigenvalue );

      Journal_PrintArray( stream, eigenvectorList[0].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[2].vector, 3 );

      Journal_Printf( stream, "2-D\n");

      /*  Matrix that gives eigenvalues and vectors  */
      /* | 5.000 1.000 | */
      /* | 1.000 5.000 | */

      eigenvectorList[0].eigenvalue = 6; eigenvectorList[1].eigenvalue = 4;
      eigenvectorList[0].vector[0] = 0.707; eigenvectorList[0].vector[1] = 0.707;
      eigenvectorList[1].vector[0] = 0.707; eigenvectorList[1].vector[1] = -0.707;

      Journal_Printf( stream, "Unsorted eigenvectorList\n");
      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );

      Journal_PrintArray( stream, eigenvectorList[0].vector, 2 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 2 );

      EigenvectorList_Sort( eigenvectorList, 2);

      Journal_Printf( stream, "Sorted eigenvectorList\n");
      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );

      Journal_PrintArray( stream, eigenvectorList[0].vector, 2 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 2 );
         
      pcu_filename_expected( "testTensorMathEigenVectorListSort.expected", expected_file );
      pcu_check_fileEq( "testTensorMathEigenVectorListSort.dat", expected_file );
      remove( "testTensorMathEigenVectorListSort.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestEigenVector1( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      Eigenvector     eigenvectorList[3];
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"EigenVector1" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathEigenVector1.dat" );

      eigenvectorList[0].eigenvalue = 4; eigenvectorList[1].eigenvalue = -1; eigenvectorList[2].eigenvalue = 1;
      eigenvectorList[0].vector[0] = 0.577; eigenvectorList[0].vector[1] = 0.577; eigenvectorList[0].vector[2] = 0.577;
      eigenvectorList[1].vector[0] = -0.707; eigenvectorList[1].vector[1] = 0; eigenvectorList[1].vector[2] = 0.707;
      eigenvectorList[2].vector[0] = 0.408; eigenvectorList[2].vector[1] = -0.816; eigenvectorList[2].vector[2] = 0.408;

      Journal_Printf(stream, "****************    Test Eigenvector 1   *********************\n");
      Journal_Printf( stream, "2D Case from Kresig, p. 371f\n\n");
      symmTensor[0] = -5;
      symmTensor[1] = -2;
      symmTensor[2] = 2;

      Journal_Printf( stream, "Matrix to solve for eigenvectors is:\n");
      Journal_PrintSymmetricTensor( stream, symmTensor, 2 );

      SymmetricTensor_CalcAllEigenvectors( symmTensor, 2, eigenvectorList );

      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintArray( stream, eigenvectorList[0].vector, 2 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 2 );

      pcu_filename_expected( "testTensorMathEigenVector1.expected", expected_file );
      pcu_check_fileEq( "testTensorMathEigenVector1.dat", expected_file );
      remove( "testTensorMathEigenVector1.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestEigenVector2( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      Eigenvector     eigenvectorList[3];
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"EigenVector2" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathEigenVector2.dat" );

      eigenvectorList[0].eigenvalue = 4; eigenvectorList[1].eigenvalue = -1; eigenvectorList[2].eigenvalue = 1;
      eigenvectorList[0].vector[0] = 0.577; eigenvectorList[0].vector[1] = 0.577; eigenvectorList[0].vector[2] = 0.577;
      eigenvectorList[1].vector[0] = -0.707; eigenvectorList[1].vector[1] = 0; eigenvectorList[1].vector[2] = 0.707;
      eigenvectorList[2].vector[0] = 0.408; eigenvectorList[2].vector[1] = -0.816; eigenvectorList[2].vector[2] = 0.408;

      Journal_Printf( stream, "****************    Test Eigenvector 2   **********************\n");
      Journal_Printf( stream, "Same test as above - but using Numerical Recipies function\n\n");
      symmTensor[0] = -5;
      symmTensor[1] = -2;
      symmTensor[2] = 2;

      SymmetricTensor_CalcAllEigenvectorsJacobi( symmTensor, 2, eigenvectorList );

      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintArray( stream, eigenvectorList[0].vector, 2 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 2 );

      pcu_filename_expected( "testTensorMathEigenVector2.expected", expected_file );
      pcu_check_fileEq( "testTensorMathEigenVector2.dat", expected_file );
      remove( "testTensorMathEigenVector2.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestEigenVector3( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      Eigenvector     eigenvectorList[3];
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"EigenVector3" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathEigenVector3.dat" );

      eigenvectorList[0].eigenvalue = 4; eigenvectorList[1].eigenvalue = -1; eigenvectorList[2].eigenvalue = 1;
      eigenvectorList[0].vector[0] = 0.577; eigenvectorList[0].vector[1] = 0.577; eigenvectorList[0].vector[2] = 0.577;
      eigenvectorList[1].vector[0] = -0.707; eigenvectorList[1].vector[1] = 0; eigenvectorList[1].vector[2] = 0.707;
      eigenvectorList[2].vector[0] = 0.408; eigenvectorList[2].vector[1] = -0.816; eigenvectorList[2].vector[2] = 0.408;

      Journal_Printf( stream, "****************    Test Eigenvector 3   *********************\n");
      Journal_Printf( stream, "3D Case -tested on 3/11/04, against: \n");
      Journal_Printf( stream, "http://www.arndt-bruenner.de/mathe/scripts/engl_eigenwert.htm\n");
      symmTensor[0] = 2; symmTensor[1] = 3; symmTensor[2] = 5;
      symmTensor[3] = 7; symmTensor[4] = 11; symmTensor[5] = 13;

      Journal_Printf( stream, "Matrix to solve for eigenvectors is:\n");
      Journal_PrintSymmetricTensor( stream, symmTensor, 3 );

      SymmetricTensor_CalcAllEigenvectors( symmTensor, 3, eigenvectorList );

      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[2].eigenvalue );
      Journal_PrintArray( stream, eigenvectorList[0].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[2].vector, 3 );

      pcu_filename_expected( "testTensorMathEigenVector3.expected", expected_file );
      pcu_check_fileEq( "testTensorMathEigenVector3.dat", expected_file );
      remove( "testTensorMathEigenVector3.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite_TestEigenVector4( TensorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;

   if (data->rank == procToWatch) {
      SymmetricTensor symmTensor;
      TensorArray     tensorArray;
      Eigenvector     eigenvectorList[3];
      XYZ             rightHandSide;
      XYZ             solution;
      int             dim;
      Stream*         stream = Journal_Register( InfoStream_Type, (Name)"EigenVector4" );
      char            expected_file[PCU_PATH_MAX];

      Stream_RedirectFile( stream, "testTensorMathEigenVector4.dat" );

      eigenvectorList[0].eigenvalue = 4; eigenvectorList[1].eigenvalue = -1; eigenvectorList[2].eigenvalue = 1;
      eigenvectorList[0].vector[0] = 0.577; eigenvectorList[0].vector[1] = 0.577; eigenvectorList[0].vector[2] = 0.577;
      eigenvectorList[1].vector[0] = -0.707; eigenvectorList[1].vector[1] = 0; eigenvectorList[1].vector[2] = 0.707;
      eigenvectorList[2].vector[0] = 0.408; eigenvectorList[2].vector[1] = -0.816; eigenvectorList[2].vector[2] = 0.408;

      Journal_Printf( stream, "****************    Test Eigenvector 4   *********************\n");
      Journal_Printf( stream, "Same test as above - but using Numerical Recipies function\n\n");
      symmTensor[0] = 2; symmTensor[1] = 3; symmTensor[2] = 5;
      symmTensor[3] = 7; symmTensor[4] = 11; symmTensor[5] = 13;

      SymmetricTensor_CalcAllEigenvectorsJacobi( symmTensor, 3, eigenvectorList );

      Journal_PrintValue( stream, eigenvectorList[0].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[1].eigenvalue );
      Journal_PrintValue( stream, eigenvectorList[2].eigenvalue );
      Journal_PrintArray( stream, eigenvectorList[0].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[1].vector, 3 );
      Journal_PrintArray( stream, eigenvectorList[2].vector, 3 );

      Journal_Printf( stream, "\n");

      /* Test Matrix Solve */
      dim = 2;
      tensorArray[ FT2D_00 ] = 10;
      tensorArray[ FT2D_01 ] = 11;
      tensorArray[ FT2D_10 ] = 12;
      tensorArray[ FT2D_11 ] = 13;
      Journal_PrintTensorArray( stream, tensorArray, dim );

      rightHandSide[0] = 24;
      rightHandSide[1] = 35;
      Journal_PrintArray( stream, rightHandSide, dim );

      TensorArray_SolveSystem( tensorArray, solution, rightHandSide, dim );
      Journal_PrintArray( stream, solution, dim );

      dim = 3;
      tensorArray[ FT3D_00 ] = 10;
      tensorArray[ FT3D_01 ] = 11;
      tensorArray[ FT3D_02 ] = 12;
      tensorArray[ FT3D_10 ] = 13;
      tensorArray[ FT3D_11 ] = 14;
      tensorArray[ FT3D_12 ] = 15;
      tensorArray[ FT3D_20 ] = 16;
      tensorArray[ FT3D_21 ] = 17;
      tensorArray[ FT3D_22 ] = 19;
      Journal_PrintTensorArray( stream, tensorArray, dim );

      rightHandSide[0] = 24;
      rightHandSide[1] = 35;
      rightHandSide[2] = 45;
      Journal_PrintArray( stream, rightHandSide, dim );

      TensorArray_SolveSystem( tensorArray, solution, rightHandSide, dim );
      Journal_PrintArray( stream, solution, dim );

      pcu_filename_expected( "testTensorMathEigenVector4.expected", expected_file );
      pcu_check_fileEq( "testTensorMathEigenVector4.dat", expected_file );
      remove( "testTensorMathEigenVector4.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void TensorMathSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, TensorMathSuiteData );
   pcu_suite_setFixtures( suite, TensorMathSuite_Setup, TensorMathSuite_Teardown );
   pcu_suite_addTest( suite, TensorMathSuite_TestSymmetricTensorVectorComponents );
   pcu_suite_addTest( suite, TensorMathSuite_TestJournalPrintTensorArray );
   pcu_suite_addTest( suite, TensorMathSuite_TestTensorToTensorArrayFunction );
   pcu_suite_addTest( suite, TensorMathSuite_TestMathArrayToMatrix );
   pcu_suite_addTest( suite, TensorMathSuite_TestSymmetricTensorToMatrix );
   pcu_suite_addTest( suite, TensorMathSuite_TestGetAntisymmetricPart );
   pcu_suite_addTest( suite, TensorMathSuite_TestGetSymmetricPart );
   pcu_suite_addTest( suite, TensorMathSuite_Test2ndInvariant );
   pcu_suite_addTest( suite, TensorMathSuite_TestVectorTensorVector );
   pcu_suite_addTest( suite, TensorMathSuite_TestZeroTensor );
   pcu_suite_addTest( suite, TensorMathSuite_TestMatrixDeterminant );
   pcu_suite_addTest( suite, TensorMathSuite_TestCubicSolver );
   pcu_suite_addTest( suite, TensorMathSuite_TestMappingFunctions );
   pcu_suite_addTest( suite, TensorMathSuite_TestEigenVectorListSort );
   pcu_suite_addTest( suite, TensorMathSuite_TestEigenVector1 );
   pcu_suite_addTest( suite, TensorMathSuite_TestEigenVector2 );
   pcu_suite_addTest( suite, TensorMathSuite_TestEigenVector3 );
   pcu_suite_addTest( suite, TensorMathSuite_TestEigenVector4 );
}
