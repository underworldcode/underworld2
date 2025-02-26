/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_types_h__
#define __StgDomain_Mesh_types_h__

typedef struct Grid Grid;
typedef struct Decomp Decomp;
typedef struct Sync Sync;
typedef struct MeshTopology MeshTopology;
typedef struct IGraph IGraph;
typedef struct SpatialTree SpatialTree;

typedef enum {
   MT_VERTEX, 
   MT_EDGE, 
   MT_FACE, 
   MT_VOLUME
} MeshTopology_Dim;
/*
  Note:
  "Local" refers to entities on local processor that are in the domain of that the current processor.
  "Domain" refers to "Local" entities as well as entities from the shadowed region.
  So Domain is a superset of Local.

  Yae verily, you may wonder why Domain is not called Local and vice-versa :)

  */

	/* Basic indexes */
	typedef unsigned int			GlobalIndex;			/* global 1D index, not constrained */
	typedef unsigned int			DomainIndex;			/* domain (local + shadow) 1D index, not 
											constrained */
	typedef unsigned int			LocalIndex;			/* local 1D index, not constrained */
	typedef unsigned int			ShadowIndex;			/* shadow 1D index, not constrained */

	/* Basic Node types */
	typedef GlobalIndex			Node_GlobalIndex;		/* global 1D, 0 -> globalNodeCount, > is error */
	typedef Index				Node_Index;			/* index, positive, not constrained */
	typedef DomainIndex			Node_DomainIndex;		/* domain 1D, 0 -> (local_shadow)NodeCount, > is 
											error */
	typedef LocalIndex			Node_LocalIndex;		/* local 1D, 0 -> localNodeCount, > is error */
	typedef ShadowIndex			Node_ShadowIndex;		/* shadow 1D, 0 -> shadowNodeCount, > is error */
	typedef Index				Node_ElementLocalIndex;		/* element local 1D, 0 -> count at given el.*/
	typedef Node_GlobalIndex		Node_GlobalIJK[3];
	typedef Node_LocalIndex			Node_LocalIJK[3];
	typedef Node_ShadowIndex		Node_ShadowIJK[3];
	typedef Node_DomainIndex		Node_DomainIJK[3];
	
	/* Basic Element types */
	typedef GlobalIndex			Element_GlobalIndex;		/* global 1D, 0 -> globalNodeCount, > is error  */
	typedef DomainIndex			Element_DomainIndex;		/* domain 1D, 0 -> (local_shadow)NodeCount, > is 
											error */
	typedef Element_GlobalIndex		Element_Index;			/* index, positive, not constrained */
	typedef LocalIndex			Element_LocalIndex;		/* local 1D, 0 -> localNodeCount, > is error */
	typedef ShadowIndex			Element_ShadowIndex;		/* shadow 1D, 0 -> shadowNodeCount, > is error */
	typedef Element_GlobalIndex		Element_GlobalIJK[3];
	typedef Element_LocalIndex		Element_LocalIJK[3];
	typedef Element_ShadowIndex		Element_ShadowIJK[3];
	typedef Element_DomainIndex		Element_DomainIJK[3];
	typedef struct MeshElement MeshElement;
	
	/* Advanced Node types */
	typedef Node_DomainIndex*		Node_Neighbours;		/* array of domain indices of neighbouring nodes */
	typedef Index				Node_NeighbourIndex; 		/* index of a "node's neighbours" array */
	typedef Node_Neighbours*		Node_NeighboursList;		/* array of "node's neighbours" */
	typedef Node_NeighbourIndex*		Node_NeighbourIndexList;	/* array of counts of "node's neighbours" */
	
	typedef Element_DomainIndex*		Node_Elements;			/* array of domain indices of adjoining elements */
	typedef Index				Node_ElementIndex; 		/* index of a "node's elements" array */
	typedef Node_Elements*			Node_ElementsList;		/* array of "node's elements" */
	typedef Node_ElementIndex*		Node_ElementIndexList;		/* array of counts of "node's elements" */
	
	/* Advanced Element types */
	typedef Node_DomainIndex*		Element_Nodes;			/* array of domain indices of element nodes */
	typedef Index				Element_NodeIndex;		/* index of an "element's nodes" array */
	typedef Element_Nodes*			Element_NodesList;		/* array of "element's nodes" */
	typedef Element_NodeIndex*		Element_NodeIndexList;		/* array of counts of "element's nodes" */
	
	typedef Element_DomainIndex*		Element_Neighbours;		/* array of domain indices of neighbouring 
											elements */
	typedef Index				Element_NeighbourIndex;		/* index of an "element's neighbours" array */
	typedef Element_Neighbours*		Element_NeighboursList;		/* array of "element's neighbours" */
	typedef Element_NeighbourIndex*		Element_NeighbourIndexList;	/* array of counts of "element's neighbours" */
	
	/* Parallel types */
	typedef Index 				Partition_Index;			/* rank / domain partition index */
	typedef Partition_Index			PartitionIndex;
	typedef Partition_Index			Processor_Index;
	typedef Partition_Index			ProcessorIndex;
	typedef Partition_Index			Partition_IJK[3];
	/* For indexing processors by their neighbour number, not their
	global rank (ie in shadowing) */
	typedef Index				Neighbour_Index; 
	typedef Neighbour_Index			Nbr_Index;
	
	/* Classes */
	typedef struct Mesh_ElementType			Mesh_ElementType;
	typedef struct Mesh_HexType			Mesh_HexType;
	typedef struct Mesh_CentroidType		Mesh_CentroidType;
	typedef struct Mesh_Algorithms			Mesh_Algorithms;
	typedef struct Mesh_HexAlgorithms		Mesh_HexAlgorithms;
	typedef struct Mesh_RegularAlgorithms		Mesh_RegularAlgorithms;
	typedef struct Mesh_CentroidAlgorithms		Mesh_CentroidAlgorithms;
	typedef struct Mesh				Mesh;
	typedef struct MeshGenerator			MeshGenerator;
	typedef struct CartesianGenerator		CartesianGenerator;
	typedef struct MeshVariable			MeshVariable;
	typedef struct Remesher Remesher;

	/* Misc */
   struct  MeshElement { 
	   Element_LocalIndex    elementIndex;
	   Mesh*                 elementMesh;
	};

	/* Streams */
	extern Stream*	Mesh_VerboseConfig;
	extern Stream*	Mesh_Debug; 	
	extern Stream*	Mesh_Warning; 
	extern Stream*	Mesh_Error;
	
#endif /* __StgDomain_Mesh_types_h__ */
