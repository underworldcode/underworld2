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
#include <mpi.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h"
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "ElementCellLayoutSuite.h"

typedef struct {
   unsigned                   nDims;
   unsigned                   meshSize[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   ExtensionManager_Register* extensionMgr_Register;
   Mesh*                      mesh;
   ElementCellLayout*         elementCellLayout;
   MPI_Comm                   comm;
   int                        rank;
   int                        nProcs;
} ElementCellLayoutSuiteData;

Mesh* ElementCellLayout_BuildMesh( unsigned nDims, unsigned* size, double* minCrds, double* maxCrds, ExtensionManager_Register* emReg ) {
   CartesianGenerator* gen;
   Mesh*               mesh;
   unsigned            maxDecomp[3] = {1, 0, 1};

   gen = CartesianGenerator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, nDims );
   CartesianGenerator_SetTopologyParams( gen, size, 0, NULL, maxDecomp );
   CartesianGenerator_SetGeometryParams( gen, minCrds, maxCrds );

   mesh = Mesh_New( "" );
   Mesh_SetExtensionManagerRegister( mesh, emReg );
   Mesh_SetGenerator( mesh, gen );

   Stg_Component_Build( mesh, NULL, False );
   Stg_Component_Initialise( mesh, NULL, False );

   FreeObject( mesh->generator );

   return mesh;
}

void ElementCellLayoutSuite_Setup( ElementCellLayoutSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */   
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );

   data->nDims = 3;
   data->meshSize[0] = 2;
   data->meshSize[1] = 3;
   data->meshSize[2] = 2;
   data->minCrds[0] = 0.0;
   data->minCrds[1] = 0.0;
   data->minCrds[2] = 0.0;
   data->maxCrds[0] = 300.0;
   data->maxCrds[1] = 12.0;
   data->maxCrds[2] = 300.0;

   /* Init mesh */
   data->extensionMgr_Register = ExtensionManager_Register_New();
   data->mesh = ElementCellLayout_BuildMesh( data->nDims, data->meshSize, data->minCrds, data->maxCrds, data->extensionMgr_Register );
   
   /* Configure the element-cell-layout */
   data->elementCellLayout = ElementCellLayout_New( "elementCellLayout", NULL, data->mesh );
   Stg_Component_Build( data->elementCellLayout, NULL, False );
   Stg_Component_Initialise( data->elementCellLayout, NULL, False );
}

void ElementCellLayoutSuite_Teardown( ElementCellLayoutSuiteData* data ) {
   /* Destroy stuff */
   Stg_Class_Delete( data->extensionMgr_Register );
   Stg_Component_Destroy( data->elementCellLayout, NULL, True );
   /*Stg_Component_Destroy( data->mesh, NULL, True );*/

   Journal_Enable_AllTypedStream( True );
}

void ElementCellLayoutSuite_TestElementCellLayout( ElementCellLayoutSuiteData* data ) {
   int                 procToWatch = data->nProcs > 1 ? 1 : 0;
   Cell_Index          cell;
   Element_DomainIndex element;
   GlobalParticle      testParticle;
      
   if( data->rank == procToWatch ) {
      for( element = 0; element < Mesh_GetLocalSize( data->mesh, data->nDims ); element++ ) {
         Cell_PointIndex   count;
         Cell_Points       cellPoints;
   
         cell = CellLayout_MapElementIdToCellId( data->elementCellLayout, element );

         pcu_check_true( cell == element );

         count = data->elementCellLayout->_pointCount( data->elementCellLayout, cell );
         cellPoints = Memory_Alloc_Array( Cell_Point, count, "cellPoints" );
         /* for the element cell layout, the elements map to cells as 1:1, as such the "points" which define the cell as the
          * same as the "nodes" which define the element */
         data->elementCellLayout->_initialisePoints( data->elementCellLayout, cell, count, cellPoints );

         testParticle.coord[0] = ( (cellPoints[0])[0] + (cellPoints[1])[0] ) / 2;
         testParticle.coord[1] = ( (cellPoints[0])[1] + (cellPoints[2])[1] ) / 2;
         testParticle.coord[2] = ( (cellPoints[0])[2] + (cellPoints[4])[2] ) / 2;
         pcu_check_true( CellLayout_IsInCell( data->elementCellLayout, cell, &testParticle ) );

         testParticle.coord[0] = (cellPoints[count-2])[0] + 1;
         testParticle.coord[1] = (cellPoints[count-2])[1] + 1;
         testParticle.coord[2] = (cellPoints[count-2])[2] + 1;
         pcu_check_true( !CellLayout_IsInCell( data->elementCellLayout, cell, &testParticle ) );

         Memory_Free( cellPoints );
      }
   }
}

void ElementCellLayoutSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ElementCellLayoutSuiteData );
   pcu_suite_setFixtures( suite, ElementCellLayoutSuite_Setup, ElementCellLayoutSuite_Teardown );
   pcu_suite_addTest( suite, ElementCellLayoutSuite_TestElementCellLayout );
}


