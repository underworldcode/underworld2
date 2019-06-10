/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_Energy_Energy_SLE_Solver_h__
#define __StgFEM_SLE_ProvidedSystems_Energy_Energy_SLE_Solver_h__

	/** Textual name of this class */
	extern const Type Energy_SLE_Solver_Type;
	
	/** Energy_SLE_Solver class contents */
	#define __Energy_SLE_Solver \
		/* General info */ \
		__SLE_Solver \
		\
		/* Virtual info */ \
		\
		/* Energy_SLE_Solver info */ \
		KSP	 ksp; \
		Vec  residual;

	/** Solves a basic SLE consisting of only one matrix, one force vector and one soln vector - see
	Energy_SLE_Solver.h */
	struct Energy_SLE_Solver { __Energy_SLE_Solver };	

	/** Constructor */
	void* Energy_SLE_Solver_DefaultNew( Name name );
	
	Energy_SLE_Solver* Energy_SLE_Solver_New( Name name ) ;

	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ENERGY_SLE_SOLVER_DEFARGS \
                SLE_SOLVER_DEFARGS

	#define ENERGY_SLE_SOLVER_PASSARGS \
                SLE_SOLVER_PASSARGS

	Energy_SLE_Solver* _Energy_SLE_Solver_New(  ENERGY_SLE_SOLVER_DEFARGS  );
	
	/** Member variable initialisation */
	void _Energy_SLE_Solver_Init( Energy_SLE_Solver* self );
	void Energy_SLE_Solver_InitAll( Energy_SLE_Solver* solver ) ;
	
	/** Class_Delete() implementation */
	void _Energy_SLE_Solver_Delete( void* sleSolver );
	
	/* --- Virtual Function Implementations --- */

	/** Stg_Class_Print() implementation */
	void _Energy_SLE_Solver_Print( void* sleSolver, Stream* stream );
	
	/* Copy */
	#define Energy_SLE_Solver_Copy( self ) \
		(Energy_SLE_Solver*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Energy_SLE_Solver_DeepCopy( self ) \
		(Energy_SLE_Solver*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Energy_SLE_Solver_Copy( void* standardSleSolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/* Stg_Component_Build() implementation */
	void _Energy_SLE_Solver_Build( void* sleSolver, void* standardSLE );
	
	void _Energy_SLE_Solver_Initialise( void* sleSolver, void* standardSLE );
	
	void _Energy_SLE_Solver_AssignFromXML( void* sleSolver, Stg_ComponentFactory* cf, void* data );

    void Energy_SLE_Solver_SetSolver( void* solver, void* heatSLE );

	void _Energy_SLE_Solver_Execute( void* sleSolver, void* data );
	
	void _Energy_SLE_Solver_Destroy( void* sleSolver, void* data );

	/* SLE_Solver_SolverSetup() implementation. */
	void _Energy_SLE_Solver_SolverSetup( void* sleSolver, void* standardSLE );

	/* SLE_Solver_Solve() implementation */
	void _Energy_SLE_Solver_Solve( void* sleSolver, void* standardSLE );

	/* Get residual implementation */
	//Vector* _Energy_SLE_GetResidual( void* sleSolver, Index fv_I );
	Vec _Energy_SLE_GetResidual( void* sleSolver, Index fv_I );

#endif

