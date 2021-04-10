/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Experimental_Rheology_ViscousPenaltyConstMatrixCartesian_h__
#define __Experimental_Rheology_ViscousPenaltyConstMatrixCartesian_h__

	/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
	extern const Type ViscousPenaltyConstMatrixCartesian_Type;

	/** ViscousPenaltyConstMatrixCartesian class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
	#define __ViscousPenaltyConstMatrixCartesian \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__ConstitutiveMatrix \
		\
		/* Virtual functions go here */ \
		\
		/* ViscousPenaltyConstMatrixCartesian info */ \
		double** Dtilda_B; \
		double*  Ni; \
		/* Parameter */ \
		double   incompressibility_Penalty; \
	    Bool     viscosityWeighting;

	struct ViscousPenaltyConstMatrixCartesian { __ViscousPenaltyConstMatrixCartesian };

	ViscousPenaltyConstMatrixCartesian* ViscousPenaltyConstMatrixCartesian_New( 
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

	#define VISCOUSPENALTYCONSTMATRIXCARTESIAN_DEFARGS \
                CONSTITUTIVEMATRIX_DEFARGS

	#define VISCOUSPENALTYCONSTMATRIXCARTESIAN_PASSARGS \
                CONSTITUTIVEMATRIX_PASSARGS

	ViscousPenaltyConstMatrixCartesian* _ViscousPenaltyConstMatrixCartesian_New(  VISCOUSPENALTYCONSTMATRIXCARTESIAN_DEFARGS  );
	
   void _ViscousPenaltyConstMatrixCartesian_Init( 
		ViscousPenaltyConstMatrixCartesian*         self,
		double                                      incompressibility_Penalty,
		Bool										viscosityWeighting );
	
	void  _ViscousPenaltyConstMatrixCartesian_Delete( void* constitutiveMatrix );
	void  _ViscousPenaltyConstMatrixCartesian_Print( void* constitutiveMatrix, Stream* stream );

	void* _ViscousPenaltyConstMatrixCartesian_DefaultNew( Name name ) ;
	void  _ViscousPenaltyConstMatrixCartesian_AssembleFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) ;
	void  _ViscousPenaltyConstMatrixCartesian_Build( void* constitutiveMatrix, void* data ) ;
	void  _ViscousPenaltyConstMatrixCartesian_Initialise( void* constitutiveMatrix, void* data ) ;
	void  _ViscousPenaltyConstMatrixCartesian_Execute( void* constitutiveMatrix, void* data ) ;
	void  _ViscousPenaltyConstMatrixCartesian_Destroy( void* constitutiveMatrix, void* data ) ;

	void  _ViscousPenaltyConstMatrixCartesian_AssembleElement( 
		void*                                                constitutiveMatrix,
		StiffnessMatrix*                                     stiffnessMatrix, 
		Element_LocalIndex                                   lElement_I, 
		SystemLinearEquations*                               sle,
		FiniteElementContext*                                context,
		double**                                             elStiffMat ) ;

	void _ViscousPenaltyConstMatrixCartesian2D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;
	void _ViscousPenaltyConstMatrixCartesian3D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;

	double _ViscousPenaltyConstMatrixCartesian2D_GetIsotropicViscosity( void* constitutiveMatrix ) ;
	double _ViscousPenaltyConstMatrixCartesian3D_GetIsotropicViscosity( void* constitutiveMatrix ) ;

	void _ViscousPenaltyConstMatrixCartesian2D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;
	void _ViscousPenaltyConstMatrixCartesian3D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;

	void _ViscousPenaltyConstMatrixCartesian2D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director );
	void _ViscousPenaltyConstMatrixCartesian3D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director );

	void _ViscousPenaltyConstMatrixCartesian2D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );
	void _ViscousPenaltyConstMatrixCartesian3D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );

	void _ViscousPenaltyConstMatrixCartesian2D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;
	void _ViscousPenaltyConstMatrixCartesian3D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;

	/* a function which defines the storage of each particle's constitutive information on the particle, 
	 * should be called before the "Build" phase */
//	void ViscousPenaltyConstMatrixCartesian_SetupParticleStorage( ViscousPenaltyConstMatrixCartesian* self );

#endif

