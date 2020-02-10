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

#include "SpaceFillerParticleLayoutSuite.h"

struct _Particle {
   __IntegrationPoint
};

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} SpaceFillerParticleLayoutSuiteData;

Mesh* SpaceFillerParticleLayoutSuite_BuildMesh( unsigned nDims, unsigned* size, double* minCrds, double* maxCrds, ExtensionManager_Register* emReg ) {
   CartesianGenerator* gen;
   Mesh*               mesh;

   gen = CartesianGenerator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, nDims );
   CartesianGenerator_SetTopologyParams( gen, size, 0, NULL, NULL );
   CartesianGenerator_SetGeometryParams( gen, minCrds, maxCrds );

   mesh = Mesh_New( "" );
   Mesh_SetExtensionManagerRegister( mesh, emReg );
   Mesh_SetGenerator( mesh, gen );
   Mesh_SetAlgorithms( mesh, Mesh_RegularAlgorithms_New( "", NULL ) );

   Stg_Component_Build( mesh, NULL, False );
   Stg_Component_Initialise( mesh, NULL, False );

   FreeObject( mesh->generator );

   return mesh;
}

void SpaceFillerParticleLayoutSuite_Setup( SpaceFillerParticleLayoutSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
   
}

void SpaceFillerParticleLayoutSuite_Teardown( SpaceFillerParticleLayoutSuiteData* data ) {
   Journal_Enable_AllTypedStream( True );
}

void SpaceFillerParticleLayoutSuite_TestSpaceFillerParticle( SpaceFillerParticleLayoutSuiteData* data ) {
   ExtensionManager_Register* extensionMgr_Register;
   SpaceFillerParticleLayout* particleLayout;
   ElementCellLayout*         elementCellLayout;
   Mesh*                      mesh;
   Swarm*                     swarm;
   Stream*                    stream;
   unsigned                   nDims;
   unsigned                   meshSize[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   int                        procToWatch = data->nProcs > 1 ? 1 : 0;
   char                       expected_file[PCU_PATH_MAX];

   if( data->rank == procToWatch ) {
      nDims = 3;
      meshSize[0] = 4;    meshSize[1] = 2;    meshSize[2] = 1;
      minCrds[0] = 0.0;   minCrds[1] = 0.0;   minCrds[2] = 0.0;
      maxCrds[0] = 400.0; maxCrds[1] = 200.0; maxCrds[2] = 100.0;

      extensionMgr_Register = ExtensionManager_Register_New();
      mesh = SpaceFillerParticleLayoutSuite_BuildMesh( nDims, meshSize, minCrds, maxCrds, extensionMgr_Register );
      
      elementCellLayout = ElementCellLayout_New( "spaceFillerParticlElementCellLayout", NULL, mesh );
      particleLayout = SpaceFillerParticleLayout_New( "spaceFillerParticleLayout", NULL, GlobalCoordSystem, False, SpaceFillerParticleLayout_Invalid, 20, nDims );
   
      swarm = Swarm_New( "testSpaceFIllerParticle", NULL, elementCellLayout, particleLayout, nDims, sizeof(Particle),
         extensionMgr_Register, NULL, data->comm, NULL );
 
      Stg_Component_Build( swarm, 0, False );
      Stg_Component_Initialise( swarm, 0, False );

      Journal_Enable_AllTypedStream( True );
      stream = Journal_Register( Info_Type, (Name)"TestSpaceFillerParticle"  );
      Stream_RedirectFile( stream, "spaceFillerParticle.dat" );
      Swarm_PrintParticleCoords_ByCell( swarm, stream );
      Journal_Enable_AllTypedStream( False );

      pcu_filename_expected( "testSpaceFillerParticleLayoutOutput.expected", expected_file );
      pcu_check_fileEq( "spaceFillerParticle.dat", expected_file );
      remove( "spaceFillerParticle.dat" );

      Stg_Class_Delete( extensionMgr_Register );
      /*Stg_Component_Destroy( mesh, NULL, True );*/
      Stg_Component_Destroy( elementCellLayout, NULL, True );
      Stg_Component_Destroy( particleLayout, NULL, True );
      Stg_Component_Destroy( swarm, NULL, True );
   }
}

void SpaceFillerParticleLayoutSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, SpaceFillerParticleLayoutSuiteData );
   pcu_suite_setFixtures( suite, SpaceFillerParticleLayoutSuite_Setup, SpaceFillerParticleLayoutSuite_Teardown );
   pcu_suite_addTest( suite, SpaceFillerParticleLayoutSuite_TestSpaceFillerParticle );
}


