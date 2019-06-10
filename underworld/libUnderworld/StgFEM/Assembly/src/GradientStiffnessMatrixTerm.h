/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Assembly_GradientStiffnessMatrixTerm_h__
#define __StgFEM_Assembly_GradientStiffnessMatrixTerm_h__

	/** Textual name of this class */
	extern const Type GradientStiffnessMatrixTerm_Type;

	/** GradientStiffnessMatrixTerm class contents */
	#define __GradientStiffnessMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		/* Virtual info */ \
		/* GradientStiffnessMatrixTerm info */ \
		int		max_nElNodes_col;    \
		double	*Ni_col;

	struct GradientStiffnessMatrixTerm { __GradientStiffnessMatrixTerm };

	GradientStiffnessMatrixTerm* GradientStiffnessMatrixTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		StiffnessMatrix*			stiffnessMatrix,
		Swarm*						integrationSwarm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define GRADIENTSTIFFNESSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define GRADIENTSTIFFNESSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	GradientStiffnessMatrixTerm* _GradientStiffnessMatrixTerm_New(  GRADIENTSTIFFNESSMATRIXTERM_DEFARGS  );

	void _GradientStiffnessMatrixTerm_Init( void* matrixTerm );
	
	void _GradientStiffnessMatrixTerm_Delete( void* matrixTerm );

	void _GradientStiffnessMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _GradientStiffnessMatrixTerm_DefaultNew( Name name );

	void _GradientStiffnessMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _GradientStiffnessMatrixTerm_Build( void* matrixTerm, void* data );

	void _GradientStiffnessMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _GradientStiffnessMatrixTerm_Execute( void* matrixTerm, void* data );

	void _GradientStiffnessMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _GradientStiffnessMatrixTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#endif

