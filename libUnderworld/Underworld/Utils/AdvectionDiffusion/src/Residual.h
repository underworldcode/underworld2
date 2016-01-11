/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_AdvectionDiffusion_Residual_h__
#define __Underworld_AdvectionDiffusion_Residual_h__

	typedef double (AdvDiffResidualForceTerm_UpwindParamFunction)( void* residual, double pecletNumber );
	
	typedef double (AdvDiffResidualForceTerm_GetDiffusivityFromIntPoint)( void* self, void* lParticle_I );

	/** Textual name of this class */
	extern const Type AdvDiffResidualForceTerm_Type;

	/** AdvDiffResidualForceTerm class contents */
	#define __AdvDiffResidualForceTerm \
		/* General info */ \
		__ForceTerm \
		\
		/* Virtual info */ \
		AdvDiffResidualForceTerm_UpwindParamFunction*			_upwindParam; \
		AdvDiffResidualForceTerm_GetDiffusivityFromIntPoint*	_getDiffusivityFromIntPoint; \
		double*																phiGrad; \
		double**																GNx; \
		double*																Ni; \
		double*																SUPGNi; \
		IArray																*incarray; \
		\
		/* AdvDiffResidualForceTerm info */ \
		FeVariable*															velocityField; \
		double																defaultDiffusivity; \
		Variable*															diffusivityVariable; \
		AdvDiffResidualForceTerm_UpwindParamFuncType				upwindParamType; \
      int                                                   last_maxNodeCount; /* behaves like a static variable to record max node count per element */ \

	struct AdvDiffResidualForceTerm { __AdvDiffResidualForceTerm };	

	AdvDiffResidualForceTerm* AdvDiffResidualForceTerm_New( 
		Name															name,
		FiniteElementContext*									context,
		ForceVector*												forceVector,
		Swarm*														integrationSwarm,
		Stg_Component*												sle, 
		FeVariable*													velocityField,
		Variable*													diffusivityVariable,
		double														defaultDiffusivity,
		AdvDiffResidualForceTerm_UpwindParamFuncType		upwindFuncType );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ADVDIFFRESIDUALFORCETERM_DEFARGS \
                FORCETERM_DEFARGS, \
                AdvDiffResidualForceTerm_UpwindParamFunction*  _upwindParam

	#define ADVDIFFRESIDUALFORCETERM_PASSARGS \
                FORCETERM_PASSARGS, \
	        _upwindParam

	AdvDiffResidualForceTerm* _AdvDiffResidualForceTerm_New(  ADVDIFFRESIDUALFORCETERM_DEFARGS  );

	void _AdvDiffResidualForceTerm_Init(
   	void*                                        residual,
   	FeVariable*                                  velocityField,
   	Variable*                                    diffusivityVariable,
   	double                                       defaultDiffusivity,
   	AdvDiffResidualForceTerm_UpwindParamFuncType upwindFuncType );
	
	void _AdvDiffResidualForceTerm_Delete( void* residual );

	void _AdvDiffResidualForceTerm_Print( void* residual, Stream* stream );

	void* _AdvDiffResidualForceTerm_DefaultNew( Name name );

	void _AdvDiffResidualForceTerm_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

	void _AdvDiffResidualForceTerm_Build( void* residual, void* data );

	void _AdvDiffResidualForceTerm_Initialise( void* residual, void* data );

	void _AdvDiffResidualForceTerm_Execute( void* residual, void* data );

	void _AdvDiffResidualForceTerm_Destroy( void* residual, void* data );

	void _AdvDiffResidualForceTerm_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elementResidual );

	/** Virtual Function Implementations */
	#define AdvDiffResidualForceTerm_UpwindParam( residual, pecletNumber ) \
		( ((AdvDiffResidualForceTerm*) residual)->_upwindParam( residual, pecletNumber ) )

	double _AdvDiffResidualForceTerm_UpwindParam( void* residual, double pecletNumber ) ;

#endif

