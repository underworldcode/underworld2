/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Assembly_IsoviscousStressTensorTerm_h__
#define __StgFEM_Assembly_IsoviscousStressTensorTerm_h__

	/** Textual name of this class */
	extern const Type IsoviscousStressTensorTerm_Type;

	/** IsoviscousStressTensorTerm class contents */
	#define __IsoviscousStressTensorTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		\
		/* Virtual info */ \
		\
		/* IsoviscousStressTensorTerm info */ \
		double	viscosity;


	struct IsoviscousStressTensorTerm { __IsoviscousStressTensorTerm };

	IsoviscousStressTensorTerm* IsoviscousStressTensorTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		StiffnessMatrix*			stiffnessMatrix,
		Swarm*						integrationSwarm,
		double						viscosity );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ISOVISCOUSSTRESSTENSORTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define ISOVISCOUSSTRESSTENSORTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	IsoviscousStressTensorTerm* _IsoviscousStressTensorTerm_New(  ISOVISCOUSSTRESSTENSORTERM_DEFARGS  );

	void _IsoviscousStressTensorTerm_Init( void* matrixTerm, double viscosity );
	
	void _IsoviscousStressTensorTerm_Delete( void* matrixTerm );

	void _IsoviscousStressTensorTerm_Print( void* matrixTerm, Stream* stream );

	void* _IsoviscousStressTensorTerm_DefaultNew( Name name );

	void _IsoviscousStressTensorTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _IsoviscousStressTensorTerm_Build( void* matrixTerm, void* data );

	void _IsoviscousStressTensorTerm_Initialise( void* matrixTerm, void* data );

	void _IsoviscousStressTensorTerm_Execute( void* matrixTerm, void* data );

	void _IsoviscousStressTensorTerm_Destroy( void* matrixTerm, void* data );

	void _IsoviscousStressTensorTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat );

#endif

