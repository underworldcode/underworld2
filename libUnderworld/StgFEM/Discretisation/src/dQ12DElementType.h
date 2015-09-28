/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StgFEM_Discretisation_dQ12DElType_h__
#define __StgFEM_Discretisation_dQ12DElType_h__
	
	/* Textual name of this class */
	extern const Type dQ12DElType_Type;
	
	/* dQ12DElType information */
	#define __dQ12DElType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* dQ12DElType info */ \
		Coord   minElLocalCoord; /** Bottom corner in elLocal mathematical space */ \
		Coord   maxElLocalCoord; /** Top corner in elLocal mathematical space */ \
		Coord   elLocalLength; /** Length of element in elLocal space */

		
	struct dQ12DElType { __dQ12DElType };


	#define DQ12DELTYPE_PASSARGS \
    	ELEMENTTYPE_PASSARGS
	
	
	/* Create a new dQ12DElType and initialise */
	void* _dQ12DElType_DefaultNew( Name name );

	dQ12DElType* dQ12DElType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DQ12DELTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define DQ12DELTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	dQ12DElType* _dQ12DElType_New(  DQ12DELTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _dQ12DElType_Init( dQ12DElType* self );
	
	/* Stg_Class_Delete a dQ12DElType construst */
	void _dQ12DElType_Delete( void* elementType );
	
	/* Print the contents of an dQ12DElType construct */
	void _dQ12DElType_Print( void* elementType, Stream* stream );
	
	/* dQ1 element type implementation */
	void _dQ12DElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _dQ12DElType_Build( void* elementType, void *data );
	
	void _dQ12DElType_Initialise( void* elementType, void *data );
	
	void _dQ12DElType_Execute( void* elementType, void *data );
	
	void _dQ12DElType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _dQ12DElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionsDerivsAt implementation. */
	void _dQ12DElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );

	int _dQ12DElType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal );

#endif /* __StgFEM_Discretisation_dQ12DElType_h__ */

