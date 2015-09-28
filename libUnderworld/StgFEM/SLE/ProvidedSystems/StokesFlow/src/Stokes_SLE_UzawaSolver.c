/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"
#include "types.h"
#include "Stokes_SLE_UzawaSolver.h"

#include <assert.h>
#include <string.h>

#include "Stokes_SLE.h"

/* Macro to checking number integrity - i.e. checks if number is infinite or "not a number" */
#define isGoodNumber( number ) \
	( (! isnan( number ) ) && ( ! isinf( number ) ) )

const Type Stokes_SLE_UzawaSolver_Type = "Stokes_SLE_UzawaSolver";

void* _Stokes_SLE_UzawaSolver_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Stokes_SLE_UzawaSolver);
	Type                                                      type = Stokes_SLE_UzawaSolver_Type;
	Stg_Class_DeleteFunction*                              _delete = _Stokes_SLE_UzawaSolver_Delete;
	Stg_Class_PrintFunction*                                _print = _Stokes_SLE_UzawaSolver_Print;
	Stg_Class_CopyFunction*                                  _copy = _Stokes_SLE_UzawaSolver_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _Stokes_SLE_UzawaSolver_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Stokes_SLE_UzawaSolver_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Stokes_SLE_UzawaSolver_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Stokes_SLE_UzawaSolver_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _SLE_Solver_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _Stokes_SLE_UzawaSolver_Destroy;
	SLE_Solver_SolverSetupFunction*                   _solverSetup = _Stokes_SLE_UzawaSolver_SolverSetup;
	SLE_Solver_SolveFunction*                               _solve = _Stokes_SLE_UzawaSolver_Solve;
	SLE_Solver_GetResidualFunc*                       _getResidual = _Stokes_SLE_UzawaSolver_GetResidual;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _Stokes_SLE_UzawaSolver_New(  STOKES_SLE_UZAWASOLVER_PASSARGS  );
}

Stokes_SLE_UzawaSolver* Stokes_SLE_UzawaSolver_New( 
		Name                                        name,
		Bool                                        useStatSolve, 
		int                                         statReps,
		StiffnessMatrix*                            preconditioner,
		Iteration_Index                             maxUzawaIterations,
		Iteration_Index                             minUzawaIterations,
		double                                      tolerance,
		Bool                                        useAbsoluteTolerance,
                Bool                                        monitor )
{		
	Stokes_SLE_UzawaSolver* self = _Stokes_SLE_UzawaSolver_DefaultNew( name );

	Stokes_SLE_UzawaSolver_InitAll( self, useStatSolve, statReps, preconditioner, maxUzawaIterations, minUzawaIterations, tolerance, useAbsoluteTolerance, monitor );

	return self;
}


/* Creation implementation / Virtual constructor */
Stokes_SLE_UzawaSolver* _Stokes_SLE_UzawaSolver_New(  STOKES_SLE_UZAWASOLVER_DEFARGS  )
{
	Stokes_SLE_UzawaSolver* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stokes_SLE_UzawaSolver) );
	self = (Stokes_SLE_UzawaSolver*) _SLE_Solver_New(  SLE_SOLVER_PASSARGS  );

	self->_formResidual  = _Stokes_SLE_UzawaSolver_FormResidual;
        self->_getRhs        = _Stokes_SLE_UzawaSolver_GetRhs;
	self->_getSolution   = _Stokes_SLE_UzawaSolver_GetSolution;
	
	/* Virtual info */
	return self;
}


void _Stokes_SLE_UzawaSolver_Init( 
		Stokes_SLE_UzawaSolver*      self,
		StiffnessMatrix*             preconditioner, 
		Iteration_Index              maxUzawaIterations,
		Iteration_Index              minUzawaIterations,
		double                       tolerance,
		Bool                         useAbsoluteTolerance,
                Bool                         monitor )
{
	self->isConstructed        = True;
	self->tolerance            = tolerance;
	self->maxUzawaIterations   = maxUzawaIterations;
	self->minUzawaIterations   = minUzawaIterations;
	self->preconditioner       = preconditioner;
	self->useAbsoluteTolerance = useAbsoluteTolerance;
        self->monitor              = monitor;
}

void Stokes_SLE_UzawaSolver_InitAll( 
		void*                        solver,
		Bool                         useStatSolve,
		int                          statReps, 
		StiffnessMatrix*             preconditioner, 
		Iteration_Index              maxUzawaIterations,
		Iteration_Index              minUzawaIterations,
		double                       tolerance,
		Bool                         useAbsoluteTolerance,
                Bool                         monitor )
{
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*)solver;

	SLE_Solver_InitAll( self, useStatSolve, statReps );
	_Stokes_SLE_UzawaSolver_Init( self, preconditioner, maxUzawaIterations, minUzawaIterations, tolerance, useAbsoluteTolerance, monitor );
}

void _Stokes_SLE_UzawaSolver_Delete( void* solver ) {
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*)solver;

   _SLE_Solver_Delete( self );
		
}       


void _Stokes_SLE_UzawaSolver_Print( void* solver, Stream* stream ) {
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*)solver;

	_SLE_Solver_Print( self, stream );

	Journal_PrintValue( stream, self->tolerance );
	Journal_PrintValue( stream, self->maxUzawaIterations );
	Journal_PrintValue( stream, self->minUzawaIterations );
}


void* _Stokes_SLE_UzawaSolver_Copy( void* stokesSleUzawaSolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*)stokesSleUzawaSolver;
	Stokes_SLE_UzawaSolver*	newStokesSleUzawaSolver;
	
	newStokesSleUzawaSolver = _SLE_Solver_Copy( self, dest, deep, nameExt, ptrMap );
	
	newStokesSleUzawaSolver->velSolver           = self->velSolver;
	newStokesSleUzawaSolver->pcSolver            = self->pcSolver;
	newStokesSleUzawaSolver->preconditioner      = self->preconditioner;
	newStokesSleUzawaSolver->pTempVec            = self->pTempVec;
	newStokesSleUzawaSolver->rVec                = self->rVec;
	newStokesSleUzawaSolver->sVec                = self->sVec;
	newStokesSleUzawaSolver->fTempVec            = self->fTempVec;
	newStokesSleUzawaSolver->vStarVec            = self->vStarVec;
	newStokesSleUzawaSolver->tolerance           = self->tolerance;
	newStokesSleUzawaSolver->maxUzawaIterations  = self->maxUzawaIterations;
	newStokesSleUzawaSolver->minUzawaIterations  = self->minUzawaIterations;
	newStokesSleUzawaSolver->useAbsoluteTolerance  = self->useAbsoluteTolerance;
	newStokesSleUzawaSolver->monitor               = self->monitor;	

	return (void*) newStokesSleUzawaSolver;
}


