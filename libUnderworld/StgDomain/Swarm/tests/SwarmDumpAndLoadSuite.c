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

#include "SwarmDumpAndLoadSuite.h"

#define CURR_MODULE_NAME "DomainContext.c"

struct _Particle {
   __IntegrationPoint;
};

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} SwarmDumpAndLoadSuiteData;

void SwarmDumpAndLoadSuite_UpdateParticlePositionsTowardsAttractor( DomainContext* context );

void SwarmDumpAndLoadSuite_Setup( SwarmDumpAndLoadSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void SwarmDumpAndLoadSuite_Teardown( SwarmDumpAndLoadSuiteData* data ) {   
}

void SwarmDumpAndLoadSuite_TestSwarmDumpAndLoad( SwarmDumpAndLoadSuiteData* data ) {
   int                   ii;
   Swarm*                swarm;
   Swarm*                newSwarm = NULL;
   Swarm*                swarmList[1];
   SwarmDump*            swarmDumper = NULL;
   DomainContext*        context;
   Dictionary*           dictionary;
   Stg_ComponentFactory* cf;
   Stream*               stream;
   FileParticleLayout*   fileParticleLayout = NULL;
   char                  input_file[PCU_PATH_MAX];
   char                  output_file[PCU_PATH_MAX];
   char                  filenameTemp[PCU_PATH_MAX];   
   double                diffSumX, diffSumY, diffSumZ;
   double                totSumX, totSumY, totSumZ;
   double                gdiffSumX, gdiffSumY, gdiffSumZ;
   double                gtotSumX, gtotSumY, gtotSumZ;
   double                tolerance = 1e-5;
   Particle_InCellIndex  cParticle_I;
   IntegrationPoint      integrationPoint;
   SwarmVariable*        posVariable;
   SwarmVariable*        posVariableNew;
   Variable_Register*    varReg = Variable_Register_New();

   stream = Journal_Register( Info_Type, (Name)"SwarmDumpStream" );
   Journal_Enable_TypedStream( DebugStream_Type, False );
   Stream_EnableBranch( Swarm_Debug, False );
   
   Stream_Enable( Journal_Register( Info_Type, (Name)ParticleCommHandler_Type ), False );
   Stream_Enable( Journal_Register( Info_Type, (Name)SwarmDump_Type ), False );
   
   pcu_filename_input( "testSwarmDump.xml", input_file );
   /* setup from XML */
   cf = stgMainInitFromXML( input_file, data->comm, NULL );
   context = (DomainContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context" );
   
   dictionary = context->dictionary;

   swarm = (Swarm*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm" );

   /*
    * Create a swarmVariable, as this will be required for the HDF5 checkpointing
    * (else it doesn't know about the required variable to save) 
    */   
   posVariable = Swarm_NewVectorVariable(
      swarm,
      "Position",
      GetOffsetOfMember( integrationPoint, xi ),
      Variable_DataType_Double,
      swarm->dim,
      "PositionX",
      "PositionY",
      "PositionZ" );

   posVariable->isCheckpointedAndReloaded = True;
   Stg_Component_Build( posVariable, data, False );
   
   stgMainBuildAndInitialise( cf );

   /* 
    * Advect particles 
    */
   SwarmDumpAndLoadSuite_UpdateParticlePositionsTowardsAttractor(context);
   Swarm_UpdateAllParticleOwners( swarm );
   swarmList[0] = swarm;
   swarmDumper = SwarmDump_New( "SwarmDumpSuiteDumper", context, swarmList, 1, True, True, False );

   /*
    * Dump swarm 
    */
   SwarmDump_Execute( swarmDumper, context );

   /*
    * Create a barrier, as other procs may try to read from files even though they are not created yet 
    */
   MPI_Barrier( data->comm );
#ifdef READ_HDF5
   sprintf( output_file, "%s/%s.%05d", context->outputPath, swarm->name, context->timeStep );
#else
   sprintf( output_file, "%s/%s.%05d.dat", context->outputPath, swarm->name, context->timeStep );
#endif

   /*
    * Create a fileParticleLayout to load files from file 
    */
   fileParticleLayout = FileParticleLayout_New(
      "fileParticleLayout",
      NULL,
      GlobalCoordSystem,
      False,
      0,
      0.0,
      output_file,
      data->nProcs );

   Stg_Component_AssignFromXML( fileParticleLayout, cf, 0, False );
   
   newSwarm = Swarm_New(
      "testSwarm2",
      (AbstractContext*)context,
      (ElementCellLayout*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"elementCellLayout" ),
      fileParticleLayout,
      3,
      sizeof(Particle),
      extensionMgr_Register,
      varReg,
      data->comm,
      NULL );

   /*
    * As with for the swarmDump,  create a swarmVariable,
    * as this will be required for the HDF5 checkpointing 
    * (else it doesn't know about the required variable to save) 
    */   
   posVariableNew = Swarm_NewVectorVariable(
      newSwarm,
      "Position",
      GetOffsetOfMember( integrationPoint, xi ),
      Variable_DataType_Double,
      newSwarm->dim,
      "PositionX",
      "PositionY",
      "PositionZ" );

   posVariableNew->isCheckpointedAndReloaded = True;
   Stg_Component_Build( posVariableNew, data, False );
   
   Stg_Component_Build( newSwarm, 0, False );
   Stg_Component_Initialise( newSwarm, 0, False );
   
   /*
    * Now check that two swarms coincide 
    */
   diffSumX = 0;
   diffSumY = 0;
   diffSumZ = 0;
   totSumX = 0;
   totSumY = 0;
   totSumZ = 0;
   pcu_check_true(swarm->particleLocalCount == newSwarm->particleLocalCount);

   for ( cParticle_I=0; cParticle_I < swarm->particleLocalCount; cParticle_I++ ) {
      pcu_check_true( swarm->particles[cParticle_I].owningCell == newSwarm->particles[cParticle_I].owningCell);
      diffSumX += pow(swarm->particles[cParticle_I].xi[0] - newSwarm->particles[cParticle_I].xi[0], 2);
      diffSumY += pow(swarm->particles[cParticle_I].xi[1] - newSwarm->particles[cParticle_I].xi[1], 2);
      diffSumZ += pow(swarm->particles[cParticle_I].xi[2] - newSwarm->particles[cParticle_I].xi[2], 2);
      totSumX += pow(swarm->particles[cParticle_I].xi[0], 2);
      totSumY += pow(swarm->particles[cParticle_I].xi[1], 2);
      totSumZ += pow(swarm->particles[cParticle_I].xi[2], 2);
   }

   (void)MPI_Allreduce( &diffSumX, &gdiffSumX, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   (void)MPI_Allreduce( &diffSumY, &gdiffSumY, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   (void)MPI_Allreduce( &diffSumZ, &gdiffSumZ, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   (void)MPI_Allreduce( &totSumX, &gtotSumX, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   (void)MPI_Allreduce( &totSumY, &gtotSumY, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   (void)MPI_Allreduce( &totSumZ, &gtotSumZ, 1, MPI_DOUBLE, MPI_SUM, swarm->comm );
   
   pcu_check_true( gdiffSumX/gtotSumX < tolerance );
   pcu_check_true( gdiffSumY/gtotSumY < tolerance );
   pcu_check_true( gdiffSumZ/gtotSumZ < tolerance );

   /* remove files */
   if( data->rank == 0 ) {
#ifdef READ_HDF5
      for( ii = 0 ; ii < data->nProcs ; ii++ ) {
         /* get the swarm checkpointing filename */
         if(data->nProcs == 1)
            sprintf( filenameTemp, "%s.h5", output_file );
         else 
            sprintf( filenameTemp, "%s.%dof%d.h5", output_file, ii+1, data->nProcs );
         remove(filenameTemp);
      }
#else
      remove(output_file);
#endif   
      remove(context->outputPath);
   }
   
   Stg_Class_Delete( varReg );

   Stg_Component_Build( posVariable, NULL, False );
   Stg_Component_Destroy( fileParticleLayout, NULL, False );
   Stg_Component_Destroy( posVariableNew, NULL, False );
   Stg_Component_Destroy( newSwarm, NULL, False );

   Stg_Class_Delete( posVariable );
   Stg_Class_Delete( fileParticleLayout );
   Stg_Class_Delete( posVariableNew );
   Stg_Class_Delete( newSwarm );
   stgMainDestroy( cf );

}

void SwarmDumpAndLoadSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, SwarmDumpAndLoadSuiteData );
   pcu_suite_setFixtures( suite, SwarmDumpAndLoadSuite_Setup, SwarmDumpAndLoadSuite_Teardown );
   pcu_suite_addTest( suite, SwarmDumpAndLoadSuite_TestSwarmDumpAndLoad );
}

void SwarmDumpAndLoadSuite_UpdateParticlePositionsTowardsAttractor( DomainContext* context ) {
   Cell_LocalIndex      lCell_I;
   Particle_InCellIndex cParticle_I;
   Particle*            currParticle;
   Index                dim_I;
   Mesh*                mesh = (Mesh*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"mesh-linear" );
   Swarm*               swarm = (Swarm*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm" );
   Coord                attractorPoint;
   double               minCrds[3], maxCrds[3];

   Mesh_GetGlobalCoordRange( mesh, minCrds, maxCrds );

   for ( dim_I=0; dim_I < 3; dim_I++ ) 
      attractorPoint[dim_I] = ( maxCrds[dim_I] - minCrds[dim_I] ) / 3;

   for ( lCell_I=0; lCell_I < swarm->cellLocalCount; lCell_I++ ) {
      for ( cParticle_I=0; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
         Coord movementVector = {0,0,0};
         Coord newParticleCoord = {0,0,0};
         Coord* oldCoord;
         
         currParticle = (Particle*)Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );
         oldCoord = &currParticle->xi;
         
         for ( dim_I=0; dim_I < 3; dim_I++ ) {
            movementVector[dim_I] = ( attractorPoint[dim_I] - (*oldCoord)[dim_I] ) / 3;
            newParticleCoord[dim_I] = (*oldCoord)[dim_I] + movementVector[dim_I];
         }

         for ( dim_I=0; dim_I < 3; dim_I++ ) {
            currParticle->xi[dim_I] = newParticleCoord[dim_I];
         }
      }
   }
}
