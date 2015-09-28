/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_SpaceFillerParticleLayout_h__
#define __StgDomain_Swarm_SpaceFillerParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type SpaceFillerParticleLayout_Type;
	
	extern const Index SpaceFillerParticleLayout_Invalid;

	/* SpaceFillerParticleLayout information */
	#define __SpaceFillerParticleLayout \
		__GlobalParticleLayout \
		\
		Dimension_Index              dim;                      \
		SobolGenerator*              sobolGenerator[3];        \

	struct SpaceFillerParticleLayout { __SpaceFillerParticleLayout };
	
   /* Create a new SpaceFillerParticleLayout and initialise */
   SpaceFillerParticleLayout* SpaceFillerParticleLayout_New( 
		Name             name,
      AbstractContext* context, 
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      unsigned int     totalInitialParticles, 
      double           averageInitialParticlesPerCell,
		Dimension_Index  dim );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SPACEFILLERPARTICLELAYOUT_DEFARGS \
                GLOBALPARTICLELAYOUT_DEFARGS, \
                Dimension_Index  dim

	#define SPACEFILLERPARTICLELAYOUT_PASSARGS \
                GLOBALPARTICLELAYOUT_PASSARGS, \
	        dim

   SpaceFillerParticleLayout* _SpaceFillerParticleLayout_New(  SPACEFILLERPARTICLELAYOUT_DEFARGS  );
	
   void _SpaceFillerParticleLayout_Init( 
      void*                   spaceFillerParticleLayout, 
      Dimension_Index         dim ); 
	
	/* 'Stg_Class' Stuff */
	void _SpaceFillerParticleLayout_Delete( void* spaceFillerParticleLayout );
	void _SpaceFillerParticleLayout_Print( void* spaceFillerParticleLayout, Stream* stream );
	#define SpaceFillerParticleLayout_Copy( self ) \
		(SpaceFillerParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SpaceFillerParticleLayout_DeepCopy( self ) \
		(SpaceFillerParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _SpaceFillerParticleLayout_Copy( void* spaceFillerParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _SpaceFillerParticleLayout_DefaultNew( Name name ) ;
	void _SpaceFillerParticleLayout_AssignFromXML( void* spaceFillerParticleLayout, Stg_ComponentFactory *cf, void* data );
	void _SpaceFillerParticleLayout_Build( void* spaceFillerParticleLayout, void* data );
	void _SpaceFillerParticleLayout_Initialise( void* spaceFillerParticleLayout, void* data );
	void _SpaceFillerParticleLayout_Execute( void* spaceFillerParticleLayout, void* data );
	void _SpaceFillerParticleLayout_Destroy( void* spaceFillerParticleLayout, void* data );
	
	/* Initialises the coordinates of a cell's particle */
	void _SpaceFillerParticleLayout_InitialiseParticles( void* spaceFillerParticleLayout, void* swarm );
	void _SpaceFillerParticleLayout_InitialiseParticle( 
			void*             spaceFillerParticleLayout,
			void*             _swarm,
			Particle_Index    newParticle_I,
			void*             particle );


#endif /* __StgDomain_Swarm_SpaceFillerParticleLayout_h__ */

