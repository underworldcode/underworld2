/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>
#include <petscksp.h>
#include <petscpc.h>

#include <petscversion.h>
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include <petsc/private/kspimpl.h>
  #else
     #include <petsc-private/kspimpl.h>
  #endif
#else
  #include <private/kspimpl.h>
#endif


#include "common-driver-utils.h"

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "Solvers/KSPSolvers/KSPSolvers.h"
#include "petsccompat.h"
#include "BSSCR.h"
#include "stokes_block_scaling.h"
#include "stokes_mvblock_scaling.h"
#include "mg.h"
#include "ksp_pressure_nullspace.h"

typedef struct {
    int num_nsp_vecs;
    Vec *nsp_vecs;
} nsp_data_t;


typedef struct {
    KSP kspA11;
    SLE_Solver *sleSolver;
} WendyContext;


//PetscErrorCode BSSCR_NSPRemoveAll(Vec v, void *_data);

PetscErrorCode BSSCR_KSPNormInfMonitor( KSP ksp, PetscInt iteration, PetscReal residualNorm, void *dummy);
PetscErrorCode BSSCR_KSPNormInfToNorm2Monitor( KSP ksp, PetscInt iteration, PetscReal residualNorm, void *dummy);



PetscErrorCode BSSCR_DRIVER_flex( KSP ksp, Mat stokes_A, Vec stokes_x, Vec stokes_b, Mat approxS, KSP ksp_K,
                                  MatStokesBlockScaling BA, PetscTruth sym, KSP_BSSCR * bsscrp_self )
{
    char name[PETSC_MAX_PATH_LEN];
    char ubefore[100];
    char uafter[100];
    char pbefore[100];
    char pafter[100];
    PetscTruth flg, flg2, truth, useAcceleratingSmoothingMG, useFancySmoothingMG;
    PetscTruth usePreviousGuess, useNormInfStoppingConditions, useNormInfMonitor, found, extractMats;
    Mat K,G,D,C;
    Vec u,p,f,h;
    Mat S;
    Vec h_hat,t,t2,q,v;

    KSP ksp_inner;
    KSP ksp_S;
    KSP ksp_cleaner;
    KSPType ksp_inner_type;

    PetscTruth has_cnst_nullspace;
    PC pc_S, pc_MG, pcInner;
    PetscInt monitor_index,max_it,min_it;
    Vec nsp_vec = PETSC_NULL;

    PetscReal scr_rtol;
    PetscReal inner_rtol;
    PetscReal vSolver_rtol;

    PetscScalar uNormInf, pNormInf;
    PetscScalar uNorm, pNorm, rNorm, fNorm;
    PetscInt  uSize, pSize;
    PetscInt    lmin,lmax;
    PetscInt  iterations, rhs_iterations;
    PetscReal   min,max;
    PetscReal p_sum;

    MGContext mgCtx;
    PC shellPC;

    double t0, t1;
    double mgSetupTime, rhsSolveTime, problemBuildTime, scrSolveTime, a11SingleSolveTime, solutionAnalysisTime;
    Index nx,ny,nz;
    PetscInt j,start,end;

    static int been_here = 0;  /* Ha Ha Ha !! */

    /* get sub matrix / vector objects */
    MatNestGetSubMat( stokes_A, 0,0, &K );
    MatNestGetSubMat( stokes_A, 0,1, &G );
    MatNestGetSubMat( stokes_A, 1,0, &D );
    MatNestGetSubMat( stokes_A, 1,1, &C );

    VecNestGetSubVec( stokes_x, 0, &u );
    VecNestGetSubVec( stokes_x, 1, &p );

    VecNestGetSubVec( stokes_b, 0, &f );
    VecNestGetSubVec( stokes_b, 1, &h );

    /* PetscPrintf( PETSC_COMM_WORLD,  "\t Adress of stokes_x is %p\n", stokes_x); */
    /* VecNorm( u, NORM_2, &uNorm ); */
    /* PetscPrintf( PETSC_COMM_WORLD,  "\t u Norm is %.6e in %s: address is %p\n",uNorm,__func__,u); */
    /* VecNorm( p, NORM_2, &pNorm ); */
    /* PetscPrintf( PETSC_COMM_WORLD,  "\t p Norm is %.6e in %s: addres is %p\n",pNorm,__func__,p); */
    /* Create Schur complement matrix */


    problemBuildTime = MPI_Wtime();


    //MatCreateSchurFromBlock( stokes_A, 0.0, "MatSchur_A11", &S );
    MatCreateSchurComplement(K,K,G,D,C, &S);
    /* configure inner solver */
    if (ksp_K!=PETSC_NULL) {
        MatSchurComplementSetKSP( S, ksp_K );
        MatSchurComplementGetKSP( S, &ksp_inner );
    }
    else {
        abort();
        MatSchurComplementGetKSP( S, &ksp_inner );
        KSPSetType( ksp_inner, "cg" );
    }
    KSPGetPC( ksp_inner, &pcInner );

    /* If we're using multigrid, replace the preconditioner here
       so we get the same options prefix. */

    if(bsscrp_self->mg) {
        mgSetupTime=setupMG( bsscrp_self, ksp_inner, pcInner, K, &mgCtx );
    }

    /* SETFROMOPTIONS MIGHT FUCK MG UP */
    KSPSetOptionsPrefix( ksp_inner, "A11_" );
    KSPSetFromOptions( ksp_inner );

    useNormInfStoppingConditions = PETSC_FALSE;
    PetscOptionsGetTruth( PETSC_NULL ,"-A11_use_norm_inf_stopping_condition", &useNormInfStoppingConditions, &found );
    if(useNormInfStoppingConditions)
        BSSCR_KSPSetNormInfConvergenceTest( ksp_inner );

    useNormInfMonitor = PETSC_FALSE;
    PetscOptionsGetTruth( PETSC_NULL, "-A11_ksp_norm_inf_monitor", &useNormInfMonitor, &found );
    if(useNormInfMonitor)
        KSPMonitorSet( ksp_inner, BSSCR_KSPNormInfMonitor, PETSC_NULL, PETSC_NULL );

    useNormInfMonitor = PETSC_FALSE;
    PetscOptionsGetTruth( PETSC_NULL, "-A11_ksp_norm_inf_to_norm_2_monitor", &useNormInfMonitor, &found );
    if(useNormInfMonitor)
        KSPMonitorSet( ksp_inner, BSSCR_KSPNormInfToNorm2Monitor, PETSC_NULL, PETSC_NULL );

    /* create right hand side */
    /* h_hat = G'*inv(K)*f - h */

    MatGetVecs(K,PETSC_NULL,&t);
    MatGetVecs( S, PETSC_NULL, &h_hat );

    KSPSetOptionsPrefix( ksp_inner, "A11_" );
    KSPSetFromOptions( ksp_inner );

    problemBuildTime = MPI_Wtime() - problemBuildTime;

    rhsSolveTime = MPI_Wtime();
    KSPSolve(ksp_inner,f,t);/* t=f/K */
    rhsSolveTime = MPI_Wtime() - rhsSolveTime;
    KSPGetIterationNumber( ksp_inner, &rhs_iterations);

    //bsscr_writeVec( t, "ts", "Writing t vector");
    MatMult(D,t,h_hat);/* G'*t */
    VecAXPY(h_hat, -1.0, h);/* h_hat = h_hat - h */
    Stg_VecDestroy(&t);
    //bsscr_writeVec( h_hat, "h_hat", "Writing h_hat Vector in Solver");
    //MatSchurApplyReductionToVecFromBlock( S, stokes_b, h_hat );

    /* create solver for S p = h_hat */

    KSPCreate( PETSC_COMM_WORLD, &ksp_S );
    KSPSetOptionsPrefix( ksp_S, "scr_");
    Stg_KSPSetOperators( ksp_S, S,S, SAME_NONZERO_PATTERN );
    KSPSetType( ksp_S, "cg" );

    /* Build preconditioner for S */
    KSPGetPC( ksp_S, &pc_S );
    BSSCR_BSSCR_StokesCreatePCSchur2( K,G,D,C,approxS,pc_S,sym, bsscrp_self );

    KSPSetFromOptions(ksp_S);

    /* Set specific monitor test */
    KSPGetTolerances( ksp_S, PETSC_NULL, PETSC_NULL, PETSC_NULL, &max_it );
    //BSSCR_KSPLogSetMonitor( ksp_S, max_it, &monitor_index );

    /* Pressure / Velocity Solve */
    scrSolveTime = MPI_Wtime();
    // PetscPrintf( PETSC_COMM_WORLD, "\t* Pressure / Velocity Solve \n");

    usePreviousGuess = PETSC_FALSE;
    if(been_here)
        PetscOptionsGetTruth( PETSC_NULL, "-scr_use_previous_guess", &usePreviousGuess, &found );

    if(usePreviousGuess) {   /* Note this should actually look at checkpoint information */
        KSPSetInitialGuessNonzero( ksp_S, PETSC_TRUE );
    }
    else {
        KSPSetInitialGuessNonzero( ksp_S, PETSC_FALSE );
    }

    //KSPSetRelativeRhsConvergenceTest( ksp_S );

    useNormInfStoppingConditions = PETSC_FALSE;
    PetscOptionsGetTruth( PETSC_NULL ,"-scr_use_norm_inf_stopping_condition", &useNormInfStoppingConditions, &found );
    if(useNormInfStoppingConditions)
        BSSCR_KSPSetNormInfConvergenceTest(ksp_S);

    useNormInfMonitor = PETSC_FALSE;
    PetscOptionsGetTruth( PETSC_NULL, "-scr_ksp_norm_inf_monitor", &useNormInfMonitor, &found );
    if(useNormInfMonitor)
        KSPMonitorSet( ksp_S, BSSCR_KSPNormInfToNorm2Monitor, PETSC_NULL, PETSC_NULL );


    // PetscPrintf( PETSC_COMM_WORLD, "\t* KSPSolve( ksp_S, h_hat, p )\n");
    /* if h_hat needs to be fixed up ..take out any nullspace vectors here */
    /* we want to check that there is no "noise" in the null-space in the h vector */
    /* this causes problems when we are trying to solve a Jacobian system when the Residual is almost converged */

    if(bsscrp_self->check_pressureNS){
        bsscrp_self->buildPNS(ksp);/* build and set nullspace vectors on bsscr - which is on ksp (function pointer is set in KSPSetUp_BSSCR */
    }

    PetscScalar norm, a, a1, a2, hnorm, pnorm, gnorm;
    MatNorm(G,NORM_INFINITY,&gnorm);
    VecNorm(h_hat, NORM_2, &hnorm);
    hnorm=hnorm/gnorm;

    if((hnorm < 1e-6) && (hnorm > 1e-20)){
        VecScale(h_hat,1.0/hnorm);
    }
    /* test to see if v or t are in nullspace of G and orthogonalize wrt h_hat if needed */
    KSPRemovePressureNullspace_BSSCR(ksp, h_hat);
    /***************************************/
    /* set convergence test to use min_it */
    found = PETSC_FALSE;
    min_it = 0;
    PetscOptionsGetInt( PETSC_NULL,"-scr_ksp_set_min_it_converge", &min_it, &found);
    if(found && min_it > 0){
        BSSCR_KSPSetConvergenceMinIts(ksp_S, min_it, bsscrp_self);
    }
    KSPSolve( ksp_S, h_hat, p );
    sprintf(pafter,"psafter_%d",been_here);
    // bsscr_writeVec( p, pafter, "Writing p Vector in Solver");
    /***************************************/
    if((hnorm < 1e-6) && (hnorm > 1e-20)){
        VecScale(h_hat,hnorm);
        VecScale(p,hnorm);
    }
    KSPRemovePressureNullspace_BSSCR(ksp, p);


    scrSolveTime =  MPI_Wtime() - scrSolveTime;
    // PetscPrintf( PETSC_COMM_WORLD, "  SCR Solve  Finished in time: %lf seconds\n\n", scrSolveTime);
    /* Resolve with this pressure to obtain solution for u */

    /* obtain solution for u */
    VecDuplicate( u, &t );
    MatMult( G, p, t);
    VecAYPX( t, -1.0, f ); /* t <- -t + f */
    MatSchurComplementGetKSP( S, &ksp_inner );


    a11SingleSolveTime = MPI_Wtime();         /* ----------------------------------  Final V Solve */

    if(usePreviousGuess)
        KSPSetInitialGuessNonzero( ksp_inner, PETSC_TRUE );

    KSPSetOptionsPrefix( ksp_inner, "backsolveA11_" );
    KSPSetFromOptions( ksp_inner );
    KSPSolve( ksp_inner, t, u );       /* Solve, then restore default tolerance and initial guess */


    a11SingleSolveTime = MPI_Wtime() - a11SingleSolveTime;            /* ------------------ Final V Solve */


    PetscPrintf( PETSC_COMM_WORLD,  "SCR Solver Summary:\n\n");

    if(bsscrp_self->mg)
        PetscPrintf( PETSC_COMM_WORLD, "  Multigrid setup:        = %.4g secs \n", mgSetupTime);


    PetscPrintf( PETSC_COMM_WORLD,     "  RHS / Pre Solve:        = %.4g secs / %d its\n", rhsSolveTime, rhs_iterations);
    KSPGetIterationNumber( ksp_S, &iterations);
    PetscPrintf( PETSC_COMM_WORLD,     "  Pressure Solve:         = %.4g secs / %d its\n", scrSolveTime, iterations);
    KSPGetIterationNumber( ksp_inner, &iterations);
    PetscPrintf( PETSC_COMM_WORLD,     "  Final V Solve:          = %.4g secs / %d its\n\n", a11SingleSolveTime, iterations);

    /* Analysis of solution:
       This can be somewhat time consuming as it requires allocation / de-allocation,
       computing vector norms etc. So we make it optional..
       This should be put into a proper KSP  monitor now?
    */
    flg = PETSC_TRUE;
    PetscOptionsGetTruth( PETSC_NULL, "-scr_ksp_solution_summary", &flg, &found );

    if(flg) {

        solutionAnalysisTime = MPI_Wtime();

        VecGetSize( u, &uSize );
        VecGetSize( p, &pSize );

        VecDuplicate( u, &t2 );
        MatMult( K, u, t2);
        VecAYPX( t2, -1.0, t ); /* t2 <- -t2 + t  ... should be the formal residual vector */

        VecNorm( t2, NORM_2, &rNorm );
        VecNorm( f,  NORM_2, &fNorm );

        PetscPrintf( PETSC_COMM_WORLD,  "  |f - K u - G p|/|f|     = %.6e\n", rNorm/fNorm );

        VecDuplicate( p, &q );
        MatMult( D, u, q );
        VecNorm( u, NORM_2, &uNorm );
        VecNorm( q, NORM_2, &rNorm );

        PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|_2/|u|_2         = %.6e\n", sqrt( (double) uSize / (double) pSize ) * rNorm / uNorm);

        VecNorm( q, NORM_INFINITY, &rNorm );

        PetscPrintf( PETSC_COMM_WORLD,  "  |G^T u|_infty/|u|_2     = %.6e\n", sqrt( (double) uSize ) * rNorm / uNorm);

        VecNorm( u, NORM_INFINITY, &uNormInf );
        VecNorm( u, NORM_2,        &uNorm );
        VecGetSize( u, &uSize );

        VecNorm( p, NORM_INFINITY, &pNormInf );
        VecNorm( p, NORM_2,        &pNorm );

        PetscPrintf( PETSC_COMM_WORLD,  "  |u|_{\\infty}  = %.6e , u_rms = %.6e\n",
                     uNormInf, uNorm / sqrt( (double) uSize ) );

        PetscPrintf( PETSC_COMM_WORLD,  "  |p|_{\\infty}  = %.6e , p_rms = %.6e\n",
                     pNormInf, pNorm / sqrt( (double) pSize ) );

        VecMax( u, &lmax, &max );
        VecMin( u, &lmin, &min );
        PetscPrintf( PETSC_COMM_WORLD,  "  min/max(u)    = %.6e [%d] / %.6e [%d]\n",min,lmin,max,lmax);

        VecMax( p, &lmax, &max );
        VecMin( p, &lmin, &min );
        PetscPrintf( PETSC_COMM_WORLD,  "  min/max(p)    = %.6e [%d] / %.6e [%d]\n",min,lmin,max,lmax);

        VecSum( p, &p_sum );
        PetscPrintf( PETSC_COMM_WORLD,  "  \\sum_i p_i    = %.6e \n", p_sum );

        solutionAnalysisTime = MPI_Wtime() - solutionAnalysisTime;

        PetscPrintf( PETSC_COMM_WORLD,  "\n  Time for this analysis  = %.4g secs\n\n",solutionAnalysisTime);

        Stg_VecDestroy(&t2 );
        Stg_VecDestroy(&q );

    }

    if(bsscrp_self->mg) {
        //MG_inner_solver_pcmg_shutdown( pcInner );
    }

    Stg_VecDestroy(&t );


//      KSPLogDestroyMonitor( ksp_S );

    Stg_KSPDestroy(&ksp_S );
    //Stg_KSPDestroy(&ksp_inner );
    Stg_VecDestroy(&h_hat );
    Stg_MatDestroy(&S );

    /* Destroy nullspace vector if it exists. */
    if(nsp_vec)
        Stg_VecDestroy(&nsp_vec);

    //been_here = 1;
    been_here++;
    PetscFunctionReturn(0);
}



