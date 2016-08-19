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
#include <StgFEM/StgFEM.h>
#include "types.h"
#include <petsc.h>
#include <petscvec.h>
#include <petscmat.h>
#include <petscksp.h>
#include <petscpc.h>
#include <petscsnes.h>
#include <petscis.h>
#include <petscviewer.h>
#include <petscsys.h>
#include <petscversion.h>

#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include "petsc/private/kspimpl.h"
  #else
     #include "petsc-private/kspimpl.h"   /*I "petscksp.h" I*/
  #endif
#else
  #include "private/kspimpl.h"   /*I "petscksp.h" I*/
#endif

//#include "ksptypes.h"
#include "ksp-register.h"
#include "StokesBlockKSPInterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Macro for checking number integrity - i.e. checks if number is infinite or "not a number" */
#define SBKSP_isGoodNumber( number ) ( (! isnan( number ) ) && ( ! isinf( number ) ) )
#define SBKSP_GetPetscMatrix( matrix ) ( (Mat)(matrix) )
#define SBKSP_GetPetscVector( vector ) ( (Vec)(vector) )
#define SBKSP_GetPetscKSP( solver ) ( (KSP)(solver)  )
PetscErrorCode _BlockSolve( void* solver, void* _stokesSLE );

const Type StokesBlockKSPInterface_Type = "StokesBlockKSPInterface";

void* _StokesBlockKSPInterface_DefaultNew( Name name ) {
    SizeT                                              _sizeOfSelf = sizeof(StokesBlockKSPInterface);
    Type                                                      type = StokesBlockKSPInterface_Type;
    Stg_Class_DeleteFunction*                              _delete = _SLE_Solver_Delete;
    Stg_Class_PrintFunction*                                _print = _SLE_Solver_Print;
    Stg_Class_CopyFunction*                                  _copy = _SLE_Solver_Copy;
    Stg_Component_BuildFunction*                            _build = _StokesBlockKSPInterface_Build;
    Stg_Component_InitialiseFunction*                  _initialise = _StokesBlockKSPInterface_Initialise;
    Stg_Component_ExecuteFunction*                        _execute = _SLE_Solver_Execute;
    Stg_Component_DestroyFunction*                        _destroy = _SLE_Solver_Destroy;
    SLE_Solver_GetResidualFunc*                       _getResidual = NULL;
    Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _StokesBlockKSPInterface_DefaultNew;
    Stg_Component_ConstructFunction*                    _construct = _StokesBlockKSPInterface_AssignFromXML;
    SLE_Solver_SolverSetupFunction*                   _solverSetup = _StokesBlockKSPInterface_SolverSetup;
    SLE_Solver_SolveFunction*                               _solve = _StokesBlockKSPInterface_Solve;

    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;
    return (void*) _StokesBlockKSPInterface_New( STOKESBLOCKKSPINTERFACE_PASSARGS );
}


/* Creation implementation / Virtual constructor */
/* Set up function pointers */
StokesBlockKSPInterface* _StokesBlockKSPInterface_New( STOKESBLOCKKSPINTERFACE_DEFARGS )
{
    StokesBlockKSPInterface* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(StokesBlockKSPInterface) );

    self = (StokesBlockKSPInterface*) _SLE_Solver_New( SLE_SOLVER_PASSARGS );

    /* Virtual info */
    return self;
}

