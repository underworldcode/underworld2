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
#include "ParticleShadowSync.h"

#include "SwarmClass.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type ParticleShadowSync_Type = "ParticleShadowSync";

/* MPI tags */
static const int SHADOW_PARTICLE_COUNTS_PER_CELL = 10;
static const int SHADOW_PARTICLES = 20;

void* ParticleShadowSync_DefaultNew( Name name )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(ParticleShadowSync);
	Type                                                      type = ParticleShadowSync_Type;
	Stg_Class_DeleteFunction*                              _delete = _ParticleShadowSync_Delete;
	Stg_Class_PrintFunction*                                _print = _ParticleShadowSync_Print;
	Stg_Class_CopyFunction*                                  _copy = _ParticleShadowSync_CopyFunc;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ParticleShadowSync_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _ParticleShadowSync_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _ParticleShadowSync_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _ParticleShadowSync_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _ParticleShadowSync_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _ParticleShadowSync_Destroy;
	Bool                                                  initFlag = False;

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

	return _ParticleShadowSync_New(  PARTICLESHADOWSYNC_PASSARGS  );
}


ParticleShadowSync* ParticleShadowSync_New( 
		Name name,
		void* swarm
		)
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(ParticleShadowSync);
	Type                                                      type = ParticleShadowSync_Type;
	Stg_Class_DeleteFunction*                              _delete = _ParticleShadowSync_Delete;
	Stg_Class_PrintFunction*                                _print = _ParticleShadowSync_Print;
	Stg_Class_CopyFunction*                                  _copy = _ParticleShadowSync_CopyFunc;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ParticleShadowSync_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _ParticleShadowSync_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _ParticleShadowSync_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _ParticleShadowSync_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _ParticleShadowSync_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _ParticleShadowSync_Destroy;
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

		return _ParticleShadowSync_New(  PARTICLESHADOWSYNC_PASSARGS  );
}


ParticleShadowSync* _ParticleShadowSync_New(  PARTICLESHADOWSYNC_DEFARGS  )
{
	ParticleShadowSync* self;
	
	/* Allocate memory */
	
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	_allocateOutgoingCountArrays                      = _ParticleCommHandler_AllocateOutgoingCountArrays;
	_allocateOutgoingParticleArrays                   = NULL;
	_freeOutgoingArrays                               = _ParticleCommHandler_FreeOutgoingArrays;
	_allocateIncomingCountArrays                      = _ParticleCommHandler_AllocateIncomingCountArrays;
	_allocateIncomingParticleArrays                   = NULL;
	_freeIncomingArrays                               = _ParticleCommHandler_FreeIncomingArrays;
	_beginReceiveOfIncomingParticleCounts             = _ParticleCommHandler_BeginReceiveOfIncomingParticleCounts;
	_finishReceiveOfIncomingParticleCounts            = _ParticleShadowSync_FinishReceiveOfIncomingParticleCounts;
	_beginReceiveOfIncomingParticles                  = _ParticleShadowSync_BeginReceiveOfIncomingParticles;
	_finishReceiveOfIncomingParticlesAndUpdateIndices = _ParticleShadowSync_FinishReceiveOfIncomingParticles;
	_sendOutgoingParticleCounts                       = _ParticleShadowSync_SendParticleTotalsInShadowCellsToNbrs;
	_beginSendingParticles                            = _ParticleShadowSync_SendShadowParticles;
	_confirmOutgoingSendsCompleted                    = _ParticleCommHandler_ConfirmOutgoingSendsCompleted;
	_commFunction                                     = ParticleShadowSync_HandleParticleMovementBetweenProcs;
	_sizeOfSelf                                       = sizeof(ParticleShadowSync);
	type                                              = ParticleShadowSync_Type;
	_delete                                           = _ParticleShadowSync_Delete;
	_print                                            = _ParticleShadowSync_Print;
	_copy                                             = _ParticleShadowSync_CopyFunc;
	_defaultConstructor                               = (Stg_Component_DefaultConstructorFunction*)ParticleShadowSync_DefaultNew;
	_construct                                        = _ParticleShadowSync_AssignFromXML;
	_build                                            = _ParticleShadowSync_Build;
	_initialise                                       = _ParticleShadowSync_Initialise;
	_execute                                          = _ParticleShadowSync_Execute;
	_destroy                                          = _ParticleShadowSync_Destroy;

	self = (ParticleShadowSync*)_ParticleCommHandler_New(  PARTICLECOMMHANDLER_PASSARGS  );
	
	/* General info */
	/* Virtual info */
	
    _ParticleShadowSync_Init( self );
	
	return self;
}


