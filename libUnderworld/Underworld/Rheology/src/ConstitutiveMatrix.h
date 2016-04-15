/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Rheology_ConstitutiveMatrix_h__
#define __Underworld_Rheology_ConstitutiveMatrix_h__


	/* typedefs for virtual functions: */
	typedef double  (ConstitutiveMatrix_GetValueFunc)            ( void* constitutiveMatrix );
	typedef void    (ConstitutiveMatrix_SetValueFunc)            ( void* constitutiveMatrix, double value );
	typedef void    (ConstitutiveMatrix_SetSecondViscosityFunc)  ( void* constitutiveMatrix, double value, const XYZ vector );
	typedef void    (ConstitutiveMatrix_Assemble_D_B_Func)       ( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );
	typedef void    (ConstitutiveMatrix_CalculateStressFunc)     ( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress );
	
	extern const Type ConstitutiveMatrix_Type;
	
	/* ConstitutiveMatrix information */
	#define __ConstitutiveMatrix  \
		/* Parent info */ \
		__StiffnessMatrixTerm \
		\
		/* Virtual functions go here */                                                            \
		ConstitutiveMatrix_SetValueFunc*             _setValue;                         \
		ConstitutiveMatrix_GetValueFunc*             _getViscosity;                     \
		ConstitutiveMatrix_SetValueFunc*             _isotropicCorrection;              \
		ConstitutiveMatrix_SetSecondViscosityFunc*   _setSecondViscosity;               \
		ConstitutiveMatrix_Assemble_D_B_Func*        _assemble_D_B;                     \
		ConstitutiveMatrix_CalculateStressFunc*      _calculateStress;                  \
		\
		/* ConstitutiveMatrix info */                                                   \
		double**                                     matrixData;                        \
    double                                       derivs[9];                         \
		Dimension_Index                              dim;                               \
		Bool                                         isSwarmTypeIntegrationPointsSwarm; \
		Bool                                         isDiagonal;                        \
		Index                                        columnSize;                        \
		Index                                        rowSize;                           \
		Bool                                         previousSolutionExists;            \
		Bool                                         restartUsingPreviousStokesSol;     \
		int                                          currentParticleIndex;              \
		SystemLinearEquations*                       sle;                               \
		Iteration_Index                              sleNonLinearIteration_I;           \
    /* below is needed to store the constitutiveMatrix per particle */ \
		Bool                                         storeConstitutiveMatrix;           \
		Index                                        storedConstHandle;                 \
		SwarmVariable*                               storedConstSwarmVar;               
    
	struct ConstitutiveMatrix { __ConstitutiveMatrix };

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONSTITUTIVEMATRIX_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS, \
                ConstitutiveMatrix_SetValueFunc*                       _setValue, \
                ConstitutiveMatrix_GetValueFunc*                   _getViscosity, \
                ConstitutiveMatrix_SetValueFunc*            _isotropicCorrection, \
                ConstitutiveMatrix_SetSecondViscosityFunc*   _setSecondViscosity, \
                ConstitutiveMatrix_Assemble_D_B_Func*              _assemble_D_B, \
                ConstitutiveMatrix_CalculateStressFunc*         _calculateStress

	#define CONSTITUTIVEMATRIX_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS, \
	        _setValue,            \
	        _getViscosity,        \
	        _isotropicCorrection, \
	        _setSecondViscosity,  \
	        _assemble_D_B,        \
	        _calculateStress    

	ConstitutiveMatrix* _ConstitutiveMatrix_New(  CONSTITUTIVEMATRIX_DEFARGS  );

	/* 'Stg_Class' Virtual Functions */
	void _ConstitutiveMatrix_Delete( void* constitutiveMatrix );
	void _ConstitutiveMatrix_Print( void* constitutiveMatrix, Stream* stream );
	#define ConstitutiveMatrix_Copy( self ) \
		(ConstitutiveMatrix*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ConstitutiveMatrix_DeepCopy( self ) \
		(ConstitutiveMatrix*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _ConstitutiveMatrix_Copy( void* constitutiveMatrix, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void _ConstitutiveMatrix_AssignFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data );
	void _ConstitutiveMatrix_Build( void* constitutiveMatrix, void* data );
	void _ConstitutiveMatrix_Initialise( void* constitutiveMatrix, void* data );
	void _ConstitutiveMatrix_Execute( void* constitutiveMatrix, void* data );
	void _ConstitutiveMatrix_Destroy( void* constitutiveMatrix, void* data );
   void _ConstitutiveMatrix_Init(
         ConstitutiveMatrix*                    self,
         Dimension_Index                        dim,
         Bool                                   storeConstitutiveMatrix,
         Bool                                   restartUsingPreviousStokesSol );

	/* Wrapper macros to virtual functions - These must be macros for the sake of speed */
	#define ConstitutiveMatrix_SetValueInAllEntries( constitutiveMatrix, value ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_setValue( constitutiveMatrix, value ))

	#define ConstitutiveMatrix_SetSecondViscosity( constitutiveMatrix, deltaViscosity, director ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_setSecondViscosity( constitutiveMatrix, deltaViscosity, director ))

	#define ConstitutiveMatrix_GetIsotropicViscosity( constitutiveMatrix ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_getViscosity( constitutiveMatrix ))

	#define ConstitutiveMatrix_IsotropicCorrection( constitutiveMatrix, isotropicCorrection ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_isotropicCorrection( constitutiveMatrix, isotropicCorrection ))

	#define ConstitutiveMatrix_Assemble_D_B( constitutiveMatrix, GNx, node_I, D_B ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_assemble_D_B( constitutiveMatrix, GNx, node_I, D_B ))

	#define ConstitutiveMatrix_CalculateStress( constitutiveMatrix, strainRate, stress ) \
		(((ConstitutiveMatrix*) constitutiveMatrix)->_calculateStress( constitutiveMatrix, strainRate, stress ) )

	/* +++ Public Functions +++ */
	void ConstitutiveMatrix_MultiplyByValue( void* constitutiveMatrix, double factor ) ;
	void ConstitutiveMatrix_PrintContents( void* constitutiveMatrix, Stream* stream ) ;
	
	void ConstitutiveMatrix_ZeroMatrix( void* constitutiveMatrix ) ;
	void ConstitutiveMatrix_SetIsotropicViscosity( void* constitutiveMatrix, double viscosity );

	void ConstitutiveMatrix_Assemble( 
		void*                                              constitutiveMatrix,
		Element_LocalIndex                                 lElement_I,
		int                                                particleIndex,
		IntegrationPoint*                                  particle );

	#define ConstitutiveMatrix_SetToNonLinear( constitutiveMatrix ) \
		StiffnessMatrix_SetToNonLinear( constitutiveMatrix->stiffnessMatrix )

	#define ConstitutiveMatrix_GetMesh( constitutiveMatrix ) \
		( (constitutiveMatrix)->stiffnessMatrix->rowVariable->feMesh )

#endif /* __Underworld_Rheology_ConstitutiveMatrix_h__ */

