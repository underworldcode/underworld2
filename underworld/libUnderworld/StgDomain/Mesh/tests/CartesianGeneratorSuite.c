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
#include <StgDomain/Mesh/src/Mesh.h>
#include "CartesianGeneratorSuite.h"

typedef struct {
   Mesh* mesh;
} CartesianGeneratorSuiteData;

void CartesianGeneratorSuite_Setup( CartesianGeneratorSuiteData* data ) {
   CartesianGenerator* gen;
   int                 nRanks;
   unsigned            sizes[3];
   double              minCrd[3];
   double              maxCrd[3];

   Journal_Enable_AllTypedStream( False );

   insist( MPI_Comm_size( MPI_COMM_WORLD, &nRanks ), == MPI_SUCCESS );
   sizes[0] = sizes[1] = sizes[2] = nRanks * 4;
   minCrd[0] = minCrd[1] = minCrd[2] = 0.0;
   maxCrd[0] = minCrd[1] = minCrd[2] = (double)nRanks;

   gen = CartesianGenerator_New( "", NULL );
   MeshGenerator_SetDimSize( gen, 3 );
   CartesianGenerator_SetShadowDepth( gen, 1 );
   CartesianGenerator_SetTopologyParams( gen, sizes, 0, NULL, NULL );
   CartesianGenerator_SetGeometryParams( gen, minCrd, maxCrd );

   data->mesh = Mesh_New( "" );
   CartesianGenerator_Generate( gen, data->mesh, NULL );
   Stg_Component_Build(data->mesh, NULL, False);
   Stg_Component_Initialise(data->mesh, NULL, False);
}

void CartesianGeneratorSuite_Teardown( CartesianGeneratorSuiteData* data ) {
   Stg_Component_Destroy( data->mesh, NULL, True );

   Journal_Enable_AllTypedStream( True );
}

void CartesianGeneratorSuite_TestElementVertexInc( CartesianGeneratorSuiteData* data ) {
   unsigned dim = Mesh_GetDimSize( data->mesh );
   Sync*    elSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, dim );
   Sync*    vertSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, MT_VERTEX );
   Grid*    elGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->elGridId );
   Grid*    vertGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->vertGridId );
   IArray*  inc = IArray_New();
   int*     incVerts;
   unsigned nIncVerts;
   unsigned el_i;
   unsigned gEl;
   unsigned dimInds[3];
   unsigned gNode0, gNode1, gNode2;
   int      checkNodes;

   for( el_i = 0; el_i < Mesh_GetLocalSize( data->mesh, dim ); el_i++ ) {
      gEl = Sync_DomainToGlobal( elSync, el_i );
      Grid_Lift( elGrid, gEl, dimInds );   

      MeshTopology_GetIncidence( (IGraph*)data->mesh->topo, dim, el_i, MT_VERTEX, inc );
      nIncVerts = IArray_GetSize( inc );
      incVerts = IArray_GetPtr( inc );

      pcu_check_true( nIncVerts == 8 );

      gNode0 = Grid_Project( vertGrid, dimInds );
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]++;
      gNode0++;
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]--; dimInds[1]++;
      gNode0--; gNode0 += vertGrid->sizes[0];
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[2] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]++;
      gNode0++;
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[3] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]--; dimInds[1]--; dimInds[2]++;
      gNode0--; gNode0 -= vertGrid->sizes[0]; gNode0 += vertGrid->sizes[0] * vertGrid->sizes[1];
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[4] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]++;
      gNode0++;
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[5] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]--; dimInds[1]++;
      gNode0--; gNode0 += vertGrid->sizes[0];
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[6] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );

      dimInds[0]++;
      gNode0++;
      gNode1 = Grid_Project( vertGrid, dimInds );
      gNode2 = Sync_DomainToGlobal( vertSync, incVerts[7] );
      checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
      pcu_check_true( checkNodes );
   }

   Stg_Class_Delete( inc );
}