/*
  A pointwise stopping condition (infinity norm)

  To activate this stopping condition, I add this piece of code somewhere before the call to KSPSolve().

  {
  PetscTruth pw,flg;
  pw = PETSC_FALSE;
  PetscOptionsGetTruth(0,"-pointwise_stopping_condition", &pw,&flg);
  if(pw==PETSC_TRUE) { KSPSetPointwiseConvergenceTest(ksp_T); }
  }

  You can activate a monitor for the max pointwise residual by using the cmd line option
  -XXX_ksp_pointwise_monitor
  where XXX is the solver prefix specified by the call KSPSetOptionsPrefix().

  DAM
*/



typedef struct {
    PetscTruth initialrtol;    /* default relative residual decrease is computing from initial residual, not rhs */
    PetscTruth mininitialrtol; /* default relative residual decrease is computing from min of initial residual and rhs */
    Vec        work;
    PetscReal  pointwise_max;
} KSPPWConvergedCtx;



#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPPWConvergedCreate"
PetscErrorCode BSSCR_KSPPWConvergedCreate(void **ctx)
{
    PetscErrorCode            ierr;
    KSPPWConvergedCtx         *cctx;

    PetscFunctionBegin;
    ierr = Stg_PetscNew(KSPPWConvergedCtx,&cctx);CHKERRQ(ierr);
    *ctx = cctx;
    PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPPWConvergedDestroy"
PetscErrorCode BSSCR_KSPPWConvergedDestroy(void *ctx)
{
    PetscErrorCode         ierr;
    KSPPWConvergedCtx *cctx = (KSPPWConvergedCtx*) ctx;

    PetscFunctionBegin;
    if (cctx->work) {ierr = Stg_VecDestroy(&cctx->work);CHKERRQ(ierr);}
    ierr = PetscFree(cctx);CHKERRQ(ierr);
    PetscFunctionReturn(0);
}



#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPNormInfConverged"
PetscErrorCode BSSCR_KSPNormInfConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *ctx)
{
    PetscErrorCode         ierr;
    KSPPWConvergedCtx     *cctx = (KSPPWConvergedCtx*)ctx;
    KSPNormType            normtype;
    PetscReal              min, max, R_max, R_min, R_Ninf;
    Vec                    R, work, w1,w2;

    PetscFunctionBegin;
    PetscValidHeaderSpecific(ksp,KSP_COOKIE,1);
    PetscValidPointer(reason,4);
    *reason = KSP_CONVERGED_ITERATING;

    ierr = VecDuplicate(ksp->vec_rhs,&work);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w1);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w2);CHKERRQ(ierr);

    KSPBuildResidual( ksp, w1,w2, &R );
    VecNorm( R, NORM_INFINITY, &R_Ninf );

    //PetscPrintf( PETSC_COMM_WORLD, "Norm inf convergence %s\n ", ksp->prefix);

    cctx->pointwise_max = R_Ninf;

    ierr = KSPGetNormType(ksp,&normtype);

    CHKERRQ(ierr);
    if (normtype == KSP_NORM_NO)
        Stg_SETERRQ(PETSC_ERR_ARG_WRONGSTATE,"Use BSSCR_KSPSkipConverged() with KSPNormType of KSP_NORM_NO");

    if (!cctx)
        Stg_SETERRQ(PETSC_ERR_ARG_NULL,"Convergence context must have been created with BSSCR_KSPDefaultConvergedCreate()");

    if (!n) {
        /* if user gives initial guess need to compute norm of b */
        if (!ksp->guess_zero && !cctx->initialrtol) {
            PetscReal      snorm;
            if (ksp->normtype == KSP_NORM_UNPRECONDITIONED || ksp->pc_side == PC_RIGHT) {
                ierr = PetscInfo(ksp,"user has provided nonzero initial guess, computing 2-norm of RHS\n");
                CHKERRQ(ierr);
                ierr = VecNorm(ksp->vec_rhs,NORM_INFINITY,&snorm);CHKERRQ(ierr);        /*     <- b'*b */

                PetscPrintf( PETSC_COMM_WORLD,  "Non Zero Guess; RHS - %g\n", snorm);

            }
            else {
                Vec z;
                if (!cctx->work) {
                    ierr = VecDuplicate(ksp->vec_rhs,&cctx->work);CHKERRQ(ierr);
                }
                z = cctx->work;
                ierr = KSP_PCApply(ksp,ksp->vec_rhs,z);CHKERRQ(ierr);
                if (ksp->normtype == KSP_NORM_PRECONDITIONED) {
                    ierr = PetscInfo(ksp,"user has provided nonzero initial guess, computing 2-norm of preconditioned RHS\n");CHKERRQ(ierr);
                    ierr = VecNorm(z,NORM_INFINITY,&snorm);CHKERRQ(ierr);                 /*    dp <- b'*B'*B*b */

                }
                else if (ksp->normtype == KSP_NORM_NATURAL) {
                    PetscScalar norm;
                    Vec bz;
                    ierr = PetscInfo(ksp,"user has provided nonzero initial guess, computing natural norm of RHS\n");CHKERRQ(ierr);
                    //        ierr  = VecDot(ksp->vec_rhs,z,&norm);
                    //        snorm = sqrt(PetscAbsScalar(norm));                            /*    dp <- b'*B*b */
                    VecDuplicate( z, &bz );
                    VecPointwiseMult( bz, ksp->vec_rhs, z );
                    ierr = VecNorm(bz,NORM_INFINITY,&snorm);CHKERRQ(ierr);
                    Stg_VecDestroy(&bz);
                }
            }
            /* handle special case of zero RHS and nonzero guess */
            if (!snorm) {
                ierr = PetscInfo(ksp,"Special case, user has provided nonzero initial guess and zero RHS\n");CHKERRQ(ierr);
                snorm = rnorm;
            }
            if (cctx->mininitialrtol) {
                ksp->rnorm0 = PetscMin(snorm,rnorm);
            } else {
                ksp->rnorm0 = snorm;
            }
        } else {
            ksp->rnorm0 = rnorm;
        }
        ksp->ttol   = PetscMax(ksp->rtol*ksp->rnorm0,ksp->abstol);
    }

    // if (n <= ksp->chknorm) PetscFunctionReturn(0);

    if ( R_Ninf != R_Ninf ) {
        ierr = PetscInfo(ksp,"Linear solver has created a not a number (NaN) as the pointwise residual norm, declaring divergence \n");CHKERRQ(ierr);
        *reason = KSP_DIVERGED_NAN;
    }
    else
        if (R_Ninf <= ksp->ttol) {
            if (R_Ninf < ksp->abstol) {
                ierr = PetscInfo3(ksp,"Linear solver has converged. Pointwise residual %G is less than absolute tolerance %G at iteration %D\n",R_Ninf,ksp->abstol,n);
                CHKERRQ(ierr);
                *reason = KSP_CONVERGED_ATOL;
            }
            else {
                if (cctx->initialrtol) {
                    ierr = PetscInfo4(ksp,"Linear solver has converged. Norm_infinity %G is less than relative tolerance %G times initial Norm_infinity %G at iteration %D\n",R_Ninf,ksp->rtol,ksp->rnorm0,n);
                    CHKERRQ(ierr);
                }
                else {
                    ierr = PetscInfo4(ksp,"Linear solver has converged. Norm_infinity %G is less than relative tolerance %G times initial norm_infinity right hand side %G at iteration %D\n",R_Ninf,ksp->rtol,ksp->rnorm0,n);CHKERRQ(ierr);
                }
                *reason = KSP_CONVERGED_RTOL;
            }
        }
        else
            if (R_Ninf >= ksp->divtol*ksp->rnorm0) {
                ierr = PetscInfo3(ksp,"Linear solver is diverging. Initial right hand size Norm_infinity value %G, current residual norm %G at iteration %D\n",ksp->rnorm0,R_Ninf,n);CHKERRQ(ierr);
                *reason = KSP_DIVERGED_DTOL;
            }

    /* trash all work vectors here */

    Stg_VecDestroy(&work);
    Stg_VecDestroy(&w1);
    Stg_VecDestroy(&w2);

    PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPNormInfToNorm2Monitor"