void _StokesBlockKSPInterface_Init(
		StokesBlockKSPInterface*      self,
		StiffnessMatrix*   preconditioner,
		Stokes_SLE *       st_sle,
		PETScMGSolver *    mg,
        Name   filename,
        char * string,
		StiffnessMatrix*  k2StiffMat,
		StiffnessMatrix*  mStiffMat,
		ForceVector*	  f2ForceVec,
		ForceVector*	  jForceVec,
		double            penaltyNumber,
		double            hFactor,
		StiffnessMatrix*  vmStiffMat,
		ForceVector*	  vmForceVec  )
{
	self->preconditioner = preconditioner;
	self->st_sle = st_sle;
	self->mg     = mg;
    self->optionsFile = filename;
    self->optionsString = string;
	self->k2StiffMat = k2StiffMat;
	self->f2ForceVec = f2ForceVec;
	self->penaltyNumber = penaltyNumber;
	self->hFactor       = hFactor;
	self->mStiffMat = mStiffMat;
	self->jForceVec = jForceVec;
	self->vmStiffMat = vmStiffMat;
	self->vmForceVec = vmForceVec;
	/* add the vecs and matrices to the Base SLE class's dynamic lists, so they can be
	initialised and built properly */

	/*
	if (k2StiffMat )
	    SystemLinearEquations_AddStiffnessMatrix( st_sle, k2StiffMat );

	if (f2ForceVec )
	    SystemLinearEquations_AddForceVector( st_sle, f2ForceVec );

	if (mStiffMat )
	    SystemLinearEquations_AddStiffnessMatrix( st_sle, mStiffMat );

	if (jForceVec )
	    SystemLinearEquations_AddForceVector( st_sle, jForceVec );

	if (vmStiffMat )
	    SystemLinearEquations_AddStiffnessMatrix( st_sle, vmStiffMat );

	if (vmForceVec )
	    SystemLinearEquations_AddForceVector( st_sle, vmForceVec );
    */
}

void _StokesBlockKSPInterface_Build( void* solver, void* sle ) {/* it is the sle here being passed in*/
	StokesBlockKSPInterface*	self  = (StokesBlockKSPInterface*)solver;

	Stream_IndentBranch( StgFEM_Debug );

	/* Build Preconditioner */
	if ( self->preconditioner ) {
		Stg_Component_Build( self->preconditioner, sle, False );
		SystemLinearEquations_AddStiffnessMatrix( self->st_sle, self->preconditioner );

	}
	if( self->mg ){
	    Stg_Component_Build( self->mg, sle, False );
	}

	Stream_UnIndentBranch( StgFEM_Debug );
}

void _StokesBlockKSPInterface_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data ) {
	StokesBlockKSPInterface* self         = (StokesBlockKSPInterface*) solver;
	//double                  tolerance;
	//Iteration_Index         maxUzawaIterations, minUzawaIterations;
	StiffnessMatrix*  preconditioner;
	StiffnessMatrix*  k2StiffMat;
	ForceVector*	  f2ForceVec;
	StiffnessMatrix*  mStiffMat;
	ForceVector*	  jForceVec;
	double            penaltyNumber;
	double            hFactor;
	StiffnessMatrix*  vmStiffMat;
	ForceVector*	  vmForceVec;
	//Bool                    useAbsoluteTolerance;
	//Bool                    monitor;
	Stokes_SLE *            st_sle;
	PETScMGSolver *         mg;
	//Name                filename;
	//char* 		        string;

	_SLE_Solver_AssignFromXML( self, cf, data );

	preconditioner = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Preconditioner", StiffnessMatrix, False, data  );
	st_sle  = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"stokesEqn", Stokes_SLE, True, data  );
	mg      = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"mgSolver", PETScMGSolver, False, data);
	k2StiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"2ndStressTensorMatrix", StiffnessMatrix, False, data  );
	f2ForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"2ndForceVector", ForceVector, False, data  );
	penaltyNumber   = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"penaltyNumber", 0.0  );
	hFactor         = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"hFactor", 0.0  );
	mStiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"MassMatrix", StiffnessMatrix, False, data  );
	jForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"JunkForceVector", ForceVector, False, data  );
	vmStiffMat = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityMassMatrix", StiffnessMatrix, False, data  );
	vmForceVec = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VMassForceVector", ForceVector, False, data  );

	_StokesBlockKSPInterface_Init( self, preconditioner, st_sle, mg, NULL, NULL, k2StiffMat, mStiffMat,
                                   f2ForceVec, jForceVec, penaltyNumber, hFactor, vmStiffMat, vmForceVec);

}