void CartesianGeneratorSuite_TestEdgeVertexInc( CartesianGeneratorSuiteData* data ) {
   Grid*    elGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->elGridId );
   Grid*    vertGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->vertGridId );
   Grid*    edgeGrid_0 = Grid_New();
   Grid*    edgeGrid_1 = Grid_New();
   Grid*    edgeGrid_2 = Grid_New();
   Sync*    vertSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, MT_VERTEX );
   Sync*    edgeSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, MT_EDGE );
   IArray*  inc = IArray_New();
   int*     incVerts;
   unsigned nIncVerts;
   unsigned dim = ((IGraph*)data->mesh->topo)->nDims;
   unsigned sizes[3];
   unsigned edge_i;
   unsigned gEdge;
   unsigned dimInds[3];
   unsigned gNode0, gNode1, gNode2;
   int      checkNodes;

   sizes[0] = elGrid->sizes[0];
   sizes[1] = elGrid->sizes[1] + 1;
   sizes[2] = elGrid->sizes[2] + 1;
   Grid_SetNumDims( edgeGrid_0, dim );
   Grid_SetSizes( edgeGrid_0, sizes );
   sizes[0] = elGrid->sizes[0] + 1;
   sizes[1] = elGrid->sizes[1];
   sizes[2] = elGrid->sizes[2] + 1;
   Grid_SetNumDims( edgeGrid_1, dim );
   Grid_SetSizes( edgeGrid_1, sizes );
   sizes[0] = elGrid->sizes[0] + 1;
   sizes[1] = elGrid->sizes[1] + 1;
   sizes[2] = elGrid->sizes[2];
   Grid_SetNumDims( edgeGrid_2, dim );
   Grid_SetSizes( edgeGrid_2, sizes );

   for( edge_i = 0; edge_i < Sync_GetNumDomains( edgeSync ); edge_i++ ) {
      gEdge = Sync_DomainToGlobal( edgeSync, edge_i );

      MeshTopology_GetIncidence( (IGraph*)data->mesh->topo, MT_EDGE, edge_i, MT_VERTEX, inc );
      nIncVerts = IArray_GetSize( inc );
      incVerts = IArray_GetPtr( inc );

      pcu_check_true( nIncVerts == 2 );

      if( gEdge < edgeGrid_0->nPoints ) {
         Grid_Lift( edgeGrid_0, gEdge, dimInds );
         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0++;
         dimInds[0]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
      else if( gEdge < edgeGrid_0->nPoints + edgeGrid_1->nPoints ) {
         Grid_Lift( edgeGrid_1, gEdge - edgeGrid_0->nPoints, dimInds );
         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0 += vertGrid->sizes[0];
         dimInds[1]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
      else if( gEdge < edgeGrid_0->nPoints + edgeGrid_1->nPoints + edgeGrid_2->nPoints ) {
         Grid_Lift( edgeGrid_2, gEdge - edgeGrid_0->nPoints - edgeGrid_1->nPoints, dimInds );
         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0 += vertGrid->sizes[0] * vertGrid->sizes[1];
         dimInds[2]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
   }

   FreeObject( edgeGrid_0 );
   FreeObject( edgeGrid_1 );
   FreeObject( edgeGrid_2 );

   Stg_Class_Delete( inc );
}

void CartesianGeneratorSuite_TestFaceVertexInc( CartesianGeneratorSuiteData* data ) {
   Grid*    elGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->elGridId );
   Grid*    vertGrid = *(Grid**)Mesh_GetExtension( data->mesh, Grid**,  data->mesh->vertGridId );
   Grid*    faceGrid_0 = Grid_New();
   Grid*    faceGrid_1 = Grid_New();
   Grid*    faceGrid_2 = Grid_New();
   Sync*    vertSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, MT_VERTEX );
   Sync*    faceSync = (Sync*)IGraph_GetDomain( (IGraph*)data->mesh->topo, MT_FACE );
   IArray*  inc = IArray_New();
   int*     incVerts;
   unsigned nIncVerts;
   unsigned dim = ((IGraph*)data->mesh->topo)->nDims;
   unsigned sizes[3];
   unsigned face_i;
   unsigned gFace;
   unsigned dimInds[3];
   unsigned gNode0, gNode1, gNode2;
   int      checkNodes;

   sizes[0] = elGrid->sizes[0];
   sizes[1] = elGrid->sizes[1];
   sizes[2] = elGrid->sizes[2] + 1;
   Grid_SetNumDims( faceGrid_0, dim );
   Grid_SetSizes( faceGrid_0, sizes );
   sizes[0] = elGrid->sizes[0];
   sizes[1] = elGrid->sizes[1] + 1;
   sizes[2] = elGrid->sizes[2];
   Grid_SetNumDims( faceGrid_1, dim );
   Grid_SetSizes( faceGrid_1, sizes );
   sizes[0] = elGrid->sizes[0] + 1;
   sizes[1] = elGrid->sizes[1];
   sizes[2] = elGrid->sizes[2];
   Grid_SetNumDims( faceGrid_2, dim );
   Grid_SetSizes( faceGrid_2, sizes );

   for( face_i = 0; face_i < ((IGraph*)data->mesh->topo)->remotes[MT_FACE]->nDomains; face_i++ ) {
      gFace = Sync_DomainToGlobal( faceSync, face_i );

      MeshTopology_GetIncidence( (IGraph*)data->mesh->topo, MT_FACE, face_i, MT_VERTEX, inc );
      nIncVerts = IArray_GetSize( inc );
      incVerts = IArray_GetPtr( inc );

      pcu_check_true( nIncVerts == 4 );

      if( gFace < faceGrid_0->nPoints ) {
         Grid_Lift( faceGrid_0, gFace, dimInds );

         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0++;
         dimInds[0]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0--; gNode0 += vertGrid->sizes[0];
         dimInds[0]--; dimInds[1]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[2] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0++;
         dimInds[0]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[3] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
      else if( gFace < faceGrid_0->nPoints + faceGrid_1->nPoints ) {
         Grid_Lift( faceGrid_1, gFace - faceGrid_0->nPoints, dimInds );

         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0++;
         dimInds[0]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0--; gNode0 += vertGrid->sizes[0] * vertGrid->sizes[1];
         dimInds[0]--; dimInds[2]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[2] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0++;
         dimInds[0]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[3] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
      else if( gFace < faceGrid_0->nPoints + faceGrid_1->nPoints + faceGrid_2->nPoints ) {
         Grid_Lift( faceGrid_2, gFace - faceGrid_0->nPoints - faceGrid_1->nPoints, dimInds );

         gNode0 = Grid_Project( vertGrid, dimInds );
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[0] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0 += vertGrid->sizes[0];
         dimInds[1]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[1] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0 -= vertGrid->sizes[0]; gNode0 += vertGrid->sizes[0] * vertGrid->sizes[1];
         dimInds[1]--; dimInds[2]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[2] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );

         gNode0 += vertGrid->sizes[0];
         dimInds[1]++;
         gNode1 = Grid_Project( vertGrid, dimInds );
         gNode2 = Sync_DomainToGlobal( vertSync, incVerts[3] );
         checkNodes = (gNode0 == gNode1) && (gNode1 == gNode2);
         pcu_check_true( checkNodes );
      }
   }

   FreeObject( faceGrid_0 );
   FreeObject( faceGrid_1 );
   FreeObject( faceGrid_2 );

   Stg_Class_Delete( inc );
}
void CartesianGeneratorSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, CartesianGeneratorSuiteData );
   pcu_suite_setFixtures( suite, CartesianGeneratorSuite_Setup, CartesianGeneratorSuite_Teardown );
   pcu_suite_addTest( suite, CartesianGeneratorSuite_TestElementVertexInc );
   pcu_suite_addTest( suite, CartesianGeneratorSuite_TestEdgeVertexInc );
   pcu_suite_addTest( suite, CartesianGeneratorSuite_TestFaceVertexInc );
}