PetscErrorCode BSSCR_KSPNormInfToNorm2Monitor(KSP ksp,PetscInt n,PetscReal rnorm, void *dummy)
{
    PetscErrorCode          ierr;
    PetscViewerASCIIMonitor viewer  = dummy ? (PetscViewer) dummy : PETSC_VIEWER_STDOUT_(((PetscObject)ksp)->comm);
    PetscReal              R_normInf, R_norm2;
    PetscInt                     R_size;
    Vec                    R, work, w1, w2;

    PetscFunctionBegin;

    ierr = VecDuplicate(ksp->vec_rhs,&work);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w1);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w2);CHKERRQ(ierr);

    KSPBuildResidual( ksp, w1,w2, &R );

    VecNorm( R, NORM_INFINITY, &R_normInf );
    VecNorm( R, NORM_2,        &R_norm2 );
    VecGetSize( R, &R_size );

    ierr = PetscViewerASCIIMonitorCreate(((PetscObject)ksp)->comm,"stdout",0,&viewer); CHKERRQ(ierr);

    if(R_norm2 == 0.0) {
        ierr = PetscViewerASCIIMonitorPrintf(viewer,"%3D KSP Residual Spikiness - INFINITY (%s) \n",
                                             n, ((PetscObject)ksp)->prefix ? ((PetscObject)ksp)->prefix: "" );
        CHKERRQ(ierr);
    }
    else{
        ierr = PetscViewerASCIIMonitorPrintf(viewer,"%3D KSP Residual Spikiness %14.12e (%s) Max / Rms \n",
                                             n,  sqrt((double) R_size) * R_normInf / R_norm2,
                                             ((PetscObject)ksp)->prefix ? ((PetscObject)ksp)->prefix: "");
        CHKERRQ(ierr);
    }

    ierr = PetscViewerASCIIMonitorDestroy(viewer);
    CHKERRQ(ierr);

    Stg_VecDestroy(&work);
    Stg_VecDestroy(&w1);
    Stg_VecDestroy(&w2);

    PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPNormInfMonitor"
