/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_LinearElementType_h__
#define __StgFEM_Discretisation_LinearElementType_h__
	
	/* Textual name of this class */
	extern const Type LinearElementType_Type;

	/* LinearElementType information */
	#define __LinearElementType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* LinearElementType info */ \
		double	minElLocalCoord[2]; /** Bottom corner in elLocal mathematical space */ \
		double	maxElLocalCoord[2]; /** Top corner in elLocal mathematical space */ \
		double	elLocalLength[2]; /** Length of element in elLocal space */ \
		\
		unsigned**		triInds; \
		
	struct LinearElementType { __LinearElementType };



	/* Create a new LinearElementType and initialise */
	void* _LinearElementType_DefaultNew( Name name );
	
	LinearElementType* LinearElementType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define BILINEARELEMENTTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define BILINEARELEMENTTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	LinearElementType* _LinearElementType_New(  BILINEARELEMENTTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _LinearElementType_Init( LinearElementType* self );
	
	/* Stg_Class_Delete a LinearElementType construst */
	void _LinearElementType_Delete( void* elementType );
	
	/* Print the contents of an LinearElementType construct */
	void _LinearElementType_Print( void* elementType, Stream* stream );
	
	/* Linear element type build implementation */
	void _LinearElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _LinearElementType_Build( void* elementType, void* data );
	
	void _LinearElementType_Initialise( void* elementType, void *data );
	
	void _LinearElementType_Execute( void* elementType, void *data );
	
	void _LinearElementType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _LinearElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionDerivsAt implementation. */
	void _LinearElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	/** ElementType_ConvertGlobalToElLocalCoord implementation. If the element type is "box" (ParallelPipedQuadEL),
		then calculates this using a faster shortcut. */
	#if 0 
	void _LinearElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord );
	#endif 

#endif /* __StgFEM_Discretisation_LinearElementType_h__ */

