/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisaton_Biquadratic_h__
#define __StgFEM_Discretisaton_Biquadratic_h__

	/** Textual name of this class */
	extern const Type Biquadratic_Type;

	/** Virtual function types */

	/** Biquadratic class contents */
	#define __Biquadratic		\
		/* General info */	\
		__ElementType		\
					\
		/* Virtual info */	\
					\
		/* Biquadratic info */  \

	struct Biquadratic { __Biquadratic };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define BIQUADRATIC_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define BIQUADRATIC_PASSARGS \
                ELEMENTTYPE_PASSARGS

	Biquadratic* Biquadratic_New( Name name );
	Biquadratic* _Biquadratic_New(  BIQUADRATIC_DEFARGS  );
	void _Biquadratic_Init( Biquadratic* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Biquadratic_Delete( void* elementType );
	void _Biquadratic_Print( void* elementType, Stream* stream );
	void _Biquadratic_AssignFromXML( void* elementType, Stg_ComponentFactory* cf, void* data );
	void _Biquadratic_Build( void* elementType, void* data );
	void _Biquadratic_Initialise( void* elementType, void* data );
	void _Biquadratic_Execute( void* elementType, void* data );
	void _Biquadratic_Destroy( void* elementType, void* data );

	void Biquadratic_EvalBasis( void* elementType, const double* localCoord, double* derivs );
	void Biquadratic_EvalLocalDerivs( void* elementType, const double* localCoord, double** derivs );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgFEM_Discretisaton_Biquadratic_h__ */

