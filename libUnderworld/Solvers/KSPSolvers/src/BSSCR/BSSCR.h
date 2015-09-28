/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __BSSCRKSP_h__
#define __BSSCRKSP_h__

#include "common-driver-utils.h" /* has #include "stokes_block_scaling.h" */
#include "petsccompat.h"
//#include "StokesBlockKSPInterface.h"
/* added K2_NULL type so that the remaining types enumerate to > 0 so can test with PETSC_NULL or K2_NULL (both == 0) */
typedef enum {K2_NULL, K2_DGMGD, K2_GMG, K2_GG, K2_SLE} K2Type;
typedef enum {DEFAULT, KONLY} Stype;
//static const char *K2Types[] = {"NULL","DGMGD","GMG","GG","K2Type","K2_",0}; /* atm on line 281 just above KSPSetFromOptions_BSSCR in BSSCR.c */

#define __KSP_BSSCR \
        __KSP_COMMON /* defined in StokesBlockKSPInterface.h has (st_sle, mg, DIsSym, preconditioner) */ \
	Mat K2,M,S,approxS; \
	Vec S1,S2,f2; /* S1 usually is sqrt(1/diag(K)) */	\
	Vec t, v; /* null space vectors for pressure */ \
	double nstol; \
	int min_it; /* minimum iterations for KSP cf max_it */ \
	K2Type k2type; \
	MatStokesBlockScaling BA; /* sticking with this instead of using S1 and S2 for the moment */ \
	PetscErrorCode (*scale)(KSP); \
	PetscErrorCode (*unscale)(KSP); \
	PetscErrorCode (*buildK2)(KSP); \
	PetscErrorCode (*buildPNS)(KSP); \
	/* check_cb_pressureNS:<- check for pressure checker-board nullspace; check_const_pressureNS:<- check for constant pressure nullspace */ \
	PetscTruth scaled, do_scaling, K2built, check_cb_pressureNS, check_const_pressureNS, check_pressureNS; \
        Stype scaletype; \
        PetscReal snesabstol;

//typedef StokesBlockKSPInterface KSP_BSSCR;
struct KSP_BSSCR { __KSP_BSSCR };
typedef struct KSP_BSSCR KSP_BSSCR;

PetscErrorCode KSPRegisterBSSCR(const char path[]);
//PETSCKSP_DLLEXPORT 

PetscErrorCode BSSCR_BSSCR_StokesCreatePCSchur2(
   Mat K, Mat G, Mat D, Mat C, Mat Smat, PC pc_S,
   PetscTruth sym, KSP_BSSCR * bsscrp );

PetscErrorCode BSSCR_KSPConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *cctx);
PetscErrorCode BSSCR_KSPSetConvergenceMinIts(KSP ksp, PetscInt n, KSP_BSSCR * bsscr);
PetscErrorCode BSSCR_KSPConverged_Destroy(void *cctx);

//extern PetscErrorCode BSSCR_DRIVER_flex( Mat stokes_A, Vec stokes_x, Vec stokes_b, Mat approxS, KSP ksp_K, MatStokesBlockScaling BA, PetscTruth sym, KSP_BSSCR * bsscr );
//extern PetscErrorCode BSSCR_DRIVER_auglag( Mat stokes_A, Vec stokes_x, Vec stokes_b, Mat approxS, KSP ksp_K, MatStokesBlockScaling BA, PetscTruth sym, KSP_BSSCR * bsscr );

#endif

