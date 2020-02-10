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
#include "ShadowInfo.h"

#include "ParticleCommHandler.h"
#include "ParticleMovementHandler.h"

#include "SwarmClass.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type ParticleMovementHandler_Type = "ParticleMovementHandler";

void *ParticleMovementHandler_DefaultNew( Name name )
{
	/* Variables set in this function */
	SizeT                                                        _sizeOfSelf = sizeof(ParticleMovementHandler);
	Type                                                                type = ParticleMovementHandler_Type;
	Stg_Class_DeleteFunction*                                        _delete = _ParticleMovementHandler_Delete;
	Stg_Class_PrintFunction*                                          _print = _ParticleMovementHandler_Print;
	Stg_Class_CopyFunction*                                            _copy = _ParticleMovementHandler_CopyFunc;
	Stg_Component_DefaultConstructorFunction*            _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ParticleMovementHandler_DefaultNew;
	Stg_Component_ConstructFunction*                              _construct = _ParticleMovementHandler_AssignFromXML;
	Stg_Component_BuildFunction*                                      _build = _ParticleMovementHandler_Build;
	Stg_Component_InitialiseFunction*                            _initialise = _ParticleMovementHandler_Initialise;
	Stg_Component_ExecuteFunction*                                  _execute = _ParticleMovementHandler_Execute;
	Stg_Component_DestroyFunction*                                  _destroy = _ParticleMovementHandler_Destroy;
	Bool                                                            initFlag = False;
	Bool                                       useGlobalFallbackCommStrategy = False;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType                                                                                        nameAllocationType = ZERO;
	ParticleCommHandler_AllocateOutgoingCountArrays*                                            _allocateOutgoingCountArrays = ZERO;
	ParticleCommHandler_AllocateOutgoingParticleArrays*                                      _allocateOutgoingParticleArrays = ZERO;
	ParticleCommHandler_FreeOutgoingArrays*                                                              _freeOutgoingArrays = ZERO;
	ParticleCommHandler_AllocateIncomingCountArrays*                                            _allocateIncomingCountArrays = ZERO;
	ParticleCommHandler_AllocateIncomingParticleArrays*                                      _allocateIncomingParticleArrays = ZERO;
	ParticleCommHandler_FreeIncomingArrays*                                                              _freeIncomingArrays = ZERO;
	ParticleCommHandler_BeginReceiveOfIncomingParticleCounts*                          _beginReceiveOfIncomingParticleCounts = ZERO;
	ParticleCommHandler_FinishReceiveOfIncomingParticleCounts*                        _finishReceiveOfIncomingParticleCounts = ZERO;
	ParticleCommHandler_BeginReceiveOfIncomingParticles*                                    _beginReceiveOfIncomingParticles = ZERO;
	ParticleCommHandler_FinishReceiveOfIncomingParticlesAndUpdateIndices*  _finishReceiveOfIncomingParticlesAndUpdateIndices = ZERO;
	ParticleCommHandler_SendOutgoingParticleCounts*                                              _sendOutgoingParticleCounts = ZERO;
	ParticleCommHandler_BeginSendingParticles*                                                        _beginSendingParticles = ZERO;
	ParticleCommHandler_ConfirmOutgoingSendsCompleted*                                        _confirmOutgoingSendsCompleted = ZERO;
	ParticleCommHandler_CommFunction*                                                                          _commFunction = ZERO;

	return _ParticleMovementHandler_New(  PARTICLEMOVEMENTHANDLER_PASSARGS  );
}


ParticleMovementHandler* ParticleMovementHandler_New( 
		Name name,
		Bool useGlobalFallbackCommStrategy
		)
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(ParticleMovementHandler);
	Type                                                      type = ParticleMovementHandler_Type;
	Stg_Class_DeleteFunction*                              _delete = _ParticleMovementHandler_Delete;
	Stg_Class_PrintFunction*                                _print = _ParticleMovementHandler_Print;
	Stg_Class_CopyFunction*                                  _copy = _ParticleMovementHandler_CopyFunc;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ParticleMovementHandler_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _ParticleMovementHandler_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _ParticleMovementHandler_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _ParticleMovementHandler_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _ParticleMovementHandler_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _ParticleMovementHandler_Destroy;
	Bool                                                  initFlag = True;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType                                                                                        nameAllocationType = ZERO;
	ParticleCommHandler_AllocateOutgoingCountArrays*                                            _allocateOutgoingCountArrays = ZERO;
	ParticleCommHandler_AllocateOutgoingParticleArrays*                                      _allocateOutgoingParticleArrays = ZERO;
	ParticleCommHandler_FreeOutgoingArrays*                                                              _freeOutgoingArrays = ZERO;
	ParticleCommHandler_AllocateIncomingCountArrays*                                            _allocateIncomingCountArrays = ZERO;
	ParticleCommHandler_AllocateIncomingParticleArrays*                                      _allocateIncomingParticleArrays = ZERO;
	ParticleCommHandler_FreeIncomingArrays*                                                              _freeIncomingArrays = ZERO;
	ParticleCommHandler_BeginReceiveOfIncomingParticleCounts*                          _beginReceiveOfIncomingParticleCounts = ZERO;
	ParticleCommHandler_FinishReceiveOfIncomingParticleCounts*                        _finishReceiveOfIncomingParticleCounts = ZERO;
	ParticleCommHandler_BeginReceiveOfIncomingParticles*                                    _beginReceiveOfIncomingParticles = ZERO;
	ParticleCommHandler_FinishReceiveOfIncomingParticlesAndUpdateIndices*  _finishReceiveOfIncomingParticlesAndUpdateIndices = ZERO;
	ParticleCommHandler_SendOutgoingParticleCounts*                                              _sendOutgoingParticleCounts = ZERO;
	ParticleCommHandler_BeginSendingParticles*                                                        _beginSendingParticles = ZERO;
	ParticleCommHandler_ConfirmOutgoingSendsCompleted*                                        _confirmOutgoingSendsCompleted = ZERO;
	ParticleCommHandler_CommFunction*                                                                          _commFunction = ZERO;

	return _ParticleMovementHandler_New(  PARTICLEMOVEMENTHANDLER_PASSARGS  );
}


