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

#include "Mesh.h"


/* Textual name of this class */
const Type Mesh_Algorithms_Type = "Mesh_Algorithms";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh_Algorithms* Mesh_Algorithms_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                                   _sizeOfSelf = sizeof(Mesh_Algorithms);
	Type                                                           type = Mesh_Algorithms_Type;
	Stg_Class_DeleteFunction*                                   _delete = _Mesh_Algorithms_Delete;
	Stg_Class_PrintFunction*                                     _print = _Mesh_Algorithms_Print;
	Stg_Class_CopyFunction*                                       _copy = NULL;
	Stg_Component_DefaultConstructorFunction*       _defaultConstructor = (void* (*)(Name))_Mesh_Algorithms_New;
	Stg_Component_ConstructFunction*                         _construct = _Mesh_Algorithms_AssignFromXML;
	Stg_Component_BuildFunction*                                 _build = _Mesh_Algorithms_Build;
	Stg_Component_InitialiseFunction*                       _initialise = _Mesh_Algorithms_Initialise;
	Stg_Component_ExecuteFunction*                             _execute = _Mesh_Algorithms_Execute;
	Stg_Component_DestroyFunction*                             _destroy = _Mesh_Algorithms_Destroy;
	AllocationType                                   nameAllocationType = NON_GLOBAL;
	Mesh_Algorithms_SetMeshFunc*                            setMeshFunc = _Mesh_Algorithms_SetMesh;
	Mesh_Algorithms_UpdateFunc*                              updateFunc = _Mesh_Algorithms_Update;
	Mesh_Algorithms_NearestVertexFunc*                nearestVertexFunc = _Mesh_Algorithms_NearestVertex;
	Mesh_Algorithms_SearchFunc*                              searchFunc = _Mesh_Algorithms_Search;
	Mesh_Algorithms_SearchElementsFunc*              searchElementsFunc = _Mesh_Algorithms_SearchElements;
	Mesh_Algorithms_GetMinimumSeparationFunc*  getMinimumSeparationFunc = _Mesh_Algorithms_GetMinimumSeparation;
	Mesh_Algorithms_GetLocalCoordRangeFunc*      getLocalCoordRangeFunc = _Mesh_Algorithms_GetLocalCoordRange;
	Mesh_Algorithms_GetDomainCoordRangeFunc*    getDomainCoordRangeFunc = _Mesh_Algorithms_GetDomainCoordRange;
	Mesh_Algorithms_GetGlobalCoordRangeFunc*    getGlobalCoordRangeFunc = _Mesh_Algorithms_GetGlobalCoordRange;

	Mesh_Algorithms* self = _Mesh_Algorithms_New(  MESH_ALGORITHMS_PASSARGS  );

	_Mesh_Algorithms_Init( self, context);
   return self;

}

Mesh_Algorithms* _Mesh_Algorithms_New(  MESH_ALGORITHMS_DEFARGS  ) {
	Mesh_Algorithms* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh_Algorithms) );
	self = (Mesh_Algorithms*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	/* Virtual info */
	self->setMeshFunc = setMeshFunc;
	self->updateFunc = updateFunc;
	self->nearestVertexFunc = nearestVertexFunc;
	self->searchFunc = searchFunc;
	self->searchElementsFunc = searchElementsFunc;
	self->getMinimumSeparationFunc = getMinimumSeparationFunc;
	self->getLocalCoordRangeFunc = getLocalCoordRangeFunc;
	self->getDomainCoordRangeFunc = getDomainCoordRangeFunc;
	self->getGlobalCoordRangeFunc = getGlobalCoordRangeFunc;

	return self;
}

