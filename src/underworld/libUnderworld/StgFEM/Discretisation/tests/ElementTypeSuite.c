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
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/Discretisation/Discretisation.h>
#include "ElementTypeSuite.h"

#define TOLERANCE 1.0e-8
#define EPSILON 1.0e-8

typedef struct {
   Dictionary* dict;
} ElementTypeSuiteData;

FeMesh* BuildMeshLinear( unsigned nDims, unsigned* sizes, double* minCrd, double* maxCrd ) {
   CartesianGenerator* gen;
   FeMesh*             feMesh;
   unsigned            maxDecomp[3] = {0, 1, 1};

   gen = CartesianGenerator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, nDims );
   CartesianGenerator_SetTopologyParams( gen, sizes, 0, NULL, maxDecomp );
   CartesianGenerator_SetGeometryParams( gen, minCrd, maxCrd );
   CartesianGenerator_SetShadowDepth( gen, 0 );

   feMesh = FeMesh_New( "" );
   Mesh_SetGenerator( feMesh, gen );
   FeMesh_SetElementFamily( feMesh, "Q1" );
   Stg_Component_Build( feMesh, NULL, False );
   Stg_Component_Initialise( feMesh, NULL, False );

   return feMesh;
}

FeMesh* BuildMeshQuadratic( unsigned nDims, unsigned* sizes, double* minCrd, double* maxCrd ) {
   CartesianGenerator* gen;
   FeMesh*             feMesh;
   unsigned            maxDecomp[3] = {0, 1, 1};

   gen = (CartesianGenerator*)C2Generator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, nDims );
   C2Generator_SetTopologyParams( gen, sizes, 0, NULL, maxDecomp );
   CartesianGenerator_SetGeometryParams( gen, minCrd, maxCrd );
   CartesianGenerator_SetShadowDepth( gen, 0 );

   feMesh = FeMesh_New( "" );
   Mesh_SetGenerator( feMesh, gen );
   FeMesh_SetElementFamily( feMesh, "Q2" );
   Stg_Component_Build( feMesh, NULL, False );
   Stg_Component_Initialise( feMesh, NULL, False );

   return feMesh;
}

void ElementTypeSuite_Setup( ElementTypeSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   data->dict = Dictionary_New();
}

void ElementTypeSuite_Teardown( ElementTypeSuiteData* data ) {
   Stg_Class_Delete( data->dict );
}

void ElementTypeSuite_TestLinear2D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   unsigned     maxTests = 40;
   unsigned     test_i;
   Coord        gCoord, lCoord, gCoord_fromLocal;
   unsigned     el, elNodeCount, elNode_i;
   ElementType* elType;
   IArray*      inc = IArray_New();
   int*         elNodes;
   double       Ni[4];
   double       vecNorm;
   int          dim = 2;
   unsigned     sizes[3] = { 6, 6, 1 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 1.2, 1.2, 1.2 };

   feMesh = BuildMeshLinear( dim, sizes, minCrd, maxCrd );

   srand48( 0 );
   for( test_i = 0; test_i < maxTests; test_i++ ) {
      gCoord[I_AXIS] = drand48();
      gCoord[J_AXIS] = drand48();
   
      Mesh_Algorithms_SearchElements( feMesh->algorithms, gCoord, &el );
      elNodeCount = FeMesh_GetElementNodeSize( feMesh, el );
      elType = FeMesh_GetElementType( feMesh, el );
      _ElementType_ConvertGlobalCoordToElLocal( elType, feMesh, el, gCoord, lCoord );
      ElementType_EvaluateShapeFunctionsAt( elType, lCoord, Ni );

      Mesh_GetIncidence( feMesh, dim, el, MT_VERTEX, inc );
      elNodes = IArray_GetPtr( inc );
      memset( gCoord_fromLocal, 0, sizeof( double ) * dim );

      for( elNode_i = 0; elNode_i < elNodeCount; elNode_i++ ) {
         gCoord_fromLocal[I_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[I_AXIS];
         gCoord_fromLocal[J_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[J_AXIS];
      }

      vecNorm = sqrt( (gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS])*(gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS]) + 
         (gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS])*(gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS]) );

      pcu_check_true( vecNorm < TOLERANCE );
   }
   Stg_Class_Delete( inc );
   Stg_Component_Destroy( feMesh, NULL, True );   
}

