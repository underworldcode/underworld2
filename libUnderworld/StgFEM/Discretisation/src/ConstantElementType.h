/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_ConstantElementType_h__
#define __StgFEM_Discretisation_ConstantElementType_h__
	
	/* Textual name of this class */
	extern const Type ConstantElementType_Type;
	
	/* ConstantElementType information */
	#define __ConstantElementType \
		/* General info */ \
		__ElementType \
		\
		/* Virtual info */ \
		\
		/* ConstantElementType info */
	struct ConstantElementType { __ConstantElementType };



	
	void* ConstantElementType_DefaultNew( Name name );

	/* Create a new ConstantElementType and initialise */
	ConstantElementType* ConstantElementType_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONSTANTELEMENTTYPE_DEFARGS \
                ELEMENTTYPE_DEFARGS

	#define CONSTANTELEMENTTYPE_PASSARGS \
                ELEMENTTYPE_PASSARGS

	ConstantElementType* _ConstantElementType_New(  CONSTANTELEMENTTYPE_DEFARGS  );
	
	/* Initialise a ConstantElementType construct */
	void _ConstantElementType_Init( ConstantElementType* self );
	
	/* Stg_Class_Delete a ConstantElementType construst */
	void _ConstantElementType_Delete( void* elementType );
	
	/* Print the contents of an ConstantElementType construct */
	void _ConstantElementType_Print( void* elementType, Stream* stream );
	
	/** Constant element type build implementation */
	
	void _ConstantElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data );
	
	void _ConstantElementType_Build( void* elementType, void *data );
	
	void _ConstantElementType_Initialise( void* elementType, void *data );
	
	void _ConstantElementType_Execute( void* elementType, void *data );
	
	void _ConstantElementType_Destroy( void* elementType, void *data );
	
	/** Calculate the shape function for all degrees of freedom for all nodes */
	void _ConstantElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues );
	
	/** Calculate the shape function local derivatives for all degrees of freedom for all nodes */
	void _ConstantElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives );
	
	/** Constant element type implementation of ElementType_ConvertGlobalCoordToElLocal().
	Nothing to do... the "constant element" doesn't really have local coordinates
	since the shape function is independent of Xi and Eta ... just return a 
	centroid {0,0,0} which is consistent with the centre of the co-ordinate system
	for the bilinear, trilinear etc element types.
	*/
	void _ConstantElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord );

#if 0
	double _ConstantElementType_JacobianDeterminantSurface(
		void*		elementType,
		void*		mesh,
		const double	localCoord[],
		unsigned*	nodes,
		unsigned	norm );
#endif 

	int _ConstantElementType_SurfaceNormal(
		void*		elementType,
		unsigned	element_I,
		unsigned	dim,
		double*		xi,
		double*		normal );

#endif /* __StgFEM_Discretisation_ConstantElementType_h__ */

