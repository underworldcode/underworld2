/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_LinearTriangleElementType_h__
#define __StgFEM_Discretisation_LinearTriangleElementType_h__
	
	/* Textual name of this class */
	extern const Type LinearTriangleElementType_Type;
	
	/* LinearTriangleElementType information */
	#define __LinearTriangleElementType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* LinearTriangleElementType info */

	struct LinearTriangleElementType { __LinearTriangleElementType };
	

	
	/* Create a new LinearTriangleElementType and initialise */
	void* _LinearTriangleElementType_DefaultNew( Name name );

	LinearTriangleElementType* LinearTriangleElementType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LINEARTRIANGLEELEMENTTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define LINEARTRIANGLEELEMENTTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	LinearTriangleElementType* _LinearTriangleElementType_New(  LINEARTRIANGLEELEMENTTYPE_DEFARGS  );			
	
	/* Initialise implementation */
	void _LinearTriangleElementType_Init( LinearTriangleElementType* self );
	
	/* Stg_Class_Delete a LinearTriangleElementType construst */
	void _LinearTriangleElementType_Delete( void* elementType );
	
	/* Print the contents of an LinearTriangleElementType construct */
	void _LinearTriangleElementType_Print( void* elementType, Stream* stream );
	
	void _LinearTriangleElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	/* LinearTriangle element type build implementation */
	void _LinearTriangleElementType_Build( void* elementType, void *data );
	
	void _LinearTriangleElementType_Initialise( void* elementType, void *data );
	
	void _LinearTriangleElementType_Execute( void* elementType, void *data );
	
	void _LinearTriangleElementType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _LinearTriangleElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _LinearTriangleElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	int _LinearTriangularElementType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal );

#endif /* __StgFEM_Discretisation_LinearTriangleElementType_h__ */

