/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Assembly_LaplacianStiffnessMatrixTerm_h__
#define __StgFEM_Assembly_LaplacianStiffnessMatrixTerm_h__

	/** Textual name of this class */
	extern const Type LaplacianStiffnessMatrixTerm_Type;

	/** LaplacianStiffnessMatrixTerm class contents */
	#define __LaplacianStiffnessMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		\
		/* Virtual info */ \
		\
		/* LaplacianStiffnessMatrixTerm info */ \

	struct LaplacianStiffnessMatrixTerm { __LaplacianStiffnessMatrixTerm };

	LaplacianStiffnessMatrixTerm* LaplacianStiffnessMatrixTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		StiffnessMatrix*			stiffnessMatrix,
		Swarm*						integrationSwarm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LAPLACIANSTIFFNESSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define LAPLACIANSTIFFNESSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	LaplacianStiffnessMatrixTerm* _LaplacianStiffnessMatrixTerm_New(  LAPLACIANSTIFFNESSMATRIXTERM_DEFARGS  );

	void _LaplacianStiffnessMatrixTerm_Init( void* matrixTerm );
	
	void _LaplacianStiffnessMatrixTerm_Delete( void* matrixTerm );

	void _LaplacianStiffnessMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _LaplacianStiffnessMatrixTerm_DefaultNew( Name name );

	void _LaplacianStiffnessMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _LaplacianStiffnessMatrixTerm_Build( void* matrixTerm, void* data );

	void _LaplacianStiffnessMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _LaplacianStiffnessMatrixTerm_Execute( void* matrixTerm, void* data );

	void _LaplacianStiffnessMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _LaplacianStiffnessMatrixTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat );

#endif

