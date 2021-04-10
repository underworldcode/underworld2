/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Geothermal_Base_IrregularMeshGaussLayout_h__
#define __Geothermal_Base_IrregularMeshGaussLayout_h__
	

	/* Textual name of this class */
	extern const Type IrregularMeshGaussLayout_Type;
	
	/* IrregularMeshGaussLayout information */
	#define __IrregularMeshGaussLayout \
		__GaussParticleLayout \
		\
		double* abscissa; \
		FeMesh* feMesh; \

	struct IrregularMeshGaussLayout { __IrregularMeshGaussLayout };
	
	/* Create a new IrregularMeshGaussLayout and initialise */
	IrregularMeshGaussLayout* IrregularMeshGaussLayout_New( Name name, Dimension_Index dim, Particle_InCellIndex* particlesPerDim, FeMesh* feMesh ) ;
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define IRREGULARMESHGAUSSLAYOUT_DEFARGS \
                GAUSSPARTICLELAYOUT_DEFARGS

	#define IRREGULARMESHGAUSSLAYOUT_PASSARGS \
                GAUSSPARTICLELAYOUT_PASSARGS

	IrregularMeshGaussLayout* _IrregularMeshGaussLayout_New(  IRREGULARMESHGAUSSLAYOUT_DEFARGS  );
		
	/* Stg_Class_Delete implementation */
	void _IrregularMeshGaussLayout_Delete( void* gaussParticleLayout );
	
	/* Print implementation */
	void _IrregularMeshGaussLayout_Print( void* gaussParticleLayout, Stream* stream );
	
	/* Copy */
	#define IrregularMeshGaussLayout_Copy( self ) \
		(IrregularMeshGaussLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IrregularMeshGaussLayout_DeepCopy( self ) \
		(IrregularMeshGaussLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _IrregularMeshGaussLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _IrregularMeshGaussLayout_DefaultNew( Name name );
	void  _IrregularMeshGaussLayout_AssignFromXML( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data );
	void  _IrregularMeshGaussLayout_Build( void* gaussParticleLayout, void* data );
	void  _IrregularMeshGaussLayout_Initialise( void* gaussParticleLayout, void* data );
	void  _IrregularMeshGaussLayout_Execute( void* gaussParticleLayout, void* data );
	void  _IrregularMeshGaussLayout_Destroy( void* gaussParticleLayout, void* data );
	
	void _IrregularMeshGaussLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* swarm, Cell_Index cell_I );

#endif /* __Geothermal_Base_IrregularMeshGaussLayout_h__ */