void ElementTypeSuite_TestLinear3D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   unsigned     maxTests = 40;
   unsigned     test_i;
   Coord        gCoord, lCoord, gCoord_fromLocal;
   unsigned     el, elNodeCount, elNode_i;
   ElementType* elType;
   IArray*      inc = IArray_New();
   int*         elNodes;
   double       Ni[8];
   double       vecNorm;
   int          dim = 3;
   unsigned     sizes[3] = { 6, 6, 1 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 1.2, 1.2, 1.2 };

   feMesh = BuildMeshLinear( dim, sizes, minCrd, maxCrd );

   srand48( 0 );
   for( test_i = 0; test_i < maxTests; test_i++ ) {
      gCoord[I_AXIS] = drand48();
      gCoord[J_AXIS] = drand48();
      gCoord[K_AXIS] = drand48();
   
      Mesh_Algorithms_SearchElements( feMesh->algorithms, gCoord, &el );
      elNodeCount = FeMesh_GetElementNodeSize( feMesh, el );
      elType = FeMesh_GetElementType( feMesh, el );
      _ElementType_ConvertGlobalCoordToElLocal( elType, feMesh, el, gCoord, lCoord );
      ElementType_EvaluateShapeFunctionsAt( elType, lCoord, Ni );

      Mesh_GetIncidence( feMesh, dim, el, MT_VERTEX, inc );
      elNodes = IArray_GetPtr( inc );
      memset( gCoord_fromLocal, 0, sizeof( double ) * dim );

      for( elNode_i = 0; elNode_i < elNodeCount; elNode_i++ ) {
         gCoord_fromLocal[I_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[I_AXIS];
         gCoord_fromLocal[J_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[J_AXIS];
         gCoord_fromLocal[K_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[K_AXIS];
      }

      vecNorm = sqrt( (gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS])*(gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS]) + 
         (gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS])*(gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS]) +
         (gCoord[K_AXIS] - gCoord_fromLocal[K_AXIS])*(gCoord[K_AXIS] - gCoord_fromLocal[K_AXIS]) );

      pcu_check_true( vecNorm < TOLERANCE );
   }
   Stg_Class_Delete( inc );
   Stg_Component_Destroy( feMesh, NULL, True );   
}

void ElementTypeSuite_TestQuadratic2D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   unsigned     maxTests = 40;
   unsigned     test_i;
   Coord        gCoord, lCoord, gCoord_fromLocal;
   unsigned     el, elNodeCount, elNode_i;
   ElementType* elType;
   IArray*      inc = IArray_New();
   int*         elNodes;
   double       Ni[9];
   double       vecNorm;
   int          dim = 2;
   unsigned     sizes[3] = { 6, 6, 1 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 1.2, 1.2, 1.2 };

   feMesh = BuildMeshQuadratic( dim, sizes, minCrd, maxCrd );

   srand48( 0 );
   for( test_i = 0; test_i < maxTests; test_i++ ) {
      gCoord[I_AXIS] = drand48();
      gCoord[J_AXIS] = drand48();
   
      Mesh_Algorithms_SearchElements( feMesh->algorithms, gCoord, &el );
      elNodeCount = FeMesh_GetElementNodeSize( feMesh, el );
      elType = FeMesh_GetElementType( feMesh, el );
      _ElementType_ConvertGlobalCoordToElLocal( elType, feMesh, el, gCoord, lCoord );
      ElementType_EvaluateShapeFunctionsAt( elType, lCoord, Ni );

      Mesh_GetIncidence( feMesh, dim, el, MT_VERTEX, inc );
      elNodes = IArray_GetPtr( inc );
      memset( gCoord_fromLocal, 0, sizeof( double ) * dim );

      for( elNode_i = 0; elNode_i < elNodeCount; elNode_i++ ) {
         gCoord_fromLocal[I_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[I_AXIS];
         gCoord_fromLocal[J_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[J_AXIS];
      }

      vecNorm = sqrt( (gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS])*(gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS]) + 
         (gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS])*(gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS]) );

      pcu_check_true( vecNorm < TOLERANCE );
   }
   Stg_Class_Delete( inc );
   Stg_Component_Destroy( feMesh, NULL, True );   
}

