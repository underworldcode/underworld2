/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_StokesFlow_Stokes_SLE_PenaltySolver_h__
#define __StgFEM_SLE_ProvidedSystems_StokesFlow_Stokes_SLE_PenaltySolver_h__

	/** Textual name of this class */
	extern const Type Stokes_SLE_PenaltySolver_Type;

	#define __Stokes_SLE_PenaltySolver \
		/* General info */ \
		__SLE_Solver \
		\
		/* Virtual info */ \
		\
		/* Stokes_SLE_PenaltySolver info */ \
		\
		/* Matrix solvers */

	/** Solves a Stokes SLE using the Penalty Method */
	struct Stokes_SLE_PenaltySolver { __Stokes_SLE_PenaltySolver };
	
	/* --- Constructors / Destructor --- */

	/** Constructor */
	void* Stokes_SLE_PenaltySolver_DefaultNew( Name name );
	
	Stokes_SLE_PenaltySolver* Stokes_SLE_PenaltySolver_New(
		Name                                        name,
		Bool                                        useStatSolve, 
		int                                         statReps );

	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STOKES_SLE_PENALTYSOLVER_DEFARGS \
                SLE_SOLVER_DEFARGS

	#define STOKES_SLE_PENALTYSOLVER_PASSARGS \
                SLE_SOLVER_PASSARGS

	Stokes_SLE_PenaltySolver* _Stokes_SLE_PenaltySolver_New(  STOKES_SLE_PENALTYSOLVER_DEFARGS  );

	/** Class member variable initialisation */
	void _Stokes_SLE_PenaltySolver_Init( void* solver ) ;
	void Stokes_SLE_PenaltySolver_InitAll( 
		void*                        solver,
		Bool                         useStatSolve,
		int                          statReps );

	/** Class_Delete() implementation */
	void _Stokes_SLE_PenaltySolver_Delete( void* solver );

	/* --- Virtual Function Implementations --- */
	void _Stokes_SLE_PenaltySolver_Print( void* solver, Stream* stream );
	
	/* Copy */
	#define Stokes_SLE_PenaltySolver_Copy( self ) \
		(SLE_Solver*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stokes_SLE_PenaltySolver_DeepCopy( self ) \
		(SLE_Solver*)Class_DeepCopy( self, NULL, True, NULL, NULL )
	
	void* _Stokes_SLE_PenaltySolver_Copy( void* stokesSlePenaltySolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/** Stg_Component_Build() implementations: allocates the 2 MatrixSolvers and additional Vectors */
	void _Stokes_SLE_PenaltySolver_Build( void* solver, void* stokesSLE );
	
	void _Stokes_SLE_PenaltySolver_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data );
	
	void _Stokes_SLE_PenaltySolver_Initialise( void* solver, void* stokesSLE ) ;
	
	void _Stokes_SLE_PenaltySolver_Execute( void* solver, void* data );
	
	void _Stokes_SLE_PenaltySolver_Destroy( void* solver, void* data );

	/** SolverSetup: sets up the 2 MatrixSolvers */
	void _Stokes_SLE_PenaltySolver_SolverSetup( void* stokesSle, void* stokesSLE );

	/**
	Solves
	Ku + Grad p = f
	Div u + C p = h
	by eliminating pressure via the penalty method.

	Hence we obtain the velocity by solving
	(K - Grad CInv Div )u = kHat u = f - Grad CInv h
	and recover pressure from
	p = CInv( h - Div u )
	*/
	void _Stokes_SLE_PenaltySolver_Solve( void* solver, void* stokesSLE );

	/* Get residual implementation */
	//Vector* _Stokes_SLE_PenaltySolver_GetResidual( void* solver, Index fv_I );
	Vec _Stokes_SLE_PenaltySolver_GetResidual( void* solver, Index fv_I );

        void Stokes_SLE_PenaltySolver_MakePenalty( Stokes_SLE_PenaltySolver* self, Stokes_SLE* sle, Vec* _penalty );

#endif	


