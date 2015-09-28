/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_Energy_Energy_SLE_h__
#define __StgFEM_SLE_ProvidedSystems_Energy_Energy_SLE_h__

	/** Textual name of this class */
	extern const Type Energy_SLE_Type;

	/** Energy_SLE class contents */
	#define __Energy_SLE \
		/* General info */ \
		__SystemLinearEquations \
		\
		/* Virtual info */ \
		\
		/* Energy_SLE info */ \
		\
		StiffnessMatrix*	stiffMat; \
		SolutionVector*	solutionVec; \
		ForceVector*		fVector; 

	struct Energy_SLE { __Energy_SLE };	

	Energy_SLE* Energy_SLE_New( 		
		Name							name,
		FiniteElementContext*	context,
		SLE_Solver*					solver,
		Bool							removeBCs,
		Bool							isNonLinear,
		double						nonLinearTolerance,
		Iteration_Index			nonLinearMaxIterations,
		Bool							killNonConvergent,
		EntryPoint_Register*		entryPoint_Register,
		MPI_Comm						comm,
		StiffnessMatrix*			stiffMat,
		SolutionVector*			solutionVec,
		ForceVector*				fVector );

	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ENERGY_SLE_DEFARGS \
                SYSTEMLINEAREQUATIONS_DEFARGS

	#define ENERGY_SLE_PASSARGS \
                SYSTEMLINEAREQUATIONS_PASSARGS

	Energy_SLE* _Energy_SLE_New(  ENERGY_SLE_DEFARGS  );

	void _Energy_SLE_Init( 
		void*					sle,
		StiffnessMatrix*	stiffMat,
		SolutionVector*	solutionVec,
		ForceVector*		fVector );

	/** 'Class' Virtual Functions */
	void _Energy_SLE_Print( void* stokesSleSolver, Stream* stream );

	/* 'Stg_Component' Virtual Functions */
	void* _Energy_SLE_DefaultNew( Name name );

	void _Energy_SLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data );

#endif

