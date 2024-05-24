/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>

#include "types.h"
#include "RegularMeshUtils.h"


Index RegularMeshUtils_ascendingIJK_ToHughesNodeNumberMap[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };


/*----------------------------------------------------------------------------------------------------------------------------------
** Mapping functions
*/

void RegularMeshUtils_Node_1DTo3D( void* _mesh, unsigned global, unsigned* inds ) {
	Mesh*	mesh = (Mesh*)_mesh;
	Grid**	grid;

	assert( mesh );
	assert( global < Mesh_GetGlobalSize( mesh, MT_VERTEX ) );
	assert( inds );

	grid = (Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );
	Grid_Lift( *grid, global, inds );
}

unsigned RegularMeshUtils_Node_3DTo1D( void* _mesh, unsigned* inds ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid**		grid;

	assert( mesh );
	assert( inds );

	grid = (Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	return Grid_Project( *grid, inds );
}
/*@
  RegularMeshUtils_Element_1DTo3D 
  -given a mesh and a global node number, returns the global (i,j,k) location indices of the node in inds[3].

  Example:
  In a 2D pressure mesh that is 4x3 on 1 processor
  The numbering would usually be as follows

  8  9  10 11
  4  5  6  7
  0  1  2  3

  So this function would map the global index 6
  6 -> (2,1,0) = (i,j,k) in inds[3]

  @*/
void RegularMeshUtils_Element_1DTo3D( void* _mesh, unsigned global, unsigned* inds ) {
	Mesh*	mesh = (Mesh*)_mesh;
	Grid**	grid;

	assert( mesh );
	assert( global < Mesh_GetGlobalSize( mesh, MT_VERTEX ) );
	assert( inds );

	grid = (Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->elGridId  );
	Grid_Lift( *grid, global, inds );
}

unsigned RegularMeshUtils_Element_3DTo1D( void* _mesh, unsigned* inds ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid**		grid;

	assert( mesh );
	assert( inds );

	grid = (Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->elGridId  );

	return Grid_Project( *grid, inds );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Set functions
*/

void RegularMeshUtils_ErrorCheckAndGetDetails( Mesh* mesh, MeshTopology_Dim INPUT, unsigned *nDomainSize, Grid** grid ) {
	/*@
		A utility function for error checking and data retrieval that is used by other functions in this file
		
		Return values:
		nDomainSize          - the number of INPUT topological elements in the processors domain space
		grid                 - the global grid of the INPUT	

		@*/
	int dim = 0;

	void *ptr=NULL;
	Grid *someGrid=NULL;
	assert( mesh );
	dim = Mesh_GetDimSize(mesh);
	assert( dim >= 2 );

	if( INPUT == MT_EDGE ) { assert(0); } //not implemented yet, an edge grid isn't useful
	else if( dim==2 && INPUT==MT_VOLUME ) {assert(0);} //error condition
	else if( dim==3 && INPUT==MT_FACE ) {assert(0);} //can handle this condition yet, needs implementation 

	/* find which mesh grid should be queried */
	if( INPUT==MT_VERTEX ) {
		/* get vertex grid */
		*grid = *(Grid**)Mesh_GetExtension(mesh,Grid**,mesh->vertGridId);
	} else {
		/* assume INPUT defines the element grid and is appropriate, i.e not MT_FACE & dim is 3*/
		ptr = _Mesh_GetExtension(mesh,mesh->elGridId);
		someGrid = *(Grid**)_Mesh_GetExtension(mesh,mesh->elGridId);
		*grid = *(Grid**)Mesh_GetExtension(mesh,Grid**,mesh->elGridId);
	}

	assert(grid);
	/* get the number of INPUT topology elements in the proc domain */ 
	*nDomainSize = Mesh_GetDomainSize(mesh,INPUT);
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureMinJSet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[1] == 0 )
			IndexSet_Add( set, n_i );
	}
	return set;
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureMinISet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[0] == 0 )
			IndexSet_Add( set, n_i );
	}
	return set;
}
IndexSet* RegularMeshUtils_CreateBoundaryFeatureMaxISet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[0] == grid->sizes[0]-1 )
			IndexSet_Add( set, n_i );
	}
	return set;
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureMinKSet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[2] == 0 )
			IndexSet_Add( set, n_i );
	}
	return set;
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureMaxKSet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[2] == grid->sizes[1] - 1 )
			IndexSet_Add( set, n_i );
	}
	return set;
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureMaxJSet( void* _mesh, MeshTopology_Dim TOPO ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid=NULL;
	unsigned	tSize;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	/* get the domain number and the assosiated grid of the topological element */
	RegularMeshUtils_ErrorCheckAndGetDetails( mesh, TOPO, &tSize, &grid );

	set = IndexSet_New( tSize );

	for( n_i = 0; n_i < tSize; n_i++ ) {
		( TOPO == MT_VERTEX ) ?
			RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk ) :
			RegularMeshUtils_Element_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, TOPO, n_i ), ijk );

		if( ijk[1] == grid->sizes[1] - 1 )
			IndexSet_Add( set, n_i );
	}
	return set;
}

