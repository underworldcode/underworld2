/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_Weights_WeightsCalculator_h__
#define __PICellerator_Weights_WeightsCalculator_h__

typedef void (WeightsCalculator_CalculateFunction)( void* self, void* swarm, Cell_LocalIndex lCell_I );

/* Textual name of this class */
extern const Type WeightsCalculator_Type;

/* WeightsCalculator information */
#define __WeightsCalculator \
	/* General info */ \
	__Stg_Component \
	\
	/* Virtual Info */ \
	FiniteElementContext*						context; \
	WeightsCalculator_CalculateFunction*  _calculate; \
	/* Other Info */ 

	struct WeightsCalculator { __WeightsCalculator };
        
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define WEIGHTSCALCULATOR_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                WeightsCalculator_CalculateFunction*  _calculate

	#define WEIGHTSCALCULATOR_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _calculate

	WeightsCalculator* _WeightsCalculator_New(  WEIGHTSCALCULATOR_DEFARGS  );

	/* Stg_Class_Delete WeightsCalculator implementation */
	void _WeightsCalculator_Delete( void* self );
	void _WeightsCalculator_Print( void* self, Stream* stream );
	#define WeightsCalculator_Copy( self )                                  \
	    (WeightsCalculator*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define WeightsCalculator_DeepCopy( self )                              \
	    (WeightsCalculator*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _WeightsCalculator_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
        
	void _WeightsCalculator_AssignFromXML( void* self, Stg_ComponentFactory* cf, void* data ) ;
	void _WeightsCalculator_Build( void* self, void* data ) ;
	void _WeightsCalculator_Destroy( void* self, void* data ) ;
	void _WeightsCalculator_Initialise( void* self, void* data ) ;
	void _WeightsCalculator_Execute( void* self, void* data );
        
	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	void WeightsCalculator_CalculateCell( void* self, void* swarm, Cell_LocalIndex lCell_I ) ;
        
	void WeightsCalculator_CalculateAll( void* self, void* _swarm ) ;
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	#define WeightsCalculator_ZeroWeights( self, swarm )                    \
	    WeightsCalculator_SetWeightsValueAll( (self), (swarm), 0.0 )
	#define WeightsCalculator_ZeroWeightsInCell( self, swarm, lCell_I )     \
	    WeightsCalculator_SetWeightsValueAllInCell( (self), (swarm), (lCell_I), 0.0 )
        
	void WeightsCalculator_SetWeightsValueAll( void* self, void* _swarm, double weight ) ;
	void WeightsCalculator_SetWeightsValueAllInCell( void* self, void* _swarm, Cell_LocalIndex lCell_I, double weight ) ;
	Constraint_Index WeightsCalculator_FindConstraintOrder( void* self, void* _swarm, Dimension_Index dim, Stream* stream ) ;
	double WeightsCalculator_TestConstraint( void* self, void* _swarm, Dimension_Index dim, Constraint_Index order ) ;
	double WeightsCalculator_TestConstraintOverCell( void* self, void* _swarm, Cell_LocalIndex lCell_I, Dimension_Index dim, Constraint_Index order ) ;
	double WeightsCalculator_GetConstraintLHS( void* self, void* _swarm, Cell_LocalIndex lCell_I, Index power_i, Index power_j, Index power_k ) ;
	double WeightsCalculator_GetLocalCoordSum( void* self, void* _swarm, Cell_LocalIndex lCell_I, Index power_i, Index power_j, Index power_k ) ;
	double WeightsCalculator_SumCellWeights( void* self, void* _swarm, Cell_LocalIndex lCell_I ) ;
	void WeightsCalculator_CheckEmptyCell( void* self, void* _swarm, Cell_LocalIndex lCell_I ) ;
        
#endif 

