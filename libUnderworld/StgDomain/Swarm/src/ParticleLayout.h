/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_ParticleLayout_h__
#define __StgDomain_Swarm_ParticleLayout_h__

/** @see ParticleLayout_SetInitialCounts */
typedef void (ParticleLayout_SetInitialCountsFunction)    ( void* particleLayout, void* swarm );
/** @see ParticleLayout_InitialiseParticles */ 
typedef void (ParticleLayout_InitialiseParticlesFunction) ( void* particleLayout, void* swarm );
	
extern const Type ParticleLayout_Type;
	
/* ParticleLayout information */
#define __ParticleLayout \
    __Stg_Component \
    AbstractContext*										context; \
    /* virtual information */ \
    ParticleLayout_SetInitialCountsFunction*		_setInitialCounts; \
    ParticleLayout_InitialiseParticlesFunction*	_initialiseParticles; \
    /* member data */ \
    Stream*													debug; /**< For sending log messages to */ \
    CoordSystem											coordSystem; /**< Set once by subclass init */ \
    Bool														weightsInitialisedAtStartup; /**< Set once by subclass init */

struct ParticleLayout { __ParticleLayout };
	
/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PARTICLELAYOUT_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                ParticleLayout_SetInitialCountsFunction*               _setInitialCounts, \
                ParticleLayout_InitialiseParticlesFunction*         _initialiseParticles, \
                CoordSystem                                                  coordSystem, \
                Bool                                         weightsInitialisedAtStartup

	#define PARTICLELAYOUT_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _setInitialCounts,           \
	        _initialiseParticles,        \
	        coordSystem,                 \
	        weightsInitialisedAtStartup

ParticleLayout* _ParticleLayout_New(  PARTICLELAYOUT_DEFARGS  );

/* Initialise implementation */
void _ParticleLayout_Init( 
      void*                     particleLayout,
      AbstractContext*          context,
      CoordSystem               coordSystem,
      Bool                      weightsInitialisedAtStartup );
	
/* Stg_Class_Delete a ParticleLayout construct */
void _ParticleLayout_Delete( void* particleLayout );
	
/* Print a ParticleLayout construct */
void _ParticleLayout_Print( void* particleLayout, Stream* stream );

void _ParticleLayout_Destroy( void* particleLayout, void* data );
	
/* Copy */
#define ParticleLayout_Copy( self )                                     \
    (ParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
#define ParticleLayout_DeepCopy( self )                                 \
    (ParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
void* _ParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void _ParticleLayout_AssignFromXML( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	
/** Setup the particle tables in a Swarm Class, both local and shadow values.
 *  Must setup cellParticleCountTbl, cellParticleSizeTbl, cellParticleTbl */
void ParticleLayout_SetInitialCounts( void* particleLayout, void* swarm );
	
/** Initialise startup positions of all particles in given swarm. */
void ParticleLayout_InitialiseParticles( void* particleLayout, void* swarm );
	
#endif /* __StgDomain_Swarm_ParticleLayout_h__ */

