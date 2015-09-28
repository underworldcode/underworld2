/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_StokesFlow_SLE_h__
#define __StgFEM_SLE_ProvidedSystems_StokesFlow_SLE_h__

	/** Textual name of this class */
	extern const Type Stokes_SLE_Type;

	/** Stokes_SLE class contents */
	#define __Stokes_SLE \
		/* General info */ \
		__SystemLinearEquations \
		\
		/* Virtual info */ \
		\
		/* Stokes_SLE info */ \
		\
		StiffnessMatrix*	kStiffMat;	/** Stress tensor matrix */ \
		StiffnessMatrix*	gStiffMat;	/** Gradient matrix */ \
		StiffnessMatrix*	dStiffMat;	/** Divergence matrix */ \
		StiffnessMatrix*	cStiffMat;	/** Compressibility matrix */\
		SolutionVector*	uSolnVec;	/** velocity vector */\
		Vec	null_vector;	/** null vector */\
		SolutionVector*	pSolnVec;	/** pressure vector */\
		ForceVector*		fForceVec;	/** forcing term vector */\
		ForceVector*		hForceVec;	/** continuity force vector */\
		/* the following are to help choose a "fudge" factor to remove null-space from Jacobian in rheology */\
		double fnorm; /* current residual of rhs of Jacobian system J*dx=-F */\
		double knorm; /* current norm of stiffness matrix from Jacobian */

	struct Stokes_SLE { __Stokes_SLE };	

	Stokes_SLE* Stokes_SLE_New( 		
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
		StiffnessMatrix*			kStiffMat,
		StiffnessMatrix*			gStiffMat,
		StiffnessMatrix*			dStiffMat,
		StiffnessMatrix*			cStiffMat,
		SolutionVector*			uSolnVec,
		SolutionVector*			pSolnVec,
		ForceVector*				fForceVec,
		ForceVector*				hForceVec );

	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STOKES_SLE_DEFARGS \
                SYSTEMLINEAREQUATIONS_DEFARGS

	#define STOKES_SLE_PASSARGS \
                SYSTEMLINEAREQUATIONS_PASSARGS

	Stokes_SLE* _Stokes_SLE_New(  STOKES_SLE_DEFARGS  );

	void _Stokes_SLE_Init( 		
		void*					sle, 
		StiffnessMatrix*	kStiffMat,
		StiffnessMatrix*	gStiffMat,
		StiffnessMatrix*	dStiffMat,
		StiffnessMatrix*	cStiffMat,
		SolutionVector*	uSolnVec,
		SolutionVector*	pSolnVec,
		ForceVector*		fForceVec,
		ForceVector*		hForceVec ) ;

	void _Stokes_SLE_Print( void* stokesSleSolver, Stream* stream );

	void* _Stokes_SLE_DefaultNew( Name name );

	void _Stokes_SLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data );
	
	void _Stokes_SLE_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms );
	
#endif

