/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include "types.h"
#include "Mesh.h"
#include "SpatialTree.h"


#define GETNODECHILDREN( tree, node )		\
  ((void**)(node))

#define GETNODEORIGIN( tree, node )		\
   ((double*)((stgByte*)(node) +		\
	      (tree)->nChld * sizeof(void*)))

#define SETNODEVERTS( tree, node, nVerts, verts )		\
   ((int*)((stgByte*)(node) + (tree)->nChld * sizeof(void*) +	\
	   (tree)->nDims * sizeof(double)))[0] = (nVerts);	\
   ((int**)((stgByte*)(node) + (tree)->nChld * sizeof(void*) +	\
	    (tree)->nDims * sizeof(double) +			\
	    sizeof(int)))[0] = (verts)

#define GETNODEVERTARRAY( tree, node )				\
   ((int**)((stgByte*)(node) + (tree)->nChld * sizeof(void*) +	\
	    (tree)->nDims * sizeof(double) +			\
	    sizeof(int)))[0]

#define GETNODENUMVERTS( tree, node )				\
   ((int*)((stgByte*)(node) + (tree)->nChld * sizeof(void*) +	\
	   (tree)->nDims * sizeof(double)))[0]


const Type SpatialTree_Type = "SpatialTree";


void SpatialTree_DestroyNode( SpatialTree* self, void* node );
void SpatialTree_SplitSet( SpatialTree* self, double* orig,
			   int nVerts, int* verts, 
			   int* subSizes, int** subSets,
			   Bool store );
void SpatialTree_SplitNode( SpatialTree* self, void* node, void** parent,
			    double* min, double* max,
			    int nVerts, int* verts );
void SpatialTree_BuildElements( SpatialTree* self, int nVerts, int* verts, 
				int* nEls, int** els );
void SpatialTree_SearchNode( SpatialTree* self, void* node, 
			     const double* pnt, int* nEls, int** els );


SpatialTree* SpatialTree_New() {
    SpatialTree* self;
    SizeT _sizeOfSelf = sizeof(SpatialTree);
    Type type = SpatialTree_Type;
    Stg_Class_DeleteFunction* _delete = _SpatialTree_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _SpatialTree_New( SPATIALTREE_PASSARGS );
    return self;
}

