/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_ElementType_h__
#define __StgDomain_Mesh_ElementType_h__

	/** Textual name of this class */
	extern const Type Mesh_ElementType_Type;

	/** Virtual function types */
	typedef void (Mesh_ElementType_UpdateFunc)( void* elementType );
	typedef Bool (Mesh_ElementType_ElementHasPointFunc)( void* elementType, unsigned element, double* point, 
							     MeshTopology_Dim* dim, unsigned* ind );
	typedef double (Mesh_ElementType_GetMinimumSeparationFunc)( void* elementType, unsigned element, double* perDim );
	typedef void (Mesh_ElementType_GetCentroidFunc)( void* elementType, unsigned element, double* centroid );

	/** Class contents */
	#define __Mesh_ElementType								\
		/* General info */								\
		__Stg_Class									\
												\
		/* Virtual info */								\
		Mesh_ElementType_UpdateFunc*			updateFunc;			\
		Mesh_ElementType_ElementHasPointFunc*		elementHasPointFunc;		\
		Mesh_ElementType_GetMinimumSeparationFunc*	getMinimumSeparationFunc;	\
		Mesh_ElementType_GetCentroidFunc*		getCentroidFunc;		\
												\
		/* Mesh_ElementType info */							\
		Mesh*			mesh;

	struct Mesh_ElementType { __Mesh_ElementType };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_ELEMENTTYPE_DEFARGS \
                STG_CLASS_DEFARGS, \
                Mesh_ElementType_UpdateFunc*                              updateFunc, \
                Mesh_ElementType_ElementHasPointFunc*            elementHasPointFunc, \
                Mesh_ElementType_GetMinimumSeparationFunc*  getMinimumSeparationFunc, \
                Mesh_ElementType_GetCentroidFunc*                    getCentroidFunc

	#define MESH_ELEMENTTYPE_PASSARGS \
                STG_CLASS_PASSARGS, \
	        updateFunc,               \
	        elementHasPointFunc,      \
	        getMinimumSeparationFunc, \
	        getCentroidFunc         

	Mesh_ElementType* _Mesh_ElementType_New(  MESH_ELEMENTTYPE_DEFARGS  );
	void _Mesh_ElementType_Init( Mesh_ElementType* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_ElementType_Delete( void* elementType );
	void _Mesh_ElementType_Print( void* elementType, Stream* stream );
	void _Mesh_ElementType_GetCentroid( void* elementType, unsigned element, double* centroid );

	#define Mesh_ElementType_Update( self )							\
		VirtualCall( self, updateFunc, self )

	#define Mesh_ElementType_ElementHasPoint( self, element, point, dim, ind )		\
		VirtualCall( self, elementHasPointFunc, self, element, point, dim, ind )

	#define Mesh_ElementType_GetMinimumSeparation( self, element, perDim )			\
		VirtualCall( self, getMinimumSeparationFunc, self, element, perDim )

	#define Mesh_ElementType_GetCentroid( self, element, centroid )				\
		VirtualCall( self, getCentroidFunc, self, element, centroid )

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_ElementType_SetMesh( void* elementType, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgDomain_Mesh_ElementType_h__ */