void _Stokes_SLE_UzawaSolver_Build( void* solver, void* stokesSLE ) {
	Stokes_SLE_UzawaSolver*	self  = (Stokes_SLE_UzawaSolver*)solver;
	Stokes_SLE*             sle   = (Stokes_SLE*)stokesSLE;

   /* ok, this is far from satisfactory, but let's just bail if we have not been called from within
      the SLE routine  JM20140618 */
   if( sle == NULL )
      return;
   
 	Journal_DPrintf( self->debug, "In %s\n", __func__ );
	Stream_IndentBranch( StgFEM_Debug );
	
 	Journal_DPrintfL( self->debug, 2, "building a standard solver for the velocity system.\n" );
	/* was also being built in _Stokes_SLE_UzawaSolver_AssignFromXML function ? */
	KSPCreate( sle->comm, &self->velSolver );
	KSPSetOptionsPrefix( self->velSolver, "Uzawa_velSolver_" );
	
	/* Build Preconditioner */
	if ( self->preconditioner ) {
		Stg_Component_Build( self->preconditioner, stokesSLE, False );
		SystemLinearEquations_AddStiffnessMatrix( sle, self->preconditioner );

		Journal_DPrintfL( self->debug, 2, "build a standard solver for the preconditioner system.\n" );
		KSPCreate( sle->comm, &self->pcSolver );
		KSPSetOptionsPrefix( self->pcSolver, "Uzawa_pcSolver_" );
	}
	else 
		self->pcSolver = PETSC_NULL;

	if( self->pTempVec != PETSC_NULL ) Stg_VecDestroy(&self->pTempVec );
	if( self->rVec != PETSC_NULL )     Stg_VecDestroy(&self->rVec );
	if( self->sVec != PETSC_NULL )     Stg_VecDestroy(&self->sVec );
	if( self->fTempVec != PETSC_NULL ) Stg_VecDestroy(&self->fTempVec );
	if( self->vStarVec != PETSC_NULL ) Stg_VecDestroy(&self->vStarVec );

 	Journal_DPrintfL( self->debug, 2, "Allocate the auxillary vectors pTemp, r, s, fTemp and vStar.\n" ); 
	VecDuplicate( sle->pSolnVec->vector, &self->pTempVec );
	VecDuplicate( sle->pSolnVec->vector, &self->rVec );
	VecDuplicate( sle->pSolnVec->vector, &self->sVec );

	VecDuplicate( sle->fForceVec->vector, &self->fTempVec );
	VecDuplicate( sle->fForceVec->vector, &self->vStarVec );

	/* Need by the Picard nonlinear solver */
//        Vector_Duplicate( sle->pTempVec->vector, (void**)&self->f_hat );
//        Vector_SetLocalSize( self->vf_hat, Vector_GetLocalSize( sle->pTempVec->vector ) );

	Stream_UnIndentBranch( StgFEM_Debug );
}

void _Stokes_SLE_UzawaSolver_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data ) {
	Stokes_SLE_UzawaSolver* self         = (Stokes_SLE_UzawaSolver*) solver;
	double                  tolerance;
	Iteration_Index         maxUzawaIterations, minUzawaIterations;
	StiffnessMatrix*        preconditioner;
	Bool                    useAbsoluteTolerance;
	Bool                    monitor;

	_SLE_Solver_AssignFromXML( self, cf, data );

	tolerance            = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"tolerance", 1.0e-5  );
	maxUzawaIterations   = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"maxIterations", 1000  );
	minUzawaIterations   = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"minIterations", 1  );
	useAbsoluteTolerance = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useAbsoluteTolerance", False  );
	monitor              = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"monitor", False  );

	preconditioner = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Preconditioner", StiffnessMatrix, False, data  );

	_Stokes_SLE_UzawaSolver_Init( self, preconditioner, maxUzawaIterations, minUzawaIterations, tolerance, useAbsoluteTolerance, monitor );

	if( self->velSolver == PETSC_NULL ) {
	    //KSPCreate( MPI_COMM_WORLD, &self->velSolver );
	    //KSPSetOptionsPrefix( self->pcSolver, "Uzawa_velSolver_" );
	}
}

void _Stokes_SLE_UzawaSolver_Execute( void* solver, void* data ) {
}

void _Stokes_SLE_UzawaSolver_Destroy( void* solver, void* data ) {
   Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*) solver;
	Journal_DPrintf( self->debug, "In: %s \n", __func__);

	Stream_IndentBranch( StgFEM_Debug );
	Journal_DPrintfL( self->debug, 2, "Destroying Solver contexts.\n" );
	Stg_KSPDestroy(&self->velSolver );
        if ( self->preconditioner ) { Stg_KSPDestroy(&self->pcSolver ); }

	Journal_DPrintfL( self->debug, 2, "Destroying temporary solver vectors.\n" );
	if( self->pTempVec != PETSC_NULL ) Stg_VecDestroy(&self->pTempVec );
	if( self->rVec != PETSC_NULL )     Stg_VecDestroy(&self->rVec );
	if( self->sVec != PETSC_NULL )     Stg_VecDestroy(&self->sVec );
	if( self->fTempVec != PETSC_NULL ) Stg_VecDestroy(&self->fTempVec );
	if( self->vStarVec != PETSC_NULL ) Stg_VecDestroy(&self->vStarVec );
	Stream_UnIndentBranch( StgFEM_Debug );
   _SLE_Solver_Destroy( self, data );

}

