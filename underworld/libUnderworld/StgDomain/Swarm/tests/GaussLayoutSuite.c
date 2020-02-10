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

#include "GaussLayoutSuite.h"

struct _Particle {
   __IntegrationPoint
};

typedef struct {
   unsigned nDims;
   unsigned meshSize[3];
   double   minCrds[3];
   double   maxCrds[3];
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} GaussLayoutSuiteData;

Mesh* GaussLayoutSuite_BuildMesh( unsigned nDims, unsigned* size, double* minCrds, double* maxCrds, ExtensionManager_Register* emReg ) {
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

void GaussLayoutSuite_Setup( GaussLayoutSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
   
   data->nDims = 3;
   data->meshSize[0] = 2;     data->meshSize[1] = 3;   data->meshSize[2] = 2;
   data->minCrds[0]  = 0.0;   data->minCrds[1] = 0.0;  data->minCrds[2] = 0.0;
   data->maxCrds[0]  = 300.0; data->maxCrds[1] = 12.0; data->maxCrds[2] = 300.0;
}

void GaussLayoutSuite_Teardown( GaussLayoutSuiteData* data ) {
   Journal_Enable_AllTypedStream( True );
}

void GaussLayoutSuite_TestDriver( GaussLayoutSuiteData* data, char* name,  char* expected, char* output, Particle_InCellIndex particlesPerDim[], int totalParticleCount ) {
   int procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {   
      ExtensionManager_Register* extensionMgr_Register;
      Mesh*                      mesh;
      GaussParticleLayout*       gaussParticleLayout;
      ElementCellLayout*         elementCellLayout;
      Swarm*                     swarm;
      Cell_PointIndex            count;
      double                     x,y,z;
      unsigned int               p, i, len;
      LocalParticle*             particle;
      Coord                      minCell;
      Coord                      maxCell;
      Stream*                    stream;
      char                       expected_file[PCU_PATH_MAX];

      /* Init mesh */
      extensionMgr_Register = ExtensionManager_Register_New();
      mesh = GaussLayoutSuite_BuildMesh( data->nDims, data->meshSize, data->minCrds, data->maxCrds, extensionMgr_Register );
   
      /* Configure the element-cell-layout */
      elementCellLayout = ElementCellLayout_New( "elementCellLayout", NULL, mesh );
      gaussParticleLayout = GaussParticleLayout_New( "gaussParticleLayout", NULL, LocalCoordSystem, True, data->nDims, particlesPerDim );

      /* Configure the swarm */
      swarm = Swarm_New( "testGaussSwarm", NULL, elementCellLayout, gaussParticleLayout, data->nDims, sizeof(Particle), extensionMgr_Register, NULL, data->comm, NULL );
      
      /* Build the swarm */
      Stg_Component_Build( swarm, 0, False );
      Stg_Component_Initialise( swarm, 0, False );
         
      len = (int) sizeof( swarm->cellParticleCountTbl );
      count = 0;

      Journal_Enable_AllTypedStream( True );
      stream = Journal_Register( Info_Type, (Name)name  );
      Stream_RedirectFile( stream, output );

      /* Checks that the particule count on each cell are the same. */
      for( i = 0; i < len; i++ ) {
         count = swarm->cellParticleCountTbl[i];
         pcu_check_true( count == totalParticleCount  );
      }   
      Swarm_GetCellMinMaxCoords( swarm, 4, minCell, maxCell );
      Journal_Printf( stream, "Particle per dim: %d %d %d\n", particlesPerDim[0], particlesPerDim[1], particlesPerDim[2]);   
            
      for( p = 0; p < count; p++ ) {
         particle = (LocalParticle*)Swarm_ParticleInCellAt( swarm, 4, p );
   
         /* convert to global coords */
         x = 0.5 * ( maxCell[0] - minCell[0] ) * ( particle->xi[0] + 1.0 ) + minCell[0];
         y = 0.5 * ( maxCell[1] - minCell[1] ) * ( particle->xi[1] + 1.0 ) + minCell[1];
         z = 0.5 * ( maxCell[2] - minCell[2] ) * ( particle->xi[2] + 1.0 ) + minCell[2];
         
         Journal_Printf( stream, "pId=%d : coords = { %.12f, %.12f, %.12f }, xi = { %.12f, %.12f, %.12f }\n", 
            p, x, y, z, particle->xi[0], particle->xi[1], particle->xi[2] );   
      }
      pcu_filename_expected( expected, expected_file );
      pcu_check_fileEq( output, expected_file );

      /* Destroy stuff */
      Stg_Class_Delete( extensionMgr_Register );
      _Stg_Component_Delete( gaussParticleLayout );
      _Stg_Component_Delete( elementCellLayout );
      _Stg_Component_Delete( swarm );
      remove( output );
   }
}

void GaussLayoutSuite_Test1ParticlePerDim_3D( GaussLayoutSuiteData* data ) {
   Particle_InCellIndex particlesPerDim[3] = {1, 1, 1};

   GaussLayoutSuite_TestDriver( data, "GaussLayoutSuite1Particle",
      "testGaussLayout1ParticlePerDimOutput.expected",
      "testGaussLayoutSuite1Particle.dat", particlesPerDim, 1 );
}

void GaussLayoutSuite_Test2ParticlesPerDim_3D( GaussLayoutSuiteData* data ) {
   Particle_InCellIndex particlesPerDim[3] = {2, 2, 2};

   GaussLayoutSuite_TestDriver( data, "GaussLayoutSuite2Particles",
      "testGaussLayout2ParticlesPerDimOutput.expected",
      "testGaussLayoutSuite2Particles.dat", particlesPerDim, 8 );
}

void GaussLayoutSuite_Test3ParticlesPerDim_3D( GaussLayoutSuiteData* data ) {
   Particle_InCellIndex particlesPerDim[3] = {3, 3, 3};

   GaussLayoutSuite_TestDriver( data, "GaussLayoutSuite3Particles",
      "testGaussLayout3ParticlesPerDimOutput.expected",
      "testGaussLayoutSuite3Particles.dat", particlesPerDim, 27 );
}

void GaussLayoutSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, GaussLayoutSuiteData );
   pcu_suite_setFixtures( suite, GaussLayoutSuite_Setup, GaussLayoutSuite_Teardown );
   pcu_suite_addTest( suite, GaussLayoutSuite_Test1ParticlePerDim_3D );
   pcu_suite_addTest( suite, GaussLayoutSuite_Test2ParticlesPerDim_3D );
   pcu_suite_addTest( suite, GaussLayoutSuite_Test3ParticlesPerDim_3D );
}


