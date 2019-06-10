/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_RandomParticleLayout_h__
#define __StgDomain_Swarm_RandomParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type RandomParticleLayout_Type;
	
	/* RandomParticleLayout information */
	#define __RandomParticleLayout \
		__PerCellParticleLayout \
		\
		Particle_InCellIndex         cellParticleCount; /**< Number of particles in each cell to create initially */ \
		unsigned int                 seed;              /**< The random seed used to generate positions. */

	struct RandomParticleLayout { __RandomParticleLayout };
	
	/* Create a new RandomParticleLayout and initialise */
   RandomParticleLayout* RandomParticleLayout_New( 
      Name name, 
      AbstractContext* context,
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      Particle_InCellIndex cellParticleCount, 
      unsigned int         seed );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define RANDOMPARTICLELAYOUT_DEFARGS \
                PERCELLPARTICLELAYOUT_DEFARGS

	#define RANDOMPARTICLELAYOUT_PASSARGS \
                PERCELLPARTICLELAYOUT_PASSARGS

   RandomParticleLayout* _RandomParticleLayout_New(  RANDOMPARTICLELAYOUT_DEFARGS  );

	void _RandomParticleLayout_Init( 
			void*                randomParticleLayout, 
			Particle_InCellIndex cellParticleCount, 
			unsigned int         seed);

	/* 'Class' Stuff */
	void _RandomParticleLayout_Delete( void* randomParticleLayout );
	void _RandomParticleLayout_Print( void* randomParticleLayout, Stream* stream );
	
	#define RandomParticleLayout_Copy( self ) \
		(RandomParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define RandomParticleLayout_DeepCopy( self ) \
		(RandomParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _RandomParticleLayout_Copy( void* randomParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _RandomParticleLayout_DefaultNew( Name name ) ;
	void _RandomParticleLayout_AssignFromXML( void* randomParticleLayout, Stg_ComponentFactory* cf, void* data );
	void _RandomParticleLayout_Build( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Initialise( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Execute( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Destroy( void* randomParticleLayout, void* data );
	
	Particle_InCellIndex _RandomParticleLayout_InitialCount( void* randomParticleLayout, void* celllayout,  Cell_Index cell_I );
	
	void _RandomParticleLayout_InitialiseParticlesOfCell( void* randomParticleLayout, void* _swarm, Cell_Index cell_I );


	
#endif /* __StgDomain_Swarm_RandomParticleLayout_h__ */

