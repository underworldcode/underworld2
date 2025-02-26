/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Mesh_h__
#define __StgDomain_Mesh_Mesh_h__
	
#include "types.h"
#include "shortcuts.h"

#include "Grid.h"
#include "Decomp.h"
#include "Sync.h"
#include "MeshTopology.h"
#include "IGraph.h"
/*
	#include "Decomposer.h"
	#include "DecompTransfer.h"
*/
#include "Mesh_ElementType.h"
#include "Mesh_HexType.h"
#include "Mesh_CentroidType.h"
#include "Mesh_Algorithms.h"
#include "Mesh_HexAlgorithms.h"
#include "Mesh_RegularAlgorithms.h"
#include "Mesh_CentroidAlgorithms.h"
#include "MeshClass.h"
#include "MeshGenerator.h"
#include "CartesianGenerator.h"
#include "MeshVariable.h"
#include "SpatialTree.h"
#include "Remesher.h"

#include "Init.h"
#include "Finalise.h"

#endif /* __StgDomain_Mesh_Mesh_h__ */
