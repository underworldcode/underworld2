/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

	
#ifndef __StgDomain_Swarm_ParticleCommHandler_h__
#define __StgDomain_Swarm_ParticleCommHandler_h__

	
	/** Textual name of this class */
	extern const Type ParticleCommHandler_Type;

	typedef void (ParticleCommHandler_CommFunction)	( ParticleCommHandler * );
	typedef void (ParticleCommHandler_AllocateOutgoingCountArrays) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_AllocateOutgoingParticleArrays) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_FreeOutgoingArrays) ( ParticleCommHandler * );
	
	typedef void (ParticleCommHandler_AllocateIncomingCountArrays) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_AllocateIncomingParticleArrays) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_FreeIncomingArrays) ( ParticleCommHandler * );
	
	typedef void (ParticleCommHandler_BeginReceiveOfIncomingParticleCounts) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_FinishReceiveOfIncomingParticleCounts) ( ParticleCommHandler * );
	
	typedef void (ParticleCommHandler_BeginReceiveOfIncomingParticles) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_FinishReceiveOfIncomingParticlesAndUpdateIndices) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_SendOutgoingParticleCounts) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_BeginSendingParticles) ( ParticleCommHandler * );
	typedef void (ParticleCommHandler_ConfirmOutgoingSendsCompleted) ( ParticleCommHandler * );


	#define __ParticleCommHandler \
		__Stg_Component \
		AbstractContext*			context; \
		/* Virtual info */ \
		ParticleCommHandler_AllocateOutgoingCountArrays						*allocateOutgoingCountArrays; \
		ParticleCommHandler_AllocateOutgoingParticleArrays						*allocateOutgoingParticleArrays; \
		ParticleCommHandler_FreeOutgoingArrays									*freeOutgoingArrays; \
		ParticleCommHandler_AllocateIncomingCountArrays						*allocateIncomingCountArrays; \
		ParticleCommHandler_AllocateIncomingParticleArrays						*allocateIncomingParticleArrays; \
		ParticleCommHandler_FreeIncomingArrays									*freeIncomingArrays; \
		ParticleCommHandler_BeginReceiveOfIncomingParticleCounts				*beginReceiveOfIncomingParticleCounts; \
		ParticleCommHandler_FinishReceiveOfIncomingParticleCounts				*finishReceiveOfIncomingParticleCounts; \
		ParticleCommHandler_BeginReceiveOfIncomingParticles					*beginReceiveOfIncomingParticles; \
		ParticleCommHandler_FinishReceiveOfIncomingParticlesAndUpdateIndices	*finishReceiveOfIncomingParticlesAndUpdateIndices; \
		ParticleCommHandler_SendOutgoingParticleCounts							*sendOutgoingParticleCounts; \
		ParticleCommHandler_BeginSendingParticles								*beginSendingParticles; \
		ParticleCommHandler_ConfirmOutgoingSendsCompleted						*confirmOutgoingSendsCompleted; \
		ParticleCommHandler_CommFunction										*_commFunction;\
		/* Member info */ \
		Stream*				debug; \
		Swarm*				swarm; \
		Particle_Index* 		shadowParticlesLeavingMeIndices; \
		Index				shadowParticlesLeavingMeTotalCount; \
		Index				shadowParticlesLeavingMeUnfilledCount; \
		Index				currShadowParticleLeavingMeIndex; \
		Index				currParticleLeavingMyDomainIndex; \
		\
		Particle_Index* 		particlesOutsideDomainIndices; \
		Index				particlesOutsideDomainTotalCount; \
		Index				particlesOutsideDomainUnfilledCount; \
		\
		/** cnts of [nbr][st_cell] outgoing particles */ \
		Particle_Index**		shadowParticlesLeavingMeCountsPerCell; \
		/** cnts of [nbr] total outgoing particles via my shadow cells to nbr procs */ \
		Particle_Index*			shadowParticlesLeavingMeTotalCounts; \
		/** transfer array [nbr] of particles to send */ \
		Particle**			shadowParticlesLeavingMe; \
		MPI_Request**			shadowParticlesLeavingMeHandles; \
		/** cnts of [nbr][st_cell] incoming particles */ \
		Particle_Index**		particlesArrivingFromNbrShadowCellCounts; \
		MPI_Request**			particlesArrivingFromNbrShadowCellCountsHandles; \
		/** cnts of [nbr] total incoming particles via shadow cells of nbr procs */\
		Particle_Index*			particlesArrivingFromNbrShadowCellsTotalCounts; \
		/** transfer array [nbr] of particles to recv */ \
		Particle**			particlesArrivingFromNbrShadowCells; \
		MPI_Request**			particlesArrivingFromNbrShadowCellsHandles;

	struct ParticleCommHandler { __ParticleCommHandler };	

	/* --- virtual functions --- */

	/** Constructor interface */
	
	/** Private Constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PARTICLECOMMHANDLER_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                Bool                                                                                                            initFlag, \
                ParticleCommHandler_AllocateOutgoingCountArrays*                                            _allocateOutgoingCountArrays, \
                ParticleCommHandler_AllocateOutgoingParticleArrays*                                      _allocateOutgoingParticleArrays, \
                ParticleCommHandler_FreeOutgoingArrays*                                                              _freeOutgoingArrays, \
                ParticleCommHandler_AllocateIncomingCountArrays*                                            _allocateIncomingCountArrays, \
                ParticleCommHandler_AllocateIncomingParticleArrays*                                      _allocateIncomingParticleArrays, \
                ParticleCommHandler_FreeIncomingArrays*                                                              _freeIncomingArrays, \
                ParticleCommHandler_BeginReceiveOfIncomingParticleCounts*                          _beginReceiveOfIncomingParticleCounts, \
                ParticleCommHandler_FinishReceiveOfIncomingParticleCounts*                        _finishReceiveOfIncomingParticleCounts, \
                ParticleCommHandler_BeginReceiveOfIncomingParticles*                                    _beginReceiveOfIncomingParticles, \
                ParticleCommHandler_FinishReceiveOfIncomingParticlesAndUpdateIndices*  _finishReceiveOfIncomingParticlesAndUpdateIndices, \
                ParticleCommHandler_SendOutgoingParticleCounts*                                              _sendOutgoingParticleCounts, \
                ParticleCommHandler_BeginSendingParticles*                                                        _beginSendingParticles, \
                ParticleCommHandler_ConfirmOutgoingSendsCompleted*                                        _confirmOutgoingSendsCompleted, \
                ParticleCommHandler_CommFunction*                                                                          _commFunction

	#define PARTICLECOMMHANDLER_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        initFlag,                                          \
	        _allocateOutgoingCountArrays,                      \
	        _allocateOutgoingParticleArrays,                   \
	        _freeOutgoingArrays,                               \
	        _allocateIncomingCountArrays,                      \
	        _allocateIncomingParticleArrays,                   \
	        _freeIncomingArrays,                               \
	        _beginReceiveOfIncomingParticleCounts,             \
	        _finishReceiveOfIncomingParticleCounts,            \
	        _beginReceiveOfIncomingParticles,                  \
	        _finishReceiveOfIncomingParticlesAndUpdateIndices, \
	        _sendOutgoingParticleCounts,                       \
	        _beginSendingParticles,                            \
	        _confirmOutgoingSendsCompleted,                    \
	        _commFunction                                    

	ParticleCommHandler* _ParticleCommHandler_New(  PARTICLECOMMHANDLER_DEFARGS  );
	
	/** Variable initialiser */
	void _ParticleCommHandler_Init(
		ParticleCommHandler*     self
		);

	/** Stg_Class_Print() implementation */
	void _ParticleCommHandler_Print( void* pCommsHandler, Stream* stream );
	
	void _ParticleCommHandler_AssignFromXML( void* pCommsHandler, Stg_ComponentFactory* cf, void* data );
	
	void _ParticleCommHandler_Build( void* pCommsHandler, void *data );
	
	void _ParticleCommHandler_Initialise( void* pCommsHandler, void *data );
	
	void _ParticleCommHandler_Execute( void* pCommsHandler, void *data );

	void _ParticleCommHandler_Destroy( void* pCommsHandler, void *data );
	
	/** Copy */
	#define ParticleCommHandler_Copy( self ) \
		(ParticleCommHandler*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ParticleCommHandler_DeepCopy( self ) \
		(ParticleCommHandler*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ParticleCommHandler_Copy( void* particleCommHandler, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Stg_Class_Delete() implementation */
	void _ParticleCommHandler_Delete(void* pCommsHandler );
	
	/* --- Public virtual function interfaces --- */
	
	/* --- virtual function implementations --- */

	/* --- private functions --- */

	/* +++ Comms via shadow cell related +++ */

	void _ParticleCommHandler_AllocateOutgoingCountArrays( ParticleCommHandler* self );
	void _ParticleCommHandler_AllocateOutgoingParticlesArrays( ParticleCommHandler* self );
	void _ParticleCommHandler_FreeOutgoingArrays( ParticleCommHandler* self );

	void _ParticleCommHandler_AllocateIncomingCountArrays( ParticleCommHandler* self );
	void _ParticleCommHandler_AllocateIncomingParticlesArrays( ParticleCommHandler* self );
	void _ParticleCommHandler_FreeIncomingArrays( ParticleCommHandler* self );

	void _ParticleCommHandler_BeginReceiveOfIncomingParticleCounts( ParticleCommHandler* self );
	void _ParticleCommHandler_FinishReceiveOfIncomingParticleCounts( ParticleCommHandler* self );

	void _ParticleCommHandler_BeginReceiveOfIncomingParticles( ParticleCommHandler* self );

	void _ParticleCommHandler_SendParticleTotalsInShadowCellsToNbrs( ParticleCommHandler* self );

	void _ParticleCommHandler_BeginSendingParticlesInShadowCellsToNbrs( ParticleCommHandler* self );
	void _ParticleCommHandler_ConfirmOutgoingSendsCompleted( ParticleCommHandler* self );

	void _ParticleCommHandler_ZeroShadowCommStrategyCounters( ParticleCommHandler* self );


	/* +++ Statistics Printing +++ */
	void _ParticleCommHandler_PrintCommunicationVolumeStats( ParticleCommHandler* self, double startTime, Stream* info );

#endif