ParticleMovementHandler* _ParticleMovementHandler_New(  PARTICLEMOVEMENTHANDLER_DEFARGS  )
{
	ParticleMovementHandler* self;
	
	/* Allocate memory */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	_allocateOutgoingCountArrays                      = _ParticleCommHandler_AllocateOutgoingCountArrays;
	_allocateOutgoingParticleArrays                   = _ParticleCommHandler_AllocateOutgoingParticlesArrays;
	_freeOutgoingArrays                               = _ParticleCommHandler_FreeOutgoingArrays;
	_allocateIncomingCountArrays                      = _ParticleCommHandler_AllocateIncomingCountArrays;
	_allocateIncomingParticleArrays                   = _ParticleCommHandler_AllocateIncomingParticlesArrays;
	_freeIncomingArrays                               = _ParticleCommHandler_FreeIncomingArrays;
	_beginReceiveOfIncomingParticleCounts             = _ParticleCommHandler_BeginReceiveOfIncomingParticleCounts;
	_finishReceiveOfIncomingParticleCounts            = _ParticleCommHandler_FinishReceiveOfIncomingParticleCounts;
	_beginReceiveOfIncomingParticles                  = _ParticleCommHandler_BeginReceiveOfIncomingParticles;
	_finishReceiveOfIncomingParticlesAndUpdateIndices = ParticleMovementHandler_FinishReceiveAndUpdateShadowParticlesEnteringMyDomain;
	_sendOutgoingParticleCounts                       = _ParticleCommHandler_SendParticleTotalsInShadowCellsToNbrs;
	_beginSendingParticles                            = _ParticleCommHandler_BeginSendingParticlesInShadowCellsToNbrs;
	_confirmOutgoingSendsCompleted                    = _ParticleCommHandler_ConfirmOutgoingSendsCompleted;
	_commFunction                                     = ParticleMovementHandler_HandleParticleMovementBetweenProcs;

	self = (ParticleMovementHandler*)_ParticleCommHandler_New(  PARTICLECOMMHANDLER_PASSARGS  );
	
	/* General info */
	/* Virtual info */
	
	/* ParticleMovementHandler info */
	if( initFlag ){
		_ParticleMovementHandler_Init( self, useGlobalFallbackCommStrategy );
	}
	
	return self;
}


void _ParticleMovementHandler_Init(
		ParticleMovementHandler*     self,
		Bool                     useGlobalFallbackCommStrategy
		)
{
	_ParticleCommHandler_Init( (ParticleCommHandler*)self );
	self->defensive = False;
	self->useGlobalFallbackCommStrategy = useGlobalFallbackCommStrategy;
	self->particlesOutsideDomainIndices = NULL;
	ParticleMovementHandler_ZeroGlobalCommStrategyCounters( self );	
}


void _ParticleMovementHandler_Delete(void* pCommsHandler ) {
	/* Nothing to do, as the temporary arrays are deleted each step as soon as they are finished with. */
	_ParticleCommHandler_Delete( pCommsHandler );
}