void _ParticleShadowSync_Init( ParticleShadowSync* self )
{
	_ParticleCommHandler_Init( (ParticleCommHandler*)self );
	self->particlesOutsideDomainIndices = NULL;
}


void _ParticleShadowSync_Delete(void* pCommsHandler )
{
	_ParticleCommHandler_Delete( pCommsHandler );
}


void _ParticleShadowSync_Print( void* pCommsHandler, Stream* stream ) {
	ParticleShadowSync*	self = (ParticleShadowSync*)pCommsHandler;
	
	/* General info */
	Journal_Printf( stream, "ParticleShadowSync (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleCommHandler_Print( self, stream );

	/* Virtual info */
}


void* _ParticleShadowSync_CopyFunc( void* particleMovementHandler, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	
	return _ParticleCommHandler_Copy( particleMovementHandler, dest, deep,
		   nameExt, ptrMap );
}

void _ParticleShadowSync_AssignFromXML( void* pCommsHandler, Stg_ComponentFactory* cf, void* data ){}
	
void _ParticleShadowSync_Build( void* pCommsHandler, void *data ){
}
	
void _ParticleShadowSync_Initialise( void* pCommsHandler, void *data ){
	
}
	
void _ParticleShadowSync_Execute( void* pCommsHandler, void *data ){
	ParticleCommHandler*	self = (ParticleCommHandler*)pCommsHandler;
	Swarm					*swarm = (Swarm*)data;
	
	self->swarm = swarm;
	assert( self->swarm );

	if( swarm->shadowTablesBuilt == False ){
		_Swarm_BuildShadowParticles( self->swarm );
	}

	self->_commFunction( self );	
}

void _ParticleShadowSync_Destroy( void* pCommsHandler, void *data ){
	
}

void _ParticleShadowSync_SendParticleTotalsInShadowCellsToNbrs( ParticleCommHandler* self )
{	
	Processor_Index			proc_I;
	ShadowInfo*			cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*			procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index		nbr_I, nbrCount;
	Cell_Index              lCellID, cellParticleCount, shadowedCell_I;	

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	self->shadowParticlesLeavingMeTotalCount = 0;

	nbrCount = procNbrInfo->procNbrCnt;
	for( nbr_I = 0 ; nbr_I < nbrCount ; nbr_I++ ) {
		proc_I = procNbrInfo->procNbrTbl[nbr_I];
		
		self->shadowParticlesLeavingMeTotalCounts[nbr_I] = 0;
		for( shadowedCell_I = 0 ; shadowedCell_I < cellShadowInfo->procShadowedCnt[ nbr_I ]; shadowedCell_I++ ) {
			lCellID = cellShadowInfo->procShadowedTbl[nbr_I][shadowedCell_I];
			cellParticleCount = self->swarm->cellParticleCountTbl[ lCellID ];
			self->shadowParticlesLeavingMeCountsPerCell[nbr_I][shadowedCell_I] = cellParticleCount;

			self->shadowParticlesLeavingMeTotalCounts[nbr_I] += cellParticleCount;
		}
		MPI_Ssend( self->shadowParticlesLeavingMeCountsPerCell[nbr_I], cellShadowInfo->procShadowCnt[nbr_I], MPI_UNSIGNED,
			proc_I, SHADOW_PARTICLE_COUNTS_PER_CELL, self->swarm->comm );
	}

	Stream_UnIndentBranch( Swarm_Debug );
}

void _ParticleShadowSync_FinishReceiveOfIncomingParticleCounts( ParticleCommHandler* self ) {
	MPI_Status			status;
	Processor_Index			proc_I;
	ShadowInfo*		        cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*		        procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index		        nbr_I;
	int i = 0;

	self->swarm->shadowParticleCount = 0;
	/* TODO: may be worth converting the below into an MPI_Test loop */
	for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
		proc_I = procNbrInfo->procNbrTbl[nbr_I];
		
		(void)MPI_Wait( self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I], &status );

		self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] = 0;
		for( i=0; i<cellShadowInfo->procShadowCnt[nbr_I]; i++ ){
			Index shadowCell = 0;

			shadowCell = cellShadowInfo->procShadowTbl[nbr_I][i];
			self->swarm->shadowCellParticleCountTbl[shadowCell-self->swarm->cellLocalCount] = 0;

			self->swarm->shadowCellParticleTbl[shadowCell] = Memory_Realloc_Array( self->swarm->shadowCellParticleTbl[shadowCell], Particle_Index,
																	(self->particlesArrivingFromNbrShadowCellCounts[nbr_I][i]) );

			self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] += self->particlesArrivingFromNbrShadowCellCounts[nbr_I][i];
		}
	
		self->swarm->shadowParticleCount+= self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I];
	}
}

