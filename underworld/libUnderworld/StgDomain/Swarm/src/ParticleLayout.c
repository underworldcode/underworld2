/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>

#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>

#include "types.h"

#include "ParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const Type ParticleLayout_Type = "ParticleLayout";

ParticleLayout* _ParticleLayout_New(  PARTICLELAYOUT_DEFARGS  )
{
   ParticleLayout*		self;
	
   /* hard-wire this var */
   nameAllocationType = NON_GLOBAL;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(ParticleLayout) );
   self = (ParticleLayout*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
   /* General info */
   self->coordSystem = coordSystem;	
   self->weightsInitialisedAtStartup = weightsInitialisedAtStartup;	

   /* Virtual functions */
   self->_setInitialCounts = _setInitialCounts;
   self->_initialiseParticles = _initialiseParticles;
	
    return self;
}


void _ParticleLayout_Init( 
   void*           particleLayout, 
   AbstractContext* context,
   CoordSystem     coordSystem,
   Bool            weightsInitialisedAtStartup ) 
{ 
   ParticleLayout* self = (ParticleLayout*)particleLayout;

   self->context = context;
   self->debug = Stream_RegisterChild( Swarm_Debug, self->type );

   self->coordSystem = coordSystem;
   self->weightsInitialisedAtStartup = weightsInitialisedAtStartup;
}

void _ParticleLayout_Delete( void* particleLayout ) {
    ParticleLayout* self = (ParticleLayout*)particleLayout;
	
    /* Stg_Class_Delete parent class */
    _Stg_Component_Delete( self );
}

void _ParticleLayout_Print( void* particleLayout, Stream* stream ) {
    ParticleLayout* self = (ParticleLayout*)particleLayout;
	
    Journal_Printf( stream, "ParticleLayout (ptr): %p\n", self );
	
    /* Parent class info */
    _Stg_Class_Print( self, stream );
	
    /* Virtual info */
    Journal_Printf( stream, "\t_setInitialCounts (func ptr): %p\n", self->_setInitialCounts );
    Journal_Printf( stream, "\t_initialiseParticles (func ptr): %p\n", self->_initialiseParticles );
	
    /* ParticleLayout info */
    Journal_Printf( 
        stream, 
        "\tcoordSystem: %s\n", 
        ( self->coordSystem == LocalCoordSystem ) ? "LocalCoordSystem" : "GlobalCoordSystem" );
    Journal_Printf( 
        stream, 
        "\tweightsInitialisedAtStartup : %s\n",
        self->weightsInitialisedAtStartup ? "True" : "False" );
}


void* _ParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    ParticleLayout*		self = (ParticleLayout*)particleLayout;
    ParticleLayout*		newParticleLayout;
	
    newParticleLayout = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
    /* Virtual methods */
    newParticleLayout->_setInitialCounts = self->_setInitialCounts;
    newParticleLayout->_initialiseParticles = self->_initialiseParticles;
    newParticleLayout->coordSystem = self->coordSystem;
    newParticleLayout->weightsInitialisedAtStartup = self->weightsInitialisedAtStartup;

    return (void*)newParticleLayout;
}

void _ParticleLayout_AssignFromXML( void* particleLayout, Stg_ComponentFactory *cf, void* data ) {
   ParticleLayout* self = (ParticleLayout*) particleLayout;
   AbstractContext* context=NULL;

   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   if( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data  );

   _ParticleLayout_Init( self, context, GlobalCoordSystem, False );

}

void _ParticleLayout_Destroy( void* particleLayout, void* data ) {}

void ParticleLayout_InitialiseParticles( void* particleLayout, void* swarm ) {
    ParticleLayout* self = (ParticleLayout*)particleLayout;
	
    if( self )
       self->_initialiseParticles( self, swarm );
}

void ParticleLayout_SetInitialCounts( void* particleLayout, void* swarm ) {
    ParticleLayout* self = (ParticleLayout*)particleLayout;
	
    if( self )
       self->_setInitialCounts( particleLayout, swarm );
}


