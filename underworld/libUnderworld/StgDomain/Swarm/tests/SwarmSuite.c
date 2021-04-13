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

#include "SwarmSuite.h"

struct _Particle {
   __GlobalParticle
};

typedef struct {
   unsigned                   nDims;
   unsigned                   meshSize[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   ExtensionManager_Register* extensionMgr_Register;
   Mesh*                      mesh;
   ElementCellLayout*         elementCellLayout;
   RandomParticleLayout*      randomParticleLayout;
   Swarm*                     swarm;
   MPI_Comm                   comm;
   int                        rank;
   int                        nProcs;
} SwarmSuiteData;

Mesh* SwarmSuite_BuildMesh( unsigned nDims, unsigned* size, double* minCrds, double* maxCrds, ExtensionManager_Register* emReg ) {
   CartesianGenerator* gen;
   Mesh*               mesh;

   gen = CartesianGenerator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, nDims );
   CartesianGenerator_SetTopologyParams( gen, size, 0, NULL, NULL );
   CartesianGenerator_SetGeometryParams( gen, minCrds, maxCrds );
   MeshGenerator_SetIncidenceState( gen, nDims, nDims, True );

   mesh = Mesh_New( "" );
   Mesh_SetExtensionManagerRegister( mesh, emReg );
   Mesh_SetGenerator( mesh, gen );

   Stg_Component_Build( mesh, NULL, False );
   Stg_Component_Initialise( mesh, NULL, False );

   FreeObject( mesh->generator );

   return mesh;
}

Bool SwarmSuite_TestParticleSearchFunc( Swarm* swarm, Coord coord, Stream* stream ) {
   double          distance;
   Particle_Index  closestParticle_I;
   Particle_Index  lParticle_I;
   GlobalParticle* particle;

   Journal_Printf( stream, "Testing coord %g %g %g\n", coord[ I_AXIS ], coord[ J_AXIS ], coord[ K_AXIS ] );

   closestParticle_I = Swarm_FindClosestParticle( swarm, 3, coord, &distance );

   if ( closestParticle_I < swarm->particleLocalCount ) {
      Journal_Printf( stream, "Closest Particle is %u with distance %g\n", closestParticle_I, distance );

      for ( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount ; lParticle_I++ ) {
         /* don't bother with testing same particle */
         if (lParticle_I == closestParticle_I)
            continue;

         particle = (GlobalParticle*)Swarm_ParticleAt( swarm , lParticle_I );

         /* Test if particle is closer */
         if (distance > StGermain_DistanceBetweenPoints( coord, particle->coord, 3 ) ) {
            Journal_Printf( stream, "Particle %u is closer to coord and has distance %g.\n" , 
                  lParticle_I, StGermain_DistanceBetweenPoints( coord, particle->coord, 3 ) );

            return False;
         }
      }
   }
   else
      Journal_Printf( stream, "Coord not found on this processor.\n" );
 
   return True;
}
   
void SwarmSuite_Setup( SwarmSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );

   data->nDims = 3;
   data->meshSize[0] = 2; data->meshSize[1] =  3; data->meshSize[2] = 2;
   data->minCrds[0] = 0.0; data->minCrds[1] = 0.0; data->minCrds[2] = 0.0;
   data->maxCrds[0] = 300.0; data->maxCrds[1] = 12.0; data->maxCrds[2] = 300.0;

   /* Init mesh */
   data->extensionMgr_Register = ExtensionManager_Register_New();
   data->mesh = SwarmSuite_BuildMesh( data->nDims, data->meshSize, data->minCrds, data->maxCrds, data->extensionMgr_Register );
   
   /* Configure the random-particle-layout */
   data->randomParticleLayout = RandomParticleLayout_New( "randomParticleCellLayout", NULL, GlobalCoordSystem, False, 4, 13 );
   
   /* Configure the element-cell-layout */
   data->elementCellLayout = ElementCellLayout_New( "elementCellLayout", NULL, data->mesh );
   
   /* Configure the swarm */
   data->swarm = Swarm_New( "testSwarm", NULL, data->elementCellLayout, data->randomParticleLayout, 3, sizeof(Particle),
      data->extensionMgr_Register, NULL, MPI_COMM_WORLD, NULL );
   
   /* Build the swarm */
   Stg_Component_Build( data->swarm, 0, False );
   Stg_Component_Initialise( data->swarm, 0, False );
}

void SwarmSuite_Teardown( SwarmSuiteData* data ) {
   /* Destroy stuff */
   _Stg_Component_Delete( data->swarm );
   _Stg_Component_Delete( data->randomParticleLayout );
   _Stg_Component_Delete( data->elementCellLayout );
   Stg_Class_Delete( data->extensionMgr_Register );

   Journal_Enable_AllTypedStream( True );
}

void SwarmSuite_TestParticleSearch( SwarmSuiteData* data ) {
   double  coord[3];
   int     procToWatch = data->nProcs > 1 ? 1 : 0;
   Stream* stream = Journal_Register( Info_Type, (Name)"TestParticleSearch");
   
   if( data->rank == procToWatch ) {
      if( data->nProcs == 1 ) {
         coord[0] = 0.60*( data->maxCrds[0] - data->minCrds[0] );
         coord[1] = 0.20*( data->maxCrds[1] - data->minCrds[1] );
         coord[2] = 0.82*( data->maxCrds[2] - data->minCrds[2]  );
         pcu_check_true( SwarmSuite_TestParticleSearchFunc( data->swarm, coord, stream ) );
      
         coord[0] = 0.20*( data->maxCrds[0] - data->minCrds[0] );
         coord[1] = 0.90*( data->maxCrds[1] - data->minCrds[1] );
         coord[2] = 0.12*( data->maxCrds[2] - data->minCrds[2] );
         pcu_check_true( SwarmSuite_TestParticleSearchFunc( data->swarm, coord, stream ) );
      }   
   }
}

void SwarmSuite_TestParticleCoords( SwarmSuiteData* data ) {
   char    expected_file[PCU_PATH_MAX];
   int     procToWatch = data->nProcs > 1 ? 1 : 0;
   Stream* stream; 

   if( data->rank == procToWatch ) {
      Journal_Enable_AllTypedStream( True );
      stream = Journal_Register( Info_Type, (Name)"TestParticleCorrds" ); 
      Stream_RedirectFile( stream, "testParticleCoords.dat" );
      Swarm_PrintParticleCoords( data->swarm, stream );
      Journal_Printf( stream, "\n" );
      Swarm_PrintParticleCoords_ByCell( data->swarm, stream );
      Journal_Enable_AllTypedStream( False );   
      pcu_filename_expected( "testSwarmOutput.expected", expected_file );
      pcu_check_fileEq( "testParticleCoords.dat", expected_file );
      remove( "testParticleCoords.dat" );
   }
}

void SwarmSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, SwarmSuiteData );
   pcu_suite_setFixtures( suite, SwarmSuite_Setup, SwarmSuite_Teardown );
   pcu_suite_addTest( suite, SwarmSuite_TestParticleSearch );
   pcu_suite_addTest( suite, SwarmSuite_TestParticleCoords );
}


