/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __StokesBlockKSPInterface_h__
#define __StokesBlockKSPInterface_h__

	/** Textual name of this class */
	extern const Type StokesBlockKSPInterface_Type;

        #define __STATS \
                int pressure_its; /** Need vanilla c-types so we can talk to Python **/ \
                int velocity_backsolve_its; \
                int velocity_presolve_its; \
                int velocity_pressuresolve_its; \
                int velocity_total_its; \
                double pressure_time; \
								double velocity_presolve_setup_time; \
								double velocity_pressuresolve_setup_time; \
								double velocity_backsolve_setup_time; \
                double velocity_backsolve_time; \
                double velocity_presolve_time; \
                double velocity_pressuresolve_time; \
                double velocity_total_time; \
                double total_time;  \
                double total_flops; \
                double pressure_flops; \
                double velocity_backsolve_flops;\
                double velocity_presolve_flops;\
                double vmin, vmax;                   \
                double pmin, pmax;                   \
                double p_sum;                        \

        struct STATS { __STATS };
        typedef struct STATS STATS;

        /** if too much stuff ends up on this struct then possibly just
        extend the BSSCR struct instead at some point */
        #define __StokesBlockKSPInterface  \
                /* General info */ \
                __SLE_Solver	   \
		/* Virtual info */ \
		StiffnessMatrix* preconditioner; \
		StiffnessMatrix*    k2StiffMat;	/** 2nd Stress tensor matrix */ \
        ForceVector*	    f2ForceVec; /** 2nd Force vector for K2  */ \
		StiffnessMatrix*    mStiffMat;	/** Mass matrix */ \
        ForceVector*	    jForceVec; /**  Junk Force vector for Mass Matrix  */ \
		StiffnessMatrix*    vmStiffMat;	/** Velocity Mass matrix */	\
        ForceVector*	    vmForceVec; /** Force vector for Velocity Mass Matrix  */ \
        double              penaltyNumber;                              \
        double              hFactor; \
		/* StokesBlockKSPInterface info */ \
		Stokes_SLE    *    st_sle;  \
        PETScMGSolver *    mg;      \
        PetscTruth         mg_active; \
        STATS              stats;     \
        int DIsSym;                                              \
        /* approxS often same as preconditioner above */         \
		Mat K2, M, approxS, S;                                   \
        /* S1 = 1/sqrt(diag(K)); S2 = scaling for pressure */    \
		Vec S1,S2;                                               \
        /* file and string for petsc options */                  \
        Name optionsFile;                                        \
        char * optionsString;  \
        int fhat_reason, backsolve_reason, outer_reason; \

	struct StokesBlockKSPInterface { __StokesBlockKSPInterface };

        #define __KSP_COMMON \
                Stokes_SLE     *   st_sle; \
                PETScMGSolver  *   mg;	   \
                PetscTruth         DIsSym; \
                StiffnessMatrix*   preconditioner; \
                StokesBlockKSPInterface* solver;


        struct KSP_COMMON { __KSP_COMMON };
        typedef struct KSP_COMMON KSP_COMMON;


	/* --- Constructors / Destructor --- */

	/** Constructor */
	void* _StokesBlockKSPInterface_DefaultNew( Name name );

	/** Creation implementation / Virtual constructor */

	#define STOKESBLOCKKSPINTERFACE_DEFARGS \
                SLE_SOLVER_DEFARGS

	#define STOKESBLOCKKSPINTERFACE_PASSARGS \
                SLE_SOLVER_PASSARGS

	StokesBlockKSPInterface* _StokesBlockKSPInterface_New(  STOKESBLOCKKSPINTERFACE_DEFARGS  );

	/** Class member variable initialisation */
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
		ForceVector*	  vmForceVec  );


	/** Stg_Component_Build() implementations: allocates the 2 MatrixSolvers and additional Vectors */
	void _StokesBlockKSPInterface_Build( void* solver, void* stokesSLE );

	void _StokesBlockKSPInterface_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data );

	void _StokesBlockKSPInterface_Initialise( void* solver, void* stokesSLE ) ;

        /* void _StokesBlockKSPInterface_Destroy( void* solver, void* data ); */

	void _StokesBlockKSPInterface_SolverSetup( void* stokesSle, void* stokesSLE );
    void _StokesBlockKSPInterface_Solve( void* solver, void* stokesSLE );

    void SBKSP_SetSolver( void* solver, void* stokesSLE );
    void SBKSP_SetPenalty( void* solver, double penalty );
    int  SBKSP_GetPressureIts(void *solver);
    //void SBKSP_SetMGActive( void* solver, PetscTruth flag );

    void SBKSP_GetStokesOperators(
		Stokes_SLE *stokesSLE,
		Mat *K,Mat *G,Mat *D,Mat *C,Mat *approxS,
		Vec *f,Vec *h,Vec *u,Vec *p );

#endif