void _Mesh_Algorithms_Init( Mesh_Algorithms* self, AbstractContext* context ) {
   self->context = context;
	self->nearestVertex = NULL;
	self->search = NULL;
	self->mesh = NULL;
	self->tree = NULL;
	MPI_Comm_rank( MPI_COMM_WORLD, &self->rank );
	self->incArray = IArray_New();
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_Algorithms_Delete( void* algorithms ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Mesh_Algorithms_Print( void* algorithms, Stream* stream ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	
	/* Print parent */
	Journal_Printf( stream, "Mesh_Algorithms (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void _Mesh_Algorithms_AssignFromXML( void* algorithms, Stg_ComponentFactory* cf, void* data ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
   AbstractContext* context = NULL;

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
	if( !context  )
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, True, data  );
   _Mesh_Algorithms_Init( self, context );
}

void _Mesh_Algorithms_Build( void* algorithms, void* data ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
   Stg_Component_Build( self->mesh, data, False );
 //  Stg_Component_Build( self->tree, data, False );
}

void _Mesh_Algorithms_Initialise( void* algorithms, void* data ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
   Stg_Component_Initialise( self->mesh, data, False );
 //  Stg_Component_Initialise( self->tree, data, False );
}

void _Mesh_Algorithms_Execute( void* algorithms, void* data ) {
}

void _Mesh_Algorithms_Destroy( void* algorithms, void* data ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Stg_Class_Delete( self->incArray );
    if(self->tree) Stg_Component_Destroy( self->tree, data, False );
}

void _Mesh_Algorithms_SetMesh( void* algorithms, void* mesh ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;

	assert( self );
	assert( mesh );

	self->mesh = (Mesh*)mesh;
}

void _Mesh_Algorithms_Update( void* algorithms ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	int nDims, d_i;

	assert( self );

#if 0
	if( !self->mesh ) {
	   if( self->tree )
	      SpatialTree_Clear( self->tree );
	   return;
	}

	if( !self->tree )
	   self->tree = SpatialTree_New();
	SpatialTree_SetMesh( self->tree, self->mesh );
	SpatialTree_Rebuild( self->tree );
	self->search = Mesh_Algorithms_SearchWithTree;
#endif

	if( Mesh_HasIncidence( self->mesh, MT_VERTEX, MT_VERTEX ) )
	{
		self->nearestVertex = Mesh_Algorithms_NearestVertexWithNeighbours;
	}
	else
		self->nearestVertex = Mesh_Algorithms_NearestVertexGeneral;

	nDims = Mesh_GetDimSize( self->mesh );
	for( d_i = 0; d_i < nDims; d_i++ ) {
	   if( (!Mesh_GetGlobalSize( self->mesh, d_i ) || !Mesh_HasIncidence( self->mesh, nDims, d_i )) )
	   {
			break;
	   }
	}
	if( d_i == nDims )
		self->search = Mesh_Algorithms_SearchWithFullIncidence;
	else if( Mesh_HasIncidence( self->mesh, MT_VERTEX, Mesh_GetDimSize( self->mesh ) ) )
		self->search = Mesh_Algorithms_SearchWithMinIncidence;
	else
		self->search = Mesh_Algorithms_SearchGeneral;
}

unsigned _Mesh_Algorithms_NearestVertex( void* algorithms, double* point ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;

	assert( self );
	assert( self->nearestVertex );

	return self->nearestVertex( self, point );
}

Bool _Mesh_Algorithms_Search( void* algorithms, double* point, 
			      MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;

	assert( self );
	assert( self->search );

	return self->search( self, point, dim, ind );
}

Bool _Mesh_Algorithms_SearchElements( void* algorithms, double* point, 
				      unsigned* elInd )
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		dim, ind;

	assert( self );
	assert( self->mesh );
	assert( elInd );

	mesh = self->mesh;
	if( Mesh_Algorithms_Search( self, point, &dim, &ind ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( mesh );
		if( dim != nDims ) {
			unsigned	nInc, *inc;
			unsigned	lowest;
			unsigned	global;
			unsigned	inc_i;

			/* Must have required incidence for this to work. */
			assert( Mesh_HasIncidence( mesh, dim, nDims ) );

			Mesh_GetIncidence( mesh, dim, ind, nDims, self->incArray );
			nInc = IArray_GetSize( self->incArray );
			inc = (unsigned*)IArray_GetPtr( self->incArray );
			assert( nInc );
			lowest = Mesh_DomainToGlobal( mesh, nDims, inc[0] );
			for( inc_i = 1; inc_i < nInc; inc_i++ ) {
				global = Mesh_DomainToGlobal( mesh, nDims, inc[inc_i] );
				if( global < lowest )
					lowest = global;
			}

			insist( Mesh_GlobalToDomain( mesh, nDims, lowest, elInd), == True );
		}
		else
			*elInd = ind;

		return True;
	}

	return False;
}

double _Mesh_Algorithms_GetMinimumSeparation( void* algorithms, double* perDim ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDomainEls;
	double			minSep;
	double*			dimSep;
	unsigned		e_i;

	assert( self );
	assert( self->mesh );

	mesh = self->mesh;
	if( perDim )
		dimSep = Memory_Alloc_Array_Unnamed( double, Mesh_GetDimSize( mesh ) );
	else
		dimSep = NULL;

	minSep = HUGE_VAL;
	nDomainEls = Mesh_GetDomainSize( mesh, Mesh_GetDimSize( mesh ) );
	for( e_i = 0; e_i < nDomainEls; e_i++ ) {
		Mesh_ElementType*	elType;
		double			curSep;

		elType = Mesh_GetElementType( mesh, e_i );
		curSep = Mesh_ElementType_GetMinimumSeparation( elType, e_i, dimSep );
		if( curSep < minSep ) {
			minSep = curSep;
			if( perDim )
				memcpy( perDim, dimSep, Mesh_GetDimSize( mesh ) * sizeof(double) );
		}
	}

   if( dimSep ) Memory_Free( dimSep );

	return minSep;
}

void _Mesh_Algorithms_GetLocalCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nVerts, nEls;
	unsigned*		verts;
	double*			vert;
	unsigned		nDims;
	unsigned		v_i, e_i, d_i;

	assert( self );
	assert( self->mesh );
	assert( min );
	assert( max );

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	nEls = Mesh_GetLocalSize( mesh, nDims );
	memcpy( min, Mesh_GetVertex( mesh, 0 ), nDims * sizeof(double) );
	memcpy( max, Mesh_GetVertex( mesh, 0 ), nDims * sizeof(double) );
	for( e_i = 0; e_i < nEls; e_i++ ) {
		Mesh_GetIncidence( mesh, nDims, e_i, 0, self->incArray );
		nVerts = IArray_GetSize( self->incArray );
		verts = (unsigned*)IArray_GetPtr( self->incArray );
		for( v_i = 0; v_i < nVerts; v_i++ ) {
			vert = Mesh_GetVertex( mesh, verts[v_i] );
			for( d_i = 0; d_i < nDims; d_i++ ) {
				if( vert[d_i] < min[d_i] )
					min[d_i] = vert[d_i];
				if( vert[d_i] > max[d_i] )
					max[d_i] = vert[d_i];
			}
		}
	}
}

void _Mesh_Algorithms_GetDomainCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nVerts;
	double*			vert;
	unsigned		nDims;
	unsigned		v_i, d_i;

	assert( self );
	assert( self->mesh );
	assert( min );
	assert( max );

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	nVerts = Mesh_GetDomainSize( mesh, MT_VERTEX );
	memcpy( min, Mesh_GetVertex( mesh, 0 ), nDims * sizeof(double) );
	memcpy( max, Mesh_GetVertex( mesh, 0 ), nDims * sizeof(double) );
	for( v_i = 1; v_i < nVerts; v_i++ ) {
		vert = Mesh_GetVertex( mesh, v_i );
		for( d_i = 0; d_i < nDims; d_i++ ) {
			if( vert[d_i] < min[d_i] )
				min[d_i] = vert[d_i];
			if( vert[d_i] > max[d_i] )
				max[d_i] = vert[d_i];
		}
	}
}