void _Stokes_SLE_UzawaSolver_Initialise( void* solver, void* stokesSLE ) {
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*) solver;
	Stokes_SLE*             sle  = (Stokes_SLE*)             stokesSLE;

   /* ok, this is far from satisfactory, but let's just bail if we have not been called from within
    the SLE routine  JM20140618 */
   if( sle == NULL )
      return;

	/* Initialise Parent */
	_SLE_Solver_Initialise( self, sle );
	
	if ( sle->context && (True == sle->context->loadFieldsFromCheckpoint) ) {
		/* The previous timestep's velocity solution will be helpful in iterating to a better
		solution faster - and thus make restarting from checkpoint more repeatable compared
		to original non-restart solution */
		SolutionVector_LoadCurrentFeVariableValuesOntoVector( sle->uSolnVec );
		SolutionVector_LoadCurrentFeVariableValuesOntoVector( sle->pSolnVec );
	}

}

/* SolverSetup */

void _Stokes_SLE_UzawaSolver_SolverSetup( void* solver, void* stokesSLE ) {
	Stokes_SLE_UzawaSolver* self = (Stokes_SLE_UzawaSolver*) solver;
	Stokes_SLE*             sle  = (Stokes_SLE*)             stokesSLE;
	
 	Journal_DPrintf( self->debug, "In %s:\n", __func__ );
	Stream_IndentBranch( StgFEM_Debug );

	Journal_DPrintfL( self->debug, 1, "Setting up MatrixSolver for the velocity eqn.\n" );
	Stg_KSPSetOperators( self->velSolver, sle->kStiffMat->matrix, sle->kStiffMat->matrix, DIFFERENT_NONZERO_PATTERN );
  	KSPSetFromOptions( self->velSolver );

	if( self->pcSolver ) {
		Journal_DPrintfL( self->debug, 1, "Setting up MatrixSolver for the Preconditioner.\n" );
		Stg_KSPSetOperators( self->pcSolver, self->preconditioner->matrix, self->preconditioner->matrix, DIFFERENT_NONZERO_PATTERN );
    		KSPSetFromOptions( self->pcSolver );
	}

	Stream_UnIndentBranch( StgFEM_Debug );
}

Bool _check_if_constant_nullsp_present( Stokes_SLE_UzawaSolver* self, Mat K, Mat G, Mat M, Vec t1, Vec ustar,  Vec r, Vec l, KSP ksp )
{
    PetscInt N;
    PetscScalar sum;
    PetscReal nrm;
    Bool nullsp_present;

    VecGetSize(l,&N);
    sum  = 1.0/N;
    VecSet(l,sum);

    /* [S] {l} = {r} */
    MatMult( G,l, t1 );
    KSPSolve( ksp, t1, ustar );
    MatMultTranspose( G, ustar, r );     
    if ( M ) {
	VecScale( r, -1.0 );
	MatMultAdd( M,l, r, r );
	VecScale( r, -1.0 );
    }

    VecNorm(r,NORM_2,&nrm);
    if (nrm < 1.e-7) {
	Journal_PrintfL( self->info, 1, "Constant null space detected, " ); 
	nullsp_present = True;
    }
    else {
	Journal_PrintfL( self->info, 1, "Constant null space not present, " );
	nullsp_present = False;
    }
    Journal_PrintfL( self->info, 1, "|| [S]{1} || = %G\n", nrm );


    return nullsp_present;
}

void _remove_constant_nullsp( Vec v )
{
        PetscInt N;
	PetscScalar sum;
                
	VecGetSize( v, &N );
	if( N > 0 ) {
		VecSum( v, &sum );
		sum  = sum/( -1.0*N );
		VecShift( v, sum );
	}
}

/* from the depreciated Vector class */
void _SLE_VectorView( Vec v, Stream* stream ) {
	unsigned	entry_i;
	PetscInt	size;
	PetscScalar*	array;

	VecGetSize( v, &size );
	VecGetArray( v, &array );

	Journal_Printf( stream, "%p = [", v );
	for( entry_i = 0; entry_i < size; entry_i++ ) 
		Journal_Printf( stream, "\t%u: \t %.12g\n", entry_i, array[entry_i] );
	Journal_Printf( stream, "];\n" );

	VecRestoreArray( v, &array );
}