void _ParticleMovementHandler_Print( void* pCommsHandler, Stream* stream ) {
	ParticleMovementHandler*	self = (ParticleMovementHandler*)pCommsHandler;
	
	/* General info */
	Journal_Printf( stream, "ParticleMovementHandler (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleCommHandler_Print( self, stream );

	/* Virtual info */

	Journal_Printf( stream, "self->particlesOutsideDomainTotalCount: %d\n", self->particlesOutsideDomainTotalCount ); 
	Journal_Printf( stream, "self->particlesOutsideDomainUnfiledCount: %d\n", self->particlesOutsideDomainUnfilledCount ); 
	Journal_Printf( stream, "self->particlesOutsideDomainIndices(ptr): %p\n", self->particlesOutsideDomainIndices ); 
}


void* _ParticleMovementHandler_CopyFunc( void* particleMovementHandler, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	
	return _ParticleCommHandler_Copy( particleMovementHandler, dest, deep,
		   nameExt, ptrMap );
}

void _ParticleMovementHandler_AssignFromXML( void* pCommsHandler, Stg_ComponentFactory* cf, void* data ){
	ParticleMovementHandler *self = (ParticleMovementHandler*)pCommsHandler;

	self->isConstructed = True;
	_ParticleMovementHandler_Init ( self, True );
}
	
void _ParticleMovementHandler_Build( void* pCommsHandler, void *data ){
	
}
	
void _ParticleMovementHandler_Initialise( void* pCommsHandler, void *data ){
	
}
	
void _ParticleMovementHandler_Execute( void* pCommsHandler, void *data ){
	ParticleCommHandler*	self = (ParticleCommHandler*)pCommsHandler;
	Swarm					*swarm = (Swarm*)data;

	if( !swarm || !Stg_CheckType( swarm, Swarm ) )
		return;

	self->swarm = swarm;
	self->_commFunction( self );	
}

void _ParticleMovementHandler_Destroy( void* pCommsHandler, void *data ){
	
}

void ParticleMovementHandler_HandleParticleMovementBetweenProcs( ParticleCommHandler* pCommsHandler ) {
	ParticleMovementHandler*	self = (ParticleMovementHandler*)pCommsHandler;
	double                  startTime = 0;
	Stream*                 info = Journal_Register( Info_Type, (Name)self->type  );

	Journal_DPrintfL( self->debug, 1, "In %s(), for swarm \"%s\":\n", __func__, self->swarm->name );
	if ( 1 == self->swarm->nProc ) {
		Journal_DPrintfL( self->debug, 1, "Serial run -> nothing to communicate, returning.\n" );
		Stream_UnIndentBranch( Swarm_Debug );
		return;
	}

	Stream_IndentBranch( Swarm_Debug );

	startTime = MPI_Wtime();

	if ( self->swarm->cellShadowCount > 0 ) {
		/* Allocate the recv count arrays and handles */
		self->allocateIncomingCountArrays( (ParticleCommHandler*)self );
		self->allocateOutgoingCountArrays( (ParticleCommHandler*)self );

		/* First thing to do is begin non-blocking receive of incoming particles (for latency hiding) */
		self->beginReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );

		/* Do a blocking send of outgoing counts, so our nbrs know what to receive */
		self->sendOutgoingParticleCounts( (ParticleCommHandler*)self );

		/* Now need to make sure that incoming particle counts are here, then begin receiving particles
		   (We do this as early as possible for latency hiding purposes */
		self->finishReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );
		self->allocateIncomingParticleArrays( (ParticleCommHandler*)self );
		self->beginReceiveOfIncomingParticles( (ParticleCommHandler*)self );
			
		/* OK, now begin sending out particles we know need to go to nbrs */
		self->allocateOutgoingParticleArrays( (ParticleCommHandler*)self );
		self->beginSendingParticles( (ParticleCommHandler*)self );
	}
	
	if ( self->useGlobalFallbackCommStrategy ) {
		self->particlesOutsideDomainIndices = Memory_Alloc_Array( Particle_Index, self->swarm->particlesArrayDelta,
			"self->particlesOutsideDomainIndices" );

		ParticleMovementHandler_DoGlobalFallbackCommunication( self );
	}

	if ( self->swarm->cellShadowCount > 0 ) {
		/* Ok, at this point make sure our non-blocking receive of incoming via domain boundary
 		 *  particles have completed: then immediately insert these particles into our local swarm */
		self->finishReceiveOfIncomingParticlesAndUpdateIndices( (ParticleCommHandler*)self );
	}

	/* final update of the 'holes' in my particles list (some may be left if we sent more than we recv'd overall */
	ParticleMovementHandler_FillRemainingHolesInLocalParticlesArray( (ParticleCommHandler*)self );

	/* NB: Can't delete these until here, as they're needed by the "fillRemainingHoles" function above. */
	if ( self->useGlobalFallbackCommStrategy ) {
		Memory_Free( self->particlesOutsideDomainIndices );
		self->particlesOutsideDomainIndices = NULL;
	}

	if ( self->swarm->cellShadowCount > 0 ) {
		self->confirmOutgoingSendsCompleted( (ParticleCommHandler*)self );
	}

	_ParticleCommHandler_PrintCommunicationVolumeStats( (ParticleCommHandler*)self, startTime, info );

	MPI_Barrier( self->swarm->comm );

	/* clean up allocated memory, and zero counters, ready for next timestep */
	if ( self->swarm->cellShadowCount > 0 ) {
		self->freeIncomingArrays( (ParticleCommHandler*)self );
		self->freeOutgoingArrays( (ParticleCommHandler*)self );
	}
	
	_ParticleCommHandler_ZeroShadowCommStrategyCounters( (ParticleCommHandler*)self );
	
	Stream_UnIndentBranch( Swarm_Debug );
}


void ParticleMovementHandler_ZeroGlobalCommStrategyCounters( ParticleMovementHandler* self ) {
	self->particlesOutsideDomainTotalCount = 0;
	self->particlesOutsideDomainUnfilledCount = 0;
	self->currParticleLeavingMyDomainIndex = 0;
	self->globalParticlesArrivingMyDomainCount = 0;
	self->globalParticlesOutsideDomainTotal = 0;
}

/* +++ Global fallback method related +++ */
void ParticleMovementHandler_DoGlobalFallbackCommunication( ParticleMovementHandler* self ) {

	ParticleMovementHandler_FindParticlesThatHaveMovedOutsideMyDomain( self );

	ParticleMovementHandler_ShareAndUpdateParticlesThatHaveMovedOutsideDomains(
		self,
		&self->globalParticlesArrivingMyDomainCount,
		&self->globalParticlesOutsideDomainTotal );

}


void ParticleMovementHandler_FindParticlesThatHaveMovedOutsideMyDomain( ParticleMovementHandler* self )
{
	Particle_Index		particlesOutsideDomainSize = 0;
	GlobalParticle*         currParticle = NULL;
	Particle_Index		lParticle_I = 0;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	self->particlesOutsideDomainTotalCount = 0;
	self->currParticleLeavingMyDomainIndex = 0;
	particlesOutsideDomainSize = self->swarm->particlesArrayDelta;


	Journal_DPrintfL( self->debug, 1, "Checking the owning cell of each of my swarm's %d particles:\n",
		self->swarm->particleLocalCount );
	Stream_IndentBranch( Swarm_Debug );

	for ( lParticle_I=0; lParticle_I < self->swarm->particleLocalCount; lParticle_I++ ) {

		currParticle = (GlobalParticle*)Swarm_ParticleAt( self->swarm, lParticle_I );
		if ( currParticle->owningCell == self->swarm->cellDomainCount ) {
			Journal_DPrintfL( self->debug, 3, "particle %d has moved outside domain to (%.2f,%.2f,%.2f): "
				"saving index\n", lParticle_I, currParticle->coord[0], currParticle->coord[1],
								currParticle->coord[2] );
			if ( self->particlesOutsideDomainTotalCount == particlesOutsideDomainSize ) { 
				particlesOutsideDomainSize += self->swarm->particlesArrayDelta;
				Journal_DPrintfL( self->debug, 3, "(Need more memory to save indexes: increasing from %d to %d.)\n",
					self->particlesOutsideDomainTotalCount, particlesOutsideDomainSize );
				self->particlesOutsideDomainIndices = Memory_Realloc_Array( self->particlesOutsideDomainIndices,
					Particle_Index, particlesOutsideDomainSize );
			}
			self->particlesOutsideDomainIndices[self->particlesOutsideDomainTotalCount++] = lParticle_I;
		}	

	}	
	Stream_UnIndentBranch( Swarm_Debug );

	self->particlesOutsideDomainUnfilledCount = self->particlesOutsideDomainTotalCount;

	#if DEBUG
	{
		Particle_Index		particle_I = 0;
		if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
			Journal_DPrintf( self->debug, "%d Particles have moved outside my domain:\n\t[",
				self->particlesOutsideDomainTotalCount );
			for ( ; particle_I < self->particlesOutsideDomainTotalCount; particle_I++ ) {
				Journal_DPrintf( self->debug, "%d, ", self->particlesOutsideDomainIndices[particle_I] );
			}
			Journal_DPrintf( self->debug, "]\n" );
		}
	}
	#endif
	Stream_UnIndentBranch( Swarm_Debug );
}


