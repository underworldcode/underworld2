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
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include "PICellerator/PopulationControl/src/PopulationControl.h"

struct _Particle {
   __GlobalParticle
   Bool toRemove;
};

typedef struct {
   FiniteElementContext* context;
   Stg_ComponentFactory* cf;
   Swarm*                swarm;
   EscapedRoutine*       escRoutine;
} EscapedRoutineSuiteData;

void EscapedRoutineSuite_Setup( EscapedRoutineSuiteData* data ) {
   char           xmlInputFilename[PCU_PATH_MAX];
   Particle_Index lParticle_I = 0;

   pcu_filename_input( "EscapedRoutineSuite.xml", xmlInputFilename );
   data->cf = stgMainInitFromXML( xmlInputFilename, MPI_COMM_WORLD, NULL );
   data->context = (FiniteElementContext*) LiveComponentRegister_Get( data->cf->LCRegister, (Name)"context" );
   data->swarm = (Swarm* ) LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"swarm" );
   data->escRoutine = (EscapedRoutine* ) LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"escapedRoutine" );
   stgMainBuildAndInitialise( data->cf );

   /* Use our test param to mark all the particles as not to be removed by default*/
   for ( lParticle_I = 0; lParticle_I < data->swarm->particleLocalCount; lParticle_I++ ) {
      ((Particle* )Swarm_ParticleAt( data->swarm, lParticle_I ))->toRemove = False;
   }
} 