void _Stokes_SLE_UzawaSolver_Solve( void* solver, void* stokesSLE ) {
	Stokes_SLE_UzawaSolver* self            = (Stokes_SLE_UzawaSolver*)solver;	
	Stokes_SLE*             sle             = (Stokes_SLE*)stokesSLE;
	
	/* Create shortcuts to stuff needed on sle */
	Mat                     K_Mat           = sle->kStiffMat->matrix;
	Mat                     G_Mat           = sle->gStiffMat->matrix;
	Mat                     D_Mat           = NULL;
	Mat                     M_Mat           = NULL;
	Vec                     uVec            = sle->uSolnVec->vector;
	Vec                     qVec            = sle->pSolnVec->vector;
	Vec                     fVec            = sle->fForceVec->vector;
	Vec                     hVec            = sle->hForceVec->vector;
	
	/* Create shortcuts to solver related stuff */
	Vec                     qTempVec        = self->pTempVec;  
	Vec                     rVec            = self->rVec;
	Vec                     sVec            = self->sVec;
	Vec                     fTempVec        = self->fTempVec;
	Vec                     vStarVec        = self->vStarVec; 
	KSP                     velSolver       = self->velSolver;	/*  Inner velocity solver */
	KSP                     pcSolver        = self->pcSolver;   /*  Preconditioner  */

	Iteration_Index         maxIterations   = self->maxUzawaIterations;
	Iteration_Index         minIterations   = self->minUzawaIterations;
	Iteration_Index         iteration_I     = 0;
	Iteration_Index         outputInterval  = 1;
	
	double                  zdotr_current	= 0.0;
	double                  zdotr_previous 	= 1.0;
	double                  sdotGTrans_v;
	double                  alpha, beta;
	double                  absResidual;  
	double                  relResidual;
	double*                 chosenResidual;	  /* We can opt to use either the absolute or relative residual in termination condition */
    	double                  uzawaRhsScale;      
	double                  divU;
	double                  weightedResidual;
	double                  weightedVelocityScale;
	double                  momentumEquationResidual;
	
	Iteration_Index         innerLoopIterations;
	Stream*                 errorStream     = Journal_Register( Error_Type, (Name)Stokes_SLE_UzawaSolver_Type  );
	
	PetscInt		fVecSize, qTempVecSize, uVecSize, qVecSize;
	PetscScalar		fVecNorm, qTempVecNorm, uVecNorm, rVecNorm, fTempVecNorm, uVecNormInf, qVecNorm, qVecNormInf;

	double                  qGlobalProblemScale;
	double                  qReciprocalGlobalProblemScale;
	int			init_info_stream_rank;	
	PetscScalar p_sum;
	/* Bool nullsp_present; */
	Bool uzawa_summary;
	double time,t0,rnorm0;

	PetscTruth     flg;
	double        ksptime;

	VecGetSize( qTempVec, &qTempVecSize );
	qGlobalProblemScale = sqrt( (double) qTempVecSize );
	qReciprocalGlobalProblemScale = 1.0 / qGlobalProblemScale;
	init_info_stream_rank = Stream_GetPrintingRank( self->info );
	Stream_SetPrintingRank( self->info, 0 ); 

	/*	DEFINITIONS:
					See accompanying documentation
					u - the displacement / velocity solution (to which constraints are applied)
					q - the pressure-like variable which constrains the divergence displacement / velocity	(= pressure for incompressible)	
					F - standard FE force vector
					Fhat - Uzawa RHS = K^{-1} G F  - h 
					K - standard FE stiffness matrix
					Khat - Uzawa transformed stiffness matrix = G^T K^{-1} G
					G matrix - discrete gradient operator
					D matrix - discrete divergence operator = G^T for this particular algorithm
					C matrix - Mass matrix (M) for compressibility 
					
		LM & DAM			
	*/

	/* CHOICE OF RESIDUAL: 
					we may opt to converge on the absolute value (self->useAbsoluteTolerance == True ... default)
					or the relative value of the residual (self->useAbsoluteTolerance == False) 
			 		(another possibility would be always to improve the residual by a given tolerance)
					The Moresi & Solomatov (Phys Fluids, 1995) approach is to use the relative tolerance	
	*/ 

	VecNorm( fVec, NORM_2, &fVecNorm );
	VecGetSize( fVec, &fVecSize );
	if ( fVecNorm / sqrt( (double)fVecSize ) <= 1e-99 ) {
		Journal_Printf( errorStream,
			"Error in func %s: The momentum force vector \"%s\" is zero. "
			"The force vector should be non-zero either because of your chosen boundary "
			"conditions, or because of the element force vector assembly. You have %d "
			"element force vectors attached.\n",
			__func__, sle->fForceVec->name, sle->fForceVec->assembleForceVector->hooks->count );
		if ( sle->fForceVec->assembleForceVector->hooks->count > 0 ) {
			Journal_Printf( errorStream, "You used the following force vector assembly terms:\n" );
			EntryPoint_PrintConcise( sle->fForceVec->assembleForceVector, errorStream );
/* 			 TODO : need to print the elementForceVector assembly, not the global guy!! */
		}	
		Journal_Printf( errorStream,
			"Please check values for building the force vector.\n" );
		Journal_Firewall( 0, errorStream, "Exiting.\n" ); 	
	}
	
					
 	Journal_DPrintf( self->debug, "In %s:\n", __func__ );
	Journal_RPrintfL( self->debug, 2, "Conjugate Gradient Uzawa solver with:\n");
	
	Stream_IndentBranch( StgFEM_Debug );
	
	Journal_RPrintfL( self->debug, 2, "Compressibility %s\n", (sle->cStiffMat)? "on" : "off");
	Journal_RPrintfL( self->debug, 2, "Preconditioning %s\n", (pcSolver)? "on" : "off" );   
	
	
	
	if ( sle->cStiffMat ) {
		Journal_DPrintfL( self->debug, 2, "(compressibility active)\n" );
		M_Mat = sle->cStiffMat->matrix;   
	}
	else {
		Journal_DPrintfL( self->debug, 2, "(compressibility inactive)\n" );
	}
	if ( sle->dStiffMat ) {
		Journal_DPrintfL( self->debug, 2, "(asymmetric geometry: handling D Matrix [incorrectly - will be ignored])\n" );
		D_Mat = sle->dStiffMat->matrix;
	}
	else {
		Journal_DPrintfL( self->debug, 2, "(No D -> symmetric geometry: D = Gt)\n" );
	}
	
	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 3 ) ) {
		Journal_DPrintf( self->debug, "Matrices and Vectors to solve are:\n" );
		Journal_DPrintf( self->debug, "K Matrix:\n" );
		/* No nice way of viewing Matrices, so commented out as incompatible with
		 * new 3D decomp at present --Kathleen Humble 30-04-07 
		 * Matrix_View( sle->kStiffMat->matrix, self->debug ); */
		Journal_DPrintf( self->debug, "G Matrix:\n" );
		if ( D_Mat ) {
			Journal_DPrintf( self->debug, "D Matrix:\n" );
		}	
		if ( M_Mat ) {
			Journal_DPrintf( self->debug, "M Matrix:\n" );
		}	
		Journal_DPrintf( self->debug, "Z (preconditioner) Matrix:\n" );
		Journal_DPrintf( self->debug, "f Vector:\n" );
		_SLE_VectorView( fVec, self->debug );
		Journal_DPrintf( self->debug, "h Vector:\n" );
		_SLE_VectorView( hVec, self->debug );
	}
	#endif
	
	/* STEP 1: Estimate the magnitude of the RHS for the transformed problem
			   we compute (usually to lower accuracy than elsewhere) the RHS (Fhat - h) 
	         and store the result in qTempVec.
		LM & DAM			
	*/
	
	Journal_DPrintfL( self->debug, 2, "Building Fhat - h.\n" );
	PetscOptionsHasName(PETSC_NULL,"-uzawa_printksptimes",&flg);
	KSPSetTolerances( velSolver, self->tolerance, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT );
	if (flg) {
	    ksptime = MPI_Wtime();
	}
	KSPSolve( velSolver, fVec, vStarVec );
	if (flg) {
	    ksptime = MPI_Wtime() - ksptime;
	    PetscPrintf( PETSC_COMM_WORLD,  "KSP on velSolver took %lf seconds in Building Fhat step\n", ksptime);
	}
	KSPGetIterationNumber( velSolver, &innerLoopIterations );
	
	Journal_DPrintfL( self->debug, 2, "Fhat inner solution: Number of iterations: %d\n", innerLoopIterations );
	
        if ( D_Mat ) {
          MatMult( D_Mat, vStarVec, qTempVec );
        }
        else {
          MatMultTranspose( G_Mat, vStarVec, qTempVec );
        }
	VecAXPY( qTempVec, -1.0, hVec );
	
	/*  WARNING:
			If D != G^T then the resulting \hat{K} is not likely to be symmetric, positive definite as
			required by this implementation of the Uzawa iteration.  This next piece of code
			is VERY unlikely to work properly so it's in the sin bin for the time being - LM.
			
			if ( D_Mat ) {
				MatrixMultiply( D_Mat, vStarVec, qTempVec );
			}
			else {
				MatrixTransposeMultiply( G_Mat, vStarVec, qTempVec );
			}
		LM & DAM			
	*/	

	
	/* STEP 2: The problem scaling - optionally normalize the uzawa residual by the magnitude of the RHS (use a relative tolerance)
			For the inner velocity solver,  Citcom uses a relative tolerance equal to that used for the Uzawa iteration as a whole
		LM & DAM			
	*/
	
	if (self->useAbsoluteTolerance) {
		chosenResidual = &absResidual;
		Journal_PrintfL( self->info, 2, "Absolute residual < %g for Uzawa stopping condition\n", self->tolerance);
		/* We should calculate the effective relative tolerance and insert that here !! */
		KSPSetTolerances( velSolver, 0.1 * self->tolerance, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT );
	}
	else {  /* The CITCOM compatible choice */
		chosenResidual = &relResidual;
		Journal_PrintfL( self->info, 2, "Relative residual < %g for Uzawa stopping condition\n", self->tolerance);	
		KSPSetTolerances( velSolver, 0.1 * self->tolerance, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT );
	}
	
	Journal_DPrintfL( self->debug, 2, "Determining scaling factor for residual:\n" );
	VecNorm( qTempVec, NORM_2, &qTempVecNorm );
	uzawaRhsScale = ((double)qTempVecNorm) * qReciprocalGlobalProblemScale;
	
	Journal_DPrintfL( self->debug, 2, "uzawaRhsScale = %f\n", uzawaRhsScale );	
	Journal_Firewall( isGoodNumber( uzawaRhsScale ), errorStream, 
			"Error in func '%s' for %s '%s' - uzawaRhsScale has illegal value '%g'.\n", __func__, self->type, self->name, uzawaRhsScale );
	
	/* STEP 3: Calculate initial residual for transformed equation  (\hat{F} - h - \hat{K} q_0)
	    Compute the solution to K u_0 = F - G q_0  (u_0 unknown)
		  Then G^T u* = \hat{F} - \hat{K} q_0 
	    u_0 is also the initial velocity solution to which the constraint is applied by the subsequent iteration
		LM & DAM			
	*/
	
	Journal_DPrintfL( self->debug, 2, "Solving for transformed Uzawa RHS.\n" );
	
	VecCopy( fVec, fTempVec );
	VecScale( fTempVec, -1.0 );
	MatMultAdd( G_Mat, qVec, fTempVec, fTempVec );
	VecScale( fTempVec, -1.0 );
	KSPSolve( velSolver, fTempVec, uVec );

	/* Handling for NON-SYMMETRIC: relegated to sin bin (see comment above) LM & DAM */
        if ( D_Mat ) {
           MatMult( D_Mat, uVec, rVec );
        }
        else {
           MatMultTranspose( G_Mat, uVec, rVec );
        }
	VecNorm( rVec, NORM_2, &rnorm0 );
	VecNorm( uVec, NORM_2, &uVecNorm );
	divU = rnorm0 / uVecNorm;
	
	Journal_PrintfL( self->info, 2, "Initial l2Norm( Div u ) / l2Norm( u ) = %f \n", divU);
	
	Journal_Firewall( isGoodNumber( divU ), errorStream, 
			"Error in func '%s' for %s '%s' - l2Norm( Div u ) has illegal value '%g'.\n",
			__func__, self->type, self->name, divU );
	
	
	Journal_DPrintfL( self->debug, 2, "Adding compressibility and prescribed divergence terms.\n" );
	
	if ( M_Mat ) {
		MatMultAdd( M_Mat, qVec, rVec, rVec );
	}	
	VecAXPY( rVec, -1.0, hVec );
			
	/* Check for existence of constant null space */