PetscErrorCode BSSCR_KSPNormInfMonitor(KSP ksp,PetscInt n,PetscReal rnorm, void *dummy)
{
    PetscErrorCode          ierr;
    PetscViewerASCIIMonitor viewer;
    PetscReal               R_normInf;
    Vec                     R, work, w1,w2;

    PetscFunctionBegin;

    ierr = VecDuplicate(ksp->vec_rhs,&work);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w1);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w2);CHKERRQ(ierr);


    KSPBuildResidual( ksp, w1,w2, &R );
    VecNorm( R, NORM_INFINITY, &R_normInf );

    ierr = PetscViewerASCIIMonitorCreate(((PetscObject)ksp)->comm,"stdout",0,&viewer);
    CHKERRQ(ierr);

    ierr = PetscViewerASCIIMonitorPrintf(viewer,"%3D KSP Residual Ninf %14.12e (%s) \n",
                                         n, R_normInf,
                                         ((PetscObject)ksp)->prefix ? ((PetscObject)ksp)->prefix: "");
    CHKERRQ(ierr);

    ierr = PetscViewerASCIIMonitorDestroy(viewer);
    CHKERRQ(ierr);

    Stg_VecDestroy(&work);
    Stg_VecDestroy(&w1);
    Stg_VecDestroy(&w2);

    PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPNorm2RawMonitor"