void _Mesh_Algorithms_GetGlobalCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDims;
	double			*localMin, *localMax;
	MPI_Comm		comm;
	unsigned		d_i;

	assert( self );
	assert( self->mesh );
	assert( min );
	assert( max );

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	localMin = Memory_Alloc_Array_Unnamed( double, nDims );
	localMax = Memory_Alloc_Array_Unnamed( double, nDims );

	comm = Comm_GetMPIComm( Mesh_GetCommTopology( mesh, MT_VERTEX ) );
	Mesh_Algorithms_GetLocalCoordRange( self, localMin, localMax );
	for( d_i = 0; d_i < Mesh_GetDimSize( mesh ); d_i++ ) {
		MPI_Allreduce( localMin + d_i, min + d_i, 1, MPI_DOUBLE, MPI_MIN, comm );
		MPI_Allreduce( localMax + d_i, max + d_i, 1, MPI_DOUBLE, MPI_MAX, comm );
	}

	FreeArray( localMin );
	FreeArray( localMax );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

#define Vec_Sep( nDims, v0, v1 )									\
	(((v0)[0] - (v1)[0]) * ((v0)[0] - (v1)[0]) +							\
	 (((nDims) >= 2) ? (((v0)[1] - (v1)[1]) * ((v0)[1] - (v1)[1]) + 				\
			    (((nDims) == 3) ? (((v0)[2] - (v1)[2]) * ((v0)[2] - (v1)[2])) : 0)) : 0))

