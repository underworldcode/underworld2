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

#ifndef __StgFEM_Discretisation_dQ13DElType_h__
#define __StgFEM_Discretisation_dQ13DElType_h__
	
	/* Textual name of this class */
	extern const Type dQ13DElType_Type;
	
	/* dQ13DElType information */
	#define __dQ13DElType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* dQ13DElType info */ \
		Coord   minElLocalCoord; /** Bottom corner in elLocal mathematical space */ \
		Coord   maxElLocalCoord; /** Top corner in elLocal mathematical space */ \
		Coord   elLocalLength; /** Length of element in elLocal space */
		
	struct dQ13DElType { __dQ13DElType };

	/* Create a new dQ13DElType and initialise */
	void* _dQ13DElType_DefaultNew( Name name );

	dQ13DElType* dQ13DElType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DQ13DELTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define DQ13DELTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	dQ13DElType* _dQ13DElType_New(  DQ13DELTYPE_DEFARGS  );
	
	/* Initialise implementation */
	void _dQ13DElType_Init( dQ13DElType* self );
	
	/* Stg_Class_Delete a dQ13DElType construst */
	void _dQ13DElType_Delete( void* elementType );
	
	/* Print the contents of an dQ13DElType construct */
	void _dQ13DElType_Print( void* elementType, Stream* stream );
	
	/* Trilinear inner element type build implementation */
	void _dQ13DElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _dQ13DElType_Build( void* elementType, void *data );
	
	void _dQ13DElType_Initialise( void* elementType, void *data );
	
	void _dQ13DElType_Execute( void* elementType, void *data );
	
	void _dQ13DElType_Destroy( void* elementType, void *data );
	
	/** ElementType_EvaluateShapeFunctionsAt implementation. */
	void _dQ13DElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** ElementType_EvaluateShapeFunctionsDerivsAt implementation. */
	void _dQ13DElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
#endif /* __StgFEM_Discretisation_dQ13DElType_h__ */

