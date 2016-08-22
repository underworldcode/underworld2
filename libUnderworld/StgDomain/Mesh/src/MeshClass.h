/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_MeshClass_h__
#define __StgDomain_Mesh_MeshClass_h__

	/** Textual name of this class */
	extern const Type Mesh_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh						\
		/* General info */				\
		__Stg_Component					\
								\
		/* Virtual info */				\
								\
		/* Mesh info */					\
		MeshTopology*			topo;		\
                double*                 vertices;		\
                Variable*               verticesVariable; \
                int*                    elgid;  \
                Variable*               eGlobalIdsVar; \
                int*                    verticesgid; \
                Variable*               vGlobalIdsVar; \
                int*                    e_n; \
                Variable*               enMapVar; \
								\
		List*				vars;		\
        /* just record these for various variable */ \
        unsigned            nverts;\
        unsigned            lVerts;\
        unsigned            lEls; \
        unsigned            localtotalNodes; \
								\
		double				minSep;		\
		double*				minAxialSep;	\
		double*				minLocalCrd;	\
		double*				maxLocalCrd;	\
		double*				minDomainCrd;	\
		double*				maxDomainCrd;	\
		double*				minGlobalCrd;	\
		double*				maxGlobalCrd;	\
								\
		Mesh_Algorithms*		algorithms;	\
		unsigned			nElTypes;	\
		Mesh_ElementType**		elTypes;	\
		unsigned*			elTypeMap;	\
								\
		UIntMap*			topoDataSizes;	\
		ExtensionManager**		topoDataInfos;	\
		void**				topoDatas;	\
		ExtensionManager*		info;		\
		unsigned int        vertGridId;	/* extension id for the vertex grid */  \
		unsigned int        elGridId;	  /* extension id for the element grid */ \
		unsigned int        periodicId;	  /* extension id for the mesh periodicity */ \
		unsigned int        localOriginId; /* extension id for the mesh's local origin */ \
		unsigned int        localRangeId;	  /* extension id for the mesh's local range */ \
								\
		Bool isRegular; /* is the mesh regularly spaced */ \
		MeshGenerator*			generator;	\
		/* should mesh be stored and reloaded ? */                    \
		Bool                            isCheckpointedAndReloaded;    \
		/* determines if mesh requires storing (it may already have been stored) */ \
		Bool                            isDeforming;        \
		ExtensionManager_Register*	emReg;                  \
        Mesh*             parentMesh;  /* If this mesh is generated based on a 'parent' mesh, record here. */
                                       /* Else record self */

	struct Mesh { __Mesh };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define MESH_PASSARGS \
                STG_COMPONENT_PASSARGS

	Mesh* Mesh_New( Name name );
	Mesh* _Mesh_New(  MESH_DEFARGS  );
	void _Mesh_Init( Mesh* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_Delete( void* mesh );
	void _Mesh_Print( void* mesh, Stream* stream );
	void _Mesh_AssignFromXML( void* mesh, Stg_ComponentFactory* cf, void* data );
	void _Mesh_Build( void* mesh, void* data );
	void _Mesh_Initialise( void* mesh, void* data );
	void _Mesh_Execute( void* mesh, void* data );
	void _Mesh_Destroy( void* mesh, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_SetGenerator( void* mesh, void* generator );
	void Mesh_SetAlgorithms( void* mesh, void* algorithms );

	void Mesh_SetExtensionManagerRegister( void* mesh, void* extMgrReg );
	void Mesh_SetTopologyDataSize( void* mesh, MeshTopology_Dim dim, unsigned size );
	void* Mesh_GetTopologyData( void* mesh, MeshTopology_Dim dim );

	unsigned Mesh_GetDimSize( void* mesh );
	unsigned Mesh_GetGlobalSize( void* mesh, MeshTopology_Dim dim );
	unsigned Mesh_GetLocalSize( void* mesh, MeshTopology_Dim dim );
	unsigned Mesh_GetRemoteSize( void* mesh, MeshTopology_Dim dim );
	unsigned Mesh_GetDomainSize( void* mesh, MeshTopology_Dim dim );
	unsigned Mesh_GetSharedSize( void* mesh, MeshTopology_Dim dim );
	MeshTopology* Mesh_GetTopology( void* mesh );
	Sync* Mesh_GetSync( void* mesh, MeshTopology_Dim dim );

	Bool Mesh_GlobalToDomain( void* mesh, MeshTopology_Dim dim, unsigned global, unsigned* domain );
	unsigned Mesh_DomainToGlobal( void* mesh, MeshTopology_Dim dim, unsigned domain );
	Bool Mesh_LocalToShared( void* meshTopology, MeshTopology_Dim dim, unsigned domain, unsigned* shared );
	unsigned Mesh_SharedToLocal( void* meshTopology, MeshTopology_Dim dim, unsigned shared );

	unsigned Mesh_GetOwner( void* mesh, MeshTopology_Dim dim, unsigned remote );
	void Mesh_GetSharers( void* mesh, MeshTopology_Dim dim, unsigned shared, 
			      unsigned* nSharers, unsigned** sharers );

	Bool Mesh_HasIncidence( void* mesh, MeshTopology_Dim fromDim, MeshTopology_Dim toDim );
	unsigned Mesh_GetIncidenceSize( void* mesh, MeshTopology_Dim fromDim, unsigned fromInd, 
					MeshTopology_Dim toDim );
	void Mesh_GetIncidence( void* mesh, MeshTopology_Dim fromDim, unsigned fromInd, MeshTopology_Dim toDim, 
				IArray* inc );

	unsigned Mesh_NearestVertex( void* mesh, double* point );
	Bool Mesh_Search( void* mesh, double* point, 
			  MeshTopology_Dim* dim, unsigned* ind );

	Bool Mesh_SearchElements( void* mesh, double* point, unsigned* elInd );
	/* Mesh_SearchElements (
	 * mesh -- is a mesh
	 * point -- is a global coordinate
	 * elInd -- will be filled in by a local elementID
	 * )
	 * returns:
	 * False if the point is not in the DOMAIN space of the proc 
	 * True if the point is in the DOMAIN space
	 */

	Bool Mesh_ElementHasPoint( void* mesh, unsigned element, double* point, 
				   MeshTopology_Dim* dim, unsigned* ind );
	Mesh_ElementType* Mesh_GetElementType( void* mesh, unsigned element );

	Comm* Mesh_GetCommTopology( void* mesh, MeshTopology_Dim dim );
	double* Mesh_GetVertex( void* mesh, unsigned domain );

	Bool Mesh_HasExtension( void* mesh, const char* name );
	#define Mesh_GetExtension( mesh, type, name ) \
		(type)_Mesh_GetExtension( mesh, name )
	void* _Mesh_GetExtension( void* mesh, const unsigned int id );

	void Mesh_GetMinimumSeparation( void* mesh, double* minSep, double* axial );
	void Mesh_GetLocalCoordRange( void* mesh, double* min, double* max );
	void Mesh_GetDomainCoordRange( void* mesh, double* min, double* max );
	void Mesh_GetGlobalCoordRange( void* mesh, double* min, double* max );

	void Mesh_DeformationUpdate( void* mesh );
	void Mesh_Sync( void* mesh );

    void Mesh_GenerateVertices( void* mesh, unsigned nVerts, unsigned nDims );
    Variable* Mesh_GenerateNodeGlobalIdVar( void* mesh );
    Variable* Mesh_GenerateENMapVar( void* mesh );
    Variable* Mesh_GenerateElGlobalIdVar( void* mesh );


	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Mesh_Destruct( Mesh* self );

#endif /* __StgDomain_Mesh_MeshClass_h__ */

