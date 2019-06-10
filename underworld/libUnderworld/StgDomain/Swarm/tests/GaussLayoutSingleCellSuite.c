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

#include "GaussLayoutSingleCellSuite.h"

struct _Particle {
   __IntegrationPoint
};

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} GaussLayoutSingleCellSuiteData;

void GaussLayoutSingleCellSuite_Setup( GaussLayoutSingleCellSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void GaussLayoutSingleCellSuite_Teardown( GaussLayoutSingleCellSuiteData* data ) {
}

void GaussLayoutSingleCellSuite_Test1ParticlePerDim_3D( GaussLayoutSingleCellSuiteData* data ) {
   unsigned                   nDims;
   unsigned                   gaussParticles[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   ExtensionManager_Register* extensionMgr_Register;
   GaussParticleLayout*       gaussParticleLayout;
   SingleCellLayout*          singleCellLayout;
   Swarm*                     swarm;
   int                        procToWatch = data->nProcs > 1 ? 1 : 0;
   Cell_PointIndex            count;
   double                     x,y,z,w;
   unsigned int               p;
   Stream*                    stream;
   Particle_InCellIndex       particlesPerDim[3] = {1, 1, 1};
   Bool                       dimExists[] = { True, True, True };   
   char                       expected_file[PCU_PATH_MAX];
   
   if( data->rank == procToWatch ) {   
      stream = Journal_Register( Info_Type, (Name)"1ParticlePerDim_3D"  );
      Stream_RedirectFile( stream, "1ParticlePerDim_3D.dat" );

      nDims = 3;
      gaussParticles[0] = 2; gaussParticles[1] = 1; gaussParticles[2] = 3;
      minCrds[0] = 0.0; minCrds[1] = 0.0; minCrds[2] = 0.0;
      maxCrds[0] = 300.0; maxCrds[1] = 12.0; maxCrds[2] = 300.0;
   
      extensionMgr_Register = ExtensionManager_Register_New();

      /* Configure the element-cell-layout */
      singleCellLayout = SingleCellLayout_New( "singleCellLayout", NULL, dimExists, NULL, NULL );
   
      /* Configure the gauss-particle-layout */
      gaussParticleLayout = GaussParticleLayout_New( "gaussParticleLayout", NULL,
           LocalCoordSystem, True, nDims, particlesPerDim );

      swarm = Swarm_New( "testGaussSwarmSingleCell", NULL, singleCellLayout, gaussParticleLayout, nDims,
         sizeof(Particle), extensionMgr_Register, NULL, data->comm, NULL );
      
      /* Build the swarm */
      Stg_Component_Build( swarm, 0, False );
      Stg_Component_Initialise( swarm, 0, False );

      count = swarm->cellParticleCountTbl[0];
    
      for( p = 0; p < count; p++ ) {
         x = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[0]; 
         y = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[1]; 
         z = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[2];    
         w = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->weight;
         Journal_Printf( stream, "pId=%d : xi = { %f, %f, %f } weight = %f\n",p,x,y,z,w );   
      }   
      pcu_filename_expected( "testGaussLayoutSingleCell1ParticlePerDimOutput.expected", expected_file );
      pcu_check_fileEq( "1ParticlePerDim_3D.dat", expected_file );
      remove( "1ParticlePerDim_3D.dat" );

      Stg_Class_Delete( extensionMgr_Register );
      Stg_Component_Destroy( singleCellLayout, NULL, True );
      Stg_Component_Destroy( gaussParticleLayout, NULL, True );
      Stg_Component_Destroy( swarm, NULL, True );
   }
}

void GaussLayoutSingleCellSuite_Test2ParticlesPerDim_3D( GaussLayoutSingleCellSuiteData* data ) {
   unsigned                   nDims;
   unsigned                   gaussParticles[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   ExtensionManager_Register* extensionMgr_Register;
   GaussParticleLayout*       gaussParticleLayout;
   SingleCellLayout*          singleCellLayout;
   Swarm*                     swarm;
   int                        procToWatch = data->nProcs > 1 ? 1 : 0;
   Cell_PointIndex            count;
   double                     x,y,z,w;
   unsigned int               p;
   Stream*                    stream;
   Particle_InCellIndex       particlesPerDim[3] = {2, 2, 2};
   Bool                       dimExists[] = { True, True, True };   
   char                       expected_file[PCU_PATH_MAX];
   
   if( data->rank == procToWatch ) {   
      stream = Journal_Register( Info_Type, (Name)"2ParticlesPerDim_3D"  );
      Stream_RedirectFile( stream, "2ParticlesPerDim_3D.dat" );

      nDims = 3;
      gaussParticles[0] = 2; gaussParticles[1] = 1; gaussParticles[2] = 3;
      minCrds[0] = 0.0; minCrds[1] = 0.0; minCrds[2] = 0.0;
      maxCrds[0] = 300.0; maxCrds[1] = 12.0; maxCrds[2] = 300.0;
   
      extensionMgr_Register = ExtensionManager_Register_New();

      /* Configure the element-cell-layout */
      singleCellLayout = SingleCellLayout_New( "singleCellLayout", NULL, dimExists, NULL, NULL );
   
      /* Configure the gauss-particle-layout */
      gaussParticleLayout = GaussParticleLayout_New( "gaussParticleLayout", NULL,
           LocalCoordSystem, True, nDims, particlesPerDim );
      
      /* Configure the swarm */
      swarm = Swarm_New( "testGaussSwarmSingleCell", NULL, singleCellLayout, gaussParticleLayout, nDims,
         sizeof(Particle), extensionMgr_Register, NULL, data->comm, NULL );
      
      /* Build the swarm */
      Stg_Component_Build( swarm, 0, False );
      Stg_Component_Initialise( swarm, 0, False );

      count = swarm->cellParticleCountTbl[0];
       
      for( p = 0; p < count; p++ ) {
         x = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[0]; 
         y = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[1]; 
         z = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[2];    
         w = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->weight;
         Journal_Printf( stream, "pId=%d : xi = { %f, %f, %f } weight = %f\n",p,x,y,z,w );   
      }   
      pcu_filename_expected( "testGaussLayoutSingleCell2ParticlesPerDimOutput.expected", expected_file );
      pcu_check_fileEq( "2ParticlesPerDim_3D.dat", expected_file );
      remove( "2ParticlesPerDim_3D.dat" );

      Stg_Class_Delete( extensionMgr_Register );
      Stg_Component_Destroy( singleCellLayout, NULL, True );
      Stg_Component_Destroy( gaussParticleLayout, NULL, True );
      Stg_Component_Destroy( swarm, NULL, True );
   }
}

void GaussLayoutSingleCellSuite_Test3ParticlesPerDim_3D( GaussLayoutSingleCellSuiteData* data ) {
   unsigned                   nDims;
   unsigned                   gaussParticles[3];
   double                     minCrds[3];
   double                     maxCrds[3];
   ExtensionManager_Register* extensionMgr_Register;
   GaussParticleLayout*       gaussParticleLayout;
   SingleCellLayout*          singleCellLayout;
   Swarm*                     swarm;
   int                        procToWatch = data->nProcs > 1 ? 1 : 0;
   Cell_PointIndex            count;
   double                     x,y,z,w;
   unsigned int               p;
   Stream*                    stream;
   Particle_InCellIndex       particlesPerDim[3] = {3, 3, 3};
   Bool                       dimExists[] = { True, True, True };   
   char                       expected_file[PCU_PATH_MAX];

   if( data->rank == procToWatch ) {   
      stream = Journal_Register( Info_Type, (Name)"3ParticlesPerDim_3D"  );
      Stream_RedirectFile( stream, "3ParticlesPerDim_3D.dat" );

      nDims = 3;
      gaussParticles[0] = 2; gaussParticles[1] = 1; gaussParticles[2] = 3;
      minCrds[0] = 0.0; minCrds[1] = 0.0; minCrds[2] = 0.0;
      maxCrds[0] = 300.0; maxCrds[1] = 12.0; maxCrds[2] = 300.0;
   
      extensionMgr_Register = ExtensionManager_Register_New();

      /* Configure the element-cell-layout */
      singleCellLayout = SingleCellLayout_New( "singleCellLayout", NULL, dimExists, NULL, NULL );
   
      /* Configure the gauss-particle-layout */
      gaussParticleLayout = GaussParticleLayout_New( "gaussParticleLayout", NULL,
           LocalCoordSystem, True, nDims, particlesPerDim );
   
      /* Configure the swarm */
      swarm = Swarm_New( "testGaussSwarmSingleCell", NULL, singleCellLayout, gaussParticleLayout, nDims,
         sizeof(Particle), extensionMgr_Register, NULL, data->comm, NULL );
      
      /* Build the swarm */
      Stg_Component_Build( swarm, 0, False );
      Stg_Component_Initialise( swarm, 0, False );
   
      count = swarm->cellParticleCountTbl[0];
    
      for( p = 0; p < count; p++ ) {
         x = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[0]; 
         y = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[1]; 
         z = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[2];    
         w = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->weight;
         Journal_Printf( stream, "pId=%d : xi = { %f, %f, %f } weight = %f\n",p,x,y,z,w );   
      }   
      pcu_filename_expected( "testGaussLayoutSingleCell3ParticlesPerDimOutput.expected", expected_file );
      pcu_check_fileEq( "3ParticlesPerDim_3D.dat", expected_file );
      remove( "3ParticlesPerDim_3D.dat" );

      Stg_Class_Delete( extensionMgr_Register );
      Stg_Component_Destroy( singleCellLayout, NULL, True );
      Stg_Component_Destroy( gaussParticleLayout, NULL, True );
      Stg_Component_Destroy( swarm, NULL, True );
   }
}


void GaussLayoutSingleCellSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, GaussLayoutSingleCellSuiteData );
   pcu_suite_setFixtures( suite, GaussLayoutSingleCellSuite_Setup, GaussLayoutSingleCellSuite_Teardown );
   pcu_suite_addTest( suite, GaussLayoutSingleCellSuite_Test1ParticlePerDim_3D );
   pcu_suite_addTest( suite, GaussLayoutSingleCellSuite_Test2ParticlesPerDim_3D );
   pcu_suite_addTest( suite, GaussLayoutSingleCellSuite_Test3ParticlesPerDim_3D );
}