void ParticleMovementHandler_ShareAndUpdateParticlesThatHaveMovedOutsideDomains(
		ParticleMovementHandler* self,
		Particle_Index*      globalParticlesArrivingMyDomainCountPtr,
		Particle_Index*      globalParticlesOutsideDomainTotalPtr )
{
	Particle_Index*		globalParticlesOutsideDomainCounts = NULL;		
	Particle_Index		maxGlobalParticlesOutsideDomainCount = 0;		
	Processor_Index		proc_I = 0;
	Particle_Index		lParticle_I = 0;
	Particle_Index		particle_I = 0;

	Journal_DPrintfL( self->debug, 2, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	(*globalParticlesArrivingMyDomainCountPtr) = 0;
	(*globalParticlesOutsideDomainTotalPtr) = 0;		

	/* Find the counts of particles	outside domain... */
	ParticleMovementHandler_GetCountOfParticlesOutsideDomainPerProcessor(
		self,
		&globalParticlesOutsideDomainCounts,
		&maxGlobalParticlesOutsideDomainCount,
		globalParticlesOutsideDomainTotalPtr );

	if ( (*globalParticlesOutsideDomainTotalPtr) > 0 ) {
		Particle*		particlesLeavingMyDomain = NULL;
		Particle*		globalParticlesLeavingDomains = NULL;
		SizeT			particlesLeavingDomainSizeBytes = 0;
		Cell_DomainIndex	lCell_I = 0;
		GlobalParticle*	        currParticle = NULL;
		Particle_Index		currProcParticlesOutsideDomainCount = 0;
		Particle_Index		currProcOffset = 0;

		particlesLeavingDomainSizeBytes = self->swarm->particleExtensionMgr->finalSize
			* maxGlobalParticlesOutsideDomainCount;
		particlesLeavingMyDomain = Memory_Alloc_Bytes( particlesLeavingDomainSizeBytes, "Particle",
			"particlesLeavingMyDomain" );

		// TODO: investigate doing this with an MPI_Indexed datatype instead...
		Journal_DPrintfL( self->debug, 2, "Copying particles leaving my domain to temp. transfer array\n" );
		Stream_IndentBranch( Swarm_Debug );

		#if 0
		MPI_Type_indexed( 
			self->particlesOutsideDomainTotalCount,
			blocklens,
			self->particlesOutsideDomainIndices,//change to contiguous indices?
			MPI_BYTE,
			ParticlesLeavingDomainTransferIndexed
			);
		#endif	

		for ( particle_I=0; particle_I < self->particlesOutsideDomainTotalCount; particle_I++ ) {
			Journal_DPrintfL( self->debug, 3, "Copying particle %d to particlesLeavingMyDomain[%d]\n",
				self->particlesOutsideDomainIndices[particle_I], particle_I );
			Swarm_CopyParticleOffSwarm( self->swarm,
				particlesLeavingMyDomain, particle_I,
				self->particlesOutsideDomainIndices[particle_I] );
		}	
		Stream_UnIndentBranch( Swarm_Debug );

		/* allocate the big global receive buffer */
		globalParticlesLeavingDomains = Memory_Alloc_Bytes( particlesLeavingDomainSizeBytes * self->swarm->nProc,
			"Particle", "globalParticlesLeavingDomains" );

		Journal_DPrintfL( self->debug, 2, "Getting the global array of particles leaving domains\n" );
		(void)MPI_Allgather( particlesLeavingMyDomain, particlesLeavingDomainSizeBytes, MPI_BYTE,
			globalParticlesLeavingDomains, particlesLeavingDomainSizeBytes, MPI_BYTE,
			self->swarm->comm );

		Journal_DPrintfL( self->debug, 2, "Checking through the global array of particles leaving domains, "
			"and snaffling those moving into my domain:\n" );
		Stream_IndentBranch( Swarm_Debug );
		for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {

			if ( proc_I == self->swarm->myRank ) continue;

			currProcOffset = proc_I * maxGlobalParticlesOutsideDomainCount;
			currProcParticlesOutsideDomainCount = globalParticlesOutsideDomainCounts[proc_I];
			
			Journal_DPrintfL( self->debug, 3, "Checking particles that left proc. %d:\n", proc_I );
			for ( particle_I=0; particle_I < currProcParticlesOutsideDomainCount; particle_I++ ) {
				currParticle = (GlobalParticle*)ParticleAt( globalParticlesLeavingDomains,
					(currProcOffset + particle_I),
					self->swarm->particleExtensionMgr->finalSize );
				lCell_I = CellLayout_CellOf( self->swarm->cellLayout, currParticle );
				if ( lCell_I < self->swarm->cellLocalCount ) { 
					#if DEBUG
					Journal_DPrintfL( self->debug, 3, "Found particle at (%.2f,%.2f,%.2f) that's moved "
						"into my local cell %d...\n", currParticle->coord[0],
						currParticle->coord[1], currParticle->coord[2], lCell_I );
					#endif	
					
					/* copy particle to the lowest available slot in my particles array */
					lParticle_I = ParticleMovementHandler_FindFreeSlotAndPrepareForInsertion( (ParticleCommHandler*)self );

					Swarm_CopyParticleOntoSwarm( self->swarm, lParticle_I,
						globalParticlesLeavingDomains, (currProcOffset + particle_I) );
					Swarm_AddParticleToCell( self->swarm, lCell_I, lParticle_I );
					(*globalParticlesArrivingMyDomainCountPtr)++;
				}
				#if DEBUG
				else {
					currParticle = (GlobalParticle*)ParticleAt( globalParticlesLeavingDomains, 
						(currProcOffset + particle_I),
						self->swarm->particleExtensionMgr->finalSize );
					Journal_DPrintfL( self->debug, 3, "Ignoring particle at (%.2f,%.2f,%.2f) since "
						"not in my local cells...\n", currParticle->coord[0],
						currParticle->coord[1], currParticle->coord[2] );
				}
				#endif
			}		
		}	
		Stream_UnIndentBranch( Swarm_Debug );

		Memory_Free( particlesLeavingMyDomain );
		Memory_Free( globalParticlesLeavingDomains );

		/* Defensive check to make sure particles not lost/created accidentally somehow */
		if( self->defensive == True ) {
			ParticleMovementHandler_EnsureParticleCountLeavingDomainsEqualsCountEnteringGlobally( self );
		}
	}	
	Memory_Free( globalParticlesOutsideDomainCounts );
	Stream_UnIndentBranch( Swarm_Debug );
}


void ParticleMovementHandler_EnsureParticleCountLeavingDomainsEqualsCountEnteringGlobally( ParticleMovementHandler* self ) {
	Particle_Index		totalParticlesFoundEnteringDomains = 0;

	MPI_Reduce( &self->globalParticlesArrivingMyDomainCount, &totalParticlesFoundEnteringDomains,
		    1, MPI_UNSIGNED, MPI_SUM, 0, self->swarm->comm );

	if ( 0 == self->swarm->myRank ) {
		Stream*   errorStream = Journal_Register( Error_Type, (Name)self->type  );

		Journal_Firewall( totalParticlesFoundEnteringDomains == self->globalParticlesOutsideDomainTotal,
				  errorStream, "Error - in %s(): %d particles were found across all processors to be "
				  "leaving the individual domains directly, but after sharing and searching %d were "
				  "found entering them directly! These must match as no particles should be "
				  "lost/created through advection.\n",
				  __func__, self->globalParticlesOutsideDomainTotal,
				  totalParticlesFoundEnteringDomains );
	}
}


void ParticleMovementHandler_GetCountOfParticlesOutsideDomainPerProcessor(
	ParticleMovementHandler*	self,
	Particle_Index**	globalParticlesOutsideDomainCountsPtr,
	Particle_Index*		maxGlobalParticlesOutsideDomainCountPtr,
	Particle_Index*		globalParticlesOutsideDomainTotalPtr )
{
	Processor_Index		proc_I;

	(*globalParticlesOutsideDomainCountsPtr) = Memory_Alloc_Array( Particle_Index, self->swarm->nProc,
		"(*globalParticlesOutsideDomainCountsPtr)" );
		
	(void)MPI_Allgather( (&self->particlesOutsideDomainTotalCount), 1, MPI_UNSIGNED, 
		(*globalParticlesOutsideDomainCountsPtr), 1, MPI_UNSIGNED, self->swarm->comm );
	
	(*globalParticlesOutsideDomainTotalPtr) = 0;
	for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {
		(*globalParticlesOutsideDomainTotalPtr) += (*globalParticlesOutsideDomainCountsPtr)[proc_I];
		if ( (*globalParticlesOutsideDomainCountsPtr)[proc_I] > (*maxGlobalParticlesOutsideDomainCountPtr) ) {
			(*maxGlobalParticlesOutsideDomainCountPtr) = (*globalParticlesOutsideDomainCountsPtr)[proc_I];
		}
	}	

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_DPrintf( self->debug, "Global counts of particles moving outside domains:\n" );
		Journal_DPrintf( self->debug, "\tTotal: %d, Counts: [", (*globalParticlesOutsideDomainTotalPtr) );
		for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {
			Journal_DPrintf( self->debug, "%d, ", (*globalParticlesOutsideDomainCountsPtr)[proc_I] );
		}	
		Journal_DPrintf( self->debug, "]\n" );
	}	
	#endif
}

