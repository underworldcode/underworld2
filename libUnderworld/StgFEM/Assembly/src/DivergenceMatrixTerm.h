/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Assembly_DivergenceMatrixTerm_h__
#define __StgFEM_Assembly_DivergenceMatrixTerm_h__

	/** Textual name of this class */
	extern const Type DivergenceMatrixTerm_Type;

	/** DivergenceMatrixTerm class contents */
	#define __DivergenceMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		/* Virtual info */ \
		/* DivergenceMatrixTerm info */ \
		int		max_nElNodes_row; \
		double	*Ni_row; 

	struct DivergenceMatrixTerm { __DivergenceMatrixTerm };

	DivergenceMatrixTerm* DivergenceMatrixTerm_New( 
	   Name							name,
	   FiniteElementContext*	context,
	   StiffnessMatrix*			stiffnessMatrix,
	   Swarm*						integrationSwarm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DIVERGENCEMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define DIVERGENCEMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	DivergenceMatrixTerm* _DivergenceMatrixTerm_New(  DIVERGENCEMATRIXTERM_DEFARGS  );

	void _DivergenceMatrixTerm_Init( void* matrixTerm );
	
	void _DivergenceMatrixTerm_Delete( void* matrixTerm );

	void _DivergenceMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _DivergenceMatrixTerm_DefaultNew( Name name );

	void _DivergenceMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _DivergenceMatrixTerm_Build( void* matrixTerm, void* data );

	void _DivergenceMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _DivergenceMatrixTerm_Execute( void* matrixTerm, void* data );

	void _DivergenceMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _DivergenceMatrixTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#endif

