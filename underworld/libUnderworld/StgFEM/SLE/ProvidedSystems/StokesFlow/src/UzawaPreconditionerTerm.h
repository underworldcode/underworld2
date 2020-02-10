/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_StokesFlow_UzawaPreconditionerTerm_h__
#define __StgFEM_SLE_ProvidedSystems_StokesFlow_UzawaPreconditionerTerm_h__

	/** Textual name of this class */
	extern const Type UzawaPreconditionerTerm_Type;

	/** UzawaPreconditionerTerm class contents */
	#define __UzawaPreconditionerTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		\
		/* Virtual info */ \
		\
		/* UzawaPreconditionerTerm info */ \

	struct UzawaPreconditionerTerm { __UzawaPreconditionerTerm };

	UzawaPreconditionerTerm* UzawaPreconditionerTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		StiffnessMatrix*			stiffnessMatrix,
		Swarm*						integrationSwarm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define UZAWAPRECONDITIONERTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define UZAWAPRECONDITIONERTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	UzawaPreconditionerTerm* _UzawaPreconditionerTerm_New(  UZAWAPRECONDITIONERTERM_DEFARGS  );

	void _UzawaPreconditionerTerm_Init( void* matrixTerm );
	
	void _UzawaPreconditionerTerm_Delete( void* matrixTerm );

	void _UzawaPreconditionerTerm_Print( void* matrixTerm, Stream* stream );

	void* _UzawaPreconditionerTerm_DefaultNew( Name name );

	void _UzawaPreconditionerTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _UzawaPreconditionerTerm_Build( void* matrixTerm, void* data );

	void _UzawaPreconditionerTerm_Initialise( void* matrixTerm, void* data );

	void _UzawaPreconditionerTerm_Execute( void* matrixTerm, void* data );

	void _UzawaPreconditionerTerm_Destroy( void* matrixTerm, void* data );

	void _UzawaPreconditionerTerm_AssembleElement( 
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#endif