void EscapedRoutineSuite_Teardown( EscapedRoutineSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void EscapedRoutineSuite_TestSetParticleToRemove( EscapedRoutineSuiteData* data ) {
   Particle_Index lParticle_I = 0;

   pcu_check_true( data->escRoutine->particlesToRemoveCount == 0 );
   pcu_check_true( data->escRoutine->particlesToRemoveAlloced == 10*data->escRoutine->particlesToRemoveDelta );

   /* Do it enough times that pToRemoveList has to be expanded at least once */
   for ( lParticle_I = 0; lParticle_I < data->swarm->particleLocalCount; lParticle_I++ ) {
      EscapedRoutine_SetParticleToRemove( data->escRoutine, data->swarm, lParticle_I );
   }
   
   pcu_check_true( data->escRoutine->particlesToRemoveCount == data->swarm->particleLocalCount );
   pcu_check_true( data->escRoutine->particlesToRemoveAlloced >= data->escRoutine->particlesToRemoveCount );
   for ( lParticle_I = 0; lParticle_I < data->swarm->particleLocalCount; lParticle_I++ ) {
      pcu_check_true( data->escRoutine->particlesToRemoveList[lParticle_I] == lParticle_I );
   }
}   

void EscapedRoutineSuite_TestSelect( EscapedRoutineSuiteData* data ) {
   Particle_Index lParticle_I = 0;
   Particle_Index pToRemove_I = 0;

   /* Modify some of the swarm particles to be outside list of global cells - in this case, only one cell,
    * so any value other than 0 should be considered as outside the box */
   for ( lParticle_I = 10; lParticle_I < data->swarm->particleLocalCount; lParticle_I++ ) {
      (Swarm_ParticleAt( data->swarm, lParticle_I ))->owningCell = 100;
   }
   
   EscapedRoutine_Select( data->escRoutine, data->swarm );

   /* Check that the particles have been set to be removed as expected */
   pcu_check_true( data->escRoutine->particlesToRemoveCount == (data->swarm->particleLocalCount-10) );
   for ( pToRemove_I = 0; pToRemove_I < (data->swarm->particleLocalCount-10); pToRemove_I++ ) {
      pcu_check_true( data->escRoutine->particlesToRemoveList[pToRemove_I] == 10+pToRemove_I );
   }
}

void EscapedRoutineSuite_TestInitialiseParticleList( EscapedRoutineSuiteData* data ) {
   Particle_Index pToRemove_I = 0;
   
   /* Fill the list with some initial random data */
   for ( pToRemove_I = 0; pToRemove_I < data->escRoutine->particlesToRemoveAlloced; pToRemove_I++ ) {
      data->escRoutine->particlesToRemoveList[pToRemove_I] = pToRemove_I*2;
   }

   EscapedRoutine_InitialiseParticleList( data->escRoutine );

   pcu_check_true( data->escRoutine->particlesToRemoveCount == 0 );
   for ( pToRemove_I = 0; pToRemove_I < data->escRoutine->particlesToRemoveAlloced; pToRemove_I++ ) {
      pcu_check_true( 0 == data->escRoutine->particlesToRemoveList[pToRemove_I] );
   }
}

void EscapedRoutineSuite_TestCompareParticles( EscapedRoutineSuiteData* data ) {
   unsigned pToRemoveList[3] = { 1, 4, 7 };

   /* This function is needed for sorting plists into order. Should just be based on value of ptrs, which
    *  implies position in particle array */
   pcu_check_true( _EscapedRoutine_CompareParticles( &pToRemoveList[0], &pToRemoveList[1] ) < 0 );
   pcu_check_true( _EscapedRoutine_CompareParticles( &pToRemoveList[2], &pToRemoveList[1] ) > 0 );
   pcu_check_true( _EscapedRoutine_CompareParticles( &pToRemoveList[1], &pToRemoveList[1] ) == 0 );
}

void EscapedRoutineSuite_TestSortParticleList( EscapedRoutineSuiteData* data ) {
   Particle_Index pToRemove_I = 0;
   Particle_Index pToRemoveCount = 4;
   unsigned       pToRemoveList[4] = { 8, 12, 1, 3 };

   /* first put some data in escRoutine's particlesToRemoveList */
   for ( pToRemove_I = 0; pToRemove_I < pToRemoveCount; pToRemove_I++ ) {
      EscapedRoutine_SetParticleToRemove( data->escRoutine, data->swarm, pToRemoveList[pToRemove_I] );
   }

   EscapedRoutine_SortParticleList( data->escRoutine );

   pcu_check_true( data->escRoutine->particlesToRemoveCount == pToRemoveCount );
   pcu_check_true( data->escRoutine->particlesToRemoveList[0] == 1 );
   pcu_check_true( data->escRoutine->particlesToRemoveList[1] == 3 );
   pcu_check_true( data->escRoutine->particlesToRemoveList[2] == 8 );
   pcu_check_true( data->escRoutine->particlesToRemoveList[3] == 12 );
}

void EscapedRoutineSuite_TestRemoveParticles( EscapedRoutineSuiteData* data ) {
   Particle_Index lParticle_I = 0;
   Particle_Index pToRemove_I = 0;
   Particle_Index pToRemoveCount = 4;
   unsigned       pToRemoveList[4] = { 8, 12, 1, 3 };

   /* First set up the particles to remove list, and manually mark these */
   for ( pToRemove_I = 0; pToRemove_I < pToRemoveCount; pToRemove_I++ ) {
      EscapedRoutine_SetParticleToRemove( data->escRoutine, data->swarm, pToRemoveList[pToRemove_I] );
      ((Particle*)Swarm_ParticleAt( data->swarm, pToRemoveList[pToRemove_I] ))->toRemove = True; 
   }
   pcu_check_true( data->escRoutine->particlesToRemoveCount == pToRemoveCount );

   EscapedRoutine_RemoveParticles( data->escRoutine, data->swarm );

   /* Check these particles have been removed from the swarm */
   for ( lParticle_I = 0; lParticle_I < data->swarm->particleLocalCount; lParticle_I++ ) {
      pcu_check_true( ((Particle*)Swarm_ParticleAt( data->swarm, lParticle_I ))->toRemove == False );
   }
}

/* We don't need to do a regular test of execute, since it just calls EscapedRoutine_RemoveFromSwarm.
 * However, since this is the main public function, test if it can correctly catch bad input data */
void EscapedRoutineSuite_TestExecuteBadInput( EscapedRoutineSuiteData* data ) {
   stJournal->enable = False;
   stJournal->firewallProtected = False;
   pcu_check_assert( Stg_Component_Execute( data->escRoutine, NULL, True ) ); 
   pcu_check_assert( Stg_Component_Execute( data->escRoutine, stJournal, True ) );
   stJournal->enable = True;
   stJournal->firewallProtected = True;
}

void EscapedRoutineSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, EscapedRoutineSuiteData );
   pcu_suite_setFixtures( suite, EscapedRoutineSuite_Setup, EscapedRoutineSuite_Teardown );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestSetParticleToRemove );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestSelect );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestInitialiseParticleList );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestCompareParticles );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestSortParticleList );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestRemoveParticles );
   pcu_suite_addTest( suite, EscapedRoutineSuite_TestExecuteBadInput );
}


