/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisaton_Triquadratic_h__
#define __StgFEM_Discretisaton_Triquadratic_h__

	/** Textual name of this class */
	extern const Type Triquadratic_Type;

	/** Virtual function types */

	/** Triquadratic class contents */
	#define __Triquadratic		\
		/* General info */	\
		__ElementType		\
					\
		/* Virtual info */	\
					\
		/* Triquadratic info */

	struct Triquadratic { __Triquadratic };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TRIQUADRATIC_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define TRIQUADRATIC_PASSARGS \
                ELEMENTTYPE_PASSARGS

	Triquadratic* Triquadratic_New( Name name );
	Triquadratic* _Triquadratic_New(  TRIQUADRATIC_DEFARGS  );
	void _Triquadratic_Init( Triquadratic* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Triquadratic_Delete( void* elementType );
	void _Triquadratic_Print( void* elementType, Stream* stream );
	void _Triquadratic_AssignFromXML( void* elementType, Stg_ComponentFactory* cf, void* data );
	void _Triquadratic_Build( void* elementType, void* data );
	void _Triquadratic_Initialise( void* elementType, void* data );
	void _Triquadratic_Execute( void* elementType, void* data );
	void _Triquadratic_Destroy( void* elementType, void* data );

	void Triquadratic_EvalBasis( void* elementType, const double* localCoord, double* derivs );
	void Triquadratic_EvalLocalDerivs( void* elementType, const double* localCoord, double** derivs );
	double Triquadratic_JacobianDeterminantSurface( void* elementType, void* mesh, unsigned element_I, 
							const double localCoord[], unsigned face_I, unsigned norm );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgFEM_Discretisaton_Triquadratic_h__ */