/* TODO: look at using MPI_Indexed instead */
void ParticleMovementHandler_FinishReceiveAndUpdateShadowParticlesEnteringMyDomain( ParticleCommHandler* self ) {
	MPI_Status	status;
	Cell_ShadowTransferIndex	stCell_I;
	Cell_LocalIndex			lCell_I;
	Neighbour_Index			nbr_I;
	Cell_ShadowTransferIndex	shadowCellsFromProcCount;
	ShadowInfo*			cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*			procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index			nbrCount = procNbrInfo->procNbrCnt;
	Particle_InCellIndex		incomingCellParticleCount;
	Particle_InCellIndex		cParticle_I;
	Particle_Index			lParticle_I;
	Index				incomingParticle_I=0; /*Index into the array of all leaving particle indices */
	Index				incomingParticleSetsNotYetReceivedCount;
	Bool*				incomingParticlesReceived;
	#if DEBUG
	GlobalParticle*                 currParticle;
	#endif

	Journal_DPrintf( self->debug, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );
	
	incomingParticlesReceived = Memory_Alloc_Array_Unnamed( Bool, nbrCount );

	/* Calculate how many particle sets we have to receive */
	incomingParticleSetsNotYetReceivedCount = 0;
	for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
		incomingParticlesReceived[nbr_I] = False;
		if (self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] > 0) {
			incomingParticleSetsNotYetReceivedCount++;
		}
	}

	while ( incomingParticleSetsNotYetReceivedCount > 0 ) {
		int flag = 0;
		Journal_DPrintfL( self->debug, 3, "%d particle sets still to go...\n", incomingParticleSetsNotYetReceivedCount );
		for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
			if ( (self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] > 0) &&
				(False == incomingParticlesReceived[nbr_I]) )
			{
				MPI_Test( self->particlesArrivingFromNbrShadowCellsHandles[nbr_I], &flag, &status );
				if ( False == flag ) {
					/* No results yet from this proc -> continue to next. */
					continue;
				}
				else {
					Journal_DPrintfL( self->debug, 3, "Received particles from nbr %d (proc %d):\n",
						nbr_I, procNbrInfo->procNbrTbl[nbr_I] );
					Stream_Indent( self->debug );

					incomingParticle_I = 0;
					shadowCellsFromProcCount = cellShadowInfo->procShadowedCnt[nbr_I];

				
					for ( stCell_I=0; stCell_I < shadowCellsFromProcCount; stCell_I++ ) {

						lCell_I = cellShadowInfo->procShadowedTbl[nbr_I][stCell_I];
						Journal_DPrintfL( self->debug, 3, "Incoming cell %d (local index %d):\n",
							stCell_I, lCell_I );
						Stream_Indent( self->debug );

						incomingCellParticleCount =
							self->particlesArrivingFromNbrShadowCellCounts[nbr_I][stCell_I];

						for ( cParticle_I=0; cParticle_I < incomingCellParticleCount; cParticle_I++ ) {	

							#if DEBUG
							currParticle = (GlobalParticle*)ParticleAt(
								self->particlesArrivingFromNbrShadowCells[nbr_I],
								incomingParticle_I,
								self->swarm->particleExtensionMgr->finalSize );
							Journal_DPrintfL( self->debug, 3, "Handling its PIC %d: - at "
								"(%.2f,%.2f,%.2f)\n", cParticle_I,
								currParticle->coord[0], currParticle->coord[1],
								currParticle->coord[2] );
							#endif

							Stream_Indent( self->debug );

							lParticle_I = ParticleMovementHandler_FindFreeSlotAndPrepareForInsertion( self );

							Swarm_CopyParticleOntoSwarm(
								self->swarm,
								lParticle_I,
								self->particlesArrivingFromNbrShadowCells[nbr_I], incomingParticle_I++ ); 

							Swarm_AddParticleToCell( self->swarm, lCell_I, lParticle_I );
							
							Stream_UnIndent( self->debug );
						}	
						Stream_UnIndent( self->debug );
					}
					incomingParticlesReceived[nbr_I] = True;
					incomingParticleSetsNotYetReceivedCount--;
					Stream_UnIndent( self->debug );
				}
			}
		}
	}	

	Memory_Free( incomingParticlesReceived );

	Stream_UnIndentBranch( Swarm_Debug );
}