IndexSet* RegularMeshUtils_CreateBoundaryElementDomainSet( void* _mesh ) {
	/*@ 
		Purpose: Creates a new set of problem domain boundary element, on this processor's domain space

		The returned IndexSet* will be dynamically allocated and the user must free it.
		i.e:
			IndexSet* bndSet = RegularMeshUtils_CreateBoundaryElementDomainSet( self->feMesh );
			/// foobar ///
			FreeObject( bndSet );

		@*/

  Mesh*		mesh = (Mesh*)_mesh;
	MeshTopology_Dim TOPO;
  unsigned	size, dim;
  IndexSet *completeSet, *tmp;

  assert( mesh );
	dim = Mesh_GetDimSize(mesh);
  assert( dim >= 2 );
	
	(dim==3) ? (TOPO = MT_VOLUME) : (TOPO = MT_FACE);

	/* preallocate the IndexSet */
  size = Mesh_GetDomainSize( mesh, TOPO );
  completeSet = IndexSet_New( size );

  tmp = RegularMeshUtils_CreateBoundaryFeatureMaxJSet( _mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  tmp = RegularMeshUtils_CreateBoundaryFeatureMinJSet( mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  tmp = RegularMeshUtils_CreateBoundaryFeatureMinISet( mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );
  
  tmp = RegularMeshUtils_CreateBoundaryFeatureMaxISet( mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  if( Mesh_GetDimSize( mesh ) > 2 ) {
    tmp = RegularMeshUtils_CreateBoundaryFeatureMaxKSet( mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
    FreeObject( tmp );

    tmp = RegularMeshUtils_CreateBoundaryFeatureMinKSet( mesh, TOPO ); IndexSet_Merge_OR( completeSet, tmp );
    FreeObject( tmp );
  }

  /* Release tmp storage */
  FreeObject( tmp );

  return completeSet;
}

IndexSet* RegularMeshUtils_CreateBoundaryFeatureSet( void* _mesh, MeshTopology_Dim TOPO, char* whichSet ){

   if (     !strcasecmp(whichSet, "back")   || !strcasecmp(whichSet, "MinK"))
      return RegularMeshUtils_CreateBoundaryFeatureMinKSet( _mesh, TOPO);
   else if (!strcasecmp(whichSet, "left")   || !strcasecmp(whichSet, "MinI"))
      return RegularMeshUtils_CreateBoundaryFeatureMinISet( _mesh, TOPO);
   else if (!strcasecmp(whichSet, "bottom") || !strcasecmp(whichSet, "MinJ"))
      return RegularMeshUtils_CreateBoundaryFeatureMinJSet( _mesh, TOPO);
   else if (!strcasecmp(whichSet, "right")  || !strcasecmp(whichSet, "MaxI"))
      return RegularMeshUtils_CreateBoundaryFeatureMaxISet( _mesh, TOPO);
   else if (!strcasecmp(whichSet, "top")    || !strcasecmp(whichSet, "MaxJ"))
      return RegularMeshUtils_CreateBoundaryFeatureMaxJSet( _mesh, TOPO);
   else if (!strcasecmp(whichSet, "front")  || !strcasecmp(whichSet, "MaxK"))
      return RegularMeshUtils_CreateBoundaryFeatureMaxKSet( _mesh, TOPO);
   else {
      Stream*  errorStr = Journal_Register( Error_Type,  "RegularMeshUtils"  );
      Journal_Firewall( 0 , errorStr, "Error- in %s: set type '%s' is invalid.  Valid Set types are: \n\n"
        "   MinK (back)\n   MinI (left)\n   MinJ (bottom)\n   MaxI (right)\n   MaxJ (top)\n   MaxK (front)\n\n"
        , __func__, whichSet );
	}
	return NULL;

}



IndexSet* RegularMeshUtils_CreateGlobalSet( void* _mesh ) {
	/*@ 
		Purpose: Creates an IndexSet of processors domain boundary nodes.

		The returned IndexSet* will be dynamically allocated and the user must free it.
		i.e:
			IndexSet* bndSet = RegularMeshUtils_CreateGlobalSet( self->feMesh );
			/// foobar ///
			FreeObject( bndSet );

		@*/

  Mesh*		mesh = (Mesh*)_mesh;
  unsigned	nNodes;
  IndexSet *completeSet, *tmp;

  assert( mesh );
  assert( Mesh_GetDimSize( mesh ) >= 2 );

  nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
  completeSet = IndexSet_New( nNodes );

  tmp = RegularMeshUtils_CreateGlobalMaxJSet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  tmp = RegularMeshUtils_CreateGlobalMinJSet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  tmp = RegularMeshUtils_CreateGlobalMinISet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );
  
  tmp = RegularMeshUtils_CreateGlobalMaxISet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
  FreeObject( tmp );

  if( Mesh_GetDimSize( mesh ) > 2 ) {
    tmp = RegularMeshUtils_CreateGlobalMaxKSet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
    FreeObject( tmp );

    tmp = RegularMeshUtils_CreateGlobalMinKSet( mesh ); IndexSet_Merge_OR( completeSet, tmp );
    FreeObject( tmp );
  }

  /* Release tmp storage */
  FreeObject( tmp );

  return completeSet;
}
IndexSet* RegularMeshUtils_CreateGlobalMaxJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == grid->sizes[1] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 3 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[2] == grid->sizes[2] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 3 );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[2] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMaxJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
        int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == grid->sizes[1] - 1 && 
		    (ijk[0] != grid->sizes[0] - 1 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[0] != 0 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[0] != grid->sizes[0] - 1 || (nDims == 3 && ijk[2] != 0)) && 
		    (ijk[0] != 0 || (nDims == 3 && ijk[2] != 0)) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMinJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
        int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if(ijk[1] == 0 &&
		   (ijk[0] != grid->sizes[0] - 1 && ijk[0] != 0) &&
		   (nDims != 3 || (ijk[2] != grid->sizes[2] - 1 && ijk[2] != 0)))
		{
		    IndexSet_Add( set, n_i );
		}
/* 		if( ijk[1] == 0 &&  */
/* 		    (ijk[0] != grid->sizes[0] - 1 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) &&  */
/* 		    (ijk[0] != 0 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) &&  */
/* 		    (ijk[0] != grid->sizes[0] - 1 || (nDims == 3 && ijk[2] != 0)) &&  */
/* 		    (ijk[0] != 0 || (nDims == 3 && ijk[2] != 0)) ) */
/* 		{ */
/* 			IndexSet_Add( set, n_i ); */
/* 		} */
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMinISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
        int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    (ijk[1] != grid->sizes[1] - 1 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[1] != 0 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[1] != grid->sizes[1] - 1 || (nDims == 3 && ijk[2] != 0)) && 
		    (ijk[1] != 0 || (nDims == 3 && ijk[2] != 0)) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMaxISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
        int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 && 
		    (ijk[1] != grid->sizes[1] - 1 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[1] != 0 || (nDims == 3 && ijk[2] != grid->sizes[2] - 1)) && 
		    (ijk[1] != grid->sizes[1] - 1 || (nDims == 3 && ijk[2] != 0)) && 
		    (ijk[1] != 0 || (nDims == 3 && ijk[2] != 0)) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[2] - 1 && 
		    (ijk[0] != grid->sizes[0] - 1 || ijk[1] != grid->sizes[1] - 1 ) && 
		    (ijk[0] != 0 || ijk[1] != grid->sizes[1] - 1 ) && 
		    (ijk[0] != grid->sizes[0] - 1 || ijk[1] != 0 ) && 
		    (ijk[0] != 0 || ijk[1] != 0 ) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    (ijk[0] != grid->sizes[0] - 1 || ijk[1] != grid->sizes[1] - 1 ) && 
		    (ijk[0] != 0 || ijk[1] != grid->sizes[1] - 1 ) && 
		    (ijk[0] != grid->sizes[0] - 1 || ijk[1] != 0 ) && 
		    (ijk[0] != 0 || ijk[1] != 0 ) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinIMinJMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    ijk[1] == 0 && 
		    ijk[2] == grid->sizes[2] - 1 )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxIMinJMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 && 
		    ijk[1] == 0 && 
		    ijk[2] == grid->sizes[2] - 1 )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinIMaxJMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
	int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    ijk[1] == grid->sizes[1] - 1 && 
		    (nDims != 3 || ijk[2] == grid->sizes[2] - 1) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxIMaxJMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
	int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 && 
		    ijk[1] == grid->sizes[1] - 1 && 
		    (nDims != 3 || ijk[2] == grid->sizes[2] - 1) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinIMinJMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	int nDims;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    ijk[1] == 0 && 
		    (nDims != 3 || ijk[2] == 0))
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxIMinJMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	int nDims;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 && 
		    ijk[1] == 0 && 
		    (nDims != 3 || ijk[2] == 0) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMinIMaxJMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
	int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 && 
		    ijk[1] == grid->sizes[1] - 1 && 
		    (nDims != 3 || ijk[2] == 0) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxIMaxJMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;
	int nDims;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 && 
		    ijk[1] == grid->sizes[1] - 1 && 
		    (nDims != 3 || ijk[2] == 0) )
		{
			IndexSet_Add( set, n_i );
		}
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMaxJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == grid->sizes[1] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMinJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMinISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMaxISet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  );

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[0] == grid->sizes[0] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMaxKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	Grid*		grid;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 3 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId  ); 

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[2] == grid->sizes[2] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalMinKSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 3 );

	nNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[2] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateContactMaxJSet( void* _mesh, int lowDepth, int uppDepth ) {
   Mesh* mesh = (Mesh*)_mesh;
   Grid* grid;
   int nNodes;
   IndexSet* set;
   int ijk[2], minI, maxI, minJ, maxJ;
   int ii;

   assert( mesh );
   assert( Mesh_GetDimSize( mesh ) == 2 );

   grid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   nNodes = Mesh_GetDomainSize( mesh, 0 );
   set = IndexSet_New( nNodes );

/*
   minI = depth + 1;
   maxI = grid->sizes[0] - (depth + 1) - 1;
   minJ = 1;
   maxJ = depth;
*/
   minI = lowDepth;
   maxI = grid->sizes[0] - 1 - uppDepth;
   minJ = grid->sizes[1] - 1;
   maxJ = grid->sizes[1] - 1;
   for( ii = 0; ii < nNodes; ii++ ) {
      Grid_Lift( grid, Mesh_DomainToGlobal( mesh, 0, ii ), ijk );
      if( ijk[0] >= minI && ijk[0] <= maxI && ijk[1] >= minJ && ijk[1] <= maxJ )
	 IndexSet_Add( set, ii );
   }

   return set;
}

