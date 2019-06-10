/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_GaussBorderParticleLayout_h__
#define __StgDomain_Swarm_GaussBorderParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type GaussBorderParticleLayout_Type;
	
	/* GaussBorderParticleLayout information */
	#define __GaussBorderParticleLayout \
                __GaussParticleLayout 	\
		Particle_InCellIndex*	particlesPerFace; // determined by particlesPerDim info, but calculated so many times it's worth making a member
	       
	struct GaussBorderParticleLayout { __GaussBorderParticleLayout };
	
	/* Create a new GaussBorderParticleLayout and initialise */
   GaussBorderParticleLayout* GaussBorderParticleLayout_New( 
      Name name,
      AbstractContext* context,
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      Dimension_Index dim, 
      Particle_InCellIndex* particlesPerDim );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define GAUSSBORDERPARTICLELAYOUT_DEFARGS \
                GAUSSPARTICLELAYOUT_DEFARGS

	#define GAUSSBORDERPARTICLELAYOUT_PASSARGS \
                GAUSSPARTICLELAYOUT_PASSARGS

   GaussBorderParticleLayout* _GaussBorderParticleLayout_New(  GAUSSBORDERPARTICLELAYOUT_DEFARGS  );

	/* Initialise implementation */
	void _GaussBorderParticleLayout_Init( void* gaussBorderParticleLayout );
	
	/* Stg_Class_Delete implementation */
	void _GaussBorderParticleLayout_Delete( void* gaussBorderParticleLayout );
	
	/* Print implementation */
	void _GaussBorderParticleLayout_Print( void* gaussBorderParticleLayout, Stream* stream );
	
	/* Copy */
	#define GaussBorderParticleLayout_Copy( self ) \
		(GaussBorderParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define GaussBorderParticleLayout_DeepCopy( self ) \
		(GaussBorderParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _GaussBorderParticleLayout_Copy( void* gaussBorderParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _GaussBorderParticleLayout_DefaultNew( Name name );
	void  _GaussBorderParticleLayout_AssignFromXML( void* gaussBorderParticleLayout, Stg_ComponentFactory* cf, void* data );
	void  _GaussBorderParticleLayout_Build( void* gaussBorderParticleLayout, void* data );
	void  _GaussBorderParticleLayout_Initialise( void* gaussBorderParticleLayout, void* data );
	void  _GaussBorderParticleLayout_Execute( void* gaussBorderParticleLayout, void* data );
	void  _GaussBorderParticleLayout_Destroy( void* gaussBorderParticleLayout, void* data );
	
	Particle_InCellIndex _GaussBorderParticleLayout_InitialCount( void* gaussBorderParticleLayout, void* celllayout, Cell_Index cell_I );
	void _GaussBorderParticleLayout_InitialiseParticlesOfCell( void* gaussBorderParticleLayout, void* swarm, Cell_Index cell_I );

        Dimension_Index GaussBorderParticleLayout_GetFaceAxis( void* gaussBorderParticleLayout, Index face_I, Dimension_Index axis);
        Index GaussBorderParticleLayout_ParticleInCellIndexToFaceIndex( void* gaussBorderParticleLayout, Particle_InCellIndex cParticle_I );

        void _GaussBorderParticleLayout_InitialiseParticlesPerFace( GaussBorderParticleLayout* self );
	
#endif /* __StgDomain_Swarm_GaussBorderParticleLayout_h__ */

