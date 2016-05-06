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

#include "types.h"
#include "EscapedRoutine.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type EscapedRoutine_Type = "EscapedRoutine";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

EscapedRoutine* _EscapedRoutine_New(  ESCAPEDROUTINE_DEFARGS  )
{
	EscapedRoutine* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(EscapedRoutine) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (EscapedRoutine*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* General info */

	/* Virtual Info */
	self->_select = _select;
	
	return self;
}

void* _EscapedRoutine_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(EscapedRoutine);
	Type                                                      type = EscapedRoutine_Type;
	Stg_Class_DeleteFunction*                              _delete = _EscapedRoutine_Delete;
	Stg_Class_PrintFunction*                                _print = _EscapedRoutine_Print;
	Stg_Class_CopyFunction*                                  _copy = _EscapedRoutine_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _EscapedRoutine_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _EscapedRoutine_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _EscapedRoutine_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _EscapedRoutine_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _EscapedRoutine_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _EscapedRoutine_Destroy;
	EscapedRoutine_SelectFunction*                         _select = _EscapedRoutine_Select;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _EscapedRoutine_New(  ESCAPEDROUTINE_PASSARGS  );
}

void _EscapedRoutine_Init( 
	void*					escapedRoutine, 
	Dimension_Index	dim,
	Particle_Index		particlesToRemoveDelta )
{
	EscapedRoutine* self = (EscapedRoutine*)escapedRoutine;

	self->dim = dim;
	self->particlesToRemoveDelta = particlesToRemoveDelta;

	self->debug = Journal_Register( Debug_Type, (Name)EscapedRoutine_Type ); /* TODO Register Child */
	self->particlesToRemoveCount = 0;
	self->particlesToRemoveAlloced = 0;
}

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _EscapedRoutine_Delete( void* escapedRoutine ) {
	EscapedRoutine* self = (EscapedRoutine*)escapedRoutine;
	
	/* Delete parent */
	_Stg_Component_Delete( self  );
}

void _EscapedRoutine_Print( void* escapedRoutine, Stream* stream ) {
	EscapedRoutine* self = (EscapedRoutine*)escapedRoutine;
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
}

void* _EscapedRoutine_Copy( void* escapedRoutine, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	EscapedRoutine*	self = (EscapedRoutine*)escapedRoutine;
	EscapedRoutine*	newEscapedRoutine;
	
	newEscapedRoutine = (EscapedRoutine*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newEscapedRoutine;
}

void _EscapedRoutine_AssignFromXML( void* escapedRoutine, Stg_ComponentFactory* cf, void* data ) {
	EscapedRoutine*	self = (EscapedRoutine*) escapedRoutine;
	Dimension_Index	dim;
	Particle_Index		particlesToRemoveDelta;

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
	particlesToRemoveDelta = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"particlesToRemoveDelta", 20  );

	_EscapedRoutine_Init( self, dim, particlesToRemoveDelta );
}

void _EscapedRoutine_Build( void* escapedRoutine, void* data ) {
	EscapedRoutine* self = (EscapedRoutine*) escapedRoutine;

	self->particlesToRemoveAlloced = self->particlesToRemoveDelta * 10;
	self->particlesToRemoveList = Memory_Alloc_Array( unsigned, self->particlesToRemoveAlloced, "particlesToRemoveList" );
}

void _EscapedRoutine_Initialise( void* escapedRoutine, void* data ) {
}

void _EscapedRoutine_Execute( void* escapedRoutine, void* data ) {
	Swarm* swarm = Stg_CheckType( data, Swarm );

	EscapedRoutine_RemoveFromSwarm( escapedRoutine, swarm );
}