IndexSet* RegularMeshUtils_CreateContactMinJSet( void* _mesh, int lowDepth, int uppDepth, int inDepth ) {
   Mesh* mesh = (Mesh*)_mesh;
   Grid* grid;
   int nNodes;
   IndexSet* set;
   int ijk[2], minI, maxI, minJ, maxJ;
   int ii;

   assert( mesh );
   assert( Mesh_GetDimSize( mesh ) == 2 );

   grid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   nNodes = Mesh_GetDomainSize( mesh, 0 );
   set = IndexSet_New( nNodes );

/*
   minI = depth + 1;
   maxI = grid->sizes[0] - (depth + 1) - 1;
   minJ = 1;
   maxJ = depth;
*/
   minI = lowDepth;
   maxI = grid->sizes[0] - 1 - uppDepth;
   minJ = 0;
   maxJ = inDepth;
   for( ii = 0; ii < nNodes; ii++ ) {
      Grid_Lift( grid, Mesh_DomainToGlobal( mesh, 0, ii ), ijk );
      if( ijk[0] >= minI && ijk[0] <= maxI && ijk[1] >= minJ && ijk[1] <= maxJ ) {
	 IndexSet_Add( set, ii );
      }
   }

   return set;
}

IndexSet* RegularMeshUtils_CreateContactMinISet( void* _mesh, int lowDepth, int uppDepth ) {
   Mesh* mesh = (Mesh*)_mesh;
   Grid* grid;
   int nNodes;
   IndexSet* set;
   int ijk[2], minI, maxI, minJ, maxJ;
   int ii;

   assert( mesh );
   assert( Mesh_GetDimSize( mesh ) == 2 );

   grid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   nNodes = Mesh_GetDomainSize( mesh, 0 );
   set = IndexSet_New( nNodes );

/*
   minI = 1;
   maxI = depth;
   minJ = depth + 1;
   maxJ = grid->sizes[1] - 2;
*/
   minI = 0;
   maxI = 0;
   minJ = lowDepth;
   maxJ = grid->sizes[1] - 1 - uppDepth;
   for( ii = 0; ii < nNodes; ii++ ) {
      Grid_Lift( grid, Mesh_DomainToGlobal( mesh, 0, ii ), ijk );
      if( ijk[0] >= minI && ijk[0] <= maxI && ijk[1] >= minJ && ijk[1] <= maxJ )
	 IndexSet_Add( set, ii );
   }

   return set;
}

