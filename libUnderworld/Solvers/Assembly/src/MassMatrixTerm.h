/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Solvers_Assembly_MassMatrixTerm_h__
#define __Solvers_Assembly_MassMatrixTerm_h__

	/** Textual name of this class */
	extern const Type MassMatrixTerm_Type;

	/** MassMatrixTerm class contents */
	#define __MassMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		/* Virtual info */ \
		/* MassMatrixTerm info */ \
		FeMesh*             geometryMesh; \
		int		max_nElNodes_col; \
      int      max_nElNodes_row; \
		double	*Ni;  \
		double	*Mi;

	struct MassMatrixTerm { __MassMatrixTerm };

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MASSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define MASSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	MassMatrixTerm* _MassMatrixTerm_New(  MASSMATRIXTERM_DEFARGS  );
	
	void _MassMatrixTerm_Delete( void* matrixTerm );

	void _MassMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _MassMatrixTerm_DefaultNew( Name name );

	void _MassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _MassMatrixTerm_Build( void* matrixTerm, void* data );

	void _MassMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _MassMatrixTerm_Execute( void* matrixTerm, void* data );

	void _MassMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _MassMatrixTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#endif

