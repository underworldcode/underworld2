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

#include "VectorMathSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} VectorMathSuiteData;

void VectorMathSuite_Setup( VectorMathSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void VectorMathSuite_Teardown( VectorMathSuiteData* data ) {
}

void VectorMathSuite_BasicTest( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"BasicTest" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      Coord a, b, c;
      Coord d = { 1.0, 1.0, 1.0 };
      Coord e = { 1.0, 2.0, -3.0 };

      Stream_RedirectFile( stream, "testBasicTest.dat" );

      Journal_Printf( stream, "Basic tests:\n" );
      Journal_Printf( stream, "d = { %g, %g, %g }\n", d[0], d[1], d[2] );
      Vec_SetScalar3D( d, 2.0, 0.5, 1.5 );
      Journal_Printf( stream, "d = { %g, %g, %g }\n", d[0], d[1], d[2] );

      Vec_Set3D( c, d );
      Journal_Printf( stream, "c = d = { %g, %g, %g }\n", c[0], c[1], c[2] );

      Vec_Add3D( b, c, d );
      Journal_Printf( stream, "b = c + d = {%g, %g, %g}\n", b[0], b[1], b[2] );

      Vec_Sub3D( a, d, b );
      Journal_Printf( stream, "a = d - b = {%g, %g, %g}\n", a[0], a[1], a[2] );

      Vec_Cross3D( d, a, e );
      Journal_Printf( stream, "d = a x e = {%g, %g, %g}\n", d[0], d[1], d[2] );

      Journal_Printf( stream, "a . c = %g\n", Vec_Dot3D( a, c ) );

      Vec_Scale3D( b, b, 2.0 );
      Journal_Printf( stream, "b = 2b = { %g, %g, %g }\n", b[0], b[1], b[2] );

      Vec_Norm3D( b, b );
      Journal_Printf( stream, "b^ = { %g, %g, %g }\n", b[0], b[1], b[2] );

      pcu_filename_expected( "testVectorMathBasicTest.expected", expected_file );
      pcu_check_fileEq( "testBasicTest.dat", expected_file );
      remove( "testBasicTest.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_CompleteTest( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"CompleteTest" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      Coord a, b, c;
      Index i;

      Stream_RedirectFile( stream, "testCompleteTest.dat" );

      Journal_Printf( stream, "Complete test:\n" );
      for( i = 1; i <= 10; i++ ) {
         Index   j;
         a[0] = 10.0 / (double)i;
         b[0] = 30.0 / (double)i;

         for( j = 1; j <= 10; j++ ) {
            Index   k;
            a[1] = 10.0 / (double)j;
            b[1] = 30.0 / (double)j;

            for( k = 1; k <= 10; k++ ) {
               a[2] = 10.0 / (double)k;
               b[2] = 30.0 / (double)k;
               Vec_Proj3D( c, a, b );
               Journal_Printf( stream, "{ %g, %g, %g }\n", c[0], c[1], c[2] );
            }
         }
      }
      Vec_SetScalar2D( a, 5.0, 9.0);
      Journal_Printf( stream, "2D: { %g, %g }\n", a[0], a[1]);

      pcu_filename_expected( "testVectorMathCompleteTest.expected", expected_file );
      pcu_check_fileEq( "testCompleteTest.dat", expected_file );
      remove( "testCompleteTest.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestVectorFunctions( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"CompleteTest" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double i[] = {1.0,0.0,0.0};
      double j[] = {0.0,1.0,0.0};
      double k[] = {0.0,0.0,1.0};
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      double D[] = {23  , 5  , -14  , 32, -21,    78};
      double angle;
      double vector[6];
      double *coordList[4];
      int d;
      
      coordList[0] = A;
      coordList[1] = B;
      coordList[2] = C;
      coordList[3] = D;

      Stream_RedirectFile( stream, "testFunctions.dat" );

      /* Check Rotation functions */
      Journal_Printf( stream, "****************************\n");
      angle = 1.0;
      StGermain_RotateCoordinateAxis( vector, k, I_AXIS, angle );
      Journal_Printf( stream, "K Rotated %2.3f degrees around I axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      StGermain_RotateVector( vector, k, i, angle );
      Journal_Printf( stream, "K Rotated %2.3f degrees around I axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      Journal_Printf( stream, "Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, k, 3 ));

      angle = 2.3;
      StGermain_RotateCoordinateAxis( vector, i, J_AXIS, angle );
      Journal_Printf( stream, "I Rotated %2.3f degrees around J axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      StGermain_RotateVector( vector, i, j, angle );
      Journal_Printf( stream, "I Rotated %2.3f degrees around J axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      Journal_Printf( stream, "Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, i, 3 ));

      angle = 1.8;
      StGermain_RotateCoordinateAxis( vector, j, K_AXIS, ( angle ) );
      Journal_Printf( stream, "J Rotated %2.3f degrees around K axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      StGermain_RotateVector( vector, j, k, ( angle ) );
      Journal_Printf( stream, "J Rotated %2.3f degrees around K axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
      Journal_Printf( stream, "Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, j, 3 ));
      
      /* Check addition function */
      Journal_Printf( stream, "\n****************************\n");
      Journal_Printf( stream, "vector = A + B\n");
      for ( d = 0 ; d <= 6 ; d++ ) {
         StGermain_VectorAddition( vector, A, B, d );
         StGermain_PrintNamedVector( stream, vector, d );
      }

      /* Check subtraction function */
      Journal_Printf( stream, "\n****************************\n");
      Journal_Printf( stream, "vector = A - B\n");
      for ( d = 0 ; d <= 6 ; d++ ) {
         StGermain_VectorSubtraction( vector, A, B, d );
         StGermain_PrintNamedVector( stream, vector, d );
      }

      pcu_filename_expected( "testVectorMathFunctions.expected", expected_file );
      pcu_check_fileEq( "testFunctions.dat", expected_file );
      remove( "testFunctions.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestMagnitudeFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"MagnitudeFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      int d;

      Stream_RedirectFile( stream, "testMagnitudeFunction.dat" );

      /* Check Magnitude Function */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check Magnitude Function\n");
      for ( d = 0 ; d <= 6 ; d++ ) {
         Journal_Printf( stream, "dim = %d magnitude = %2.3f\n", d, StGermain_VectorMagnitude( A, d ) );
         Journal_Printf( stream, "dim = %d magnitude = %2.3f\n", d, StGermain_VectorMagnitude( B, d ) );
      }

      pcu_filename_expected( "testVectorMathMagnitudeFunction.expected", expected_file );
      pcu_check_fileEq( "testMagnitudeFunction.dat", expected_file );
      remove( "testMagnitudeFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestDotProductFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"DotProductFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      int d;

      Stream_RedirectFile( stream, "testDotProductFunction.dat" );

      /* Check Dot Product */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check Dot Product Function\n");
      for ( d = 0 ; d <= 6 ; d++ ) 
         Journal_Printf( stream, "dim = %d dot product = %2.3f\n", d, StGermain_VectorDotProduct( A, B, d ) );

      pcu_filename_expected( "testVectorMathDotProductFunction.expected", expected_file );
      pcu_check_fileEq( "testDotProductFunction.dat", expected_file );
      remove( "testDotProductFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestCrossProductFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"CrossProductFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double vector[6];

      Stream_RedirectFile( stream, "testCrossProductFunction.dat" );

      /* Check Cross Product */
      /* Tested against http://www.engplanet.com/redirect.html?3859 */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check Cross Product Function\n");
      StGermain_VectorCrossProduct( vector, A, B );
      StGermain_PrintNamedVector( stream, vector, 3 );

      pcu_filename_expected( "testVectorMathCrossProductFunction.expected", expected_file );
      pcu_check_fileEq( "testCrossProductFunction.dat", expected_file );
      remove( "testCrossProductFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestDistancePointsFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"DistancePointsFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      int d;

      Stream_RedirectFile( stream, "testDistancePointsFunction.dat" );

      /* Checking distance between points function */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Checking distance between points Function\n");
      for ( d = 0 ; d <= 6 ; d++ ) 
         Journal_Printf( stream, "dim = %d distance = %2.3f\n", d, StGermain_DistanceBetweenPoints( A, B, d ) );

      pcu_filename_expected( "testVectorMathDistancePointsFunction.expected", expected_file );
      pcu_check_fileEq( "testDistancePointsFunction.dat", expected_file );
      remove( "testDistancePointsFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestNormalToPlaneFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"NormalToPlaneFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      double vector[6];

      Stream_RedirectFile( stream, "testNormalToPlaneFunction.dat" );

      /* Checking normal to plane function */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Checking normal to plane function function\n");
      vector[0] = vector[1] = vector[2] = vector[3] = vector[4] = vector[5] = 0.0;
      StGermain_NormalToPlane( vector, A, B, C );
      StGermain_PrintNamedVector( stream, vector, 3 );

      pcu_filename_expected( "testVectorMathNormalToPlaneFunction.expected", expected_file );
      pcu_check_fileEq( "testNormalToPlaneFunction.dat", expected_file );
      remove( "testNormalToPlaneFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestCentroidFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"CentroidFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      int d;
      double vector[6];

      Stream_RedirectFile( stream, "testCentroidFunction.dat" );

      /* Checking centroid function */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Checking centroid function\n");
      for ( d = 0 ; d <= 6 ; d++ ) {
         vector[0] = vector[1] = vector[2] = vector[3] = vector[4] = vector[5] = 0.0;
         StGermain_TriangleCentroid( vector, A, B, C, d );
         StGermain_PrintNamedVector( stream, vector, d );
      }

      pcu_filename_expected( "testVectorMathCentroidFunction.expected", expected_file );
      pcu_check_fileEq( "testCentroidFunction.dat", expected_file );
      remove( "testCentroidFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestTriangleArea( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"TriangleArea" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      int d;

      Stream_RedirectFile( stream, "testTriangleArea.dat" );

      /* Check Triangle Area */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check Triangle Area Function\n");
      for ( d = 0 ; d <= 6 ; d++ ) 
         Journal_Printf( stream, "dim = %d Triangle Area = %2.3f\n", d, StGermain_TriangleArea( A, B, C, d ) );

      pcu_filename_expected( "testVectorMathTriangleArea.expected", expected_file );
      pcu_check_fileEq( "testTriangleArea.dat", expected_file );
      remove( "testTriangleArea.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestNormalisationFunction( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"NormalisationFunction" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      int d;

      Stream_RedirectFile( stream, "testNormalisationFunction.dat" );

      /* Check Normalisation Function */
      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check Normalisation Function\n");
      d = 2;
      StGermain_VectorNormalise( A, d );
      StGermain_PrintNamedVector( stream, A, d);
      Journal_Printf( stream, "mag = %2.3f\n", StGermain_VectorMagnitude( A, d ) );

      d = 3;
      StGermain_VectorNormalise( B, d );
      StGermain_PrintNamedVector( stream, B, d);
      Journal_Printf( stream, "mag = %2.3f\n", StGermain_VectorMagnitude( B, d ) );

      d = 5;
      StGermain_VectorNormalise( C, d );
      StGermain_PrintNamedVector( stream, C, d);
      Journal_Printf( stream, "mag = %2.3f\n", StGermain_VectorMagnitude( C, d ) );

      pcu_filename_expected( "testVectorMathNormalisationFunction.expected", expected_file );
      pcu_check_fileEq( "testNormalisationFunction.dat", expected_file );
      remove( "testNormalisationFunction.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestVectorCrossProductMagnitude( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"VectorCrossProductMagnitude" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};

      Stream_RedirectFile( stream, "testVectorCrossProducMagnitude.dat" );

      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check StGermain_VectorCrossProductMagnitude\n");
      A[0] = 1.0; A[1] = 2.0 ; A[2] = 3.0;
      B[0] = 4.0; B[1] = 5.0 ; B[2] = 6.0;
      StGermain_PrintNamedVector( stream, A, 3);
      StGermain_PrintNamedVector( stream, B, 3);
      Journal_Printf( stream, "mag = %2.3g (2D)\n", StGermain_VectorCrossProductMagnitude( A, B, 2 ) );
      Journal_Printf( stream, "mag = %2.3g (3D)\n", StGermain_VectorCrossProductMagnitude( A, B, 3 ) );

      pcu_filename_expected( "testVectorMathCrossProductMagnitude.expected", expected_file );
      pcu_check_fileEq( "testVectorCrossProducMagnitude.dat", expected_file );
      remove( "testVectorCrossProducMagnitude.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestConvexQuadrilateralArea( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"ConvexQuadrilateralArea" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      double D[] = {23  , 5  , -14  , 32, -21,    78};

      Stream_RedirectFile( stream, "testConvexQuadrilateralArea.dat" );

      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check StGermain_ConvexQuadrilateralArea\n");
      A[0] = 0.0; A[1] = 0.0 ; A[2] = 0.0;
      B[0] = 0.0; B[1] = 4.0 ; B[2] = 2.3;
      C[0] = 1.2; C[1] = 4.0 ; C[2] = 2.3;
      D[0] = 1.2; D[1] = 0.0 ; D[2] = 0.0;
      Journal_Printf( stream, "area = %2.3g (2D)\n", StGermain_ConvexQuadrilateralArea( A, B, C, D, 2 ) );
      Journal_Printf( stream, "area = %2.3g (3D)\n", StGermain_ConvexQuadrilateralArea( A, B, C, D, 3 ) );

      pcu_filename_expected( "testVectorMathConvexQuadrilateralArea.expected", expected_file );
      pcu_check_fileEq( "testConvexQuadrilateralArea.dat", expected_file );
      remove( "testConvexQuadrilateralArea.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestScalarTripleProduct( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"ScalarTripleProduct" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double **matrix;

      Stream_RedirectFile( stream, "testScalarTripleProduct.dat" );

      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check StGermain_ScalarTripleProduct \n");
      matrix = Memory_Alloc_2DArray( double, 3, 3, (Name)"matrix"  );
      matrix[0][0] = 1.0; matrix[0][1] = 2.0 ; matrix[0][2] = 3.0;
      matrix[1][0] = 4.0; matrix[1][1] = 5.0 ; matrix[1][2] = 6.0;
      matrix[2][0] = 7.0; matrix[2][1] = 8.0 ; matrix[2][2] = 11.0;
      Journal_Printf( stream, "scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[0], matrix[1], matrix[2] ));
      Journal_Printf( stream, "scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[2], matrix[0], matrix[1] ));
      Journal_Printf( stream, "scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[1], matrix[2], matrix[0] ));
      Journal_Printf( stream, "scalar triple product = %2.3g\n", StGermain_MatrixDeterminant( matrix , 3 ));
      Memory_Free( matrix );

      pcu_filename_expected( "testVectorMathScalarTripleProduct.expected", expected_file );
      pcu_check_fileEq( "testScalarTripleProduct.dat", expected_file );
      remove( "testScalarTripleProduct.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestParallelPipedVolume( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"ParallelPipedVolume" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      double D[] = {23  , 5  , -14  , 32, -21,    78};

      Stream_RedirectFile( stream, "testParallelPipedVolume.dat" );

      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check StGermain_ParallelepipedVolume \n");
      A[0] = 0.0; A[1] = 0.0 ; A[2] = 0.0;
      B[0] = 1.1; B[1] = 0.0 ; B[2] = 0.0;
      C[0] = 0.2; C[1] = 1.7 ; C[2] = 0.0;
      D[0] = 0.0; D[1] = 0.0 ; D[2] = 1.3;
      Journal_Printf( stream, "volume = %2.3g\n", StGermain_ParallelepipedVolume( A, B, C, D ));

      pcu_filename_expected( "testVectorMathParallelPipedVolume.expected", expected_file );
      pcu_check_fileEq( "testParallelPipedVolume.dat", expected_file );
      remove( "testParallelPipedVolume.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite_TestAverageCoord( VectorMathSuiteData* data ) {
   unsigned procToWatch = data->nProcs >=2 ? 1 : 0;
   Stream*  stream = Journal_Register( Info_Type, (Name)"AverageCoord" );
   char     expected_file[PCU_PATH_MAX];   

   if (data->rank == procToWatch ) {
      double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
      double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
      double C[] = {23  , 5  , -14  , 32, -21,    78};
      double D[] = {23  , 5  , -14  , 32, -21,    78};
      double *coordList[4];

      coordList[0] = A;
      coordList[1] = B;
      coordList[2] = C;
      coordList[3] = D;

      Stream_RedirectFile( stream, "testAverageCoord.dat" );

      Journal_Printf( stream, "****************************\n");
      Journal_Printf( stream, "Check StGermain_AverageCoord \n");
      A[0] = 1.0;  A[1] = 2.2 ; A[2] = 3.2;
      B[0] = 41.0; B[1] = 5.0 ; B[2] = 6.9;
      C[0] = -7.0; C[1] = 8.7 ; C[2] = 11.0;

      StGermain_AverageCoord( D, coordList, 3, 3 );
      StGermain_PrintNamedVector( stream, D, 3);

      pcu_filename_expected( "testVectorMathAverageCoord.expected", expected_file );
      pcu_check_fileEq( "testAverageCoord.dat", expected_file );
      remove( "testAverageCoord.dat" );

      Stream_CloseAndFreeFile( stream );
   }
}

void VectorMathSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, VectorMathSuiteData );
   pcu_suite_setFixtures( suite, VectorMathSuite_Setup, VectorMathSuite_Teardown );
   pcu_suite_addTest( suite, VectorMathSuite_BasicTest );
   pcu_suite_addTest( suite, VectorMathSuite_CompleteTest );
   pcu_suite_addTest( suite, VectorMathSuite_TestVectorFunctions );
   pcu_suite_addTest( suite, VectorMathSuite_TestMagnitudeFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestDotProductFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestCrossProductFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestDistancePointsFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestNormalToPlaneFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestCentroidFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestTriangleArea );
   pcu_suite_addTest( suite, VectorMathSuite_TestNormalisationFunction );
   pcu_suite_addTest( suite, VectorMathSuite_TestVectorCrossProductMagnitude );
   pcu_suite_addTest( suite, VectorMathSuite_TestConvexQuadrilateralArea );
   pcu_suite_addTest( suite, VectorMathSuite_TestScalarTripleProduct );
   pcu_suite_addTest( suite, VectorMathSuite_TestParallelPipedVolume );
   pcu_suite_addTest( suite, VectorMathSuite_TestAverageCoord );
}


