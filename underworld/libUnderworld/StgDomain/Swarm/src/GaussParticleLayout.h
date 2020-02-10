/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_GaussParticleLayout_h__
#define __StgDomain_Swarm_GaussParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type GaussParticleLayout_Type;
	
	/* GaussParticleLayout information */
	#define __GaussParticleLayout \
		__PerCellParticleLayout \
		\
		Dimension_Index      dim;                /**< Number of dimensions to use */ \
		Particle_InCellIndex particlesPerDim[3]; /**< Number of points per dimension */

	struct GaussParticleLayout { __GaussParticleLayout };
	
	/* Create a new GaussParticleLayout and initialise */
GaussParticleLayout* GaussParticleLayout_New( 
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

	#define GAUSSPARTICLELAYOUT_DEFARGS \
                PERCELLPARTICLELAYOUT_DEFARGS, \
                Dimension_Index                    dim, \
                Particle_InCellIndex*  particlesPerDim

	#define GAUSSPARTICLELAYOUT_PASSARGS \
                PERCELLPARTICLELAYOUT_PASSARGS, \
	        dim,             \
	        particlesPerDim

   GaussParticleLayout* _GaussParticleLayout_New(  GAUSSPARTICLELAYOUT_DEFARGS  );
		
	/* Initialise implementation */
	void _GaussParticleLayout_Init( void* gaussParticleLayout, Dimension_Index dim, Particle_InCellIndex* particlesPerDim );
	
	/* Stg_Class_Delete implementation */
	void _GaussParticleLayout_Delete( void* gaussParticleLayout );
	
	/* Print implementation */
	void _GaussParticleLayout_Print( void* gaussParticleLayout, Stream* stream );
	
	/* Copy */
	#define GaussParticleLayout_Copy( self ) \
		(GaussParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define GaussParticleLayout_DeepCopy( self ) \
		(GaussParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _GaussParticleLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _GaussParticleLayout_DefaultNew( Name name );
	void  _GaussParticleLayout_AssignFromXML( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data );
	void  _GaussParticleLayout_Build( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Initialise( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Execute( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Destroy( void* gaussParticleLayout, void* data );
	
	Particle_InCellIndex _GaussParticleLayout_InitialCount( void* gaussParticleLayout, void* celllayout, Cell_Index cell_I );
	void _GaussParticleLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* swarm, Cell_Index cell_I );

	/** Calculates the weight and abscissa for the given number of points. Upto 5 points can be used.
	 *     @param weight Array of weights, length equal to pointCount
	 *     @param abscissa Array of abscissa, length equal to pointCount
	 *     @param pointCount Number of points to create in a dimension */
	void GaussParticleLayout_GetAbscissaAndWeights1D( double* weight, double* abscissa, Index pointCount ) ;
	
#endif /* __StgDomain_Swarm_GaussParticleLayout_h__ */

