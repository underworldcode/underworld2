/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*
This provides a couple of routines which allowed the time per iteration
to be stored. This is achieved useing the KSPMonitors. The issue with using
these monitors is the petsc support for these is terrible. We need to be
able to access our user monitor context, but petsc only lets you get the
context from the FIRST monitor, which we cannot guarentee will be the one 
defined to store the timing information.

To get around this we store the index into the monitor array which our 
monitor gets inersted into. This works, but we have no 'nice' way to 
recover the user monitor context except via the monitor_index. Thus when
the monitor is set, we return the value of the monitor index. This number
is required whenever you wish to extract the timing information.



*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE needed for asprintf */
#include <stdio.h>
#include <stdlib.h>

#include <petsc.h>
#include <petscvec.h>
#include <petscmat.h>
#include <petscksp.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "common-driver-utils.h"

#include <petscversion.h>
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >=3) )
  #if (PETSC_VERSION_MINOR >=6)
     #include "petsc/private/kspimpl.h"
  #else
     #include "petsc-private/kspimpl.h"
  #endif
#else
  #include "private/kspimpl.h"
#endif

struct timed_cvg_ctx {
	PetscInt monitor_index;
	PetscInt hist_len, hist_len_max;
	PetscLogDouble *time_log;
	PetscLogDouble time;
	PetscReal *r_log;
};


