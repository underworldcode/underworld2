/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_Weights_ConstantWeightsClass_h__
#define __PICellerator_Weights_ConstantWeightsClass_h__

	/* Textual name of this class */
	extern const Type ConstantWeights_Type;

	/* ConstantWeights information */
	#define __ConstantWeights \
    /* General info */ \
	__WeightsCalculator \
	\
	/* Virtual Info */ \

	struct ConstantWeights { __ConstantWeights };

	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	ConstantWeights* ConstantWeights_New( Name name, int dim );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONSTANTWEIGHTS_DEFARGS \
		WEIGHTSCALCULATOR_DEFARGS

	#define CONSTANTWEIGHTS_PASSARGS \
		WEIGHTSCALCULATOR_PASSARGS

	ConstantWeights* _ConstantWeights_New(  CONSTANTWEIGHTS_DEFARGS  );

	void _ConstantWeights_Init( void* constantWeights );

	/* Stg_Class_Delete ConstantWeights implementation */
	void _ConstantWeights_Delete( void* constantWeights );

	void _ConstantWeights_Print( void* constantWeights, Stream* stream );

	#define ConstantWeights_Copy( self ) \
		(ConstantWeights*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ConstantWeights_DeepCopy( self ) \
		(ConstantWeights*) Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _ConstantWeights_Copy( void* constantWeights, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
        
	void* _ConstantWeights_DefaultNew( Name name );

	void _ConstantWeights_AssignFromXML( void* constantWeights, Stg_ComponentFactory* cf, void* data );

	void _ConstantWeights_Build( void* constantWeights, void* data );

	void _ConstantWeights_Destroy( void* constantWeights, void* data );

	void _ConstantWeights_Initialise( void* constantWeights, void* data );

	void _ConstantWeights_Execute( void* constantWeights, void* data );
        
	void _ConstantWeights_Calculate( void* constantWeights, void* _swarm, Cell_LocalIndex lCell_I );

	/*---------------------------------------------------------------------------------------------------------------------
	** Private functions
	*/
        
	/*---------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
        
#endif 