/* +++ Managment of particle array insertions/deletions +++ */


Particle_Index ParticleMovementHandler_FindFreeSlotAndPrepareForInsertion( ParticleCommHandler* self )
{
	Particle_Index	lParticle_I = 0;

	if ( self->shadowParticlesLeavingMeUnfilledCount > 0 ) {
		Journal_DPrintfL( self->debug, 3, "Still %d holes available from "
			"particles leaving via shadow cells\n-> free slot to add into is %d\n",
			self->shadowParticlesLeavingMeUnfilledCount,
			self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex]);
			
		lParticle_I = self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex];

		self->currShadowParticleLeavingMeIndex++;
		self->shadowParticlesLeavingMeUnfilledCount--;
	}
	else if ( self->particlesOutsideDomainUnfilledCount ) {
		Journal_DPrintfL( self->debug, 3, "Still %d holes available from "
			"particles leaving domain direct\n-> free slot to add into is %d\n", 
			self->particlesOutsideDomainUnfilledCount,
			self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex]);

		lParticle_I = self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex];

		self->currParticleLeavingMyDomainIndex++;
		self->particlesOutsideDomainUnfilledCount--;
	}
	else {
		Journal_DPrintfL( self->debug, 3, "No holes left from leaving "
			"particles\n-> slot to insert into is end of array %d\n", 
			self->swarm->particleLocalCount );
		lParticle_I = self->swarm->particleLocalCount;
		/*
		if ( self->swarm->particleLocalCount == self->swarm->particlesArraySize ) {
			Journal_DPrintfL( self->debug, 3, "Particles array memory used up "
				"-> increasing from %d entries by %d\n",
				self->swarm->particlesArraySize, self->swarm->particlesArrayDelta );
			self->swarm->particlesArraySize += self->swarm->particlesArrayDelta;
			self->swarm->particles = Memory_Realloc_Array_Bytes(
				self->swarm->particles,
				self->swarm->particleExtensionMgr->finalSize,
				self->swarm->particlesArraySize );
		}
		*/
		self->swarm->particleLocalCount++;
		Swarm_Realloc( self->swarm );
	}

	return lParticle_I;
}