void _ParticleShadowSync_BeginReceiveOfIncomingParticles( ParticleCommHandler* pCommHandler ) {
	ParticleShadowSync		*self = (ParticleShadowSync*)pCommHandler;
	ShadowInfo*		        cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*		        procNbrInfo = cellShadowInfo->procNbrInfo;
	long                           incomingViaShadowArrayBytes = 0;
	Neighbour_Index		        nbr_I;
	Processor_Index			proc_I;
	int i = 0;
	char* recvLocation  = NULL;

	self->particlesArrivingFromNbrShadowCellsHandles = Memory_Alloc_Array_Unnamed( MPI_Request*, procNbrInfo->procNbrCnt );
	for( i=0; i<procNbrInfo->procNbrCnt; i++ ){
		self->particlesArrivingFromNbrShadowCellsHandles[i] = Memory_Alloc_Array_Unnamed( MPI_Request, 1 );
	}

	self->swarm->shadowParticles = Memory_Realloc( self->swarm->shadowParticles,
			self->swarm->particleExtensionMgr->finalSize*(self->swarm->shadowParticleCount) );
	
	recvLocation = (char*)self->swarm->shadowParticles;
	for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
		
		if ( self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] != 0 ) {
			proc_I = procNbrInfo->procNbrTbl[nbr_I];

			/* start non-blocking recv of particles */
			incomingViaShadowArrayBytes = self->swarm->particleExtensionMgr->finalSize * 
				self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I];
			
			/*printf( "receiving %ld bytes\n", incomingViaShadowArrayBytes );*/
			(void)MPI_Irecv( recvLocation, incomingViaShadowArrayBytes, MPI_BYTE,
				proc_I, SHADOW_PARTICLES, self->swarm->comm,
				self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] );
			
			recvLocation += incomingViaShadowArrayBytes;
		}
	}
}