unsigned Mesh_Algorithms_NearestVertexWithNeighbours( void* algorithms, double* point ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDims;
	unsigned		curVert;
	double*			vert;
	double			sep;
	Bool			done;
	unsigned		nNbrs;
	unsigned*		nbrs;
	double			nbrSep;
	unsigned		nbr_i;

	assert( self );
	assert( self->mesh );
	assert( Mesh_HasIncidence( self->mesh, MT_VERTEX, MT_VERTEX ) );

	/* Get dimensionality. */
	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );

	/* Begin somewhere in the middle. */
	curVert = Mesh_GetDomainSize( mesh, MT_VERTEX ) / 2;

	/* Calc distance squared to current node. */
	vert = Mesh_GetVertex( mesh, curVert );
	sep = Vec_Sep( nDims, vert, point );

	/* Loop until we've found closest local node. */
	do {
		/* Get neighbouring vertices. */
		Mesh_GetIncidence( mesh, MT_VERTEX, curVert, MT_VERTEX, self->incArray );
		nNbrs = IArray_GetSize( self->incArray );
		nbrs = (unsigned*)IArray_GetPtr( self->incArray );

		/* Assume we'll be done after this loop. */
		done = True;

		/* Compare to neighbours. */
		for( nbr_i = 0; nbr_i < nNbrs; nbr_i++ ) {
			/* Calculate neighbour separation. */
			nbrSep = Vec_Sep( nDims, Mesh_GetVertex( mesh, nbrs[nbr_i] ), point );

			/* Closer? */
			if( nbrSep < sep ) {
				curVert = nbrs[nbr_i];
				sep = nbrSep;
				done = False;
			}
		}
	}
	while( !done );

	return curVert;
}

unsigned Mesh_Algorithms_NearestVertexGeneral( void* algorithms, double* point ) {
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDims;
	unsigned		nDomainVerts;
	double*			vert;
	unsigned		minVertInd;
	double			curSep, minSep;
	unsigned		v_i;

	assert( self );
	assert( self->mesh );
	assert( Mesh_GetDomainSize( self->mesh, MT_VERTEX ) );

	/* TODO: This is going to be hella slow, need to use some kind of spatial partitioning scheme. */

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	nDomainVerts = Mesh_GetDomainSize( mesh, MT_VERTEX );

	vert = Mesh_GetVertex( mesh, 0 );
	minSep = Vec_Sep( nDims, vert, point );
	minVertInd = 0;

	for( v_i = 1; v_i < nDomainVerts; v_i++ ) {
		vert = Mesh_GetVertex( mesh, v_i );
		curSep = Vec_Sep( nDims, vert, point );
		if( curSep < minSep ) {
			minSep = curSep;
			minVertInd = v_i;
		}
	}

	return minVertInd;
}

