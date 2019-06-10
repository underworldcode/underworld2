/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Solvers_AugLagStokes_SLE_h__
#define __Solvers_AugLagStokes_SLE_h__

	/** Textual name of this class */
	extern const Type AugLagStokes_SLE_Type;

	/** AugLagStokes_SLE class contents */
	#define __AugLagStokes_SLE \
		/* General info */ \
		__Stokes_SLE \
		/* AugLagStokes_SLE info */ \
		\
		StiffnessMatrix*    k2StiffMat;	/** 2nd Stress tensor matrix */ \
                ForceVector*	    f2ForceVec; /** 2nd Force vector for K2  */		\
		StiffnessMatrix*    mStiffMat;	/** Mass matrix */ \
                ForceVector*	    jForceVec; /**  Junk Force vector for Mass Matrix  */		\
		StiffnessMatrix*    vmStiffMat;	/** Velocity Mass matrix */	\
                ForceVector*	    vmForceVec; /** Force vector for Velocity Mass Matrix  */		\
                double              penaltyNumber; \
                double              hFactor;

	struct AugLagStokes_SLE { __AugLagStokes_SLE };	

	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define AUGLAGSTOKES_SLE_DEFARGS \
            STOKES_SLE_DEFARGS

	#define AUGLAGSTOKES_SLE_PASSARGS \
            STOKES_SLE_PASSARGS

	AugLagStokes_SLE* _AugLagStokes_SLE_New(  AUGLAGSTOKES_SLE_DEFARGS  );

	void _AugLagStokes_SLE_Init( 		
		void*			sle, 
		StiffnessMatrix*	k2StiffMat, 
		StiffnessMatrix*        mStiffMat,
		ForceVector*	        f2ForceVec,
		ForceVector*	        jForceVec,
		double                  penaltyNumber,
		double                  hFactor,
		StiffnessMatrix*        vmStiffMat,
                ForceVector*	        vmForceVec
		) ;

        /* void _AugLagStokes_SLE_Delete( void* stokesSleSolver ); */

	void _AugLagStokes_SLE_Print( void* stokesSleSolver, Stream* stream );

	void* _AugLagStokes_SLE_DefaultNew( Name name );

	void _AugLagStokes_SLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data );
	
	void _AugLagStokes_SLE_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms );
	
#endif

