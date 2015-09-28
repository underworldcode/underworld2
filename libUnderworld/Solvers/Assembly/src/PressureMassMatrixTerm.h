/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Solvers_Assembly_PressMassMatrixTerm_h__
#define __Solvers_Assembly_PressMassMatrixTerm_h__

	/** Textual name of this class */
	extern const Type PressMassMatrixTerm_Type;

	/** PressMassMatrixTerm class contents */
	#define __PressMassMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		/* Virtual info */ \
		/* PressMassMatrixTerm info */ \
		FeMesh*             geometryMesh; \
		int		max_nElNodes_col; \
		double	*Ni;

	struct PressMassMatrixTerm { __PressMassMatrixTerm };

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PRESSMASSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define PRESSMASSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	PressMassMatrixTerm* _PressMassMatrixTerm_New(  PRESSMASSMATRIXTERM_DEFARGS  );

	void _PressMassMatrixTerm_Init( void* matrixTerm, FeMesh* geometryMesh );
	
	void _PressMassMatrixTerm_Delete( void* matrixTerm );

	void _PressMassMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _PressMassMatrixTerm_DefaultNew( Name name );

	void _PressMassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _PressMassMatrixTerm_Build( void* matrixTerm, void* data );

	void _PressMassMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _PressMassMatrixTerm_Execute( void* matrixTerm, void* data );

	void _PressMassMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _PressMassMatrixTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#endif

