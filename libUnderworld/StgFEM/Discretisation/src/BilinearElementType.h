/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_BilinearElementType_h__
#define __StgFEM_Discretisation_BilinearElementType_h__
	
	/* Textual name of this class */
	extern const Type BilinearElementType_Type;

	/* BilinearElementType information */
	#define __BilinearElementType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* BilinearElementType info */ \
		double	minElLocalCoord[2]; /** Bottom corner in elLocal mathematical space */ \
		double	maxElLocalCoord[2]; /** Top corner in elLocal mathematical space */ \
		double	elLocalLength[2]; /** Length of element in elLocal space */ \
		\
		unsigned**		triInds; \
		
	struct BilinearElementType { __BilinearElementType };



	/* Create a new BilinearElementType and initialise */
	void* _BilinearElementType_DefaultNew( Name name );
	
	BilinearElementType* BilinearElementType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define BILINEARELEMENTTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define BILINEARELEMENTTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	BilinearElementType* _BilinearElementType_New(  BILINEARELEMENTTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _BilinearElementType_Init( BilinearElementType* self );
	
	/* Stg_Class_Delete a BilinearElementType construst */
	void _BilinearElementType_Delete( void* elementType );
	
	/* Print the contents of an BilinearElementType construct */
	void _BilinearElementType_Print( void* elementType, Stream* stream );
	
	/* Bilinear element type build implementation */
	void _BilinearElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _BilinearElementType_Build( void* elementType, void* data );
	
	void _BilinearElementType_Initialise( void* elementType, void *data );
	
	void _BilinearElementType_Execute( void* elementType, void *data );
	
	void _BilinearElementType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _BilinearElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionDerivsAt implementation. */
	void _BilinearElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	/** ElementType_ConvertGlobalToElLocalCoord implementation. If the element type is "box" (ParallelPipedQuadEL),
		then calculates this using a faster shortcut. */
#if 0
	void _BilinearElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord );
#endif 

	double _BilinearElementType_JacobianDeterminantSurface(
		void*		elementType,
		void*		mesh,
		unsigned	element_I,
		const double	localCoord[],
		unsigned	face_I,
		unsigned	norm );

#endif /* __StgFEM_Discretisation_BilinearElementType_h__ */

