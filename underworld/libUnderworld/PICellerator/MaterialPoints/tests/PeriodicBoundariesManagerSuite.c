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
#include <string.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include "PICellerator/PopulationControl/PopulationControl.h"
#include "PICellerator/Weights/Weights.h"
#include "PICellerator/MaterialPoints/MaterialPoints.h"

struct _Node {
   Coord coord;
};

struct _Element {
   Coord coord;
};

struct _Particle {
   __GlobalParticle
};

void UpdateParticlePositionsToLeft( Swarm* swarm );

typedef struct {
   PICelleratorContext*  context;
   Stg_ComponentFactory* cf;
} PeriodicBoundariesManagerSuiteData;

void PeriodicBoundariesManagerSuite_Setup( PeriodicBoundariesManagerSuiteData* data ) {
   char xmlInputFilename[PCU_PATH_MAX];

   pcu_filename_input( "testPeriodicBoundariesManager.xml", xmlInputFilename );
   data->cf = stgMainInitFromXML( xmlInputFilename, MPI_COMM_WORLD, NULL );
   stgMainBuildAndInitialise( data->cf );
   data->context = (PICelleratorContext*)LiveComponentRegister_Get( data->cf->LCRegister, (Name)"context" );
} 

void PeriodicBoundariesManagerSuite_Teardown( PeriodicBoundariesManagerSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void PeriodicBoundariesManagerSuite_TestAdvectOverLeftBoundary( PeriodicBoundariesManagerSuiteData* data ) {
   Swarm*                     swarm;
   PeriodicBoundariesManager* perBCsManager;
   Index                      timeStep;
   GlobalParticle*            currParticle;
   Particle_Index             lParticle_I;

   swarm = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"swarm" );
   perBCsManager = (PeriodicBoundariesManager*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"perBCsManager" );

   for ( timeStep=1; timeStep <= 10; timeStep++ ) {
      UpdateParticlePositionsToLeft( swarm );
      for ( lParticle_I = 0; lParticle_I < swarm->particleLocalCount ; lParticle_I++  ) {
         PeriodicBoundariesManager_UpdateParticle( perBCsManager, lParticle_I );
      }
      Swarm_UpdateAllParticleOwners( swarm );
   }

   /* After 10 timesteps, all the particles should have been advected from the left row of cells, to the right row,
    *  to a coord somewhere between 0.9 and 1.0 */   
   /* TODO: first check the particles all still exist */
   for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
      currParticle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );
      pcu_check_true( currParticle->coord[0] >= 0.9 && currParticle->coord[0] <= 1.0 ); 
   }
   /* TODO: check that their cell ownership is correct */
}

void PeriodicBoundariesManagerSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, PeriodicBoundariesManagerSuiteData );
   pcu_suite_setFixtures( suite, PeriodicBoundariesManagerSuite_Setup, PeriodicBoundariesManagerSuite_Teardown );
   pcu_suite_addTest( suite, PeriodicBoundariesManagerSuite_TestAdvectOverLeftBoundary );
}

void UpdateParticlePositionsToLeft( Swarm* swarm ) {
   Cell_LocalIndex      lCell_I;
   Particle_InCellIndex cParticle_I;
   GlobalParticle*      currParticle;
   Index                dim_I;
   Stream*              debugStream = Journal_Register( Debug_Type, (Name)"UpdateParticlesLeft" );

   Stream_Indent( debugStream );
   for ( lCell_I=0; lCell_I < swarm->cellLocalCount; lCell_I++  ) {
      Journal_Printf( debugStream, "Updating Particles positions in local cell %d:\n", lCell_I );
      for ( cParticle_I=0; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
         Coord movementVector = {0,0,0};
         Coord newParticleCoord = {0,0,0};
         Coord* oldCoord;

         currParticle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );
         oldCoord = &currParticle->coord;

         Stream_Indent( debugStream );
         Journal_Printf( debugStream, "Updating particleInCell %d:\n", cParticle_I );

         movementVector[I_AXIS] = -0.01;

         for ( dim_I=0; dim_I < 3; dim_I++ ) {
            newParticleCoord[dim_I] = (*oldCoord)[dim_I] + movementVector[dim_I];
         }

         Journal_Printf( debugStream, "Changing its coords from (%f,%f,%f)to (%f,%f,%f):\n",
            (*oldCoord)[0], (*oldCoord)[1], (*oldCoord)[2],
            newParticleCoord[0], newParticleCoord[1], newParticleCoord[2] );

         for ( dim_I=0; dim_I < 3; dim_I++ ) {
            currParticle->coord[dim_I] = newParticleCoord[dim_I];
         }
         Stream_UnIndent( debugStream );
      }
   }
   Stream_UnIndent( debugStream );
}


