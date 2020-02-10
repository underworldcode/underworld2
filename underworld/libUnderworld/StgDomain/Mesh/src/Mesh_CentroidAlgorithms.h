/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_CentroidAlgorithms_h__
#define __StgDomain_Mesh_CentroidAlgorithms_h__

	/** Textual name of this class */
	extern const Type Mesh_CentroidAlgorithms_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh_CentroidAlgorithms		\
		/* General info */			\
		__Mesh_Algorithms			\
							\
		/* Virtual info */			\
							\
		/* Mesh_CentroidAlgorithms info */	\
		Mesh*			elMesh;

	struct Mesh_CentroidAlgorithms { __Mesh_CentroidAlgorithms };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	Mesh_CentroidAlgorithms* Mesh_CentroidAlgorithms_New( Name name, AbstractContext* context );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_CENTROIDALGORITHMS_DEFARGS \
                MESH_ALGORITHMS_DEFARGS

	#define MESH_CENTROIDALGORITHMS_PASSARGS \
                MESH_ALGORITHMS_PASSARGS

	Mesh_CentroidAlgorithms* _Mesh_CentroidAlgorithms_New(  MESH_CENTROIDALGORITHMS_DEFARGS  );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_CentroidAlgorithms_Init( void* centroidAlgorithms );

	void _Mesh_CentroidAlgorithms_Delete( void* centroidAlgorithms );

	void _Mesh_CentroidAlgorithms_Print( void* centroidAlgorithms, Stream* stream );

	void _Mesh_CentroidAlgorithms_AssignFromXML( void* centroidAlgorithms, Stg_ComponentFactory* cf, void* data );

	void _Mesh_CentroidAlgorithms_Build( void* centroidAlgorithms, void* data );

	void _Mesh_CentroidAlgorithms_Initialise( void* centroidAlgorithms, void* data );

	void _Mesh_CentroidAlgorithms_Execute( void* centroidAlgorithms, void* data );

	void _Mesh_CentroidAlgorithms_Destroy( void* centroidAlgorithms, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_CentroidAlgorithms_SetElementMesh( void* centroidAlgorithms, void* mesh );

	void Mesh_CentroidAlgorithms_Update( void* centroidAlgorithms );

	unsigned Mesh_CentroidAlgorithms_NearestVertex( void* centroidAlgorithms, double* point );

	Bool Mesh_CentroidAlgorithms_Search( void* centroidAlgorithms, double* point, MeshTopology_Dim* dim, unsigned* ind );

	Bool Mesh_CentroidAlgorithms_SearchElements( void* centroidAlgorithms, double* point, unsigned* elInd );

	void Mesh_CentroidAlgorithms_GetLocalCoordRange( void* algorithms, double* min, double* max );

	void Mesh_CentroidAlgorithms_GetDomainCoordRange( void* algorithms, double* min, double* max );

	void Mesh_CentroidAlgorithms_GetGlobalCoordRange( void* algorithms, double* min, double* max );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgDomain_Mesh_CentroidAlgorithms_h__ */