void _StokesBlockKSPInterface_Initialise( void* solver, void* data ) {
	StokesBlockKSPInterface* self = (StokesBlockKSPInterface*) solver;
	Stokes_SLE*             sle  = (Stokes_SLE*)  self->st_sle;

	/* Initialise Parent */
	_SLE_Solver_Initialise( self, sle );

	KSPRegisterAllKSP("Solvers/KSPSolvers/src");
}

/* SolverSetup */

void _StokesBlockKSPInterface_SolverSetup( void* solver, void* stokesSLE ) {
	StokesBlockKSPInterface* self = (StokesBlockKSPInterface*) solver;
	//Stokes_SLE*             sle  = (Stokes_SLE*)             stokesSLE;

 	Journal_DPrintf( self->debug, "In %s:\n", __func__ );
	Stream_IndentBranch( StgFEM_Debug );

	Stream_UnIndentBranch( StgFEM_Debug );
}
void SBKSP_SetSolver( void* solver, void* stokesSLE ) {
	SLE_Solver* self = (SLE_Solver*) solver;
	Stokes_SLE*              sle  = (Stokes_SLE*) stokesSLE;

    sle->solver=self;

}
void SBKSP_SetPenalty( void* solver, double penalty ) {
  StokesBlockKSPInterface* self = (StokesBlockKSPInterface*) solver;
  self->penaltyNumber=penalty;
}

int SBKSP_GetPressureIts(void *solver){
  StokesBlockKSPInterface* self = (StokesBlockKSPInterface*) solver;
  return self->stats.pressure_its;
}

/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/

