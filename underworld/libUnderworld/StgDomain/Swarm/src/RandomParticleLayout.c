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

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>
#include <StgDomain/Utils/Utils.h>

#include "types.h"

#include "ParticleLayout.h"
#include "PerCellParticleLayout.h"
#include "RandomParticleLayout.h"

#include "SwarmClass.h"
#include "Random.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <stdlib.h>

const Type RandomParticleLayout_Type = "RandomParticleLayout";


RandomParticleLayout* RandomParticleLayout_New( 
   Name name, 
   AbstractContext* context,
   CoordSystem      coordSystem,
   Bool             weightsInitialisedAtStartup,
   Particle_InCellIndex cellParticleCount, 
   unsigned int         seed ) 
{
	RandomParticleLayout* self = (RandomParticleLayout*) _RandomParticleLayout_DefaultNew( name );
   _ParticleLayout_Init( self, context, coordSystem, weightsInitialisedAtStartup );
   _PerCellParticleLayout_Init( self );
	_RandomParticleLayout_Init( self, cellParticleCount, seed );

	return self;
}

RandomParticleLayout* _RandomParticleLayout_New(  RANDOMPARTICLELAYOUT_DEFARGS  )
{
	RandomParticleLayout* self;
	
   /* hard-wire these */
   coordSystem = GlobalCoordSystem;
   weightsInitialisedAtStartup = False;
   nameAllocationType = NON_GLOBAL;

	/* Allocate memory */
	self = (RandomParticleLayout*)_PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_PASSARGS  );

	return self;
}

void _RandomParticleLayout_Init( void* randomParticleLayout, Particle_InCellIndex cellParticleCount, unsigned int seed ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;

	self->isConstructed     = True;
	self->cellParticleCount = cellParticleCount;
	self->seed              = seed;
	
	Swarm_Random_Seed( self->seed );
}


void _RandomParticleLayout_Delete( void* randomParticleLayout ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	
	/* Stg_Class_Delete parent class */
	_PerCellParticleLayout_Delete( self );
}

void _RandomParticleLayout_Print( void* randomParticleLayout, Stream* stream ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	
	/* Set the Journal for printing informations */
	Stream* randomParticleLayoutStream = stream;
	
	/* General info */
	Journal_Printf( randomParticleLayoutStream, "RandomParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* RandomParticleLayout */
	Journal_Printf( randomParticleLayoutStream, "\tcellParticleCount: %u\n", self->cellParticleCount );
	Journal_Printf( randomParticleLayoutStream, "\tseed: %u\n", self->seed );
}


void* _RandomParticleLayout_Copy( void* randomParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	RandomParticleLayout*		self = (RandomParticleLayout*)randomParticleLayout;
	RandomParticleLayout*		newRandomParticleLayout;
	
	newRandomParticleLayout = (RandomParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newRandomParticleLayout->cellParticleCount = self->cellParticleCount;
	newRandomParticleLayout->seed = self->seed;
	
	return (void*)newRandomParticleLayout;
}


void* _RandomParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                     _sizeOfSelf = sizeof(RandomParticleLayout);
	Type                                                                             type = RandomParticleLayout_Type;
	Stg_Class_DeleteFunction*                                                     _delete = _RandomParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                       _print = _RandomParticleLayout_Print;
	Stg_Class_CopyFunction*                                                         _copy = _RandomParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                         _defaultConstructor = _RandomParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                           _construct = _RandomParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                                   _build = _RandomParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                         _initialise = _RandomParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                               _execute = _RandomParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                               _destroy = _RandomParticleLayout_Destroy;
	AllocationType                                                     nameAllocationType = NON_GLOBAL;
	ParticleLayout_SetInitialCountsFunction*                            _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                      _initialiseParticles = _PerCellParticleLayout_InitialiseParticles;
	CoordSystem                                                               coordSystem = GlobalCoordSystem;
	Bool                                                      weightsInitialisedAtStartup = False;
	PerCellParticleLayout_InitialCountFunction*                             _initialCount = _RandomParticleLayout_InitialCount;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*   _initialiseParticlesOfCell = _RandomParticleLayout_InitialiseParticlesOfCell;

	return (void*)_RandomParticleLayout_New(  RANDOMPARTICLELAYOUT_PASSARGS  );
}

void _RandomParticleLayout_AssignFromXML( void* randomParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	RandomParticleLayout*       self = (RandomParticleLayout*)randomParticleLayout;
	Particle_InCellIndex        cellParticleCount;
	unsigned int                seed;

   _PerCellParticleLayout_AssignFromXML( self, cf, data );

	cellParticleCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"cellParticleCount", 0  );
	seed = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"seed", 13  );
	
	_RandomParticleLayout_Init( self, cellParticleCount, seed );
}
	
void _RandomParticleLayout_Build( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Initialise( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Execute( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Destroy( void* randomParticleLayout, void* data ) {
   RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;

   _PerCellParticleLayout_Destroy( self, data );
}


Particle_InCellIndex _RandomParticleLayout_InitialCount( void* randomParticleLayout, void* celllayout, Cell_Index cell_I ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	return self->cellParticleCount;
}

void _RandomParticleLayout_InitialiseParticlesOfCell( void* randomParticleLayout, void* _swarm, Cell_Index cell_I ) {
	Swarm*              	swarm = (Swarm*)_swarm;
	Coord               	min;
	Coord               	max;
	Dimension_Index     	dim_I;
	Particle_InCellIndex	particlesThisCell = swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex	cParticle_I = 0;
	GlobalParticle*	        particle = NULL;
	
	Swarm_GetCellMinMaxCoords( swarm, cell_I, min, max );

	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {	
		
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;
		
		for ( dim_I = 0 ; dim_I < 3 ; dim_I++ ) {
			particle->coord[ dim_I ] = Swarm_Random_Random_WithMinMax( min[ dim_I ], max[ dim_I ] );
		}
	}
}