void ElementTypeSuite_TestQuadratic3D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   unsigned     maxTests = 40;
   unsigned     test_i;
   Coord        gCoord, lCoord, gCoord_fromLocal;
   unsigned     el, elNodeCount, elNode_i;
   ElementType* elType;
   IArray*      inc = IArray_New();
   int*         elNodes;
   double       Ni[27];
   double       vecNorm;
   int          dim = 3;
   unsigned     sizes[3] = { 6, 6, 1 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 1.2, 1.2, 1.2 };

   feMesh = BuildMeshQuadratic( dim, sizes, minCrd, maxCrd );

   srand48( 0 );
   for( test_i = 0; test_i < maxTests; test_i++ ) {
      gCoord[I_AXIS] = drand48();
      gCoord[J_AXIS] = drand48();
      gCoord[K_AXIS] = drand48();
   
      Mesh_Algorithms_SearchElements( feMesh->algorithms, gCoord, &el );
      elNodeCount = FeMesh_GetElementNodeSize( feMesh, el );
      elType = FeMesh_GetElementType( feMesh, el );
      _ElementType_ConvertGlobalCoordToElLocal( elType, feMesh, el, gCoord, lCoord );
      ElementType_EvaluateShapeFunctionsAt( elType, lCoord, Ni );

      Mesh_GetIncidence( feMesh, dim, el, MT_VERTEX, inc );
      elNodes = IArray_GetPtr( inc );
      memset( gCoord_fromLocal, 0, sizeof( double ) * dim );

      for( elNode_i = 0; elNode_i < elNodeCount; elNode_i++ ) {
         gCoord_fromLocal[I_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[I_AXIS];
         gCoord_fromLocal[J_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[J_AXIS];
         gCoord_fromLocal[K_AXIS] += Ni[elNode_i] * Mesh_GetVertex( feMesh, elNodes[elNode_i] )[K_AXIS];
      }

      vecNorm = sqrt( (gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS])*(gCoord[I_AXIS] - gCoord_fromLocal[I_AXIS]) + 
         (gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS])*(gCoord[J_AXIS] - gCoord_fromLocal[J_AXIS]) +
         (gCoord[K_AXIS] - gCoord_fromLocal[K_AXIS])*(gCoord[K_AXIS] - gCoord_fromLocal[K_AXIS]) );

      pcu_check_true( vecNorm < TOLERANCE );
   }

   Stg_Class_Delete( inc );
   Stg_Component_Destroy( feMesh, NULL, True );   
}

void ElementTypeSuite_TestSurfaceJacobian_Linear2D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   ElementType* elType;
   unsigned     sizes[2] = { 4, 3 };
   double       minCrd[2] = { 0.0, 0.0 };
   double       maxCrd[2] = { 4.0, 3.0 };
   double       xi[4][2][2]; /* [face][point][dim] */
   unsigned     face_i, ip_i;
   double       norm[3];
   double       weight = 1.0;
   double       detJac;
   double       faceLen;

   xi[0][0][0] = -0.57735026918962584; xi[0][0][1] = -1.0;
   xi[0][1][0] = +0.57735026918962584; xi[0][1][1] = -1.0;

   xi[1][0][0] = -0.57735026918962584; xi[1][0][1] = +1.0;
   xi[1][1][0] = +0.57735026918962584; xi[1][1][1] = +1.0;

   xi[2][0][0] = -1.0;                 xi[2][0][1] = -0.57735026918962584;
   xi[2][1][0] = -1.0;                 xi[2][1][1] = +0.57735026918962584;

   xi[3][0][0] = +1.0;                 xi[3][0][1] = -0.57735026918962584;
   xi[3][1][0] = +1.0;                 xi[3][1][1] = +0.57735026918962584;

   feMesh = BuildMeshLinear( 2, sizes, minCrd, maxCrd );
   elType = FeMesh_GetElementType( feMesh, 0 );

   for( face_i = 0; face_i < 4; face_i++ ) {
      faceLen = 0.0;
      for( ip_i = 0; ip_i < 2; ip_i++ ) {
         ElementType_SurfaceNormal( elType, 0, 2, xi[face_i][ip_i], norm );
         detJac = ElementType_SurfaceJacobianDeterminant( elType, feMesh, 0, xi[face_i][ip_i], 2, norm );
         faceLen += detJac * weight;
      }
      pcu_check_true( fabs( faceLen - 1.0 ) < EPSILON );
   }
   Stg_Component_Destroy( feMesh, NULL, True );
}