#if 0
	nullsp_present = _check_if_constant_nullsp_present( self, K_Mat,G_Mat,M_Mat, fTempVec,vStarVec,qTempVec,sVec, velSolver );
#endif
			
	/* STEP 4: Preconditioned conjugate gradient iteration loop */	
		
	Journal_DPrintfL( self->debug, 1, "Beginning main Uzawa conjugate gradient loop:\n" );	

	iteration_I = 0;

        /* outer_it, residual, time */
        uzawa_summary = self->monitor;
	time = 0.0;
	t0 = MPI_Wtime();
//	Journal_PrintfL( self->info, 1, "  |r0| = %.8e \n", rnorm0 );

	do{	
		/* reset initial time and end time for inner its back to 0 - probs don't need to do this but just in case */
		self->outeritsinitialtime = 0;
		self->outeritsendtime = 0;
		//BEGINNING OF OUTER ITERATIONS!!!!!
		/*get wall time for start of outer loop*/
		self->outeritsinitialtime = MPI_Wtime();
		
		Journal_DPrintfL( self->debug, 2, "Beginning solve '%u'.\n", iteration_I );
		Stream_IndentBranch( StgFEM_Debug );
		
		/* STEP 4.1: Preconditioner
			Solve:
				Q_\hat{K} z_1 =  r_1
				Q_\hat{K} is an approximation to \hat{K} which is simple / trivial / quick to invert
			LM & DAM			
		*/
		
		if ( pcSolver ) {
		    PetscOptionsHasName(PETSC_NULL,"-uzawa_printksptimes",&flg);
		    if (flg) {
			ksptime = MPI_Wtime();
		    }
		    KSPSolve( pcSolver, rVec, qTempVec );
		    if (flg) {
			ksptime = MPI_Wtime() - ksptime;
			PetscPrintf( PETSC_COMM_WORLD,  "KSP on pcSolver took %lf seconds\n", ksptime);
		    }
		}
		else {
			VecCopy( rVec, qTempVec );
		}

		/* Remove the constant null space, but only if NOT compressible */
#if 0
		if( nullsp_present == True ) {
			_remove_constant_nullsp( qTempVec );
		}
#endif
				
		/* STEP 4.2: Calculate s_I, the pressure search direction
				z_{I-1} . r_{I-1}  
				\beta = (z_{I-1} . r_{I-1}) / (z_{I-2} . r_{I-2})  
					\beta = 0 for the first iteration
		      s_I = z_(I-1) + \beta * s_(I-1) 
			LM & DAM			
		*/ 
		
		VecDot( qTempVec, rVec, &zdotr_current );
	
		VecNorm( qTempVec, NORM_2, &qTempVecNorm );
		VecNorm( rVec, NORM_2, &rVecNorm );	
		Journal_DPrintfL( self->debug, 2, "l2Norm (qTempVec) %g; (rVec) %g \n", 
			qTempVecNorm * qReciprocalGlobalProblemScale,
			rVecNorm * qReciprocalGlobalProblemScale );
		
		if ( iteration_I == 0 ) {
			VecCopy( qTempVec, sVec );
		}
		else {
			beta = zdotr_current/zdotr_previous;
			VecAYPX( sVec, beta, qTempVec );
		}
		
		/* STEP 4.3: Velocity search direction corresponding to s_I is found by solving
				K u* = G s_I
			LM & DAM			
		*/
			
		MatMult( G_Mat, sVec, fTempVec );
		
		Journal_DPrintfL( self->debug, 2, "Uzawa inner iteration step\n");
		
		//START OF INNER ITERATIONS!!!!
		PetscOptionsHasName(PETSC_NULL,"-uzawa_printksptimes",&flg);
		/*get initial wall time for inner loop*/
		self->inneritsinitialtime = MPI_Wtime();
		if (flg) {
		    ksptime = MPI_Wtime();
		}
		KSPSolve( velSolver, fTempVec, vStarVec );
		if (flg) {
		    ksptime = MPI_Wtime() - ksptime;
		    PetscPrintf( PETSC_COMM_WORLD,  "KSP on velSolver took %lf seconds in Uzawa inner iteration step\n", ksptime);
		}
		/*get end wall time for inner loop*/
		self->inneritsendtime = MPI_Wtime();
		
		/* add time to total time inner its: */
		self->totalinneritstime = self->totalinneritstime + (-self->inneritsinitialtime + self->inneritsendtime);
		/* reset initial time and end time for inner its back to 0 - probs don't need to do this but just in case */
		self->inneritsinitialtime = 0;
		self->inneritsendtime = 0;
		
		KSPGetIterationNumber( velSolver, &innerLoopIterations );
		/* add the inner loop iterations to the total inner iterations */
		self->totalnuminnerits = self->totalnuminnerits + innerLoopIterations;
		
		Journal_DPrintfL( self->debug, 2, "Completed Uzawa inner iteration in '%u' iterations \n", innerLoopIterations );
				
		/* STEP 4.4: Calculate the step size ( \alpha = z_{I-1} . r_{I-1} / (s_I . \hat{K} s_I) )
				 \hat{K} s_I = G^T u* - M s_I (u* from step 4.3) 	
			LM & DAM			
		*/ 
		
                if ( D_Mat ) {
                   MatMult( D_Mat, vStarVec, qTempVec );
                }
                else {
                   MatMultTranspose( G_Mat, vStarVec, qTempVec );
                }
		
		/* Handling for NON-SYMMETRIC: relegated to sin bin (see comment above) 
		
			if ( D_Mat ) {
				MatrixMultiply( D_Mat, vStarVec, qTempVec );
			}
			else {
				MatrixTransposeMultiply( G_Mat, vStarVec, qTempVec );
			}
			LM & DAM			
		*/

		if ( M_Mat ) {
			Journal_DPrintfL( self->debug, 2, "Correcting for Compressibility\n" );
			VecScale( qTempVec, -1.0 );
			MatMultAdd( M_Mat, sVec, qTempVec, qTempVec );
			VecScale( qTempVec, -1.0 );
		}

		VecDot( sVec, qTempVec, &sdotGTrans_v );
		
		alpha = zdotr_current/sdotGTrans_v;
		
		/* STEP 4.5: Update pressure, velocity and value of residual
				 by \alpha times corresponding search direction 
			LM & DAM			
		*/
		
		Journal_DPrintfL( self->debug, 2, "zdotr_current = %g \n", zdotr_current);
		Journal_DPrintfL( self->debug, 2, "sdotGTrans_v = %g \n", sdotGTrans_v);
		Journal_DPrintfL( self->debug, 2, "alpha = %g \n", alpha);
	
		Journal_Firewall( 
				isGoodNumber( zdotr_current ) && isGoodNumber( sdotGTrans_v ) && isGoodNumber( alpha ), 
				errorStream, 
				"Error in func '%s' for %s '%s' - zdotr_current, sdotGTrans_v or alpha has an illegal value: '%g','%g' or '%g'\n",
				__func__, self->type, self->name, zdotr_current, sdotGTrans_v, alpha );
		
		VecAXPY( qVec, alpha, sVec );
		VecAXPY( uVec, -alpha, vStarVec );
		VecAXPY( rVec, -alpha, qTempVec );
		
		/* STEP 4.6: store the value of z_{I-1} . r_{I-1} for the next iteration
		 LM & DAM
		*/
		
		zdotr_previous = zdotr_current; 
		
		VecNorm( rVec, NORM_2, &rVecNorm );
		absResidual = rVecNorm * qReciprocalGlobalProblemScale;
		relResidual = absResidual / uzawaRhsScale;
		
		Stream_UnIndentBranch( StgFEM_Debug );
		
		if( iteration_I % outputInterval == 0 ) {
			Journal_PrintfL( self->info, 2, "\tLoop = %u, absResidual = %.8e, relResidual = %.8e\n", 
				iteration_I, absResidual, relResidual );
		}
		
		Journal_Firewall( isGoodNumber( absResidual ), errorStream, 
				"Error in func '%s' for %s '%s' - absResidual has an illegal value: '%g'\n",
				__func__, self->type, self->name, absResidual );
		
		Journal_Firewall( iteration_I < maxIterations, 
				errorStream, "In func %s: Reached maximum number of iterations %u without converging; absResidual = %.5g, relResidual = %.5g \n",
				__func__, iteration_I, absResidual, relResidual );

/* 		 TODO: test for small change in 10 iterations and if so restart? */

		time = MPI_Wtime()-t0;
		if (uzawa_summary) {
                	Journal_PrintfL( self->info, 1, "  %1.4d uzawa residual norm %12.13e, cpu time %5.5e\n", iteration_I+1,*chosenResidual,time );
        	}
			
	iteration_I++;  
	//END OF OUTER ITERATION LOOP!!!
		/*get wall time for end of outer loop*/
		self->outeritsendtime = MPI_Wtime();
		/* add time to total time inner its: */
		self->totalouteritstime = self->totalouteritstime + (-self->outeritsinitialtime + self->outeritsendtime);
		/* reset initial time and end time for inner its back to 0 - probs don't need to do this but just in case */
		self->outeritsinitialtime = 0;
		self->outeritsendtime = 0;
		/* add the outer loop iterations to the total outer iterations */
		self->totalnumouterits++; 
	}  while ( (*chosenResidual > self->tolerance) || (iteration_I<minIterations) );  