IndexSet* RegularMeshUtils_CreateContactMaxISet( void* _mesh, int lowDepth, int uppDepth, int inDepth ) {
   Mesh* mesh = (Mesh*)_mesh;
   Grid* grid;
   int nNodes;
   IndexSet* set;
   int ijk[2], minI, maxI, minJ, maxJ;
   int ii;

   assert( mesh );
   assert( Mesh_GetDimSize( mesh ) == 2 );

   grid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   nNodes = Mesh_GetDomainSize( mesh, 0 );
   set = IndexSet_New( nNodes );

/*
   minI = grid->sizes[0] - depth - 1;
   maxI = grid->sizes[0] - 2;
   minJ = depth + 1;
   maxJ = grid->sizes[1] - 2;
*/
   minI = grid->sizes[0] - 1 - inDepth;
   maxI = grid->sizes[0] - 1;
   minJ = lowDepth;
   maxJ = grid->sizes[1] - 1- uppDepth;
   for( ii = 0; ii < nNodes; ii++ ) {
      Grid_Lift( grid, Mesh_DomainToGlobal( mesh, 0, ii ), ijk );
      if( ijk[0] >= minI && ijk[0] <= maxI && ijk[1] >= minJ && ijk[1] <= maxJ )
	 IndexSet_Add( set, ii );
   }

   return set;
}