void ElementTypeSuite_TestSurfaceJacobian_Linear3D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   ElementType* elType;
   unsigned     sizes[3] = { 4, 3, 5 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 4.0, 3.0, 5.0 };
   double       xi[6][4][3]; /* [face][point][dim] */
   unsigned     face_i, ip_i;
   double       norm[3];
   double       weight = 1.0;
   double       detJac;
   double       faceArea;

   xi[0][0][0] = -0.57735026918962584; xi[0][0][1] = -1.0;                 xi[0][0][2] = -0.57735026918962584;
   xi[0][1][0] = +0.57735026918962584; xi[0][1][1] = -1.0;                 xi[0][1][2] = -0.57735026918962584;
   xi[0][2][0] = -0.57735026918962584; xi[0][2][1] = -1.0;                 xi[0][2][2] = +0.57735026918962584;
   xi[0][3][0] = +0.57735026918962584; xi[0][3][1] = -1.0;                 xi[0][3][2] = +0.57735026918962584;

   xi[1][0][0] = -0.57735026918962584; xi[1][0][1] = +1.0;                 xi[1][0][2] = -0.57735026918962584;
   xi[1][1][0] = +0.57735026918962584; xi[1][1][1] = +1.0;                 xi[1][1][2] = -0.57735026918962584;
   xi[1][2][0] = -0.57735026918962584; xi[1][2][1] = +1.0;                 xi[1][2][2] = +0.57735026918962584;
   xi[1][3][0] = +0.57735026918962584; xi[1][3][1] = +1.0;                 xi[1][3][2] = +0.57735026918962584;

   xi[2][0][0] = -1.0;                 xi[2][0][1] = -0.57735026918962584; xi[2][0][2] = -0.57735026918962584;
   xi[2][1][0] = -1.0;                 xi[2][1][1] = +0.57735026918962584; xi[2][1][2] = -0.57735026918962584;
   xi[2][2][0] = -1.0;                 xi[2][2][1] = -0.57735026918962584; xi[2][2][2] = +0.57735026918962584;
   xi[2][3][0] = -1.0;                 xi[2][3][1] = +0.57735026918962584; xi[2][3][2] = +0.57735026918962584;

   xi[3][0][0] = +1.0;                 xi[3][0][1] = -0.57735026918962584; xi[3][0][2] = -0.57735026918962584;
   xi[3][1][0] = +1.0;                 xi[3][1][1] = +0.57735026918962584; xi[3][1][2] = -0.57735026918962584;
   xi[3][2][0] = +1.0;                 xi[3][2][1] = -0.57735026918962584; xi[3][2][2] = +0.57735026918962584;
   xi[3][3][0] = +1.0;                 xi[3][3][1] = +0.57735026918962584; xi[3][3][2] = +0.57735026918962584;

   xi[4][0][0] = -0.57735026918962584; xi[4][0][1] = -0.57735026918962584; xi[4][0][2] = -1.0;
   xi[4][1][0] = +0.57735026918962584; xi[4][1][1] = -0.57735026918962584; xi[4][1][2] = -1.0;
   xi[4][2][0] = -0.57735026918962584; xi[4][2][1] = +0.57735026918962584; xi[4][2][2] = -1.0;
   xi[4][3][0] = +0.57735026918962584; xi[4][3][1] = +0.57735026918962584; xi[4][3][2] = -1.0;

   xi[5][0][0] = -0.57735026918962584; xi[5][0][1] = -0.57735026918962584; xi[5][0][2] = +1.0;
   xi[5][1][0] = +0.57735026918962584; xi[5][1][1] = -0.57735026918962584; xi[5][1][2] = +1.0;
   xi[5][2][0] = -0.57735026918962584; xi[5][2][1] = +0.57735026918962584; xi[5][2][2] = +1.0;
   xi[5][3][0] = +0.57735026918962584; xi[5][3][1] = +0.57735026918962584; xi[5][3][2] = +1.0;

   feMesh = BuildMeshLinear( 3, sizes, minCrd, maxCrd );
   elType = FeMesh_GetElementType( feMesh, 0 );

   for( face_i = 0; face_i < 6; face_i++ ) {
      faceArea = 0.0;
      for( ip_i = 0; ip_i < 4; ip_i++ ) {
         ElementType_SurfaceNormal( elType, 0, 3, xi[face_i][ip_i], norm );
         detJac = ElementType_SurfaceJacobianDeterminant( elType, feMesh, 0, xi[face_i][ip_i], 3, norm );
         faceArea += detJac * weight;
      }
      pcu_check_true( fabs( faceArea - 1.0 ) < EPSILON );
   }
   Stg_Component_Destroy( feMesh, NULL, True );
}