void ParticleMovementHandler_FillRemainingHolesInLocalParticlesArray( ParticleCommHandler* self )
{
	Particle_Index		prevParticlesArraySize = self->swarm->particlesArraySize;
	Particle_Index		numHolesToFill;
	Particle_InCellIndex	cParticle_I;
	StandardParticle*	oldPtrToMovedParticle;
	Cell_LocalIndex		owningCell;
	Particle_Index		indexToInsertAt;
	Particle_Index*		leavingParticlesArray = NULL;
	Index			currLeavingParticleArrayEntry = 0;
	Index			highestLeavingParticleArrayEntry;
	Index			leavingParticlesUnfilledCount = 0;
	Particle_Index		highestLeavingParticleIndex;
	Particle_Index		candidateParticleToMove;
	Bool			finishedFlag = False;
	Bool			mergedArrayCreated = False;
	Particle_Index		prevParticleCount = self->swarm->particleLocalCount;
	
	Journal_DPrintf( self->debug, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	numHolesToFill = self->particlesOutsideDomainUnfilledCount + self->shadowParticlesLeavingMeUnfilledCount;
	if ( numHolesToFill == 0 ) {
		Journal_DPrintfL( self->debug, 2, "No holes to fill -> nothing to do, returning.\n" );
		Stream_UnIndentBranch( Swarm_Debug );
		return;
	}
	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		ParticleMovementHandler_PrintParticleSlotsYetToFill( self );
	}	
	#endif

	/* work out the list we have to iterate over: */
	if ( self->shadowParticlesLeavingMeUnfilledCount && !self->particlesOutsideDomainUnfilledCount ) {
		Journal_DPrintfL( self->debug, 2, "Particles have only left via shadow cells -> no need to merge lists\n" );
		leavingParticlesArray = &self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex];
	}
	else if ( self->particlesOutsideDomainUnfilledCount && !self->shadowParticlesLeavingMeUnfilledCount ) {
		Journal_DPrintfL( self->debug, 2, "Particles have only left domain directly -> no need to merge lists\n" );
		leavingParticlesArray = &self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex];
	} 
	else {
		Journal_DPrintfL( self->debug, 2, "Particles have left both via shadow cells and domain directly -> merge lists\n" );
		leavingParticlesArray = ParticleMovementHandler_MergeListsOfUnfilledParticleSlots( self );
		mergedArrayCreated = True;
	}

	/* Ok: while there are holes left to fill, find the highest candidate, move it, and reduce the count. */
	Journal_DPrintfL( self->debug, 2, "Starting run through the %d particles to fill:\n", numHolesToFill );
	Stream_IndentBranch( Swarm_Debug );

	currLeavingParticleArrayEntry = 0;
	highestLeavingParticleArrayEntry = numHolesToFill-1;
	leavingParticlesUnfilledCount = numHolesToFill;

	while ( leavingParticlesUnfilledCount > 0 ) {

		indexToInsertAt = leavingParticlesArray[currLeavingParticleArrayEntry];
		Journal_DPrintfL( self->debug, 3, "Attempting to fill leaving slot %d (at particle index %d):\n",
			currLeavingParticleArrayEntry, indexToInsertAt );

		Stream_Indent( self->debug );

		/* This is where we work out the index of which particle to move into the free slot.
		 * We Start from the end of the particles array, then decrement by 1 until we find a candidate that
		 * hasn't itself already left. 
		 * We also need to consider the possibility that every candidate higher than the current index
		 * has also left, in which case we are done, and finish the while process. 
		 * See the ParticleCommHandler Twiki page for diagrams illustrating this algorithm.
		 */

		candidateParticleToMove = self->swarm->particleLocalCount-1;
		highestLeavingParticleIndex = leavingParticlesArray[highestLeavingParticleArrayEntry];
		Journal_DPrintfL( self->debug, 3, "Searching for highest particle that hasn't also moved:\n" );
		Stream_Indent( self->debug );	

		while ( candidateParticleToMove == leavingParticlesArray[highestLeavingParticleArrayEntry] ) {
			/* Check if that was the last candidate particle above the current one: */
			/* This test needs to be at the top of this loop to handle the case where we have one
			particle that's leaving */

			if ( candidateParticleToMove <= indexToInsertAt ) {
				Journal_DPrintfL( self->debug, 3, "** No more particles above current "
					"hole %d to fill: we're done. **\n", indexToInsertAt );
				/* Need the line below to mark the fact we failed to fill the current indexToInsertAt hole */
				self->swarm->particleLocalCount--;
				finishedFlag = True;
				break;
			}

			Journal_DPrintfL( self->debug, 3, "Candidate particle %d has also left...\n",
				candidateParticleToMove );

			highestLeavingParticleArrayEntry--;
			highestLeavingParticleIndex = leavingParticlesArray[highestLeavingParticleArrayEntry];
			leavingParticlesUnfilledCount--;
			self->swarm->particleLocalCount--;
			candidateParticleToMove--;
		}	
		Stream_UnIndent( self->debug );	

		if ( True == finishedFlag ) {
			/* We must have hit the "no more candidate particles" criterion in the search loop, so
			 * quit trying to fill empty holes entirely. */
			Stream_UnIndent( self->debug );
			break;
		}

		Journal_DPrintfL( self->debug, 3, "Highest valid particle found at index %d:\n",
			candidateParticleToMove );
		Journal_DFirewall( (candidateParticleToMove > indexToInsertAt), Swarm_Error,
			"Error in %s: Empty hole filling\nalgorithm has stuffed up somehow,"
			" since particle to be moved %d is <= slot to insert into %d.\n",
			__func__, candidateParticleToMove, indexToInsertAt );
		Stream_Indent( self->debug );	

		Journal_DPrintfL( self->debug, 3, "Copying particle data from %d to %d\n",
			candidateParticleToMove, indexToInsertAt );
		Swarm_CopyParticleWithinSwarm( self->swarm, indexToInsertAt, candidateParticleToMove );

		/* update the cell that the moved particle lives in to have the correct index into the
		 * particle array for it. */
		oldPtrToMovedParticle = Swarm_ParticleAt( self->swarm, candidateParticleToMove );
		owningCell = oldPtrToMovedParticle->owningCell;
		cParticle_I = Swarm_GetParticleIndexWithinCell( self->swarm, owningCell, candidateParticleToMove );
		Journal_DPrintfL( self->debug, 3, "Updating owning cell: (Cell %d, PIC index %d) now -> p.i. %d\n",
			owningCell, cParticle_I, indexToInsertAt );
		self->swarm->cellParticleTbl[owningCell][cParticle_I] = indexToInsertAt;


		Stream_UnIndent( self->debug );	

		/* update the counters/indices */
		currLeavingParticleArrayEntry++;
		leavingParticlesUnfilledCount--;
		self->swarm->particleLocalCount--;

		Stream_UnIndent( self->debug );
	}
	Stream_UnIndentBranch( Swarm_Debug );

	/* we only need to free the array of leaving particle slots if its a new merged list */
	if ( mergedArrayCreated == True ) {
		Memory_Free( leavingParticlesArray );
	}

	/* ------------------------- */
	Journal_DPrintfL( self->debug, 2, "Local particle count reduced from %d to %d\n", prevParticleCount,
		self->swarm->particleLocalCount );

	/* Update the memory allocated to the particles array if particle count has reduced significantly */
	while ( self->swarm->particlesArraySize > self->swarm->particleLocalCount + self->swarm->particlesArrayDelta ) {
		self->swarm->particlesArraySize -= self->swarm->particlesArrayDelta;
	}
	if ( self->swarm->particlesArraySize < prevParticlesArraySize ) {
		Journal_DPrintfL( self->debug, 2, "Reducing particles array entries from %d to %d\n",
			prevParticlesArraySize, self->swarm->particlesArraySize );
		Swarm_Realloc( self->swarm );
		/*
		self->swarm->particles = Memory_Realloc_Array_Bytes(
			self->swarm->particles,
			self->swarm->particleExtensionMgr->finalSize,
			self->swarm->particlesArraySize );
		*/
	}	

	Stream_UnIndentBranch( Swarm_Debug );
}