void SBKSP_GetStokesOperators(
		Stokes_SLE *stokesSLE,
		Mat *K,Mat *G,Mat *D,Mat *C,Mat *approxS,
		Vec *f,Vec *h,Vec *u,Vec *p )
{

	*K = *G = *D = *C = PETSC_NULL;
	if (stokesSLE->kStiffMat){      *K = SBKSP_GetPetscMatrix( stokesSLE->kStiffMat->matrix );     }
	if (stokesSLE->gStiffMat){      *G = SBKSP_GetPetscMatrix( stokesSLE->gStiffMat->matrix );     }
	if (stokesSLE->dStiffMat){      *D = SBKSP_GetPetscMatrix( stokesSLE->dStiffMat->matrix );     }
	if (stokesSLE->cStiffMat){      *C = SBKSP_GetPetscMatrix( stokesSLE->cStiffMat->matrix );     }

	/* preconditioner */
	*approxS = PETSC_NULL;
	if( ((StokesBlockKSPInterface*)stokesSLE->solver)->preconditioner ) {
		StiffnessMatrix *preconditioner;

		preconditioner = ((StokesBlockKSPInterface*)stokesSLE->solver)->preconditioner;
		*approxS = SBKSP_GetPetscMatrix( preconditioner->matrix );
	}

	*f = *h = PETSC_NULL;
	if (stokesSLE->fForceVec){      *f = SBKSP_GetPetscVector( stokesSLE->fForceVec->vector );     }
	if (stokesSLE->hForceVec){      *h = SBKSP_GetPetscVector( stokesSLE->hForceVec->vector );     }

	*u = *p = PETSC_NULL;
	if (stokesSLE->uSolnVec){       *u = SBKSP_GetPetscVector( stokesSLE->uSolnVec->vector );      }
	if (stokesSLE->pSolnVec){       *p = SBKSP_GetPetscVector( stokesSLE->pSolnVec->vector );      }

}
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/* Sets up Solver to be a custom ksp (KSP_BSSCR) solve by default: */
void _StokesBlockKSPInterface_Solve( void* solver, void* _stokesSLE ) {
  StokesBlockKSPInterface* self    = (StokesBlockKSPInterface*)solver;
  PetscLogDouble flopsA,flopsB;
  PetscTruth found, get_flops;

  found = PETSC_FALSE;
  get_flops = PETSC_FALSE;
  PetscOptionsGetTruth( PETSC_NULL, "-get_flops", &get_flops, &found);
  if(get_flops){
    PetscGetFlops(&flopsA); }

  _BlockSolve(solver, _stokesSLE);

  if(get_flops){
    PetscGetFlops(&flopsB);
    self->stats.total_flops=(double)(flopsB-flopsA); }
}
PetscErrorCode _BlockSolve( void* solver, void* _stokesSLE ) {
  Stokes_SLE*  stokesSLE  = (Stokes_SLE*)_stokesSLE;
  StokesBlockKSPInterface* Solver    = (StokesBlockKSPInterface*)solver;

  /* Create shortcuts to stuff needed on sle */
  Mat       K;
  Mat       G;
  Mat       Gt;
  Mat       D;
  Mat       C;
  Mat       approxS;
  Vec       u;
  Vec       p;
  Vec       f;
  Vec       h;
  Mat stokes_P;
  Mat stokes_A;
  Vec stokes_x;
  Vec stokes_b;
  Mat a[2][2];
  Vec x[2];
  Vec b[2];
  KSP stokes_ksp;
  PC  stokes_pc;
  PetscTruth sym,flg;
  PetscErrorCode ierr;

  PetscInt   N,n;

  SBKSP_GetStokesOperators( stokesSLE, &K,&G,&D,&C, &approxS, &f,&h, &u,&p );

  /* create Gt */
  if( !D ) {
    ierr = MatTranspose( G, MAT_INITIAL_MATRIX, &Gt);CHKERRQ(ierr);
    sym = PETSC_TRUE;
    Solver->DIsSym = sym;
  }
  else {
    Gt = D;
    sym = PETSC_FALSE;
    Solver->DIsSym = sym;
  }
  flg=PETSC_FALSE;
  PetscOptionsHasName(PETSC_NULL,"-use_petsc_ksp",&flg);
  if (flg) {
    if( !C ) {
      /* Everything in this bracket, dependent on !C, is to build
         a matrix with diagonals of 0 for C the previous comment ways

      need a 'zero' matrix to keep fieldsplit happy in petsc? */
      MatType mtype;
      Vec V;
      //MatGetSize( G, &M, &N );
      VecGetSize(p, &N);
      VecGetLocalSize( p, &n );
      MatCreate( PetscObjectComm((PetscObject) K), &C );
      MatSetSizes( C, PETSC_DECIDE ,PETSC_DECIDE, N, N );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
      MatSetUp(C);
#endif
      MatGetType( G, &mtype );
      MatSetType( C, mtype );
      MatGetVecs( G, &V, PETSC_NULL );
      VecSet(V, 0.0);
      //VecSet(h, 1.0);
      ierr = VecAssemblyBegin( V );CHKERRQ(ierr);
      ierr = VecAssemblyEnd  ( V );CHKERRQ(ierr);
      ierr = MatDiagonalSet(C,V,INSERT_VALUES);CHKERRQ(ierr);
      ierr = MatAssemblyBegin( C, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr);
      ierr = MatAssemblyEnd  ( C, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr);
    }
  }
  a[0][0]=K;  a[0][1]=G;
  a[1][0]=Gt; a[1][1]=C;
  ierr = MatCreateNest(PetscObjectComm((PetscObject) K), 2, NULL, 2, NULL, (Mat *)a, &stokes_A);CHKERRQ(ierr);
  ierr = MatAssemblyBegin( stokes_A, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr);
  ierr = MatAssemblyEnd( stokes_A, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr);



  x[0]=u;
  x[1]=p;
  ierr = VecCreateNest(PetscObjectComm((PetscObject) u), 2, NULL, x, &stokes_x);CHKERRQ(ierr);
  ierr = VecAssemblyBegin( stokes_x );CHKERRQ(ierr);
  ierr = VecAssemblyEnd( stokes_x);CHKERRQ(ierr);

  b[0]=f;
  b[1]=h;
  ierr = VecCreateNest(PetscObjectComm((PetscObject) f), 2, NULL, b, &stokes_b);CHKERRQ(ierr);
  ierr = VecAssemblyBegin( stokes_b );CHKERRQ(ierr);
  ierr = VecAssemblyEnd( stokes_b);CHKERRQ(ierr);

  /* if( approxS ) { */
  /*   a[0][0]=K;    a[0][1]=G; */
  /*   a[1][0]=NULL; a[1][1]=approxS; */
  /*   ierr = MatCreateNest(PetscObjectComm((PetscObject) K), 2, NULL, 2, NULL, (Mat *)a, &stokes_P);CHKERRQ(ierr); */
  /*   ierr = MatAssemblyBegin( stokes_P, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr); */
  /*   ierr = MatAssemblyEnd( stokes_P, MAT_FINAL_ASSEMBLY );CHKERRQ(ierr); */
  /* } */
  /* else { */
    stokes_P = stokes_A;
  /* } */

  /* probably should make a Destroy function for these two */
  /* Update options from file and/or string here so we can change things on the fly */
  //PetscOptionsInsertFile(PETSC_COMM_WORLD, Solver->optionsFile, PETSC_FALSE);
  //PetscOptionsInsertString(Solver->optionsString);

  ierr = KSPCreate( PETSC_COMM_WORLD, &stokes_ksp );CHKERRQ(ierr);
  Stg_KSPSetOperators( stokes_ksp, stokes_A, stokes_P, SAME_NONZERO_PATTERN );
  ierr = KSPSetType( stokes_ksp, "bsscr" );/* i.e. making this the default solver : calls KSPCreate_XXX */CHKERRQ(ierr);

  ierr = KSPGetPC( stokes_ksp, &stokes_pc );CHKERRQ(ierr);
  ierr = PCSetType( stokes_pc, PCNONE );CHKERRQ(ierr);
  ierr = KSPSetInitialGuessNonzero( stokes_ksp, PETSC_TRUE );CHKERRQ(ierr);
  ierr = KSPSetFromOptions( stokes_ksp );CHKERRQ(ierr);

  /*
    Doing this so the KSP Solver has access to the StgFEM Multigrid struct (PETScMGSolver).
    As well as any custom stuff on the Stokes_SLE struct
  */
  if( stokes_ksp->data ){/* then ksp->data has been created in a KSpSetUp_XXX function */
    /* testing for our KSP types that need the data that is on Solver... */
    /* for the moment then, this function not completely agnostic about our KSPs */
    //if(!strcmp("bsscr",stokes_ksp->type_name)){/* if is bsscr then set up the data on the ksp */
    flg=PETSC_FALSE;
    PetscOptionsHasName(PETSC_NULL,"-use_petsc_ksp",&flg);
    if (!flg) {
      ((KSP_COMMON*)(stokes_ksp->data))->st_sle         = Solver->st_sle;
      ((KSP_COMMON*)(stokes_ksp->data))->mg             = Solver->mg;
      ((KSP_COMMON*)(stokes_ksp->data))->DIsSym         = Solver->DIsSym;
      ((KSP_COMMON*)(stokes_ksp->data))->preconditioner = Solver->preconditioner;
      ((KSP_COMMON*)(stokes_ksp->data))->solver         = Solver;
    }
  }

  ierr = KSPSolve( stokes_ksp, stokes_b, stokes_x );CHKERRQ(ierr);

  Stg_KSPDestroy(&stokes_ksp );
  //if( ((StokesBlockKSPInterface*)stokesSLE->solver)->preconditioner )
  if(stokes_P != stokes_A) { Stg_MatDestroy(&stokes_P ); }

  Stg_MatDestroy(&stokes_A );

  Stg_VecDestroy(&stokes_x);
  Stg_VecDestroy(&stokes_b);

  if(!D){ Stg_MatDestroy(&Gt); }
  if(C && (stokesSLE->cStiffMat->matrix != C) ){ Stg_MatDestroy(&C); }

  PetscFunctionReturn(0);
}