void ElementTypeSuite_TestSurfaceJacobian_Quadratic2D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   ElementType* elType;
   unsigned     sizes[2] = { 4, 3 };
   double       minCrd[2] = { 0.0, 0.0 };
   double       maxCrd[2] = { 4.0, 3.0 };
   double       xi[4][3][2]; /* [face][point][dim] */
   unsigned     face_i, ip_i;
   double       norm[3];
   double       weight[3] = { 0.5555555555555555, 0.8888888888888889, 0.5555555555555555 };
   double       detJac;
   double       faceLen;

   xi[0][0][0] = -0.7745966692414834; xi[0][0][1] = -1.0;
   xi[0][1][0] = 0.0;                 xi[0][1][1] = -1.0;
   xi[0][2][0] = +0.7745966692414834; xi[0][2][1] = -1.0;

   xi[1][0][0] = -0.7745966692414834; xi[1][0][1] = +1.0;
   xi[1][1][0] = 0.0;                 xi[1][1][1] = +1.0;
   xi[1][2][0] = +0.7745966692414834; xi[1][2][1] = +1.0;

   xi[2][0][0] = -1.0;                xi[2][0][1] = -0.7745966692414834;
   xi[2][1][0] = -1.0;                xi[2][1][1] = 0.0;
   xi[2][2][0] = -1.0;                xi[2][2][1] = +0.7745966692414834;

   xi[3][0][0] = +1.0;                xi[3][0][1] = -0.7745966692414834;
   xi[3][1][0] = +1.0;                xi[3][1][1] = 0.0;
   xi[3][2][0] = +1.0;                xi[3][2][1] = +0.7745966692414834;

   feMesh = BuildMeshQuadratic( 2, sizes, minCrd, maxCrd );
   elType = FeMesh_GetElementType( feMesh, 0 );

   for( face_i = 0; face_i < 4; face_i++ ) {
      faceLen = 0.0;
      for( ip_i = 0; ip_i < 3; ip_i++ ) {
         ElementType_SurfaceNormal( elType, 0, 2, xi[face_i][ip_i], norm );
         detJac = ElementType_SurfaceJacobianDeterminant( elType, feMesh, 0, xi[face_i][ip_i], 2, norm );
         faceLen += detJac * weight[ip_i];
      }
      pcu_check_true( fabs( faceLen - 1.0 ) < EPSILON );
   }
   Stg_Component_Destroy( feMesh, NULL, True );
}

