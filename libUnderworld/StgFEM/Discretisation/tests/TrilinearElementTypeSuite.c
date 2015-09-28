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
#include <StgDomain/StgDomain.h>
#include <StgFEM/Discretisation/Discretisation.h>
#include "TrilinearElementTypeSuite.h"

typedef struct {
} TrilinearElementTypeSuiteData;

FeMesh* buildMesh() {
   CartesianGenerator* gen;
   int                 nRanks;
   unsigned            sizes[3];
   double              minCrd[3];
   double              maxCrd[3];
   FeMesh*             mesh;

   insist( MPI_Comm_size( MPI_COMM_WORLD, &nRanks ), == MPI_SUCCESS );
   sizes[0] = sizes[1] = sizes[2] = nRanks * 4;
   minCrd[0] = minCrd[1] = minCrd[2] = 0.0;
   maxCrd[0] = minCrd[1] = minCrd[2] = (double)nRanks;

   gen = CartesianGenerator_New( "", NULL );
   MeshGenerator_SetDimSize( gen, 3 );
   CartesianGenerator_SetShadowDepth( gen, 1 );
   CartesianGenerator_SetTopologyParams( gen, sizes, 0, NULL, NULL );
   CartesianGenerator_SetGeometryParams( gen, minCrd, maxCrd );

   mesh = FeMesh_New( "" );
   Mesh_SetGenerator( mesh, gen );
   FeMesh_SetElementFamily( mesh, "linear" );
   Stg_Component_Build( mesh, NULL, False );

   return mesh;
}

void TrilinearElementTypeSuite_Setup( TrilinearElementTypeSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   //Stream_RedirectAllToFile( "TrilinearElementTypeSuite" );
}

void TrilinearElementTypeSuite_Teardown( TrilinearElementTypeSuiteData* data ) {
   //Stream_PurgeAllRedirectedFiles();
}

void TrilinearElementTypeSuite_TestShape( TrilinearElementTypeSuiteData* data ) {
   FeMesh*   mesh = NULL;
   int       nEls, nVerts, nDims;
   const int *verts;
   double*   vert = NULL;
   double    lCrd[3] = { 0.0, 0.0, 0.0 };
   double    basis[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   IArray*   inc;
   int       e_i, v_i, v_j;

   mesh = buildMesh();
   pcu_check_true( mesh );
   Stg_Component_Initialise( mesh, data, True );

   nDims = Mesh_GetDimSize( mesh );
   nEls = Mesh_GetDomainSize( mesh, nDims );
   inc = IArray_New();

   for( e_i = 0; e_i < nEls; e_i++ ) {
      Mesh_GetIncidence( mesh, nDims, e_i, 0, inc );
      nVerts = IArray_GetSize( inc );
      verts = IArray_GetPtr( inc );

      for( v_i = 0; v_i < nVerts; v_i++ ) {
         vert = Mesh_GetVertex( mesh, verts[v_i] );
         FeMesh_CoordGlobalToLocal( mesh, e_i, vert, lCrd );
         FeMesh_EvalBasis( mesh, e_i, lCrd, basis );

         for( v_j = 0; v_j < nVerts; v_j++ ) {
            if( (v_i == v_j && !Num_Approx( basis[v_j], 1.0 )) || (v_i != v_j && !Num_Approx( basis[v_j], 0.0 )) ) {
               break;
            }
         }
         if( v_j < nVerts )
            break;
      }
      if( v_i < nVerts )
         break;
   }
   pcu_check_true( e_i == nEls );

   Stg_Class_Delete( inc );

   _Stg_Component_Delete( mesh );
}

void TrilinearElementTypeSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, TrilinearElementTypeSuiteData );
   pcu_suite_setFixtures( suite, TrilinearElementTypeSuite_Setup, TrilinearElementTypeSuite_Teardown );
   pcu_suite_addTest( suite, TrilinearElementTypeSuite_TestShape );
}


