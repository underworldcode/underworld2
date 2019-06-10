/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

	
#ifndef __StgDomain_Swarm_ParticleShadowSync_h__
#define __StgDomain_Swarm_ParticleShadowSync_h__

	/** Textual name of this class */
	extern const Type ParticleShadowSync_Type;

	#define __ParticleShadowSync \
		__ParticleCommHandler 
		/* Virtual info */ 
		/* Member info */ 


	struct ParticleShadowSync { __ParticleShadowSync };	

	/* --- virtual functions --- */

	/** Constructor interface */
	void* ParticleShadowSync_DefaultNew( Name name );
	
	ParticleShadowSync* ParticleShadowSync_New(
			Name name,
			void* swarm
			);
	
	/** Private Constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PARTICLESHADOWSYNC_DEFARGS \
                PARTICLECOMMHANDLER_DEFARGS

	#define PARTICLESHADOWSYNC_PASSARGS \
                PARTICLECOMMHANDLER_PASSARGS

	ParticleShadowSync* _ParticleShadowSync_New(  PARTICLESHADOWSYNC_DEFARGS  );
	
	/** Variable initialiser */
	void _ParticleShadowSync_Init(
		ParticleShadowSync*     self );

	/** Stg_Class_Print() implementation */
	void _ParticleShadowSync_Print( void* pCommsHandler, Stream* stream );
	
	void _ParticleShadowSync_AssignFromXML( void* pCommsHandler, Stg_ComponentFactory* cf, void* data );
	
	void _ParticleShadowSync_Build( void* pCommsHandler, void *data );
	
	void _ParticleShadowSync_Initialise( void* pCommsHandler, void *data );
	
	void _ParticleShadowSync_Execute( void* pCommsHandler, void *data );

	void _ParticleShadowSync_Destroy( void* pCommsHandler, void *data );
	
	/** Copy */
	#define ParticleShadowSync_Copy( self ) \
		(ParticleShadowSync*)ParticleShadowSync_CopyFunc( self, NULL, False, NULL, NULL )
	#define ParticleShadowSync_DeepCopy( self ) \
		(ParticleShadowSync*)ParticleShadowSync_CopyFunc( self, NULL, True, NULL, NULL )
	
	void* _ParticleShadowSync_CopyFunc( void* ParticleShadowSync, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Stg_Class_Delete() implementation */
	void _ParticleShadowSync_Delete(void* pCommsHandler );
	
	/* --- Public virtual function interfaces --- */
	
	/** Handle particle movement between processors */
	void ParticleShadowSync_HandleParticleMovementBetweenProcs( ParticleCommHandler* pCommsHandler );

	/* --- virtual function implementations --- */

	/* +++ Global fallback method related +++ */
	void ParticleShadowSync_FindParticlesThatHaveMovedOutsideMyDomain( ParticleShadowSync* self );

	void ParticleShadowSync_GetCountOfParticlesOutsideDomainPerProcessor(
		ParticleShadowSync*	self,
		Particle_Index**	globalParticlesOutsideDomainCountsPtr,
		Particle_Index*		maxGlobalParticlesOutsideDomainCountPtr,
		Particle_Index*		globalParticlesOutsideDomainTotalPtr );
		
	void _ParticleShadowSync_FinishReceiveOfIncomingParticleCounts( ParticleCommHandler* self );
	void _ParticleShadowSync_BeginReceiveOfIncomingParticles( ParticleCommHandler* self );
	void _ParticleShadowSync_SendParticleTotalsInShadowCellsToNbrs( ParticleCommHandler* self );
	void _ParticleShadowSync_SendShadowParticles( ParticleCommHandler* self );
	void _ParticleShadowSync_FinishReceiveOfIncomingParticles( ParticleCommHandler* pCommHandler );

	/* --- private functions --- */

#endif

