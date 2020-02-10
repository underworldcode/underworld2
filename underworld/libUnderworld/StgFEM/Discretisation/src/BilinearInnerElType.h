/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_BilinearInnerElType_h__
#define __StgFEM_Discretisation_BilinearInnerElType_h__
	
	/* Textual name of this class */
	extern const Type BilinearInnerElType_Type;

	/* BilinearInnerElType information */
	#define __BilinearInnerElType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* BilinearInnerElType info */ \
		double	minElLocalCoord[2]; /** Bottom corner in elLocal mathematical space */ \
		double	maxElLocalCoord[2]; /** Top corner in elLocal mathematical space */ \
		double	elLocalLength[2]; /** Length of element in elLocal space */ \
		\
		unsigned**		triInds;
		
	struct BilinearInnerElType { __BilinearInnerElType };



	/* Create a new BilinearInnerElType and initialise */
	void* BilinearInnerElType_DefaultNew( Name name );
	
	BilinearInnerElType* BilinearInnerElType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define BILINEARINNERELTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define BILINEARINNERELTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	BilinearInnerElType* _BilinearInnerElType_New(  BILINEARINNERELTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _BilinearInnerElType_Init( BilinearInnerElType* self );
	
	/* Stg_Class_Delete a BilinearInnerElType construst */
	void _BilinearInnerElType_Delete( void* elementType );
	
	/* Print the contents of an BilinearInnerElType construct */
	void _BilinearInnerElType_Print( void* elementType, Stream* stream );
	
	/* Bilinear inner element type build implementation */
	void _BilinearInnerElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _BilinearInnerElType_Build( void* elementType, void* data );
	
	void _BilinearInnerElType_Initialise( void* elementType, void *data );
	
	void _BilinearInnerElType_Execute( void* elementType, void *data );
	
	void _BilinearInnerElType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _BilinearInnerElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionDerivsAt implementation. */
	void _BilinearInnerElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );

#endif /* __StgFEM_Discretisation_BilinearInnerElType_h__ */

