/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_LumpedMassMatrixForceTerm_h__
#define __StgFEM_SLE_ProvidedSystems_LumpedMassMatrixForceTerm_h__

	/** Textual name of this class */
	extern const Type LumpedMassMatrixForceTerm_Type;

	/** LumpedMassMatrixForceTerm class contents */
	#define __LumpedMassMatrixForceTerm \
		/* General info */ \
		__ForceTerm \
		\
		/* Virtual info */ \
		\
		/* Member info */ \

	struct LumpedMassMatrixForceTerm { __LumpedMassMatrixForceTerm };

	LumpedMassMatrixForceTerm* LumpedMassMatrixForceTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		ForceVector*				forceVector,
		Swarm*						integrationSwarm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LUMPEDMASSMATRIXFORCETERM_DEFARGS \
                FORCETERM_DEFARGS

	#define LUMPEDMASSMATRIXFORCETERM_PASSARGS \
                FORCETERM_PASSARGS

	LumpedMassMatrixForceTerm* _LumpedMassMatrixForceTerm_New(  LUMPEDMASSMATRIXFORCETERM_DEFARGS  );

	void _LumpedMassMatrixForceTerm_Init( void* forceTerm );
	
	void _LumpedMassMatrixForceTerm_Delete( void* residual );

	void _LumpedMassMatrixForceTerm_Print( void* residual, Stream* stream );

	void* _LumpedMassMatrixForceTerm_DefaultNew( Name name );

	void _LumpedMassMatrixForceTerm_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data ) ;

	void _LumpedMassMatrixForceTerm_Build( void* residual, void* data ) ;

	void _LumpedMassMatrixForceTerm_Initialise( void* residual, void* data ) ;

	void _LumpedMassMatrixForceTerm_Execute( void* residual, void* data ) ;

	void _LumpedMassMatrixForceTerm_Destroy( void* residual, void* data ) ;

	void _LumpedMassMatrixForceTerm_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec ) ;

	void _LumpedMassMatrixForceTerm_AssembleElement_General( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVector ) ;

#endif

