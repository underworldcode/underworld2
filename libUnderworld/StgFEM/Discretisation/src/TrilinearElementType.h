/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_TrilinearElementType_h__
#define __StgFEM_Discretisation_TrilinearElementType_h__
	
	/* Textual name of this class */
	extern const Type TrilinearElementType_Type;
	
	/* TrilinearElementType information */
	#define __TrilinearElementType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* TrilinearElementType info */ \
		Coord	minElLocalCoord; /** Bottom corner in elLocal mathematical space */ \
		Coord	maxElLocalCoord; /** Top corner in elLocal mathematical space */ \
		double	elLocalLength[3]; /** Length of element in elLocal space */ \
		\
		unsigned**	tetInds;
		
	struct TrilinearElementType { __TrilinearElementType };


	
	/* Create a new TrilinearElementType and initialise */
	void* _TrilinearElementType_DefaultNew( Name name );

	TrilinearElementType* TrilinearElementType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TRILINEARELEMENTTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define TRILINEARELEMENTTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	TrilinearElementType* _TrilinearElementType_New(  TRILINEARELEMENTTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _TrilinearElementType_Init( TrilinearElementType* self );
	
	/* Stg_Class_Delete a TrilinearElementType construst */
	void _TrilinearElementType_Delete( void* elementType );
	
	/* Print the contents of an TrilinearElementType construct */
	void _TrilinearElementType_Print( void* elementType, Stream* stream );
	
	/* Trilinear element type build implementation */
	void _TrilinearElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _TrilinearElementType_Build( void* elementType, void *data );
	
	void _TrilinearElementType_Initialise( void* elementType, void *data );
	
	void _TrilinearElementType_Execute( void* elementType, void *data );
	
	void _TrilinearElementType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _TrilinearElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionsDerivsAt implementation. */
	void _TrilinearElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	/** ElementType_ConvertGlobalCoordToElLocal implementation.
	Uses a shortcut approach if using "box" elements - otherwise uses the general function. */
	#if 0 
	void _TrilinearElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord );
	#endif 

#endif /* __StgFEM_Discretisation_TrilinearElementType_h__ */