void _ParticleShadowSync_FinishReceiveOfIncomingParticles( ParticleCommHandler* pCommHandler ) {
	ParticleShadowSync *self = (ParticleShadowSync*)pCommHandler;
	MPI_Status			status;
	Processor_Index			proc_I;
	ShadowInfo*		        cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*		        procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index		        nbr_I;
	int i = 0, j = 0;
	int shadowParticleCounter;
	int shadowCell = 0;

	shadowParticleCounter = 0;
	/* TODO: may be worth converting the below into an MPI_Test loop */
	for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
		proc_I = procNbrInfo->procNbrTbl[nbr_I];
		
		if( self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] > 0 ){
			(void)MPI_Wait( self->particlesArrivingFromNbrShadowCellsHandles[nbr_I], &status );
		}
	}

	for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
		for( i=0; i<cellShadowInfo->procShadowCnt[nbr_I]; i++ ){
			
			shadowCell = cellShadowInfo->procShadowTbl[nbr_I][i];
			for( j=0; j<self->particlesArrivingFromNbrShadowCellCounts[nbr_I][i]; j++ ){

				Swarm_AddShadowParticleToShadowCell( self->swarm, shadowCell, shadowParticleCounter );
				shadowParticleCounter++;
			}
		}
	}
}

void _ParticleShadowSync_SendShadowParticles( ParticleCommHandler *self )
{
	ShadowInfo*		        cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*		        procNbrInfo = cellShadowInfo->procNbrInfo;
	Processor_Index			proc_I;
	int i = 0, j = 0, k = 0, cell = 0;
	unsigned int	arrayIndex = 0;
	long			arraySize = 0;
	unsigned int	pIndex = 0;

	self->shadowParticlesLeavingMeHandles = Memory_Alloc_Array_Unnamed( MPI_Request*, procNbrInfo->procNbrCnt );
	self->shadowParticlesLeavingMe = Memory_Alloc_Array_Unnamed( Particle*, procNbrInfo->procNbrCnt );

	for( i=0; i<procNbrInfo->procNbrCnt; i++ ){
		proc_I = procNbrInfo->procNbrTbl[i];

		if( self->shadowParticlesLeavingMeTotalCounts[i] != 0 ){

			self->shadowParticlesLeavingMeHandles[i] = Memory_Alloc_Array_Unnamed( MPI_Request, 1 );

			arraySize =  self->swarm->particleExtensionMgr->finalSize * self->shadowParticlesLeavingMeTotalCounts[i];
			self->shadowParticlesLeavingMe[i] = Memory_Alloc_Bytes( arraySize, "Particle", "pCommHandler->outgoingPArray" );
			memset( self->shadowParticlesLeavingMe[i], 0, arraySize );

			arrayIndex = 0;
			for( j=0; j<cellShadowInfo->procShadowedCnt[i]; j++ ){
				cell = cellShadowInfo->procShadowedTbl[i][j];

				for( k=0; k<self->swarm->cellParticleCountTbl[cell]; k++ ){
					pIndex = self->swarm->cellParticleTbl[cell][k];
				
					Swarm_CopyParticleOffSwarm( self->swarm,
							self->shadowParticlesLeavingMe[i], arrayIndex++,
							pIndex );
				}
			}
			
			/*printf( "sending %ld bytes\n", self->shadowParticlesLeavingMeTotalCounts[i] * self->swarm->particleExtensionMgr->finalSize );*/
			MPI_Issend( self->shadowParticlesLeavingMe[i],
			self->shadowParticlesLeavingMeTotalCounts[i] * self->swarm->particleExtensionMgr->finalSize,
			MPI_BYTE, proc_I, SHADOW_PARTICLES, self->swarm->comm,
			self->shadowParticlesLeavingMeHandles[i] );
		}
	}
}