void _EscapedRoutine_Destroy( void* escapedRoutine, void* data ) {
	EscapedRoutine* self = (EscapedRoutine*) escapedRoutine;

	Memory_Free( self->particlesToRemoveList );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void EscapedRoutine_Select( void* escapedRoutine, void* _swarm ) {
	EscapedRoutine* self = (EscapedRoutine*) escapedRoutine;

	self->_select( self, _swarm );
}

void _EscapedRoutine_Select( void* escapedRoutine, void* _swarm ) {
	EscapedRoutine*	self = (EscapedRoutine*)escapedRoutine;
	Swarm*				swarm = (Swarm*)_swarm;
	unsigned	p_i;

	assert( self );
	assert( swarm );

	/* Check all particles for removal. */
	for( p_i = 0; p_i < swarm->particleLocalCount; p_i++ ) {
		StandardParticle*	particle = (StandardParticle*)Swarm_ParticleAt( swarm, p_i );

		if( particle->owningCell >= swarm->cellDomainCount ) {
			EscapedRoutine_SetParticleToRemove( self, swarm, p_i );
		}
	}
}


void EscapedRoutine_RemoveFromSwarm( void* escapedRoutine, void* _swarm ) {
	EscapedRoutine*	self = (EscapedRoutine*) escapedRoutine;
	Swarm*				swarm = (Swarm*) _swarm;
	
	EscapedRoutine_InitialiseParticleList( self );
	
	/* Select particles to remove. */
	EscapedRoutine_Select( self, swarm );

	/* Actually remove particles */
	EscapedRoutine_RemoveParticles( self, swarm );
}

void EscapedRoutine_InitialiseParticleList( void* escapedRoutine ) {
	EscapedRoutine* self = (EscapedRoutine*) escapedRoutine;

	self->particlesToRemoveCount = 0;
	memset( self->particlesToRemoveList, 0, sizeof(unsigned) * self->particlesToRemoveAlloced );
}

void EscapedRoutine_SetParticleToRemove( void* escapedRoutine, Swarm* swarm, Particle_Index lParticle_I ) {
	EscapedRoutine* self = (EscapedRoutine*) escapedRoutine;

	/* Check memory */
	if ( self->particlesToRemoveCount >= self->particlesToRemoveAlloced ) {
		self->particlesToRemoveAlloced += self->particlesToRemoveDelta;
		self->particlesToRemoveList = Memory_Realloc_Array( self->particlesToRemoveList, unsigned, self->particlesToRemoveAlloced );
	}

	self->particlesToRemoveList[ self->particlesToRemoveCount ] = lParticle_I;
	self->particlesToRemoveCount++;
}

/* This typedef is required by qsort: the two arguments must be ptrs to particle indexes, as in particlesToRemoveList */
int _EscapedRoutine_CompareParticles( const void* _aParticleInfo, const void* _bParticleInfo ) {
	return (*(unsigned*)_aParticleInfo - *(unsigned*)_bParticleInfo );
}

void EscapedRoutine_SortParticleList( void* escapedRoutine ) {
	EscapedRoutine*	     self                = (EscapedRoutine*) escapedRoutine;

	qsort( self->particlesToRemoveList, self->particlesToRemoveCount, sizeof(unsigned), _EscapedRoutine_CompareParticles );
}

void EscapedRoutine_RemoveParticles( void* escapedRoutine, void* _swarm ) {
	EscapedRoutine*		self = (EscapedRoutine*) escapedRoutine;
    Swarm*              swarm = (Swarm*) _swarm;
	Index                 array_I;
	StandardParticle*     particleToRemove;
	Particle_Index        particleToRemove_I;
	
	StandardParticle*     lastParticle;
	Cell_Index            lastParticle_CellIndex;
	Particle_Index        lastParticle_I;
	Particle_InCellIndex  lastParticle_IndexWithinCell;
	SizeT                 particleSize        = swarm->particleExtensionMgr->finalSize;

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_Printf( self->debug, "Particles to remove:\n{ " );
		for ( array_I = 0 ; array_I < self->particlesToRemoveCount ; array_I++ ) {
			Journal_Printf( self->debug, "%u, ", self->particlesToRemoveList[ array_I ] );
		}
		Journal_Printf( self->debug, "%u }\n", self->particlesToRemoveList[ array_I ] );
	}
	#endif


	EscapedRoutine_SortParticleList( self );

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_Printf( self->debug, "Particles to remove:\n{ " );
		for ( array_I = 0 ; array_I < self->particlesToRemoveCount ; array_I++ ) {
			Journal_Printf( self->debug, "%u, ", self->particlesToRemoveList[ array_I ] );
		}
		Journal_Printf( self->debug, "%u }\n", self->particlesToRemoveList[ array_I ] );
	}
	#endif

	for ( array_I = self->particlesToRemoveCount - 1 ; array_I < self->particlesToRemoveCount ; array_I-- ) {
		particleToRemove_I = self->particlesToRemoveList[ array_I ];
		particleToRemove = Swarm_ParticleAt( swarm, particleToRemove_I );

		Journal_DPrintfL( self->debug, 2, "Removing particle %u\n", particleToRemove_I );
		
		/* Copy over particle with last particle in array - as long as it isn't the last one */
		lastParticle_I = swarm->particleLocalCount - 1;
		lastParticle   = Swarm_ParticleAt( swarm, lastParticle_I );
		if ( particleToRemove_I != lastParticle_I ) {
			/* Get last Particle information */
			lastParticle_CellIndex       = lastParticle->owningCell;
			lastParticle_IndexWithinCell = Swarm_GetParticleIndexWithinCell( swarm, lastParticle_CellIndex, lastParticle_I);

			Journal_DPrintfL( self->debug, 2, 
					"Copying over particle %u using last particle %u from cell %u (cell particle index - %u)\n", 
					particleToRemove_I, lastParticle_I, lastParticle_CellIndex, lastParticle_IndexWithinCell );

			/* Copy over particle */
			memcpy( particleToRemove, lastParticle, particleSize );
			
			/* Change value in cell particle table to point to new index in array */
			swarm->cellParticleTbl[lastParticle_CellIndex][ lastParticle_IndexWithinCell ] = particleToRemove_I;
		}

		/* Initialise memory to zero so it is clear that it's been deleted */
		memset( lastParticle, 0, particleSize );
		swarm->particleLocalCount--;
	}

	Swarm_Realloc( swarm );
}


