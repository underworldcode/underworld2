/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_shortcuts_h__
#define __StgDomain_Mesh_shortcuts_h__
	
	/* Mesh shortcuts */
	#define Mesh_Node_Neighbour_I( mesh, node, index )		mesh->nodeNeighbourTbl[node][index]
	#define Mesh_Node_Element_I( mesh, node, index )		mesh->nodeElementTbl[node][index]
	#define Mesh_Element_Neighbour_I( mesh, element, index )	mesh->elementNeighbourTbl[element][index]
	#define Mesh_Element_Node_I( mesh, element, index )		mesh->elementNodeTbl[element][index]
	
	#define Mesh_Node_Neighbour_P( mesh, node, index )		Mesh_NodeAt( mesh, Mesh_Node_Neighbour_I( mesh, node, index ) )
	#define Mesh_Node_Element_P( mesh, node, index )		Mesh_ElementAt( mesh, Mesh_Node_Element_I( mesh, node, index ) )
	#define Mesh_Element_Neighbour_P( mesh, element, index )	Mesh_ElementAt( mesh, Mesh_Element_Neighbour_I( mesh, node, index ) )
	#define Mesh_Element_Node_P( mesh, element, index )		Mesh_NodeAt( mesh, Mesh_Element_Node_I( mesh, element, index ) )
	
#endif /* __StgDomain_Mesh_shortcuts_h__ */
