/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_PerCellParticleLayout_h__
#define __StgDomain_Swarm_PerCellParticleLayout_h__
	
	/** @see PerCellParticleLayout_InitialCount */
	typedef Particle_InCellIndex (PerCellParticleLayout_InitialCountFunction) ( 
			void* particleLayout, 
			void* cellLayout, 
			Cell_Index cell_I );

	/** @see PerCellParticleLayout_InitialiseParticlesOfCell */
	typedef void (PerCellParticleLayout_InitialiseParticlesOfCellFunction) ( 
			void* particleLayout, 
			void* swarm, 
			Cell_Index cell_I );
	
	/* Textual name of this class */
	extern const Type PerCellParticleLayout_Type;
	
	/* ParticleLayout information */
	#define __PerCellParticleLayout \
		/* General info */ \
		__ParticleLayout \
		\
		/* virtual information */ \
		PerCellParticleLayout_InitialCountFunction*			_initialCount; \
		PerCellParticleLayout_InitialiseParticlesOfCellFunction*	_initialiseParticlesOfCell;

	struct PerCellParticleLayout { __PerCellParticleLayout };
	
	/* No "_New" and "_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PERCELLPARTICLELAYOUT_DEFARGS \
                PARTICLELAYOUT_DEFARGS, \
                PerCellParticleLayout_InitialCountFunction*                            _initialCount, \
                PerCellParticleLayout_InitialiseParticlesOfCellFunction*  _initialiseParticlesOfCell

	#define PERCELLPARTICLELAYOUT_PASSARGS \
                PARTICLELAYOUT_PASSARGS, \
	        _initialCount,              \
	        _initialiseParticlesOfCell

   PerCellParticleLayout* _PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_DEFARGS  );
	
	/* Initialise implementation */
   void _PerCellParticleLayout_Init(
		void*                  particleLayout );
	
	/* Stg_Class_Delete a PerCellParticleLayout construct */
	void _PerCellParticleLayout_Delete( void* particleLayout );
	
	/* Print a PerCellParticleLayout construct */
	void _PerCellParticleLayout_Print( void* particleLayout, Stream* stream );
	
	/* Copy */
	#define PerCellParticleLayout_Copy( self ) \
		(PerCellParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PerCellParticleLayout_DeepCopy( self ) \
		(PerCellParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _PerCellParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
   void _PerCellParticleLayout_Destroy( void* particleLayout, void* data );
   void _PerCellParticleLayout_AssignFromXML( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	
	void _PerCellParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm );
	void _PerCellParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm );

	/** Initialise all particles in a given cell */
	void PerCellParticleLayout_InitialiseParticlesOfCell( void* particleLayout, void* swarm, Cell_Index cell_I );

	/** Return the initial number of particles in cell */
	Particle_InCellIndex PerCellParticleLayout_InitialCount( void* particleLayout, void* _swarm, Cell_Index cell_I );
	
#endif /* __StgDomain_Swarm_PerCellParticleLayout_h__ */