void ElementTypeSuite_TestSurfaceJacobian_Quadratic3D( ElementTypeSuiteData* data ) {
   FeMesh*      feMesh;
   ElementType* elType;
   unsigned     sizes[3] = { 4, 3, 5 };
   double       minCrd[3] = { 0.0, 0.0, 0.0 };
   double       maxCrd[3] = { 4.0, 3.0, 5.0 };
   double       xi[6][9][3]; /* [face][point][dim] */
   unsigned     face_i, ip_i;
   double       norm[3];
   double       weight[9] = { 0.30864197530864201, 0.49382716049382713, 0.30864197530864201, 
                   0.49382716049382713, 0.79012345679012341, 0.49382716049382713, 
                   0.30864197530864201, 0.49382716049382713, 0.30864197530864201 };
   double       detJac;
   double       faceArea;

   xi[0][0][0] = -0.7745966692414834; xi[0][0][1] = -1.0;                xi[0][0][2] = -0.7745966692414834;
   xi[0][1][0] = 0.0;                 xi[0][1][1] = -1.0;                xi[0][1][2] = -0.7745966692414834;
   xi[0][2][0] = +0.7745966692414834; xi[0][2][1] = -1.0;                xi[0][2][2] = -0.7745966692414834;
   xi[0][3][0] = -0.7745966692414834; xi[0][3][1] = -1.0;                xi[0][3][2] = 0.0;
   xi[0][4][0] = 0.0;                 xi[0][4][1] = -1.0;                xi[0][4][2] = 0.0;
   xi[0][5][0] = +0.7745966692414834; xi[0][5][1] = -1.0;                xi[0][5][2] = 0.0;
   xi[0][6][0] = -0.7745966692414834; xi[0][6][1] = -1.0;                xi[0][6][2] = +0.7745966692414834;
   xi[0][7][0] = 0.0;                 xi[0][7][1] = -1.0;                xi[0][7][2] = +0.7745966692414834;
   xi[0][8][0] = +0.7745966692414834; xi[0][8][1] = -1.0;                xi[0][8][2] = +0.7745966692414834;

   xi[1][0][0] = -0.7745966692414834; xi[1][0][1] = +1.0;                xi[1][0][2] = -0.7745966692414834;
   xi[1][1][0] = 0.0;                 xi[1][1][1] = +1.0;                xi[1][1][2] = -0.7745966692414834;
   xi[1][2][0] = +0.7745966692414834; xi[1][2][1] = +1.0;                xi[1][2][2] = -0.7745966692414834;
   xi[1][3][0] = -0.7745966692414834; xi[1][3][1] = +1.0;                xi[1][3][2] = 0.0;
   xi[1][4][0] = 0.0;                 xi[1][4][1] = +1.0;                xi[1][4][2] = 0.0;
   xi[1][5][0] = +0.7745966692414834; xi[1][5][1] = +1.0;                xi[1][5][2] = 0.0;
   xi[1][6][0] = -0.7745966692414834; xi[1][6][1] = +1.0;                xi[1][6][2] = +0.7745966692414834;
   xi[1][7][0] = 0.0;                 xi[1][7][1] = -1.0;                xi[1][7][2] = +0.7745966692414834;
   xi[1][8][0] = +0.7745966692414834; xi[1][8][1] = -1.0;                xi[1][8][2] = +0.7745966692414834;

   xi[2][0][0] = -1.0;                xi[2][0][1] = -0.7745966692414834; xi[2][0][2] = -0.7745966692414834;
   xi[2][1][0] = -1.0;                xi[2][1][1] = 0.0;                 xi[2][1][2] = -0.7745966692414834;
   xi[2][2][0] = -1.0;                xi[2][2][1] = +0.7745966692414834; xi[2][2][2] = -0.7745966692414834;
   xi[2][3][0] = -1.0;                xi[2][3][1] = -0.7745966692414834; xi[2][3][2] = 0.0;
   xi[2][4][0] = -1.0;                xi[2][4][1] = 0.0;                 xi[2][4][2] = 0.0;
   xi[2][5][0] = -1.0;                xi[2][5][1] = +0.7745966692414834; xi[2][5][2] = 0.0;
   xi[2][6][0] = -1.0;                xi[2][6][1] = -0.7745966692414834; xi[2][6][2] = +0.7745966692414834;
   xi[2][7][0] = -1.0;                xi[2][7][1] = 0.0;                 xi[2][7][2] = +0.7745966692414834;
   xi[2][8][0] = -1.0;                xi[2][8][1] = +0.7745966692414834; xi[2][8][2] = +0.7745966692414834;

   xi[3][0][0] = +1.0;                xi[3][0][1] = -0.7745966692414834; xi[3][0][2] = -0.7745966692414834;
   xi[3][1][0] = +1.0;                xi[3][1][1] = 0.0;                 xi[3][1][2] = -0.7745966692414834;
   xi[3][2][0] = +1.0;                xi[3][2][1] = +0.7745966692414834; xi[3][2][2] = -0.7745966692414834;
   xi[3][3][0] = +1.0;                xi[3][3][1] = -0.7745966692414834; xi[3][3][2] = 0.0;
   xi[3][4][0] = +1.0;                xi[3][4][1] = 0.0;                 xi[3][4][2] = 0.0;
   xi[3][5][0] = +1.0;                xi[3][5][1] = +0.7745966692414834; xi[3][5][2] = 0.0;
   xi[3][6][0] = +1.0;                xi[3][6][1] = -0.7745966692414834; xi[3][6][2] = +0.7745966692414834;
   xi[3][7][0] = +1.0;                xi[3][7][1] = 0.0;                 xi[3][7][2] = +0.7745966692414834;
   xi[3][8][0] = +1.0;                xi[3][8][1] = +0.7745966692414834; xi[3][8][2] = +0.7745966692414834;

   xi[4][0][0] = -0.7745966692414834; xi[4][0][1] = -0.7745966692414834; xi[4][0][2] = -1.0;
   xi[4][1][0] = 0.0;                 xi[4][1][1] = -0.7745966692414834; xi[4][1][2] = -1.0;
   xi[4][2][0] = +0.7745966692414834; xi[4][2][1] = -0.7745966692414834; xi[4][2][2] = -1.0;
   xi[4][3][0] = -0.7745966692414834; xi[4][3][1] = 0.0;                 xi[4][3][2] = -1.0;
   xi[4][4][0] = 0.0;                 xi[4][4][1] = 0.0;                 xi[4][4][2] = -1.0;
   xi[4][5][0] = +0.7745966692414834; xi[4][5][1] = 0.0;                 xi[4][5][2] = -1.0;
   xi[4][6][0] = -0.7745966692414834; xi[4][6][1] = +0.7745966692414834; xi[4][6][2] = -1.0;
   xi[4][7][0] = 0.0;                 xi[4][7][1] = +0.7745966692414834; xi[4][7][2] = -1.0;
   xi[4][8][0] = +0.7745966692414834; xi[4][8][1] = +0.7745966692414834; xi[4][8][2] = -1.0;

   xi[5][0][0] = -0.7745966692414834; xi[5][0][1] = -0.7745966692414834; xi[5][0][2] = +1.0;
   xi[5][1][0] = 0.0;                 xi[5][1][1] = -0.7745966692414834; xi[5][1][2] = +1.0;
   xi[5][2][0] = +0.7745966692414834; xi[5][2][1] = -0.7745966692414834; xi[5][2][2] = +1.0;
   xi[5][3][0] = -0.7745966692414834; xi[5][3][1] = 0.0;                 xi[5][3][2] = +1.0;
   xi[5][4][0] = 0.0;                 xi[5][4][1] = 0.0;                 xi[5][4][2] = +1.0;
   xi[5][5][0] = +0.7745966692414834; xi[5][5][1] = 0.0;                 xi[5][5][2] = +1.0;
   xi[5][6][0] = -0.7745966692414834; xi[5][6][1] = +0.7745966692414834; xi[5][6][2] = +1.0;
   xi[5][7][0] = 0.0;                 xi[5][7][1] = +0.7745966692414834; xi[5][7][2] = +1.0;
   xi[5][8][0] = +0.7745966692414834; xi[5][8][1] = +0.7745966692414834; xi[5][8][2] = +1.0;

   feMesh = BuildMeshQuadratic( 3, sizes, minCrd, maxCrd );
   elType = FeMesh_GetElementType( feMesh, 0 );

   for( face_i = 0; face_i < 6; face_i++ ) {
      faceArea = 0.0;
      for( ip_i = 0; ip_i < 9; ip_i++ ) {
         ElementType_SurfaceNormal( elType, 0, 3, xi[face_i][ip_i], norm );
         detJac = ElementType_SurfaceJacobianDeterminant( elType, feMesh, 0, xi[face_i][ip_i], 3, norm );
         faceArea += detJac * weight[ip_i];
      }
      pcu_check_true( fabs( faceArea - 1.0 ) < EPSILON );
   }
   Stg_Component_Destroy( feMesh, NULL, True );
}

void ElementTypeSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ElementTypeSuiteData );
   pcu_suite_setFixtures( suite, ElementTypeSuite_Setup, ElementTypeSuite_Teardown );
   pcu_suite_addTest( suite, ElementTypeSuite_TestLinear2D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestLinear3D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestQuadratic2D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestQuadratic3D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestSurfaceJacobian_Linear2D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestSurfaceJacobian_Linear3D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestSurfaceJacobian_Quadratic2D );
   pcu_suite_addTest( suite, ElementTypeSuite_TestSurfaceJacobian_Quadratic3D );
}
