/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_TrilinearInnerElType_h__
#define __StgFEM_Discretisation_TrilinearInnerElType_h__
	
	/* Textual name of this class */
	extern const Type TrilinearInnerElType_Type;
	
	/* TrilinearInnerElType information */
	#define __TrilinearInnerElType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* TrilinearInnerElType info */ \
		Coord	minElLocalCoord; /** Bottom corner in elLocal mathematical space */ \
		Coord	maxElLocalCoord; /** Top corner in elLocal mathematical space */ \
		double	elLocalLength[3]; /** Length of element in elLocal space */ \
		\
		unsigned**	tetInds;
		
	struct TrilinearInnerElType { __TrilinearInnerElType };


	#define TRILINEARINEERELTYPE_PASSARGS \
    	ELEMENTTYPE_PASSARGS
	
	
	/* Create a new TrilinearInnerElType and initialise */
	void* _TrilinearInnerElType_DefaultNew( Name name );

	TrilinearInnerElType* TrilinearInnerElType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TRILINEARINNERELTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define TRILINEARINNERELTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	TrilinearInnerElType* _TrilinearInnerElType_New(  TRILINEARINNERELTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _TrilinearInnerElType_Init( TrilinearInnerElType* self );
	
	/* Stg_Class_Delete a TrilinearInnerElType construst */
	void _TrilinearInnerElType_Delete( void* elementType );
	
	/* Print the contents of an TrilinearInnerElType construct */
	void _TrilinearInnerElType_Print( void* elementType, Stream* stream );
	
	/* Trilinear inner element type build implementation */
	void _TrilinearInnerElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _TrilinearInnerElType_Build( void* elementType, void *data );
	
	void _TrilinearInnerElType_Initialise( void* elementType, void *data );
	
	void _TrilinearInnerElType_Execute( void* elementType, void *data );
	
	void _TrilinearInnerElType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _TrilinearInnerElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionsDerivsAt implementation. */
	void _TrilinearInnerElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	/** ElementType_ConvertGlobalCoordToElLocal implementation.
	Uses a shortcut approach if using "box" elements - otherwise uses the general function. */
	/*void _TrilinearInnerElType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord );
	*/

	int _TrilinearInnerElType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal );

#endif /* __StgFEM_Discretisation_TrilinearInnerElType_h__ */