/* ~~~~~~~~~~~~~~~~ SolverMonitor routines ~~~~~~~~~~~~~~~~~~~~~~ */
/*
I shouldn't need to include the residual log, but the petsc one does strange things, when used
with gmres and fgmres. I know it's related to restarts but I just don't get it. This one will 
produce the same data as that which is displayed on the screen when you used -ksp_monitor.
*/

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_check_validity_of_timing_monitor"
PetscErrorCode BSSCR_check_validity_of_timing_monitor( KSP ksp, PetscInt monitor_index )
{
	
	if( monitor_index < 0 ) {
		Stg_SETERRQ1( PETSC_ERR_ARG_WRONG, "Monitor index cannot be negative. You had %D", monitor_index );
	}
	if( monitor_index >= ksp->numbermonitors ) {
		Stg_SETERRQ2( PETSC_ERR_ARG_WRONG, "Monitor index >= number of monitors set (%D). You had %D", ksp->numbermonitors, monitor_index );
	}
	
	PetscFunctionReturn(0);	
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogHistory"
PetscErrorCode BSSCR_KSPLogHistory(struct timed_cvg_ctx *ctx,PetscReal norm) 
{
	PetscLogDouble t, time;
	
	PetscGetTime(&t);
	time = t - ctx->time;
	if (ctx->hist_len==0) {
		/* Get reference time and zero first result */
		PetscGetTime(&ctx->time);
		time = 0.0;
	}
	
	if (ctx->r_log && ctx->time_log && ctx->hist_len_max > ctx->hist_len) {
		ctx->r_log[ctx->hist_len] = norm;
		ctx->time_log[ctx->hist_len] = time;
		
		ctx->hist_len++;
	}
	
	PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "BSSCR_timed_cvg_test"
PetscErrorCode BSSCR_timed_cvg_test(KSP ksp, int it, PetscReal rnorm, void *mctx)
{
	struct timed_cvg_ctx *ctx = (struct timed_cvg_ctx*)(mctx);
	
	BSSCR_KSPLogHistory(ctx,rnorm);
	
	PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogDestroyMonitor"
PetscErrorCode BSSCR_KSPLogDestroyMonitor(void *_ctx)
{
	struct timed_cvg_ctx *ctx = (struct timed_cvg_ctx*)_ctx;
	
	PetscFree( ctx->r_log );
	PetscFree( ctx->time_log );
	PetscFree( ctx );
	
	PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogSetMonitor"
PetscErrorCode BSSCR_KSPLogSetMonitor(KSP ksp,PetscInt len,PetscInt *monitor_index)
{
	PetscLogDouble *time_log;
	PetscReal *r_log;
	struct timed_cvg_ctx *ctx;
	
	
	PetscMalloc( sizeof(PetscLogDouble)*len, &time_log );
	PetscMalloc( sizeof(PetscReal)*len, &r_log );
	PetscMalloc( sizeof(struct timed_cvg_ctx), &ctx );
	
	ctx->hist_len_max  = len;
	ctx->hist_len      = 0;
	ctx->time_log      = time_log;
	ctx->r_log         = r_log;
	
	ctx->monitor_index = ksp->numbermonitors;
	*monitor_index = ksp->numbermonitors;
	
	//KSPSetConvergenceTest(ksp,timed_cvg_test,(void*)ctx );
	KSPMonitorSet( ksp, BSSCR_timed_cvg_test, (void*)ctx, BSSCR_KSPLogDestroyMonitor );
	
	PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogGetTimeHistory"
PetscErrorCode BSSCR_KSPLogGetTimeHistory(KSP ksp,PetscInt monitor_index, PetscInt *na,PetscLogDouble **log)
{
	struct timed_cvg_ctx *ctx;
	PetscInt its;
	
	
	ctx = (struct timed_cvg_ctx*)ksp->monitorcontext[ monitor_index ];
	
	KSPGetIterationNumber(ksp,&its);
	its = its + 1;
	*na = PetscMin( (PetscInt)(its), (PetscInt)(ctx->hist_len_max) );
	*log = ctx->time_log;
	
	PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogGetResidualHistory"
PetscErrorCode BSSCR_KSPLogGetResidualHistory(KSP ksp,PetscInt monitor_index, PetscInt *na,PetscReal **rlog)
{
	struct timed_cvg_ctx *ctx;
	PetscInt its;
	
	
	ctx = (struct timed_cvg_ctx*)ksp->monitorcontext[ monitor_index ];
	KSPGetIterationNumber(ksp,&its);
	its = its + 1;
	*na = PetscMin( (PetscInt)(its), (PetscInt)(ctx->hist_len_max) );
	*rlog = ctx->r_log;
	
	PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogGetResidualTimeHistory"
PetscErrorCode BSSCR_KSPLogGetResidualTimeHistory(KSP ksp,PetscInt monitor_index, PetscInt *na,PetscReal **rlog,PetscLogDouble **tlog)
{
	struct timed_cvg_ctx *ctx;
	PetscInt its;
	
	KSPGetIterationNumber(ksp,&its);
	ctx = (struct timed_cvg_ctx*)ksp->monitorcontext[ monitor_index ];
	its = its + 1;
	*na = PetscMin( (PetscInt)(its), (PetscInt)(ctx->hist_len_max) );
	*rlog = ctx->r_log;
	*tlog = ctx->time_log;
	
	PetscFunctionReturn(0);
}


KSPNormType  BS_NO_PC=  KSP_NORM_UNPRECONDITIONED;
KSPNormType  BS_W_PC=   KSP_NORM_PRECONDITIONED;

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPGetNormType"
PetscErrorCode BSSCR_KSPGetNormType(KSP ksp, KSPNormType *normtype) {
	*normtype = ksp->normtype;
	return(0);
}

#undef __FUNCT__  
#define __FUNCT__ "BSSCR_KSPLogSolve"
PetscErrorCode BSSCR_KSPLogSolve(PetscViewer v,PetscInt monitor_index, KSP ksp)
{
	Vec b,u;
	PetscInt i;
	KSPType type;
	PetscReal b_nrm, *rlog;
	PC pc;
	PetscLogDouble *log;
	PetscInt nits;
	KSPNormType ntype;
	PCSide side;
	PetscTruth has_unpc_config, ksp_matches;
	const char *list[]            = { "cg"   , "bicg" , "bcgs" , "gmres" , "fgmres", "minres", "lgmres", 0 }; 
	const PCSide   pc_side[]      = { PC_LEFT, PC_LEFT, PC_LEFT, PC_RIGHT, PC_RIGHT, PC_LEFT , PC_RIGHT, 0 };
	const KSPNormType norm_type[] = { BS_NO_PC  , BS_NO_PC  , BS_NO_PC  , BS_W_PC    , BS_NO_PC   , BS_NO_PC   , BS_W_PC    , 0 };
	
	
	KSPGetRhs( ksp, &b );
	KSPGetSolution( ksp, &u );
	
	KSPGetType( ksp, &type );
	BSSCR_KSPGetNormType( ksp, &ntype );
	KSPGetPreconditionerSide( ksp, &side );
	has_unpc_config = PETSC_FALSE;
	
	i = 0;
	while (list[i]) {
		PetscStrcmp( "fgmres", type, &ksp_matches );		
		if (ksp_matches) {
			has_unpc_config = PETSC_TRUE;
			break;
		}

		PetscStrcmp( list[i],type,&ksp_matches );
		if (ksp_matches) {
			if( pc_side[i] == side && ntype == norm_type[i] ) {
				has_unpc_config = PETSC_TRUE;
				break; 
			}
		}
		i++;
	}
	
	if (has_unpc_config) {
		PetscViewerASCIIPrintf( v, "[%s] Detected options consistent with unpreconditioned norm: using side=%d, norm_type=%d \n", type, side, ntype );
	}
	
	
	BSSCR_KSPLogGetResidualTimeHistory(ksp,monitor_index, &nits,&rlog,&log);
	if (!rlog || !log) {
		Stg_SETERRQ( PETSC_ERR_ORDER, "You need to call BSSCR_KSPLogSetMonitor() before BSSCR_KSPLogGetResidualTimeHistory()" );
	}
	
	if (!has_unpc_config) {
		Vec pc_b;
		VecDuplicate( u, &pc_b );
		KSPGetPC( ksp, &pc );
		PCApply( pc, b, pc_b );
		VecNorm( pc_b, NORM_2, &b_nrm );
		
		Stg_VecDestroy(&pc_b );
		
		PetscViewerASCIIPrintf( v, "# it        |Qr|               |Qr0|              |f|             |Qr|/|Qr0|         log10(Qr/Qr0)       |Qr|/|Qf|          log10(Qr/Qf)           time\n" );
	}
	else if (has_unpc_config) {
		VecNorm( b, NORM_2, &b_nrm );
		PetscViewerASCIIPrintf( v, "# it         |r|               |r0|               |f|              |r|/|r0|          log10(r/r0)          |r|/|f|           log10(r/f)            time\n" );
	}
	
	for( i=0; i<nits; i++ ) {
		PetscViewerASCIIPrintf( v, "%1.4d %14.12e %14.12e %14.12e %14.12e %14.12e %14.12e %14.12e %14.12e\n", 
				i, rlog[i],rlog[0],b_nrm, rlog[i]/rlog[0],log10(rlog[i]/rlog[0]), rlog[i]/b_nrm,log10(rlog[i]/b_nrm), log[i] );
	}
	
	
	PetscFunctionReturn(0);
}



#undef __FUNCT__  
#define __FUNCT__ "BSSCR_BSSCR_KSPLogSolveSummary"
PetscErrorCode BSSCR_BSSCR_KSPLogSolveSummary(PetscViewer v,PetscInt monitor_index, KSP ksp)
{
	KSPConvergedReason reason;
	char *reas;
	PetscReal *rlog;
	PetscLogDouble *log;
	PetscInt nits;
	
	
	KSPGetConvergedReason( ksp, &reason );
	if( reason == KSP_CONVERGED_RTOL )
		asprintf(&reas, "KSP_CONVERGED_RTOL" );
	else if( reason == KSP_CONVERGED_ATOL )
		asprintf(&reas, "KSP_CONVERGED_ATOL" );
	else if( reason == KSP_CONVERGED_ITS )
		asprintf(&reas, "KSP_CONVERGED_ITS" );
	else if( reason == KSP_DIVERGED_ITS )
		asprintf(&reas, "KSP_DIVERGED_ITS" );
	else if( reason == KSP_DIVERGED_DTOL )
		asprintf(&reas, "KSP_DIVERGED_DTOL" );
	else if( reason == KSP_DIVERGED_INDEFINITE_PC )
		asprintf(&reas, "KSP_DIVERGED_INDEFINITE_PC" );
	else {
		Stg_SETERRQ( PETSC_ERR_SUP, "unknown convergence reason detected");
	}
	
	BSSCR_KSPLogGetResidualTimeHistory(ksp,monitor_index, &nits,&rlog,&log);
	if (!rlog || !log) {
		Stg_SETERRQ( PETSC_ERR_ORDER, "You need to call BSSCR_KSPLogSetMonitor() before BSSCR_KSPLogGetResidualTimeHistory()" );
	}
	PetscViewerASCIIPrintf( v, "\n");
	PetscViewerASCIIPrintf( v, "# ===========================================================\n");
	PetscViewerASCIIPrintf( v, "#  KSP summary \n" );
	PetscViewerASCIIPrintf( v, "#  time (its) [rnorm] reason \n" );
	PetscViewerASCIIPrintf( v, "#  %.2f (%d) %.2e %s \n", log[nits-1], nits-1, rlog[nits-1], reas );
	PetscViewerASCIIPrintf( v, "# ===========================================================\n");
	
	PetscFunctionReturn(0);
}