Bool Mesh_Algorithms_SearchIncidentRecursive( void* algorithms, unsigned vert, double* point, 
					      MeshTopology_Dim* dim, unsigned* ind, Bool recurse)
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDims;
	unsigned		nInc, *inc;
	unsigned		inc_i;
  	IArray*     vArray; 
   unsigned		nVinc, *vInc, v_i;
  	unsigned		el_i;

	/* Get dimensionality. */
	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );

	/* Get vertex/element incidence. */
	Mesh_GetIncidence( mesh, MT_VERTEX, vert, nDims, self->incArray );
	nInc = IArray_GetSize( self->incArray );
	inc = (unsigned*)IArray_GetPtr( self->incArray );

	/* Search each of these incident elements in turn. */
	for( inc_i = 0; inc_i < nInc; inc_i++ ) 
		if( Mesh_ElementHasPoint( mesh, inc[inc_i], point, dim, ind ) )
			return True;

   /* If still not found go on to search at next level. */
   if (recurse)
   {
      vArray = IArray_New();

      for( el_i = 0; el_i < nInc; el_i++ ) {
         /* Get incident vertices to each incident element */
         Mesh_GetIncidence( mesh, nDims, inc[el_i], MT_VERTEX, vArray );
         nVinc = IArray_GetSize( vArray );
         vInc = (unsigned*)IArray_GetPtr( vArray );
         for( v_i = 0; v_i < nVinc; v_i++ ) {
            /* Search over each vertex in increasing levels until max reached */
            if (vInc[v_i] == vert) continue;
            if (Mesh_Algorithms_SearchIncidentRecursive(algorithms, vInc[v_i], point, dim, ind, False))
            {
               Stg_Class_Delete( vArray );
               return True;
            }
         }
      }

      Stg_Class_Delete( vArray );
   }

   return False;
}

Bool Mesh_Algorithms_SearchWithFullIncidence( void* algorithms, double* point, 
					      MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	double		maxCrd[3], minCrd[3];
	unsigned		nDims;
	unsigned		nEls;
	unsigned		nearVert;
	unsigned		e_i, d_i;

	assert( self );
	assert( self->mesh );
	assert( Mesh_HasIncidence( self->mesh, MT_VERTEX, Mesh_GetDimSize( self->mesh ) ) );
	assert( dim );
	assert( ind );

	/* Get dimensionality. */
	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );

	/* If outside local range, immediately return false. */
	Mesh_GetDomainCoordRange( mesh, minCrd, maxCrd );
	for( d_i = 0; d_i < nDims; d_i++ ) {
		if( point[d_i] < minCrd[d_i] || point[d_i] > maxCrd[d_i] )
			return False;
	}

	/* Start by locating the closest vertex. */
	nearVert = Mesh_NearestVertex( mesh, point );

   /* Search the elements incident to this vertex, 
    * if still not found search another level of vertices */
   if (Mesh_Algorithms_SearchIncidentRecursive(algorithms, nearVert, point, dim, ind, True))
      return True;

	/* Brute force, search every element in turn (last resort). */
	nEls = Mesh_GetDomainSize( mesh, nDims );
	for( e_i = 0; e_i < nEls; e_i++ ) {
		if( Mesh_ElementHasPoint( mesh, e_i, point, dim, ind ) )
			return True;
	}

	return False;
}

