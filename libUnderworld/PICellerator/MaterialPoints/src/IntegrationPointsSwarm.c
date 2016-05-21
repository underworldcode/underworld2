/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include <PICellerator/PopulationControl/PopulationControl.h>

#include "MaterialPoints.h"
#include "PICelleratorContext.h"
#include "SwarmMap.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type IntegrationPointsSwarm_Type = "IntegrationPointsSwarm";

void* _IntegrationPointsSwarm_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                             _sizeOfSelf = sizeof(IntegrationPointsSwarm);
   Type                                                     type = IntegrationPointsSwarm_Type;
   Stg_Class_DeleteFunction*                             _delete = _IntegrationPointsSwarm_Delete;
   Stg_Class_PrintFunction*                               _print = _IntegrationPointsSwarm_Print;
   Stg_Class_CopyFunction*                                 _copy = NULL;
   Stg_Component_DefaultConstructorFunction* _defaultConstructor = _IntegrationPointsSwarm_DefaultNew;
   Stg_Component_ConstructFunction*                   _construct = _IntegrationPointsSwarm_AssignFromXML;
   Stg_Component_BuildFunction*                           _build = _IntegrationPointsSwarm_Build;
   Stg_Component_InitialiseFunction*                 _initialise = _IntegrationPointsSwarm_Initialise;
   Stg_Component_ExecuteFunction*                       _execute = _IntegrationPointsSwarm_Execute;
   Stg_Component_DestroyFunction*                       _destroy = _IntegrationPointsSwarm_Destroy;
   SizeT                                            particleSize = sizeof(IntegrationPoint);

   /*
    * Variables that are set to ZERO are variables that will be set either by the current _New function
    * or another parent _New function further up the hierachy
    */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;
   void*                          ics = ZERO;

   return (void*) _IntegrationPointsSwarm_New( INTEGRATIONPOINTSSWARM_PASSARGS );
}

IntegrationPointsSwarm* _IntegrationPointsSwarm_New( INTEGRATIONPOINTSSWARM_DEFARGS ) {
   IntegrationPointsSwarm* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(IntegrationPointsSwarm) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   ics = NULL;

   self = (IntegrationPointsSwarm*)_Swarm_New( SWARM_PASSARGS );

   return self;
}

void _IntegrationPointsSwarm_AssignFromXML( void* integrationPoints, Stg_ComponentFactory* cf, void* data ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*) integrationPoints;
   FeMesh*                 mesh;
   
   /* This will also call _Swarm_Init */
   _Swarm_AssignFromXML( self, cf, data );

   mesh               = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FeMesh", FeMesh, True, data );

   _IntegrationPointsSwarm_Init( self, mesh );
}

void _IntegrationPointsSwarm_Init( 
   void*                   swarm,
   FeMesh*                 mesh )
{
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)swarm;
   LocalParticle           localParticle;
   IntegrationPoint        particle;

   self->mesh               = mesh;

   self->weightVariable = Swarm_NewScalarVariable( self, (Name)"Weight", GetOffsetOfMember( particle , weight ), 
      Variable_DataType_Double );

   LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->weightVariable );
   LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->weightVariable->variable );

   self->localCoordVariable = Swarm_NewVectorVariable( self, (Name)"LocalElCoord", GetOffsetOfMember( localParticle , xi ),
      Variable_DataType_Double, self->dim, "Xi", "Eta", "Zeta" );

   LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->localCoordVariable );
   LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->localCoordVariable->variable );
   
   /* _Construct calls _Swarm_Init */

   /* Lock down the extension manager.
    * It doesn't make sense for the IntegrationPointsSwarm to allow IntegrationPoints to be extended
    * This means attempts to extend integration points are firewalled to pickup errors.
    * -- Alan 20060506
    */
   ExtensionManager_SetLockDown( self->particleExtensionMgr, True );

   self->swarmsMappedTo = List_New("swarmsMappedTo");
   List_SetItemSize(self->swarmsMappedTo, sizeof(SwarmMap*));

}

void _IntegrationPointsSwarm_Delete( void* integrationPoints ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)integrationPoints;

   Stg_Class_Delete( self->swarmsMappedTo );

   _Swarm_Delete( self );
}

void _IntegrationPointsSwarm_Print( void* integrationPoints, Stream* stream ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)integrationPoints;
   
   _Swarm_Print( self, stream );
}

void _IntegrationPointsSwarm_Build( void* integrationPoints, void* data ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*) integrationPoints;

   _Swarm_Build( self, data );

   Stg_Component_Build( self->localCoordVariable, data, False );
   Stg_Component_Build( self->weightVariable, data, False );
   Stg_Component_Build( self->mesh, data, False );
}

void _IntegrationPointsSwarm_Initialise( void* integrationPoints, void* data ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*) integrationPoints;

   Journal_DPrintf( self->debug, "In %s(): for swarm \"%s\":\n", __func__, self->name );
   Stream_IndentBranch( Swarm_Debug );

   _Swarm_Initialise( self, data );

   Stg_Component_Initialise( self->localCoordVariable, data, False );
   Stg_Component_Initialise( self->weightVariable, data, False );
   Stg_Component_Initialise( self->mesh, data, False );

   Stream_UnIndentBranch( Swarm_Debug );
   Journal_DPrintf( self->debug, "...done in %s() for swarm \"%s\".\n",
      __func__, self->name );
}

void _IntegrationPointsSwarm_Execute( void* integrationPoints, void* data ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)integrationPoints;
   
   _Swarm_Execute( self, data );
}

void _IntegrationPointsSwarm_Destroy( void* integrationPoints, void* data ) {
   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)integrationPoints;

   Stg_Component_Destroy( self->localCoordVariable, data, False );
   Stg_Component_Destroy( self->weightVariable, data, False );
   Stg_Component_Destroy( self->mesh, data, False );
   
   _Swarm_Destroy( self, data );
}

void _IntegrationPointsSwarm_UpdateHook( void* timeIntegrator, void* swarm ) {
//   IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)swarm;
//   // whatever is calling this needs to also ensure the new mapper is working
//   // and is called.  previously it was called here.
//   assert(0);
//   WeightsCalculator_CalculateAll(self->weights, self );
}

void IntegrationPointsSwarm_ClearSwarmMaps( void* integrationPoints ) {
    IntegrationPointsSwarm* self = (IntegrationPointsSwarm*)integrationPoints;
    SwarmMap* map = NULL;

    int ii;
    for (ii=0; ii<List_GetSize(self->swarmsMappedTo); ii++) {
        map = *(SwarmMap**)List_GetItem(self->swarmsMappedTo, ii);
        SwarmMap_Clear(map);
    }

}