void ParticleShadowSync_HandleParticleMovementBetweenProcs( ParticleCommHandler* pCommsHandler ) {
	ParticleShadowSync*	self = (ParticleShadowSync*)pCommsHandler;

	Journal_DPrintfL( self->debug, 1, "In %s(), for swarm \"%s\":\n", __func__, self->swarm->name );
	if ( 1 == self->swarm->nProc ) {
		Journal_DPrintfL( self->debug, 1, "Serial run -> nothing to communicate in %s, returning.\n", __func__ );
		Stream_UnIndentBranch( Swarm_Debug );
		return;
	}

	Stream_IndentBranch( Swarm_Debug );
	
	if ( self->swarm->cellShadowCount > 0 ) {
		/* Allocate the recv count arrays and handles */
		
		/*---_ParticleCommHandler_AllocateIncomingCountArrays( (ParticleCommHandler*)self );
		_ParticleCommHandler_AllocateOutgoingCountArrays( (ParticleCommHandler*)self );*/
		self->allocateIncomingCountArrays( (ParticleCommHandler*)self );
		self->allocateOutgoingCountArrays( (ParticleCommHandler*)self );



		/* First thing to do is begin non-blocking receive of incoming particles (for latency hiding) */
		/*---_ParticleCommHandler_BeginReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );*/
		self->beginReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );


		/* Do a blocking send of outgoing counts, so our nbrs know what to receive */
		//_ParticleCommHandler_SendParticleTotalsInShadowCellsToNbrs( (ParticleCommHandler*)self );
	
		/*---_ParticleShadowSync_SendParticleTotalsInShadowCellsToNbrs( (ParticleCommHandler*)self );*/
		self->sendOutgoingParticleCounts( (ParticleCommHandler*)self );

		
		/* Now need to make sure that incoming particle counts are here, then begin receiving particles
		   (We do this as early as possible for latency hiding purposes */
		
		/*---_ParticleShadowSync_FinishReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );*/
		self->finishReceiveOfIncomingParticleCounts( (ParticleCommHandler*)self );


		/*---_ParticleShadowSync_BeginReceiveOfIncomingParticles( (ParticleCommHandler*)self );
		_ParticleShadowSync_SendShadowParticles( (ParticleCommHandler*)self );*/
		self->beginReceiveOfIncomingParticles( (ParticleCommHandler*)self );
		self->beginSendingParticles( (ParticleCommHandler*)self );


		/*---_ParticleCommHandler_ConfirmOutgoingSendsCompleted( (ParticleCommHandler*)self );
		_ParticleShadowSync_FinishReceiveOfIncomingParticles( self );*/
		self->confirmOutgoingSendsCompleted( (ParticleCommHandler*)self );
		self->finishReceiveOfIncomingParticlesAndUpdateIndices( (ParticleCommHandler*)self );


		/*---_ParticleCommHandler_FreeOutgoingArrays( (ParticleCommHandler*)self );
		_ParticleCommHandler_FreeIncomingArrays( (ParticleCommHandler*)self );*/
		self->freeIncomingArrays( (ParticleCommHandler*)self );
		self->freeOutgoingArrays( (ParticleCommHandler*)self );


		/* This is where our own Data structure comes into play  Jules and Raq*/
		//----_ParticleCommHandler_AllocateIncomingParticlesArrays( (ParticleCommHandler*)self );
		//----_ParticleCommHandler_BeginReceiveOfIncomingParticles( (ParticleCommHandler*)self );
			
		/* OK, now begin sending out particles we know need to go to nbrs */
		//----_ParticleCommHandler_AllocateOutgoingParticlesArrays( (ParticleCommHandler*)self );
		//----_ParticleCommHandler_BeginSendingParticlesInShadowCellsToNbrs( (ParticleCommHandler*)self );
	}
	
	MPI_Barrier( self->swarm->comm );

	_ParticleCommHandler_ZeroShadowCommStrategyCounters( (ParticleCommHandler*)self );
	
	Stream_UnIndentBranch( Swarm_Debug );
}

void ParticleShadowSync_FindParticlesThatHaveMovedOutsideMyDomain( ParticleShadowSync* self )
{
	Particle_Index		particlesOutsideDomainSize = 0;
	GlobalParticle*         currParticle = NULL;
	Particle_Index		lParticle_I = 0;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	self->particlesOutsideDomainTotalCount = 0;
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

void ParticleShadowSync_GetCountOfParticlesOutsideDomainPerProcessor(
	ParticleShadowSync*	self,
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



