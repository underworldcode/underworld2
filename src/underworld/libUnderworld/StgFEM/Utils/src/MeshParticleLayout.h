/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Swarm_MeshParticleLayout_h__
#define __StgFEM_Swarm_MeshParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type MeshParticleLayout_Type;
	
	/* MeshParticleLayout information */
	#define __MeshParticleLayout \
		__PerCellParticleLayout \
		\
		Mesh*			     mesh; \
		Particle_InCellIndex         cellParticleCount; /**< Number of particles in each cell to create initially */ \
		unsigned int                 seed;              /**< The random seed used to generate positions. */ \
        unsigned int                 filltype;          /**< fill type.  0:sobol, 1:random */ \
        SobolGenerator*              sobolGenerator[3];

	struct MeshParticleLayout { __MeshParticleLayout };
	
	/* Create a new MeshParticleLayout and initialise */
   MeshParticleLayout* MeshParticleLayout_New( 
      Name                 name, 
      AbstractContext* context,
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      Mesh*            mesh,
      Particle_InCellIndex cellParticleCount, 
      unsigned int         seed,
      unsigned int         filltype  );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESHPARTICLELAYOUT_DEFARGS \
                PERCELLPARTICLELAYOUT_DEFARGS, \
                Mesh*                              mesh, \
                Particle_InCellIndex  cellParticleCount, \
                unsigned int                       seed

	#define MESHPARTICLELAYOUT_PASSARGS \
                PERCELLPARTICLELAYOUT_PASSARGS, \
	        mesh,              \
	        cellParticleCount, \
	        seed             

   MeshParticleLayout* _MeshParticleLayout_New(  MESHPARTICLELAYOUT_DEFARGS  );

	void _MeshParticleLayout_Init( 
			void*                meshParticleLayout, 
         Mesh*                mesh,
			Particle_InCellIndex cellParticleCount, 
			unsigned int         seed,
            unsigned int         filltype);

	/* 'Class' Stuff */
	void _MeshParticleLayout_Delete( void* meshParticleLayout );
	void _MeshParticleLayout_Print( void* meshParticleLayout, Stream* stream );
	
	#define MeshParticleLayout_Copy( self ) \
		(MeshParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define MeshParticleLayout_DeepCopy( self ) \
		(MeshParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _MeshParticleLayout_Copy( void* meshParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _MeshParticleLayout_DefaultNew( Name name ) ;
	void _MeshParticleLayout_AssignFromXML( void* meshParticleLayout, Stg_ComponentFactory* cf, void* data );
	void _MeshParticleLayout_Build( void* meshParticleLayout, void* data );
	void _MeshParticleLayout_Initialise( void* meshParticleLayout, void* data );
	void _MeshParticleLayout_Execute( void* meshParticleLayout, void* data );
	void _MeshParticleLayout_Destroy( void* meshParticleLayout, void* data );
	
	Particle_InCellIndex _MeshParticleLayout_InitialCount( void* meshParticleLayout, void* celllayout,  Cell_Index cell_I );
	
	void _MeshParticleLayout_InitialiseParticlesOfCell( void* meshParticleLayout, void* _swarm, Cell_Index cell_I );


	
#endif /* __StgFEM_Swarm_MeshParticleLayout_h__ */