PetscErrorCode BSSCR_KSPNorm2RawMonitor(KSP ksp,PetscInt n,PetscReal rnorm, void *dummy)
{
    PetscErrorCode          ierr;
    PetscViewerASCIIMonitor viewer;
    PetscReal               R_norm2;
    Vec                     R, work, w1,w2;

    PetscFunctionBegin;

    ierr = VecDuplicate(ksp->vec_rhs,&work);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w1);CHKERRQ(ierr);
    ierr = VecDuplicate(ksp->vec_rhs,&w2);CHKERRQ(ierr);


    KSPBuildResidual( ksp, w1,w2, &R );
    VecNorm( R, NORM_2, &R_norm2 );

    ierr = PetscViewerASCIIMonitorCreate(((PetscObject)ksp)->comm,"stdout",0,&viewer);
    CHKERRQ(ierr);

    ierr = PetscViewerASCIIMonitorPrintf(viewer,"%3D KSP Residual Norm2raw %14.12e (%s) \n",
                                         n, R_norm2,
                                         ((PetscObject)ksp)->prefix ? ((PetscObject)ksp)->prefix: "");
    CHKERRQ(ierr);

    ierr = PetscViewerASCIIMonitorDestroy(viewer);
    CHKERRQ(ierr);

    Stg_VecDestroy(&work);
    Stg_VecDestroy(&w1);
    Stg_VecDestroy(&w2);

    PetscFunctionReturn(0);
}




#undef __FUNCT__
#define __FUNCT__ "BSSCR_KSPSetNormInfConvergenceTest"
PetscErrorCode BSSCR_KSPSetNormInfConvergenceTest(KSP ksp)
{
    KSPPWConvergedCtx *ctx;
    PetscTruth monitor, flg;
    char *opt;
    const char *prefix;

    BSSCR_KSPPWConvergedCreate( (void**)&ctx );
    KSPGetOptionsPrefix( ksp, &prefix );

#if(PETSC_VERSION_MAJOR == 3)
    KSPSetConvergenceTest( ksp, BSSCR_KSPNormInfConverged, ctx, BSSCR_KSPPWConvergedDestroy );     // 3.0.0 only
#else
    KSPSetConvergenceTest( ksp, BSSCR_KSPNormInfConverged, ctx );                            // 2.3.3
#endif

    PetscFunctionReturn(0);
}
