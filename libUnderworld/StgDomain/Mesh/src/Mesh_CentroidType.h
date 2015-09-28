/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_CentroidType_h__
#define __StgDomain_Mesh_CentroidType_h__

	/** Textual name of this class */
	extern const Type Mesh_CentroidType_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh_CentroidType			\
		/* General info */			\
		__Mesh_ElementType			\
							\
		/* Virtual info */			\
							\
		/* Mesh_CentroidType info */		\
		Mesh*			elMesh;		\
		IArray*			incArray;

	struct Mesh_CentroidType { __Mesh_CentroidType };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_CENTROIDTYPE_DEFARGS \
                MESH_ELEMENTTYPE_DEFARGS

	#define MESH_CENTROIDTYPE_PASSARGS \
                MESH_ELEMENTTYPE_PASSARGS

	Mesh_CentroidType* Mesh_CentroidType_New();
	Mesh_CentroidType* _Mesh_CentroidType_New(  MESH_CENTROIDTYPE_DEFARGS  );
	void _Mesh_CentroidType_Init( Mesh_CentroidType* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_CentroidType_Delete( void* centroidType );
	void _Mesh_CentroidType_Print( void* centroidType, Stream* stream );

	void Mesh_CentroidType_Update( void* centroidType );
	Bool Mesh_CentroidType_ElementHasPoint( void* centroidType, unsigned elInd, double* point, 
					   MeshTopology_Dim* dim, unsigned* ind );
	double Mesh_CentroidType_GetMinimumSeparation( void* centroidType, unsigned elInd, double* perDim );
	void Mesh_CentroidType_GetCentroid( void* centroidType, unsigned element, double* centroid );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_CentroidType_SetElementMesh( void* centroidType, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgDomain_Mesh_CentroidType_h__ */