Node_DomainIndex RegularMeshUtils_GetDiagOppositeAcrossElementNodeIndex( void* _mesh, 
									 Element_DomainIndex refElement_dI, 
									 Node_DomainIndex refNode_dI )
{
	Mesh*              mesh = (Mesh*)_mesh;
	const Node_Index   oppositeNodesMap2D[] = { 3, 2, 1, 0 };
	Node_Index         oppositeNodesMap3D[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
	Node_DomainIndex*  currElementNodes = NULL;
	Node_Index         currElementNodeCount = 0;
	Node_Index         refNode_eI = 0;
	Node_DomainIndex   oppositeNode_dI = 0;
	Node_Index         oppositeNode_eI = 0;
	Stream*            errorStrm = Journal_Register( Error_Type, (Name)"RegularMeshUtils"  );
	IArray*		   inc;

	Journal_Firewall( Mesh_GetElementType( mesh, refElement_dI )->type == Mesh_HexType_Type, errorStrm, 
			  "Error (%s:%s:%d):\n\tIncorrect element type (%s); require %s.\n", 
			  __func__, __FILE__, __LINE__, Mesh_GetElementType( mesh, refElement_dI )->type, 
			  Mesh_HexType_Type );

#if 0
	Journal_Firewall( CornerNL_Type == mesh->layout->nodeLayout->type , errorStr,
		"Error- in %s: Given mesh has node layout of type \"%s\", different to "
		"required type \"%s\".\n", __func__, mesh->layout->nodeLayout->type, CornerNL_Type );
#endif

	inc = IArray_New();
	Mesh_GetIncidence( mesh, Mesh_GetDimSize( mesh ), refElement_dI, MT_VERTEX, 
			   inc );
	currElementNodeCount = IArray_GetSize( inc );
	currElementNodes = IArray_GetPtr( inc );

	/* Find index of reference node within reference element */
	for( refNode_eI = 0; refNode_eI < currElementNodeCount; refNode_eI++ ) {
		if ( refNode_dI == currElementNodes[refNode_eI] )
			break;
	}
	Journal_Firewall( refNode_eI < currElementNodeCount, errorStrm,
		"Error - in %s(): Reference node %d (domain) not found within reference element %d (domain).\n",
		__func__, refNode_dI, refElement_dI );

	/* Use mapping table to get diagonally opposite node, then convert minK to domain index */
	
	if ( Mesh_GetDimSize( mesh ) == 2 ) {
		oppositeNode_eI = oppositeNodesMap2D[refNode_eI];
	}
	else {
		oppositeNode_eI = oppositeNodesMap3D[refNode_eI];
	}

	oppositeNode_dI = currElementNodes[oppositeNode_eI];
	Stg_Class_Delete( inc );
	return oppositeNode_dI;
}

IndexSet* RegularMeshUtils_CreateGlobalMinIMinJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	int nDims;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

        nDims = Mesh_GetDimSize( mesh );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == 0 && ijk[0] == 0 )
			IndexSet_Add( set, n_i );
	}

	return set;
}

IndexSet* RegularMeshUtils_CreateGlobalMaxIMinJSet( void* _mesh ) {
	Mesh*		mesh = (Mesh*)_mesh;
	unsigned	nNodes;
	IndexSet*	set;
	IJK		ijk;
	Grid*	grid;
	int nDims;
	unsigned	n_i;

	assert( mesh );
	assert( Mesh_GetDimSize( mesh ) >= 2 );

	grid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->vertGridId );

        nDims = Mesh_GetDimSize( mesh  );
	nNodes = Mesh_GetDomainSize( mesh, MT_VERTEX );
	set = IndexSet_New( nNodes );

	for( n_i = 0; n_i < nNodes; n_i++ ) {
		RegularMeshUtils_Node_1DTo3D( mesh, Mesh_DomainToGlobal( mesh, MT_VERTEX, n_i ), ijk );
		if( ijk[1] == 0 && ijk[0] == grid->sizes[0] - 1 )
			IndexSet_Add( set, n_i );
	}

	return set;
}
