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
#include <StgDomain/Mesh/src/Mesh.h>
#include <StgDomain/Utils/src/Utils.h>

#include "types.h"

#include "ParticleLayout.h"
#include "PerCellParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SwarmClass.h"
#include "StandardParticle.h"

const Type PerCellParticleLayout_Type = "PerCellParticleLayout";

PerCellParticleLayout* _PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_DEFARGS  )
{
   PerCellParticleLayout*		self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(PerCellParticleLayout) );
   self = (PerCellParticleLayout*)_ParticleLayout_New(  PARTICLELAYOUT_PASSARGS  );

   /* General info */

   /* Virtual functions */
   self->_initialCount = _initialCount;
   self->_initialiseParticlesOfCell = _initialiseParticlesOfCell;

   return self;
}


void _PerCellParticleLayout_Init( void* particleLayout )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	self->isConstructed = True;
}

void _PerCellParticleLayout_Delete( void* particleLayout ) {
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	_ParticleLayout_Delete( self );
}

void _PerCellParticleLayout_Print( void* particleLayout, Stream* stream ) {
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	Journal_Printf( stream, "PerCellParticleLayout (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleLayout_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_initialCount (func ptr): %p\n", self->_initialCount );
	Journal_Printf( stream, "\t_initialiseParticlesOfCell (func ptr): %p\n", self->_initialiseParticlesOfCell );
	
}

void _PerCellParticleLayout_Destroy( void* particleLayout, void* data ) {}

void _PerCellParticleLayout_AssignFromXML( void* particleLayout, Stg_ComponentFactory *cf, void* data ) {
	PerCellParticleLayout*		self = (PerCellParticleLayout*)particleLayout;

   _ParticleLayout_AssignFromXML( self, cf, data );

   _PerCellParticleLayout_Init( self );
}

void* _PerCellParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PerCellParticleLayout*		self = (PerCellParticleLayout*)particleLayout;
	PerCellParticleLayout*		newPerCellParticleLayout;
	
	newPerCellParticleLayout = (PerCellParticleLayout*)_ParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );

	/* Virtual methods */
	newPerCellParticleLayout->_initialCount = self->_initialCount;
	newPerCellParticleLayout->_initialiseParticlesOfCell = self->_initialiseParticlesOfCell;
	
	return (void*)newPerCellParticleLayout;
}


/* Note: this function is required to also set the total particle count */
void _PerCellParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm )
{
	PerCellParticleLayout*	self = (PerCellParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_DomainIndex	cell_I = 0;

	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		swarm->cellParticleCountTbl[cell_I] = PerCellParticleLayout_InitialCount( self, swarm->cellLayout, cell_I );
		
		/* Initially, just set to the same sizes as initial particle counts */
		swarm->cellParticleSizeTbl[cell_I] += swarm->cellParticleCountTbl[cell_I];
		swarm->particleLocalCount += swarm->cellParticleCountTbl[cell_I];
	}

}

void _PerCellParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	PerCellParticleLayout*	self = (PerCellParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_Index		cell_I;
	Particle_InCellIndex	cellParticle_I;
	Particle_Index		currParticleIndex = 0;
	StandardParticle*	particle = NULL;
	
	/* Go through and init particles, cell-by-cell */
	/* Note: don't put particles in the shadow cells... */
	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		Particle_InCellIndex		count = swarm->cellParticleCountTbl[cell_I];
		
		for( cellParticle_I = 0; cellParticle_I < count; cellParticle_I++ ) {
			swarm->cellParticleTbl[cell_I][cellParticle_I] = currParticleIndex++;
			particle = Swarm_ParticleInCellAt( swarm, cell_I, cellParticle_I );
			particle->owningCell = cell_I;
		}	

		PerCellParticleLayout_InitialiseParticlesOfCell( self, swarm, cell_I );
	}
}

void PerCellParticleLayout_InitialiseParticlesOfCell( void* particleLayout, void* _swarm, Cell_Index cell_I )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	self->_initialiseParticlesOfCell( self, _swarm, cell_I );
}

Particle_InCellIndex PerCellParticleLayout_InitialCount( void* particleLayout, void* _swarm, Cell_Index cell_I )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	return self->_initialCount( self, _swarm, cell_I );
}






