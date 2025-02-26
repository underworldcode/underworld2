/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

	
#ifndef __StgDomain_Swarm_ParticleMovementHandler_h__
#define __StgDomain_Swarm_ParticleMovementHandler_h__

	/** Textual name of this class */
	extern const Type ParticleMovementHandler_Type;

	#define __ParticleMovementHandler \
		__ParticleCommHandler \
		/* Virtual info */ \
		/* Member info */ \
		Index                           globalParticlesArrivingMyDomainCount; \
		Index                           globalParticlesOutsideDomainTotal; \
		Bool                            useGlobalFallbackCommStrategy; \
		Bool                            defensive;


	struct ParticleMovementHandler { __ParticleMovementHandler };	

	/* --- virtual functions --- */

	/** Constructor interface */
	void* ParticleMovementHandler_DefaultNew( Name name );
	
	ParticleMovementHandler* ParticleMovementHandler_New(
			Name name,
			Bool useGlobalFallbackCommStrategy
			);
	
	/** Private Constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PARTICLEMOVEMENTHANDLER_DEFARGS \
                PARTICLECOMMHANDLER_DEFARGS, \
                Bool  useGlobalFallbackCommStrategy

	#define PARTICLEMOVEMENTHANDLER_PASSARGS \
                PARTICLECOMMHANDLER_PASSARGS, \
	        useGlobalFallbackCommStrategy

	ParticleMovementHandler* _ParticleMovementHandler_New(  PARTICLEMOVEMENTHANDLER_DEFARGS  );
	
	/** Variable initialiser */
	void _ParticleMovementHandler_Init(
		ParticleMovementHandler*     self,
		Bool                     useGlobalFallbackCommStrategy
		);

	/** Stg_Class_Print() implementation */
	void _ParticleMovementHandler_Print( void* pCommsHandler, Stream* stream );
	
	void _ParticleMovementHandler_AssignFromXML( void* pCommsHandler, Stg_ComponentFactory* cf, void* data );
	
	void _ParticleMovementHandler_Build( void* pCommsHandler, void *data );
	
	void _ParticleMovementHandler_Initialise( void* pCommsHandler, void *data );
	
	void _ParticleMovementHandler_Execute( void* pCommsHandler, void *data );

	void _ParticleMovementHandler_Destroy( void* pCommsHandler, void *data );
	
	/** Copy */
	#define ParticleMovementHandler_Copy( self ) \
		(ParticleMovementHandler*)ParticleMovementHandler_CopyFunc( self, NULL, False, NULL, NULL )
	#define ParticleMovementHandler_DeepCopy( self ) \
		(ParticleMovementHandler*)ParticleMovementHandler_CopyFunc( self, NULL, True, NULL, NULL )
	
	void* _ParticleMovementHandler_CopyFunc( void* ParticleMovementHandler, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Stg_Class_Delete() implementation */
	void _ParticleMovementHandler_Delete(void* pCommsHandler );
	
	/* --- Public virtual function interfaces --- */
	
	/** Handle particle movement between processors */
	void ParticleMovementHandler_HandleParticleMovementBetweenProcs( ParticleCommHandler* pCommsHandler );

	/* --- virtual function implementations --- */

	/* +++ Global fallback method related +++ */
	void ParticleMovementHandler_DoGlobalFallbackCommunication( ParticleMovementHandler* self );

	void ParticleMovementHandler_FindParticlesThatHaveMovedOutsideMyDomain( ParticleMovementHandler* self );

	void ParticleMovementHandler_GetCountOfParticlesOutsideDomainPerProcessor(
		ParticleMovementHandler*	self,
		Particle_Index**	globalParticlesOutsideDomainCountsPtr,
		Particle_Index*		maxGlobalParticlesOutsideDomainCountPtr,
		Particle_Index*		globalParticlesOutsideDomainTotalPtr );
		
	void ParticleMovementHandler_ShareAndUpdateParticlesThatHaveMovedOutsideDomains(
		ParticleMovementHandler* self,
		Particle_Index*      globalParticlesArrivingMyDomainCountPtr,
		Particle_Index*      globalParticlesOutsideDomainTotalPtr );

	void ParticleMovementHandler_EnsureParticleCountLeavingDomainsEqualsCountEnteringGlobally( ParticleMovementHandler* self );

	void ParticleMovementHandler_ZeroGlobalCommStrategyCounters( ParticleMovementHandler* self );

	/* --- private functions --- */
	/* +++ Managment of particle array insertions/deletions +++ */
	Particle_Index ParticleMovementHandler_FindFreeSlotAndPrepareForInsertion( ParticleCommHandler* self );

	void ParticleMovementHandler_FillRemainingHolesInLocalParticlesArray( ParticleCommHandler*	self );

	Particle_Index* ParticleMovementHandler_MergeListsOfUnfilledParticleSlots( ParticleCommHandler* self );

	void ParticleMovementHandler_PrintParticleSlotsYetToFill( ParticleCommHandler* self );
	void ParticleMovementHandler_FinishReceiveAndUpdateShadowParticlesEnteringMyDomain( ParticleCommHandler* self );

#endif