//	}  while ( *chosenResidual > self->tolerance );

	Journal_DPrintfL( self->debug, 1, "Pressure solution converged. Exiting uzawa \n ");
	
	/* STEP 5:  Check all the relevant residuals and report back */
	
	if (Stream_IsEnable( self->info ) ) {
	
	/* This information should be in an info stream */
	Journal_PrintfL( self->info, 1, "Summary:\n");
	Journal_PrintfL( self->info, 1, "  Uzawa its. = %04d , Uzawa residual = %12.13e\n", iteration_I, relResidual );
	MatMultTranspose( G_Mat, uVec, rVec );
	VecNorm( rVec, NORM_2, &rVecNorm );
	VecNorm( uVec, NORM_2, &uVecNorm );
	divU = rVecNorm / uVecNorm;
	Journal_PrintfL( self->info, 1, "  |G^T u|/|u|               = %.8e\n", divU);
	
	/* Residual for the momentum equation 
		Compute r = || F - Ku - Gp || / || F ||
	*/
	
	MatMult( G_Mat, qVec, vStarVec );
	MatMultAdd( K_Mat, uVec, vStarVec, fTempVec );
	VecAYPX( fTempVec, -1.0, fVec );
	
	VecNorm( fTempVec, NORM_2, &fTempVecNorm );
	VecNorm( fVec, NORM_2, &fVecNorm );
	momentumEquationResidual = fTempVecNorm / fVecNorm;
	Journal_PrintfL( self->info, 1, "  |f - K u - G p|/|f|       = %.8e\n", momentumEquationResidual );
	Journal_Firewall( isGoodNumber( momentumEquationResidual ), errorStream, 
			"Bad residual for the momentum equation (|| F - Ku - Gp || / || F || = %g):\n"
			"\tCheck to see if forcing term is zero or nan - \n\t|| F - Ku - Gp || = %g \n\t|| F || = %g.\n", 
			momentumEquationResidual,
			fTempVecNorm, fVecNorm );
		
	/* "Preconditioned"	residual for the momentum equation 
	 		r_{w} = || Q_{K}(r) || / || Q_{K}(F)
			fTempVec contains the residual but is overwritten once used
			vStarVec is used to hold the diagonal preconditioner Q_{K} 
	*/
	
	MatGetDiagonal( K_Mat, vStarVec );
	VecReciprocal( vStarVec );
	VecPointwiseMult( vStarVec, fTempVec, fTempVec );
	VecNorm( fTempVec, NORM_2, &weightedResidual );
	VecPointwiseMult( vStarVec, fVec, fTempVec );
	VecNorm( fTempVec, NORM_2, &weightedVelocityScale );
		
	Journal_PrintfL( self->info, 1, "  |f - K u - G p|_w/|f|_w   = %.8e\n", weightedResidual / weightedVelocityScale );	
		
	/* Report back on the solution - velocity and pressure 
	 Note - correction for dof in Vrms ??
	*/

	VecNorm( uVec, NORM_INFINITY, &uVecNormInf );
	VecNorm( uVec, NORM_2, &uVecNorm );
	VecGetSize( uVec, &uVecSize );
	VecNorm( qVec, NORM_INFINITY, &qVecNormInf );
	VecNorm( qVec, NORM_2, &qVecNorm );
	VecGetSize( qVec, &qVecSize );
        Journal_PrintfL( self->info, 1, "  |u|_{\\infty} = %.8e , u_rms = %.8e\n", 
		uVecNormInf, uVecNorm / sqrt( (double)uVecSize ) );
	Journal_PrintfL( self->info, 1, "  |p|_{\\infty} = %.8e , p_rms = %.8e\n",
               	qVecNormInf, qVecNorm / sqrt( (double)qVecSize ) );

	{	PetscInt lmin,lmax;
		PetscReal min,max;
		VecMax( uVec, &lmax, &max );
		VecMin( uVec, &lmin, &min );
		Journal_PrintfL( self->info, 1, "  min/max(u) = %.8e [%d] / %.8e [%d]\n",min,lmin,max,lmax);
                VecMax( qVec, &lmax, &max );
                VecMin( qVec, &lmin, &min );
                Journal_PrintfL( self->info, 1, "  min/max(p) = %.8e [%d] / %.8e [%d]\n",min,lmin,max,lmax);
        }
	VecSum( qVec, &p_sum );
	Journal_PrintfL( self->info, 1, "  \\sum_i p_i = %.8e \n", p_sum );

	} /* journal stream enabled */

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 3 ) ) {
		Journal_DPrintf( self->debug, "Velocity solution:\n" );
		_SLE_VectorView( uVec, self->debug );
		Journal_DPrintf( self->debug, "Pressure solution:\n" );
		_SLE_VectorView( qVec, self->debug );
	}
	#endif
	Stream_UnIndentBranch( StgFEM_Debug );

        Stream_SetPrintingRank( self->info, init_info_stream_rank );
		/* Now gather up data for printing out to FrequentOutput file: */
	
	
	/*!!! if non-linear need to divide by number of nonlinear iterations and we do this in SystemLinearEquations */
	if((sle->isNonLinear != True)){
		self->avgnuminnerits = self->totalnuminnerits/self->totalnumouterits;
		self->avgnumouterits = self->totalnumouterits;
		self->avgtimeouterits = (self->totalouteritstime - self->totalinneritstime)/self->totalnumouterits;
		self->avgtimeinnerits = self->totalinneritstime/self->totalnuminnerits;
	}	
}

