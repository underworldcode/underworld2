/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Solvers_Assembly_Matrix_NaiNbj_h__
#define __Solvers_Assembly_Matrix_NaiNbj_h__

	/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
	extern const Type Matrix_NaiNbj_Type;

	/** Matrix_NaiNbj class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
	#define __Matrix_NaiNbj \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__ConstitutiveMatrix \
		\
		/* Virtual functions go here */ \
		\
		/* Matrix_NaiNbj info */ \
		double** Dtilda_B; \
		double*  Ni; \
		/* Parameter */ \
		double   incompressibility_Penalty; \
	    Bool     viscosityWeighting;

	struct Matrix_NaiNbj { __Matrix_NaiNbj };

	Matrix_NaiNbj* Matrix_NaiNbj_New( 
		Name                                                name,
		StiffnessMatrix*                                    stiffnessMatrix,
		Swarm*                                              swarm,
		Dimension_Index                                     dim,
		FiniteElementContext*                               context,	
		double                                              incompressibility_Penalty,
		Bool                                                viscosityWeighting
		);

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MATRIX_NAINBJ_DEFARGS \
                CONSTITUTIVEMATRIX_DEFARGS

	#define MATRIX_NAINBJ_PASSARGS \
                CONSTITUTIVEMATRIX_PASSARGS

	Matrix_NaiNbj* _Matrix_NaiNbj_New(  MATRIX_NAINBJ_DEFARGS  );
	
   void _Matrix_NaiNbj_Init( 
		Matrix_NaiNbj*         self,
		double                                      incompressibility_Penalty,
		Bool										viscosityWeighting );
	
	void  _Matrix_NaiNbj_Delete( void* constitutiveMatrix );
	void  _Matrix_NaiNbj_Print( void* constitutiveMatrix, Stream* stream );

	void* _Matrix_NaiNbj_DefaultNew( Name name ) ;
	void  _Matrix_NaiNbj_AssembleFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) ;
	void  _Matrix_NaiNbj_Build( void* constitutiveMatrix, void* data ) ;
	void  _Matrix_NaiNbj_Initialise( void* constitutiveMatrix, void* data ) ;
	void  _Matrix_NaiNbj_Execute( void* constitutiveMatrix, void* data ) ;
	void  _Matrix_NaiNbj_Destroy( void* constitutiveMatrix, void* data ) ;

	void  _Matrix_NaiNbj_AssembleElement( 
		void*                                                constitutiveMatrix,
		StiffnessMatrix*                                     stiffnessMatrix, 
		Element_LocalIndex                                   lElement_I, 
		SystemLinearEquations*                               sle,
		FiniteElementContext*                                context,
		double**                                             elStiffMat ) ;

	void _Matrix_NaiNbj2D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;
	void _Matrix_NaiNbj3D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;

	double _Matrix_NaiNbj2D_GetIsotropicViscosity( void* constitutiveMatrix ) ;
	double _Matrix_NaiNbj3D_GetIsotropicViscosity( void* constitutiveMatrix ) ;

	void _Matrix_NaiNbj2D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;
	void _Matrix_NaiNbj3D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;

	void _Matrix_NaiNbj2D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director );
	void _Matrix_NaiNbj3D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director );

	void _Matrix_NaiNbj2D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );
	void _Matrix_NaiNbj3D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );

	void _Matrix_NaiNbj2D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;
	void _Matrix_NaiNbj3D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;

	/* a function which defines the storage of each particle's constitutive information on the particle, 
	 * should be called before the "Build" phase */
	//void Matrix_NaiNbj_SetupParticleStorage( Matrix_NaiNbj* self );

#endif

