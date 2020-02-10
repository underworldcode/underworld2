/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_RegularAlgorithms_h__
#define __StgDomain_Mesh_RegularAlgorithms_h__

	/** Textual name of this class */
	extern const Type Mesh_RegularAlgorithms_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh_RegularAlgorithms		\
		/* General info */			\
		__Mesh_Algorithms			\
							\
		/* Virtual info */			\
							\
		/* Mesh_RegularAlgorithms info */	\
		double*		sep;			\
		double*		minCrd;			\
		double*		maxCrd;

	struct Mesh_RegularAlgorithms { __Mesh_RegularAlgorithms };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	Mesh_RegularAlgorithms* Mesh_RegularAlgorithms_New( Name name, AbstractContext* context );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_REGULARALGORITHMS_DEFARGS \
                MESH_ALGORITHMS_DEFARGS

	#define MESH_REGULARALGORITHMS_PASSARGS \
                MESH_ALGORITHMS_PASSARGS

	Mesh_RegularAlgorithms* _Mesh_RegularAlgorithms_New(  MESH_REGULARALGORITHMS_DEFARGS  );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_RegularAlgorithms_Init( void* algorithms );

	void _Mesh_RegularAlgorithms_Delete( void* algorithms );

	void _Mesh_RegularAlgorithms_Print( void* algorithms, Stream* stream );

	void _Mesh_RegularAlgorithms_AssignFromXML( void* algorithms, Stg_ComponentFactory* cf, void* data );

	void _Mesh_RegularAlgorithms_Build( void* algorithms, void* data );

	void _Mesh_RegularAlgorithms_Initialise( void* algorithms, void* data );

	void _Mesh_RegularAlgorithms_Execute( void* algorithms, void* data );

	void _Mesh_RegularAlgorithms_Destroy( void* algorithms, void* data );


	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_RegularAlgorithms_SetMesh( void* algorithms, void* mesh );

	void Mesh_RegularAlgorithms_Update( void* algorithms );

	Bool Mesh_RegularAlgorithms_SearchElements( void* algorithms, double* point, unsigned* elInd );

	#if 0 
	Bool Mesh_RegularAlgorithms_Search( void* algorithms, void* mesh, double* point, MeshTopology_Dim* dim, unsigned* ind );

	double Mesh_RegularAlgorithms_GetMinimumSeparation( void* algorithms, void* mesh, double* perDim );

	void Mesh_RegularAlgorithms_GetLocalCoordRange( void* algorithms, void* mesh, double* min, double* max );

	void Mesh_RegularAlgorithms_GetDomainCoordRange( void* algorithms, void* mesh, double* min, double* max );

	void Mesh_RegularAlgorithms_GetGlobalCoordRange( void* algorithms, void* mesh, double* min, double* max );
	#endif 

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Mesh_RegularAlgorithms_Destruct( Mesh_RegularAlgorithms* self );

#endif /* __StgDomain_Mesh_RegularAlgorithms_h__ */

