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

#include "ShadowSyncSuite.h"

/* silly stgermain, I must define this */
#define CURR_MODULE_NAME "DomainContext.c"

struct _Particle {
   __GlobalParticle
   double velocity[3];
   double randomColour;
};

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} ShadowSyncSuiteData;

void ShadowSyncSuite_ValidateShadowing( DomainContext* context );

double ShadowSyncSuite_Dt( void* context ) {
   return 2.0;
}

void ShadowSyncSuite_Setup( ShadowSyncSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void ShadowSyncSuite_Teardown( ShadowSyncSuiteData* data ) {
   Journal_Enable_AllTypedStream( True );
}

void ShadowSyncSuite_TestShadowSync( ShadowSyncSuiteData* data ) {
   DomainContext*        context = NULL;
   Dictionary*           dictionary;
   Dictionary*           componentDict;
   Stg_ComponentFactory* cf;
   Swarm*                swarm = NULL;
   Particle              particle;
   Particle*             currParticle = NULL;
   Particle_Index        lParticle_I = 0;
   Dimension_Index       dim_I = 0;
   char                  input_file[PCU_PATH_MAX];
   unsigned              procToWatch;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   pcu_filename_input( "testSwarmParticleShadowSync.xml", input_file );

   cf = stgMainInitFromXML( input_file, data->comm, NULL );
   context = (DomainContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context"  );
   Stream_Enable( cf->infoStream, False );
   Stream_Enable( context->info, False );
   Stream_Enable( context->debug, False );
   Stream_Enable( context->verbose, False );

   dictionary = context->dictionary;
   Journal_ReadFromDictionary( dictionary );
   Dictionary_Add( dictionary, (Dictionary_Entry_Key)"procToWatch", Dictionary_Entry_Value_FromUnsignedInt( procToWatch )  );
   componentDict = Dictionary_GetDictionary( dictionary, "components" );
   assert( componentDict );

   KeyCall( context, context->constructExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(context,context->constructExtensionsK), context );

   swarm = (Swarm*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm"  );
   ExtensionManager_Add( swarm->particleExtensionMgr, (Name)"ParticleVelocity", sizeof(double[3])  );
   ExtensionManager_Add( swarm->particleExtensionMgr, (Name)"ParticleColour", sizeof(double)  );

   Swarm_NewVectorVariable(
      swarm,
      "Velocity",
      (ArithPointer) &particle.velocity - (ArithPointer) &particle,
      StgVariable_DataType_Double,
      swarm->dim,
      "VelocityX",
      "VelocityY",
      "VelocityZ" );

   Swarm_NewScalarVariable(
      swarm,
      "RandomColour",
      (ArithPointer) &particle.randomColour - (ArithPointer) &particle,
      StgVariable_DataType_Double );

   stgMainBuildAndInitialise( cf );

   /* for each particle, set a random colour */
   for ( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
      currParticle = (Particle*)Swarm_ParticleAt( swarm, lParticle_I );
      for ( dim_I=0; dim_I < 3; dim_I++ ) {
         currParticle->velocity[dim_I] = 0;
      }   
      currParticle->randomColour = ( (double)  rand() ) / RAND_MAX;
   }
   
   ContextEP_ReplaceAll( context, AbstractContext_EP_Dt, ShadowSyncSuite_Dt );
   ContextEP_Append( context, AbstractContext_EP_Sync, ShadowSyncSuite_ValidateShadowing );

   Stg_Component_Execute( context, 0, False );
   Stg_Component_Destroy( context, 0, False );

   stgMainDestroy( cf );
}

void ShadowSyncSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ShadowSyncSuiteData );
   pcu_suite_setFixtures( suite, ShadowSyncSuite_Setup, ShadowSyncSuite_Teardown );
   pcu_suite_addTest( suite, ShadowSyncSuite_TestShadowSync );
}

int ShadowSyncSuite_listCompareFunction( void *a, void *b ){
   if( a>b )
      return 1;
   else if( a<b )
      return -1;
   else
      return 0;
}

void ShadowSyncSuite_listDeleteFunction( void *a ){
}

void ShadowSyncSuite_ValidateShadowing( DomainContext* context ) {
   DomainContext *self = context;
   Swarm*        swarm = (Swarm*)LiveComponentRegister_Get( self->CF->LCRegister, (Name)"swarm" );

   Swarm_UpdateAllParticleOwners( swarm );

   if(swarm->nProc > 1 ) {
      int ii = 0, jj = 0;
      ShadowInfo*  cellShadowInfo = CellLayout_GetShadowInfo( swarm->cellLayout );
      ProcNbrInfo* procNbrInfo = cellShadowInfo->procNbrInfo;
      {
         MemoryPool         *requestPool = MemoryPool_New( MPI_Request, 100, 10 );
         MemoryPool         *particlePool = MemoryPool_NewFunc( swarm->particleExtensionMgr->finalSize, 100, 10 );
         LinkedList         *list = LinkedList_New( ShadowSyncSuite_listCompareFunction, NULL, NULL, ShadowSyncSuite_listDeleteFunction, LINKEDLIST_UNSORTED );
         LinkedList         *particleList = LinkedList_New( ShadowSyncSuite_listCompareFunction, NULL, NULL, ShadowSyncSuite_listDeleteFunction, LINKEDLIST_UNSORTED );
         LinkedListIterator *iter = LinkedListIterator_New( list );
         void               *data = NULL;
         {
            Neighbour_Index nbr_I;
            int             i = 0, j = 0;
            int             shadowCell = 0;

            for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
               for( i=0; i<cellShadowInfo->procShadowCnt[nbr_I]; i++ ){
                  MPI_Request *req;
                  char        *particle = NULL;
                  int         proc = 0;
               
                  proc = procNbrInfo->procNbrTbl[nbr_I];

                  shadowCell = cellShadowInfo->procShadowTbl[nbr_I][i];
                  for( j=0; j<swarm->shadowCellParticleCountTbl[shadowCell]; j++ ){
                     
                     req = MemoryPool_NewObject( MPI_Request, requestPool );
                     particle = MemoryPool_NewObjectFunc( swarm->particleExtensionMgr->finalSize, particlePool );
                     LinkedList_InsertNode( list, req, sizeof(void*) );
                     LinkedList_InsertNode( particleList, particle, sizeof(void*) );
                  
                     (void)MPI_Irecv( particle, swarm->particleExtensionMgr->finalSize, MPI_BYTE, proc, 2001, MPI_COMM_WORLD, req );
                  }
               }
            }
         
         }

         {
            Neighbour_Index nbr_I;
            int             i = 0, j = 0;
            int             shadowedCell = 0;
            char            *array = malloc( swarm->particleExtensionMgr->finalSize );

            for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
               for( i=0; i<cellShadowInfo->procShadowedCnt[nbr_I]; i++ ){
                  MPI_Request *req;
                  int         proc = 0;

                  proc = procNbrInfo->procNbrTbl[nbr_I];

                  shadowedCell = cellShadowInfo->procShadowedTbl[nbr_I][i];
                  for( j=0; j<swarm->cellParticleCountTbl[shadowedCell]; j++ ){
                     int pIndex = 0;

                     pIndex = swarm->cellParticleTbl[shadowedCell][j];
                     Swarm_CopyParticleOffSwarm( swarm, array, 0, pIndex );
                     req = MemoryPool_NewObject( MPI_Request, requestPool );
                     LinkedList_InsertNode( list, req, sizeof(void*) );
                     (void)MPI_Isend( array, swarm->particleExtensionMgr->finalSize, MPI_BYTE, proc, 2001, MPI_COMM_WORLD, req );
                  }
               }
            }
            free( array );
         }

         for( data=LinkedListIterator_First(iter); data!=NULL; data=LinkedListIterator_Next(iter) ){
            MPI_Status s;
            (void)MPI_Wait( ((MPI_Request*)data), &s );
         }

         {
            LinkedListIterator *pIter = LinkedListIterator_New( particleList );
            for( data=LinkedListIterator_First(pIter); data!=NULL; data=LinkedListIterator_Next(pIter) ){
               Particle *p = (Particle*)data;
               int      found = 0;
               double   epsilon=1e-5;

               for( ii=0; ii<procNbrInfo->procNbrCnt; ii++ ){
                  for( jj=0; jj<cellShadowInfo->procShadowCnt[ii]; jj++ ){
                     int shadowCell = 0;
                     int kk = 0;

                     shadowCell = cellShadowInfo->procShadowTbl[ii][jj];
                     for( kk=0; kk<swarm->shadowCellParticleCountTbl[shadowCell]; kk++ ){
                        int            shadowParticleIdx = 0;
                        GlobalParticle *gp = NULL;

                        shadowParticleIdx = swarm->shadowCellParticleTbl[shadowCell][kk];
                        gp = ((GlobalParticle*)Swarm_ShadowParticleAt( swarm, shadowParticleIdx));
                        if( (fabs(p->coord[0]-gp->coord[0])<epsilon) && (fabs(p->coord[1]-gp->coord[1])<epsilon) && (fabs(p->coord[2]-gp->coord[2])<epsilon) ){
                           found = 1;
                        }
                     }
                  }
               }
               pcu_check_true( found == 1 );
               pcu_check_true( Journal_Firewall( found, swarm->debug, "Shadow particle validation failed" ) );
            }
         }
         Stg_Class_Delete( list );
         Stg_Class_Delete( iter );
         Stg_Class_Delete( requestPool );
         Stg_Class_Delete( particlePool );
         Stg_Class_Delete( particleList );
      }
   }
}