Bool Mesh_Algorithms_SearchWithMinIncidence( void* algorithms, double* point, 
					      MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	double			maxCrd[3], minCrd[3];
	unsigned		lowest;
	unsigned		nDims;
	unsigned		nEls;
	unsigned		nearVert;
	unsigned		nInc, *inc;
	unsigned		e_i, d_i, inc_i;

	assert( self );
	assert( self->mesh );
	assert( Mesh_HasIncidence( self->mesh, MT_VERTEX, Mesh_GetDimSize( self->mesh ) ) );
	assert( dim );
	assert( ind );

	/* Get dimensionality. */
	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );

	/* If outside local range, immediately return false. */
	Mesh_GetDomainCoordRange( mesh, minCrd, maxCrd );
	for( d_i = 0; d_i < nDims; d_i++ ) {
		if( point[d_i] < minCrd[d_i] || point[d_i] > maxCrd[d_i] )
			return False;
	}

	/* Start by locating the closest vertex. */
	nearVert = Mesh_NearestVertex( mesh, point );

	/* Get vertex/element incidence. */
	Mesh_GetIncidence( mesh, MT_VERTEX, nearVert, nDims, self->incArray );
	nInc = IArray_GetSize( self->incArray );
	inc = (unsigned*)IArray_GetPtr( self->incArray );

	/* Search all of these elements and return the element with lowest global index. */
	lowest = (unsigned)-1;
	for( inc_i = 0; inc_i < nInc; inc_i++ ) {
		if( Mesh_ElementHasPoint( mesh, inc[inc_i], point, dim, ind ) ) {
			unsigned	global;

			global = Mesh_DomainToGlobal( mesh, nDims, inc[inc_i] );
			if( global < lowest )
				lowest = global;
		}
	}
	if( lowest != (unsigned)-1 ) {
		insist( Mesh_GlobalToDomain( mesh, nDims, lowest, ind ), == True );
		*dim = nDims;
		return True;
	}

	/* Brute force, search every element in turn (last resort). */
	lowest = (unsigned)-1;
	nEls = Mesh_GetDomainSize( mesh, nDims );
	for( e_i = 0; e_i < nEls; e_i++ ) {
		if( Mesh_ElementHasPoint( mesh, e_i, point, dim, ind ) ) {
			unsigned	global;

			global = Mesh_DomainToGlobal( mesh, nDims, e_i );
			if( global < lowest )
				lowest = global;
		}
	}
	if( lowest != (unsigned)-1 ) {
		insist( Mesh_GlobalToDomain( mesh, nDims, lowest, ind ), == True );
		*dim = nDims;
		return True;
	}

	return False;
}

Bool Mesh_Algorithms_SearchGeneral( void* algorithms, double* point, 
				    MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_Algorithms*	self = (Mesh_Algorithms*)algorithms;
	Mesh*			mesh;
	double			maxCrd[3], minCrd[3];
	unsigned		nDims;
	unsigned		nEls;
	unsigned		e_i, d_i;

	assert( self );
	assert( self->mesh );
	assert( dim );
	assert( ind );

	/* Get dimensionality. */
	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );

	/* If outside local range, immediately return false. */
	Mesh_GetDomainCoordRange( mesh, minCrd, maxCrd );
	for( d_i = 0; d_i < nDims; d_i++ ) {
		if( point[d_i] < minCrd[d_i] || point[d_i] > maxCrd[d_i] )
			return False;
	}

	/* Brute force, search every element in turn. */
	nEls = Mesh_GetDomainSize( mesh, nDims );
	for( e_i = 0; e_i < nEls; e_i++ ) {
		if( Mesh_ElementHasPoint( mesh, e_i, point, dim, ind ) )
			return True;
	}

	return False;
}

Bool Mesh_Algorithms_SearchWithTree( void* _self, double* pnt, unsigned* dim, unsigned* el ) {
   Mesh_Algorithms* self = (Mesh_Algorithms*)_self;
   int nEls, *els;
   unsigned curDim, curEl;
	int curRank;
   int nLocals, owner;
   int ii;

   *dim = Mesh_GetDimSize( self->mesh );
   MPI_Comm_size( MPI_COMM_WORLD, &curRank );
   nLocals = Mesh_GetLocalSize( self->mesh, *dim );
   if( !SpatialTree_Search( self->tree, pnt, &nEls, &els ) )
      return False;

   *el = nLocals;
   for( ii = 0; ii < nEls; ii++ ) {
      if( Mesh_ElementHasPoint( self->mesh, els[ii], pnt, &curDim, &curEl ) ) {
	 if( curEl >= nLocals ) {
	    owner = Mesh_GetOwner( self->mesh, curDim, curEl - nLocals );
	    owner = Comm_RankLocalToGlobal( self->mesh->topo->comm, owner );
	    if( owner <= curRank ) {
	       curRank = owner;
	       *el = curEl;
	    }
	 }
	 else if( self->rank <= curRank && curEl < *el ) {
	    curRank = self->rank;
	    *el = curEl;
	 }
      }
   }

   return True;
}
/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


