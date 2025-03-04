/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_GlobalParticleLayout_h__
#define __StgDomain_Swarm_GlobalParticleLayout_h__
	
	/** @see GlobalParticleLayout_InitialiseParticle */
	typedef void (GlobalParticleLayout_InitialiseParticleFunction) ( 
			void*              particleLayout,
			void*              swarm, 
			Particle_Index     newParticle_I, 
			void*              particle );
	
	/* Textual name of this class */
	extern const Type GlobalParticleLayout_Type;
	
	/* ParticleLayout information */
	#define __GlobalParticleLayout \
		__ParticleLayout \
		/* virtual information */ \
		GlobalParticleLayout_InitialiseParticleFunction*	_initialiseParticle; \
		/* member data */ \
		Particle_Index	totalInitialParticles;          /**< Subclasses must set during init */\
		double			averageInitialParticlesPerCell; /**< Subclasses must set during init */\
      Bool           mustUseAllParticles;            /** signifies that all particles from layout must fall within the domain */

	struct GlobalParticleLayout { __GlobalParticleLayout };
	
	/* No "_New" and "_Init" as this is an abstract class */
	
   /* Creation implementation / Virtual constructor */

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define GLOBALPARTICLELAYOUT_DEFARGS \
                PARTICLELAYOUT_DEFARGS, \
                GlobalParticleLayout_InitialiseParticleFunction*             _initialiseParticle, \
                Particle_Index                                             totalInitialParticles, \
                double                                            averageInitialParticlesPerCell

	#define GLOBALPARTICLELAYOUT_PASSARGS \
                PARTICLELAYOUT_PASSARGS, \
	        _initialiseParticle,            \
	        totalInitialParticles,          \
	        averageInitialParticlesPerCell

GlobalParticleLayout* _GlobalParticleLayout_New(  GLOBALPARTICLELAYOUT_DEFARGS  );

	void  _GlobalParticleLayout_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data );

   void _GlobalParticleLayout_Init( 
      void*                                               particleLayout,
      Particle_Index                                      totalInitialParticles,
      double                                              averageInitialParticlesPerCell );
	
	void _GlobalParticleLayout_Delete( void* particleLayout );
	
	void _GlobalParticleLayout_Print( void* particleLayout, Stream* stream );
	
	void _GlobalParticleLayout_Destroy( void* particleLayout, void* data );
	/* Copy */
	#define GlobalParticleLayout_Copy( self ) \
		(GlobalParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define GlobalParticleLayout_DeepCopy( self ) \
		(GlobalParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _GlobalParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _GlobalParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm );

	/** Initialise all particles in system, adding it to the swarm if its within the local processor's cell. */
	void _GlobalParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm );
	
	/** Init given particle's position.
	 *     @param newParticle_I The global particle index (of all particles, not just local processor) */
	void GlobalParticleLayout_InitialiseParticle( 
			void*             particleLayout, 
			void*             swarm, 
			Particle_Index    newParticle_I,
			void*             particle );
	
#endif /* __StgDomain_Swarm_GlobalParticleLayout_h__ */