SpatialTree* _SpatialTree_New( SPATIALTREE_DEFARGS ) {
    SpatialTree* self;

    self = (SpatialTree*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _SpatialTree_Init( self );
    return self;
}

void SpatialTree_Init( SpatialTree* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _SpatialTree_Init( self );
}

void _SpatialTree_Init( void* _self ) {
   SpatialTree* self = (SpatialTree*)_self;

   self->mesh = NULL;
   self->nDims = 0;
   self->min = NULL;
   self->max = NULL;
   self->root = NULL;
   self->tol = 10;
   self->nNodes = 0;
}

void SpatialTree_Destruct( SpatialTree* self ) {
   if( self->root )
      SpatialTree_DestroyNode( self, self->root );
}

void _SpatialTree_Delete( void* _self ) {
   SpatialTree* self = (SpatialTree*)_self;

   SpatialTree_Destruct( self );
   Stg_Class_Delete( self );
}

void SpatialTree_Copy( void* _self, const void* _op ) {
   /*SpatialTree* self = (SpatialTree*)_self;*/
   /*const SpatialTree* op = Class_ConstCast( _op, SpatialTree );*/

   abort();
}

void SpatialTree_SetMesh( void* _self, void* mesh ) {
   SpatialTree* self = (SpatialTree*)_self;

   SpatialTree_Clear( self );
   self->mesh = mesh;
}

void SpatialTree_Rebuild( void* _self ) {
   SpatialTree* self = (SpatialTree*)_self;
   int nVerts, *verts;
   int ii;

   if( !self->mesh )
      return;

   SpatialTree_Clear( self );
   self->nDims = Mesh_GetDimSize( self->mesh );
   self->nChld = 2;
   for( ii = 1; ii < self->nDims; ii++ )
      self->nChld *= 2;

   self->min = AllocArray( double, self->nDims );
   self->max = AllocArray( double, self->nDims );

   Mesh_GetDomainCoordRange( self->mesh, self->min, self->max );
   nVerts = Mesh_GetDomainSize( self->mesh, 0 );
   verts = AllocArray( int, nVerts );
   for( ii = 0; ii < nVerts; ii++ )
      verts[ii] = ii;
   self->root = AllocArray( stgByte, self->nChld * sizeof(void*) + 
			     self->nDims * sizeof(double) );
   SpatialTree_SplitNode( self, self->root, &self->root,
			  self->min, self->max, nVerts, verts );
}

Bool SpatialTree_Search( void* _self, const double* pnt, int* nEls, int** els ) {
   SpatialTree* self = (SpatialTree*)_self;
   int ii;

   for( ii = 0; ii < self->nDims; ii++ ) {
      if( pnt[ii] < self->min[ii] || pnt[ii] > self->max[ii] )
	 return False;
   }

   SpatialTree_SearchNode( self, self->root, pnt, nEls, els );
   return True;
}

void SpatialTree_Clear( void* _self ) {
   SpatialTree* self = (SpatialTree*)_self;

   if( self->root ) {
      SpatialTree_DestroyNode( self, self->root );
      self->root = NULL;
      self->nNodes = 0;
   }


   FreeArray( self->min ); self->min = NULL;
   FreeArray( self->max ); self->max = NULL;
}

void SpatialTree_SplitNode( SpatialTree* self, void* node, void** parent,
			    double* min, double* max,
			    int nVerts, int* verts )
{
   int ii;

   for( ii = 0; ii < self->nDims; ii++ )
      GETNODEORIGIN( self, node )[ii] = min[ii] + (max[ii] - min[ii]) * 0.5;

   if( nVerts <= self->tol ) {
      int nEls, *els;

      node = (void*)ReallocArray( node, stgByte,
				   self->nChld * sizeof(void*) + 
				   self->nDims * sizeof(double) + 
				   sizeof(int) + sizeof(int*) );
      *parent = node;
      memset( node, 0, self->nChld * sizeof(void*) );

      SpatialTree_BuildElements( self, nVerts, verts, 
				 &nEls, &els );
      FreeArray( verts );
      SETNODEVERTS( self, node, nEls, els );
   }
   else {
      void *newNode, *newNodePtr;
      int* subSizes;
      int** subSets;
      double *subMin, *subMax;
      int jj;

      for( ii = 0; ii < self->nChld; ii++ ) {
	 newNode = AllocArray( stgByte, self->nChld * sizeof(void*) + 
				self->nDims * sizeof(double) );
	 GETNODECHILDREN( self, node )[ii] = newNode;
      }

      subSizes = AllocArray( int, self->nChld );
      SpatialTree_SplitSet( self, GETNODEORIGIN( self, node ),
			    nVerts, verts, subSizes, NULL, False );

      subSets = AllocArray( int*, self->nChld );
      for( ii = 0; ii < self->nChld; ii++ )
	 subSets[ii] = AllocArray( int, subSizes[ii] );
      SpatialTree_SplitSet( self, GETNODEORIGIN( self, node ),
			    nVerts, verts, subSizes, subSets, True );

      FreeArray( verts );

      subMin = AllocArray( double, self->nDims );
      subMax = AllocArray( double, self->nDims );

      for( ii = 0; ii < self->nChld; ii++ ) {
	 for( jj = 0; jj < self->nDims; jj++ ) {
	    if( ii & (1 << jj) ) {
	       subMin[jj] = GETNODEORIGIN( self, node )[jj];
	       subMax[jj] = max[jj];
	    }
	    else {
	       subMin[jj] = min[jj];
	       subMax[jj] = GETNODEORIGIN( self, node )[jj];
	    }
	 }

	 newNode = GETNODECHILDREN( self, node )[ii];
	 newNodePtr = GETNODECHILDREN( self, node ) + ii;
	 SpatialTree_SplitNode( self, newNode, newNodePtr, subMin, subMax,
				subSizes[ii], subSets[ii] );
      }

      FreeArray( subMin );
      FreeArray( subMax );
   }
}

void SpatialTree_SplitSet( SpatialTree* self, double* orig,
			   int nVerts, int* verts, 
			   int* subSizes, int** subSets,
			   Bool store )
{
   double* crd;
   int code;
   int ii, jj;

   memset( subSizes, 0, self->nChld * sizeof(int) );
   for( ii = 0; ii < nVerts; ii++ ) {
      crd = Mesh_GetVertex( self->mesh, verts[ii] );
      code = 0;
      for( jj = 0; jj < self->nDims; jj++ ) {
	 if( crd[jj] > orig[jj] )
	    code |= 1 << jj;
      }

      if( store )
	 subSets[code][subSizes[code]++] = verts[ii];
      else
	 subSizes[code]++;
   }
}

void SpatialTree_BuildElements( SpatialTree* self, int nVerts, int* verts, 
				int* nEls, int** els )
{
   int maxEls, *curEls;
   IArray* inc;
   int nIncEls, *incEls;
   int ii, jj, kk;

   maxEls = 0;
   for( ii = 0; ii < nVerts; ii++ )
      maxEls += Mesh_GetIncidenceSize( self->mesh, 0, verts[ii], self->nDims );

   curEls = AllocArray( int, maxEls );

   inc = IArray_New();
   maxEls = 0;
   for( ii = 0; ii < nVerts; ii++ ) {
      Mesh_GetIncidence( self->mesh, 0, verts[ii], self->nDims, inc );
      nIncEls = IArray_GetSize( inc );
      incEls = IArray_GetPtr( inc );
      for( jj = 0; jj < nIncEls; jj++ ) {
	 for( kk = 0; kk < maxEls; kk++ ) {
	    if( curEls[kk] == incEls[jj] )
	       break;
	 }
	 if( kk == maxEls )
	    curEls[maxEls++] = incEls[jj];
      }
   }
   Stg_Class_Delete( inc );

   *nEls = maxEls;
   *els = ReallocArray( curEls, int, maxEls );
}

void SpatialTree_SearchNode( SpatialTree* self, void* node, 
			     const double* pnt, int* nEls, int** els )
{
   if( GETNODECHILDREN( self, node )[0] == NULL ) {
      *nEls = GETNODENUMVERTS( self, node );
      *els = GETNODEVERTARRAY( self, node );
   }
   else {
      double* orig;
      int code;
      int ii;

      orig = GETNODEORIGIN( self, node );
      code = 0;
      for( ii = 0; ii < self->nDims; ii++ ) {
	 if( pnt[ii] > orig[ii] )
	    code |= 1 << ii;
      }

      SpatialTree_SearchNode( self, GETNODECHILDREN( self, node )[code], 
			      pnt, nEls, els );
   }
}

void SpatialTree_DestroyNode( SpatialTree* self, void* node ) {
   Bool leaf;
   int ii;

   if( !node )
      return;

   leaf = True;
   for( ii = 0; ii < self->nChld; ii++ ) {
      if( GETNODECHILDREN( self, node )[ii] )
	 leaf = False;
      SpatialTree_DestroyNode( self, GETNODECHILDREN( self, node )[ii] );
   }

   if( leaf ) {
      FreeArray( GETNODEVERTARRAY( self, node ) );
   }

   FreeArray( node );
}