void _Stokes_SLE_UzawaSolver_GetSolution( void *stokesSLE, void *solver, Vec *x )
{
	Stokes_SLE *sle = (Stokes_SLE*)stokesSLE;
	Vec p = sle->pSolnVec->vector;

	(*x) = p;
}


#undef __FUNCT__
#define __FUNCT__ "_Stokes_SLE_UzawaSolver_GetRhs"
PetscErrorCode _Stokes_SLE_UzawaSolver_GetRhs( void *stokesSLE, void *solver, Vec rhs )
{
        Stokes_SLE_UzawaSolver *self = (Stokes_SLE_UzawaSolver*)solver;
	Stokes_SLE *sle = (Stokes_SLE*)stokesSLE;
	/* stg linear algebra */
	KSP     A11_solver = self->velSolver;
        Mat         A12 = sle->gStiffMat->matrix;
        Vec         b1  = sle->fForceVec->vector;
        Vec         b2  = sle->hForceVec->vector;
        Vec         u_star = self->vStarVec;
	
	/* petsc variables */
	KSP ksp_A11;

	/* check operations will be valid */
	if (sle->dStiffMat!=NULL) {   Stg_SETERRQ( PETSC_ERR_SUP, "A21 must be NULL" ); }
	if (A11_solver==NULL){    Stg_SETERRQ( PETSC_ERR_ARG_NULL, "vel_solver is NULL" ); }
        if (A12==NULL){           Stg_SETERRQ( PETSC_ERR_ARG_NULL, "A12 is NULL" ); }
        if (b1==NULL){       Stg_SETERRQ( PETSC_ERR_ARG_NULL, "b1 is NULL" ); }
        if (b2==NULL){       Stg_SETERRQ( PETSC_ERR_ARG_NULL, "b2 is NULL" ); }
        if (u_star==NULL){   Stg_SETERRQ( PETSC_ERR_ARG_NULL, "u* is NULL" ); }

	/* Extract petsc objects */
	ksp_A11 = A11_solver;

	/* compute rhs = A12^T A11^{-1} b1 - b2 */
	KSPSolve( ksp_A11, b1, u_star );         /* u* = A11^{-1} b1 */
	MatMultTranspose( A12, u_star, rhs );    /* b2 = A12^T u* */
	VecAXPY( rhs, -1.0, b2 );  /* rhs <- rhs - b2 */
}