Particle_Index* ParticleMovementHandler_MergeListsOfUnfilledParticleSlots( ParticleCommHandler* self )
{
	Particle_Index*		mergedLeavingParticleArray = NULL;
	Particle_Index		slotsToFillTotalCount = 0;
	Index			currMergedLeavingParticleEntry = 0;
	Index			lowestUnmergedLeavingViaShadow = self->currShadowParticleLeavingMeIndex;
	Index			lowestUnmergedLeavingDomain = self->currParticleLeavingMyDomainIndex;
	Particle_Index		indexOfLowestUnmergedLeavingDomain = 0;
	Index*			lowestUnmergedLeavingEntryToUpdatePtr = NULL;
	Particle_Index		candidateMergeParticle = 0;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_Indent( self->debug );

	slotsToFillTotalCount = self->particlesOutsideDomainUnfilledCount + self->shadowParticlesLeavingMeUnfilledCount;
	mergedLeavingParticleArray = Memory_Alloc_Array( Particle_Index, slotsToFillTotalCount, "mergedLeavingParticlesArray" );

	while ( currMergedLeavingParticleEntry < slotsToFillTotalCount ) {
		/* Need to initialise this to the max particle count every loop, in case the first condition is false,
		so the 2nd will always hit it. */
		candidateMergeParticle = self->swarm->particleLocalCount;
		 
		if ( lowestUnmergedLeavingViaShadow < self->shadowParticlesLeavingMeTotalCount ) {
			candidateMergeParticle = self->shadowParticlesLeavingMeIndices[lowestUnmergedLeavingViaShadow];
			lowestUnmergedLeavingEntryToUpdatePtr = &lowestUnmergedLeavingViaShadow;
		}
		if ( lowestUnmergedLeavingDomain < self->particlesOutsideDomainTotalCount ) {
			indexOfLowestUnmergedLeavingDomain = self->particlesOutsideDomainIndices[lowestUnmergedLeavingDomain];

			if ( indexOfLowestUnmergedLeavingDomain < candidateMergeParticle ) { 
				candidateMergeParticle = indexOfLowestUnmergedLeavingDomain;
				lowestUnmergedLeavingEntryToUpdatePtr = &lowestUnmergedLeavingDomain;
			}	
		}
		
		mergedLeavingParticleArray[currMergedLeavingParticleEntry++] = candidateMergeParticle;
		(*lowestUnmergedLeavingEntryToUpdatePtr)++;

		#if DEBUG
		Journal_Firewall( lowestUnmergedLeavingViaShadow <= self->shadowParticlesLeavingMeTotalCount,
			Swarm_Error, "Error: merging of unfilled particle lists stuffed up.\n" );
		Journal_Firewall( lowestUnmergedLeavingDomain <= self->particlesOutsideDomainTotalCount,
			Swarm_Error, "Error: merging of unfilled particle lists stuffed up.\n" );
		#endif
	}

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_DPrintf( self->debug, "Merged list of particles leaving proc:\n\t{" );
		for ( currMergedLeavingParticleEntry=0; currMergedLeavingParticleEntry < slotsToFillTotalCount;
			currMergedLeavingParticleEntry++ ) 
		{
			Journal_DPrintf( self->debug, "%d, ",
				mergedLeavingParticleArray[currMergedLeavingParticleEntry] );
		}
		Journal_DPrintf( self->debug, "}\n" );
	}
	#endif
	
	Stream_UnIndent( self->debug );

	return mergedLeavingParticleArray;
}


void ParticleMovementHandler_PrintParticleSlotsYetToFill( ParticleCommHandler* self ) {	
	Index leavingParticleEntry;

	Journal_DPrintf( self->debug, "%d slots yet to fill from particles leaving via shadow cells:\n",
		self->shadowParticlesLeavingMeUnfilledCount );
	leavingParticleEntry = self->currShadowParticleLeavingMeIndex;
	Journal_DPrintf( self->debug, "\t{ " );
	for ( ; leavingParticleEntry < self->shadowParticlesLeavingMeTotalCount; leavingParticleEntry++ ) {
		Journal_DPrintf( self->debug, "%d, ", 
			self->shadowParticlesLeavingMeIndices[leavingParticleEntry] );
	}
	Journal_DPrintf( self->debug, "}\n" );
		
	Journal_DPrintf( self->debug, "%d slots yet to fill from particles leaving domain directly:\n",
		self->particlesOutsideDomainUnfilledCount );
	leavingParticleEntry = self->currParticleLeavingMyDomainIndex;
	Journal_DPrintf( self->debug, "\t{ " );
	for ( ; leavingParticleEntry < self->particlesOutsideDomainTotalCount; leavingParticleEntry++ ) {
		Journal_DPrintf( self->debug, "%d, ", 
			self->particlesOutsideDomainIndices[leavingParticleEntry] );
	}
	Journal_DPrintf( self->debug, "}\n" );
}



