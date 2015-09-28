/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_CartesianGenerator_h__
#define __StgDomain_Mesh_CartesianGenerator_h__

	/** Textual name of this class */
	extern const Type CartesianGenerator_Type;

	/** Virtual function types */
	typedef void (CartesianGenerator_SetTopologyParamsFunc)( void* meshGenerator, unsigned* sizes, 
								 unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp );
	typedef void (CartesianGenerator_GenElementsFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenFacesFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenEdgesFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenVerticesFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenElementVertexIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenVolumeEdgeIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenVolumeFaceIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenFaceVertexIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenFaceEdgeIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenEdgeVertexIncFunc)( void* meshGenerator, IGraph* topo, Grid*** grids );
	typedef void (CartesianGenerator_GenElementTypesFunc)( void* meshGenerator, Mesh* mesh );
	typedef void (CartesianGenerator_CalcGeomFunc) (void* meshGenerator, Mesh* mesh, Sync* sync, Grid* grid, unsigned* inds, double* steps );

	/** CartesianGenerator class contents */
	#define __CartesianGenerator								\
		/* General info */								\
		__MeshGenerator									\
												\
		/* Virtual info */								\
		CartesianGenerator_SetTopologyParamsFunc*	setTopologyParamsFunc;		\
		CartesianGenerator_GenElementsFunc*		genElementsFunc;		\
		CartesianGenerator_GenFacesFunc*		genFacesFunc;			\
		CartesianGenerator_GenEdgesFunc*		genEdgesFunc;			\
		CartesianGenerator_GenVerticesFunc*		genVerticesFunc;		\
		CartesianGenerator_GenElementVertexIncFunc*	genElementVertexIncFunc;	\
		CartesianGenerator_GenVolumeEdgeIncFunc*	genVolumeEdgeIncFunc;		\
		CartesianGenerator_GenVolumeFaceIncFunc*	genVolumeFaceIncFunc;		\
		CartesianGenerator_GenFaceVertexIncFunc*	genFaceVertexIncFunc;		\
		CartesianGenerator_GenFaceEdgeIncFunc*		genFaceEdgeIncFunc;		\
		CartesianGenerator_GenEdgeVertexIncFunc*	genEdgeVertexIncFunc;		\
		CartesianGenerator_GenElementTypesFunc*		genElementTypesFunc;		\
		CartesianGenerator_CalcGeomFunc*          calcGeomFunc;              \
												\
		/* CartesianGenerator info */							\
		Comm*		comm;								\
		Bool		regular;							\
		Bool		periodic[3];							\
		/* read cartesian mesh data from checkpoint file? */                            \
		Bool		readFromFile;							\
		unsigned	maxDecompDims;							\
		unsigned*	minDecomp;							\
		unsigned*	maxDecomp;							\
		unsigned	shadowDepth;							\
		double*		crdMin;								\
		double*		crdMax;								\
												\
		Grid*		vertGrid;		/* Grid of the global vertice distribution */ \
		Grid*		elGrid;		  /* Grid of the global element distribution */ \
		Grid*		procGrid;		/* Grid of the global processor decomposition */ \
		unsigned*	origin;		/* Local element grid origin coord for this processor */ \
		unsigned*	range;		/* Local element grid range for this processor */ \
		unsigned*	vertOrigin;	/* Local node grid origin for this processor */ \
		unsigned*	vertRange;  /* Local node grid range for this processor */ \
                int             contactDepth[3][2];                     \
                double          contactGeom[3]; \
		char*           initVtkFile; /* for reading Rozel's mesh JG - 22Oct2012 */ \
		char*           initMeshFile;

	struct CartesianGenerator { __CartesianGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CARTESIANGENERATOR_DEFARGS \
                MESHGENERATOR_DEFARGS, \
                CartesianGenerator_SetTopologyParamsFunc*      setTopologyParamsFunc, \
                CartesianGenerator_GenElementsFunc*                  genElementsFunc, \
                CartesianGenerator_GenFacesFunc*                        genFacesFunc, \
                CartesianGenerator_GenEdgesFunc*                        genEdgesFunc, \
                CartesianGenerator_GenVerticesFunc*                  genVerticesFunc, \
                CartesianGenerator_GenElementVertexIncFunc*  genElementVertexIncFunc, \
                CartesianGenerator_GenVolumeEdgeIncFunc*        genVolumeEdgeIncFunc, \
                CartesianGenerator_GenVolumeFaceIncFunc*        genVolumeFaceIncFunc, \
                CartesianGenerator_GenFaceVertexIncFunc*        genFaceVertexIncFunc, \
                CartesianGenerator_GenFaceEdgeIncFunc*            genFaceEdgeIncFunc, \
                CartesianGenerator_GenEdgeVertexIncFunc*        genEdgeVertexIncFunc, \
                CartesianGenerator_GenElementTypesFunc*         genElementTypesFunc, \
                CartesianGenerator_CalcGeomFunc*                calcGeomFunc          \
                

	#define CARTESIANGENERATOR_PASSARGS \
                MESHGENERATOR_PASSARGS, \
	        setTopologyParamsFunc,   \
	        genElementsFunc,         \
	        genFacesFunc,            \
	        genEdgesFunc,            \
	        genVerticesFunc,         \
	        genElementVertexIncFunc, \
	        genVolumeEdgeIncFunc,    \
	        genVolumeFaceIncFunc,    \
	        genFaceVertexIncFunc,    \
	        genFaceEdgeIncFunc,      \
	        genEdgeVertexIncFunc,    \
	        genElementTypesFunc,     \
          calcGeomFunc

  void* _CartesianGenerator_DefaultNew( Name name );
	CartesianGenerator* CartesianGenerator_New( Name name, AbstractContext* context );
	CartesianGenerator* _CartesianGenerator_New(  CARTESIANGENERATOR_DEFARGS  );
	void _CartesianGenerator_Init( CartesianGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _CartesianGenerator_Delete( void* meshGenerator );
	void _CartesianGenerator_Print( void* meshGenerator, Stream* stream );
	void _CartesianGenerator_AssignFromXML( void* meshGenerator, Stg_ComponentFactory* cf, void* data );
	void _CartesianGenerator_Build( void* meshGenerator, void* data );
	void _CartesianGenerator_Initialise( void* meshGenerator, void* data );
	void _CartesianGenerator_Execute( void* meshGenerator, void* data );
	void _CartesianGenerator_Destroy( void* meshGenerator, void* data );

	void CartesianGenerator_SetDimSize( void* meshGenerator, unsigned nDims );
	void CartesianGenerator_Generate( void* meshGenerator, void* mesh, void* data );
	void _CartesianGenerator_SetTopologyParams( void* meshGenerator, unsigned* sizes, 
						    unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp );
	void _CartesianGenerator_GenElements( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenFaces( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenEdges( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenVertices( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenElementVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenVolumeEdgeInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenVolumeFaceInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenFaceVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenFaceEdgeInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenEdgeVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void _CartesianGenerator_GenElementTypes( void* meshGenerator, Mesh* mesh );

	#define CartesianGenerator_SetTopologyParams( self, sizes, maxDecompDims, minDecomp, maxDecomp )	\
		VirtualCall( self, setTopologyParamsFunc, self, sizes, maxDecompDims, minDecomp, maxDecomp )
	#define CartesianGenerator_GenElements( self, topo, grids )						\
		VirtualCall( self, genElementsFunc, self, topo, grids )
	#define CartesianGenerator_GenFaces( self, topo, grids )						\
		VirtualCall( self, genFacesFunc, self, topo, grids )
	#define CartesianGenerator_GenEdges( self, topo, grids )						\
		VirtualCall( self, genEdgesFunc, self, topo, grids )
	#define CartesianGenerator_GenVertices( self, topo, grids )						\
		VirtualCall( self, genVerticesFunc, self, topo, grids )
	#define CartesianGenerator_GenElementVertexInc( self, topo, grids )					\
		VirtualCall( self, genElementVertexIncFunc, self, topo, grids )
	#define CartesianGenerator_GenVolumeEdgeInc( self, topo, grids )					\
		VirtualCall( self, genVolumeEdgeIncFunc, self, topo, grids )
	#define CartesianGenerator_GenVolumeFaceInc( self, topo, grids )					\
		VirtualCall( self, genVolumeFaceIncFunc, self, topo, grids )
	#define CartesianGenerator_GenFaceVertexInc( self, topo, grids )					\
		VirtualCall( self, genFaceVertexIncFunc, self, topo, grids )
	#define CartesianGenerator_GenFaceEdgeInc( self, topo, grids )						\
		VirtualCall( self, genFaceEdgeIncFunc, self, topo, grids )
	#define CartesianGenerator_GenEdgeVertexInc( self, topo, grids )					\
		VirtualCall( self, genEdgeVertexIncFunc, self, topo, grids )
	#define CartesianGenerator_GenElementTypes( self, mesh )						\
		VirtualCall( self, genElementTypesFunc, self, mesh )

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void CartesianGenerator_SetGeometryParams( void* meshGenerator, double* min, double* max );
	void CartesianGenerator_SetShadowDepth( void* meshGenerator, unsigned depth );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void CartesianGenerator_BuildDecomp( CartesianGenerator* self );
	void CartesianGenerator_RecurseDecomps( CartesianGenerator* self, 
						unsigned dim, unsigned max, 
						unsigned* nSubDomains, 
						unsigned* nPos, unsigned*** posNSubDomains );
	void CartesianGenerator_GenTopo( CartesianGenerator* self, IGraph* topo );
	void CartesianGenerator_GenEdges2D( CartesianGenerator* self, IGraph* topo, Grid*** grids );
	void CartesianGenerator_GenEdges3D( CartesianGenerator* self, IGraph* topo, Grid*** grids );
	void CartesianGenerator_GenBndVerts( CartesianGenerator* self, IGraph* topo, Grid*** grids );
	void CartesianGenerator_CompleteVertexNeighbours( CartesianGenerator* self, IGraph* topo, Grid*** grids );
	void CartesianGenerator_MapToDomain( CartesianGenerator* self, Sync* sync, 
					     unsigned nIncEls, unsigned* incEls );
	void CartesianGenerator_GenGeom( CartesianGenerator* self, Mesh* mesh, void* data );
	void CartesianGenerator_CalcGeom( void* _self, Mesh* mesh, Sync* sync, Grid* grid, unsigned* inds, double* steps );
	void CartesianGenerator_Destruct( CartesianGenerator* self );
	void CartesianGenerator_DestructTopology( CartesianGenerator* self );
	void CartesianGenerator_DestructGeometry( CartesianGenerator* self );
	void CartesianGenerator_ReadFromHDF5(  CartesianGenerator* self, Mesh* mesh, const char* filename );
	void CartesianGenerator_ReadFromASCII( CartesianGenerator* self, Mesh* mesh, const char* filename ); 
	void CartesianGenerator_ReadFromVTK(  CartesianGenerator* self, Mesh* mesh, const char* filename );

#endif /* __StgDomain_Mesh_CartesianGenerator_h__ */