/* Computes r = f_hat - S p */
#undef __FUNCT__
#define __FUNCT__ "_Stokes_SLE_UzawaSolver_FormResidual"
PetscErrorCode _Stokes_SLE_UzawaSolver_FormResidual( void *stokesSLE, void *solver, Vec r )
{
        Stokes_SLE_UzawaSolver *self = (Stokes_SLE_UzawaSolver*)solver;
        Stokes_SLE *sle = (Stokes_SLE*)stokesSLE;
 	/* stg linear algebra objects */
        Mat         A12 = sle->gStiffMat->matrix;
        Mat         A22 = NULL;
        Vec         x2 = sle->pSolnVec->vector;
        Vec         f_star = self->fTempVec;
        Vec         u_star = self->vStarVec;
	Vec         q_star = self->pTempVec;
        KSP     A11_solver = self->velSolver;

	/* petsc objects */
	KSP ksp_A11;
	PetscInt r_N, x2_N;

        /* check operations will be valid */
	if (A11_solver==NULL){   Stg_SETERRQ( PETSC_ERR_ARG_NULL, "vel_solver is NULL" ); }
        if (sle->dStiffMat!=NULL) {  Stg_SETERRQ( PETSC_ERR_SUP, "A21 must be NULL" ); }
        if (A12==NULL){          Stg_SETERRQ( PETSC_ERR_ARG_NULL, "A12 is NULL" ); }
        if (x2==NULL){         Stg_SETERRQ( PETSC_ERR_ARG_NULL, "x2 is NULL" ); }
        if (u_star==NULL){     Stg_SETERRQ( PETSC_ERR_ARG_NULL, "u* is NULL" ); }
        if (f_star==NULL){     Stg_SETERRQ( PETSC_ERR_ARG_NULL, "f* is NULL" ); }
	if (q_star==NULL) {    Stg_SETERRQ( PETSC_ERR_ARG_NULL, "q* is NULL" ); }

	A22 = PETSC_NULL;
	if (sle->cStiffMat!=NULL) {
		A22 = sle->cStiffMat->matrix;
	}


        /* Extract petsc objects */
        ksp_A11 =     A11_solver;

	/* Check sizes match */
	VecGetSize( r, &r_N );
	VecGetSize( x2, &x2_N );
	if (r_N!=x2_N) {
		Stg_SETERRQ2( PETSC_ERR_ARG_SIZ, "Solution vector for pressure (N=%D) is not compatible with residual vector (N=%D)", x2_N, r_N );
	}	
	
	/* r = f_hat - (G^T K^{-1} G - M) p */
	_Stokes_SLE_UzawaSolver_GetRhs( stokesSLE, solver, r );  /* r <- f_hat */

	/* correct for non zero A22 */
	if (A22!=PETSC_NULL) {
		MatMultAdd( A22, x2, r, r );  /* r <- r + A22 p */
	}

	/* make correction r <- r - G^T K^{-1} G */
	MatMult( A12, x2, f_star );                 /* f* <- A12 x2 */
	KSPSolve( ksp_A11, f_star, u_star );        /* u* <- A11^{-1} f* */
	MatMultTranspose( A12, u_star, q_star );    /* q* <- A12 u* */
	
	VecAXPY( r, -1.0, q_star );  /* r <- r - q* */
}

Vec _Stokes_SLE_UzawaSolver_GetResidual( void* solver, Index fv_I ) {
	return NULL;
}



