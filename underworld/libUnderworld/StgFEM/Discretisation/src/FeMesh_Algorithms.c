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

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "Discretisation.h"


/* Textual name of this class */
const Type FeMesh_Algorithms_Type = "FeMesh_Algorithms";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

FeMesh_Algorithms* FeMesh_Algorithms_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                                   _sizeOfSelf = sizeof(FeMesh_Algorithms);
	Type                                                           type = FeMesh_Algorithms_Type;
	Stg_Class_DeleteFunction*                                   _delete = _FeMesh_Algorithms_Delete;
	Stg_Class_PrintFunction*                                     _print = _FeMesh_Algorithms_Print;
	Stg_Class_CopyFunction*                                       _copy = NULL;
	Stg_Component_DefaultConstructorFunction*       _defaultConstructor = (void* (*)(Name))_FeMesh_Algorithms_New;
	Stg_Component_ConstructFunction*                         _construct = _FeMesh_Algorithms_AssignFromXML;
	Stg_Component_BuildFunction*                                 _build = _FeMesh_Algorithms_Build;
	Stg_Component_InitialiseFunction*                       _initialise = _FeMesh_Algorithms_Initialise;
	Stg_Component_ExecuteFunction*                             _execute = _FeMesh_Algorithms_Execute;
	Stg_Component_DestroyFunction*                             _destroy = _FeMesh_Algorithms_Destroy;
	AllocationType                                   nameAllocationType = NON_GLOBAL;
	Mesh_Algorithms_SetMeshFunc*                            setMeshFunc = _Mesh_Algorithms_SetMesh;
	Mesh_Algorithms_UpdateFunc*                              updateFunc = _Mesh_Algorithms_Update;
	Mesh_Algorithms_NearestVertexFunc*                nearestVertexFunc = _Mesh_Algorithms_NearestVertex;
	Mesh_Algorithms_SearchFunc*                              searchFunc = _FeMesh_Algorithms_Search;
	Mesh_Algorithms_SearchElementsFunc*              searchElementsFunc = _FeMesh_Algorithms_SearchElements;
	Mesh_Algorithms_GetMinimumSeparationFunc*  getMinimumSeparationFunc = _Mesh_Algorithms_GetMinimumSeparation;
	Mesh_Algorithms_GetLocalCoordRangeFunc*      getLocalCoordRangeFunc = _Mesh_Algorithms_GetLocalCoordRange;
	Mesh_Algorithms_GetDomainCoordRangeFunc*    getDomainCoordRangeFunc = _Mesh_Algorithms_GetDomainCoordRange;
	Mesh_Algorithms_GetGlobalCoordRangeFunc*    getGlobalCoordRangeFunc = _Mesh_Algorithms_GetGlobalCoordRange;

   FeMesh_Algorithms* self =  _FeMesh_Algorithms_New(  FEMESH_ALGORITHMS_PASSARGS  );

   _Mesh_Algorithms_Init( (Mesh_Algorithms*)self, context );
	_FeMesh_Algorithms_Init( self );
   return self;
}

FeMesh_Algorithms* _FeMesh_Algorithms_New(  FEMESH_ALGORITHMS_DEFARGS  ) {
	FeMesh_Algorithms* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(FeMesh_Algorithms) );
	self = (FeMesh_Algorithms*)_Mesh_Algorithms_New(  MESH_ALGORITHMS_PASSARGS  );

	return self;
}

void _FeMesh_Algorithms_Init( FeMesh_Algorithms* self ) {
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _FeMesh_Algorithms_Delete( void* algorithms ) {
	FeMesh_Algorithms*	self = (FeMesh_Algorithms*)algorithms;

	/* Delete the parent. */
	_Mesh_Algorithms_Delete( self );
}

void _FeMesh_Algorithms_Print( void* algorithms, Stream* stream ) {
	FeMesh_Algorithms*	self = (FeMesh_Algorithms*)algorithms;
	
	/* Set the Journal for printing informations */
	Stream* algorithmsStream;
	algorithmsStream = Journal_Register( InfoStream_Type, (Name)"FeMesh_AlgorithmsStream"  );

	/* Print parent */
	Journal_Printf( stream, "FeMesh_Algorithms (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void _FeMesh_Algorithms_AssignFromXML( void* algorithms, Stg_ComponentFactory* cf, void* data ) {
   _FeMesh_Algorithms_Init( algorithms );
}

void _FeMesh_Algorithms_Build( void* algorithms, void* data ) {
   FeMesh_Algorithms* self = (FeMesh_Algorithms*)algorithms;
   _Mesh_Algorithms_Build( self, data );
}

void _FeMesh_Algorithms_Initialise( void* algorithms, void* data ) {
   FeMesh_Algorithms* self = (FeMesh_Algorithms*)algorithms;
   _Mesh_Algorithms_Initialise( self, data );
}

void _FeMesh_Algorithms_Execute( void* algorithms, void* data ) {
}

void _FeMesh_Algorithms_Destroy( void* algorithms, void* data ) {
   FeMesh_Algorithms* self = (FeMesh_Algorithms*)algorithms;
   _Mesh_Algorithms_Destroy( self, data );
}

Bool _FeMesh_Algorithms_Search( void* algorithms, double* point, 
			      MeshTopology_Dim* dim, unsigned* ind )
{
	FeMesh_Algorithms*	self = (FeMesh_Algorithms*)algorithms;

	return FeMesh_Algorithms_SearchWithTree( self, point, dim, ind );
}

Bool _FeMesh_Algorithms_SearchElements( void* algorithms, double* point, 
				      unsigned* elInd )
{
	FeMesh_Algorithms*	self = (FeMesh_Algorithms*)algorithms;
	unsigned		dim;

	assert( self );

	return FeMesh_Algorithms_SearchWithTree( self, point, &dim, elInd );
}



/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

/*
 * Did I write this function?  What the hell does it do?  Damn me.
 *  - Luke.
 */

Bool FeMesh_Algorithms_SearchWithTree( void* _self, double* pnt, unsigned* dim, unsigned* el ) {
   FeMesh_Algorithms* self = (FeMesh_Algorithms*)_self;
   int nEls, *els;
   int curRank, ii;
   unsigned curDim, curEl;
   int nLocals, owner;
   Mesh_ElementType* elType;

   *dim = Mesh_GetDimSize( self->mesh );
   MPI_Comm_size( MPI_COMM_WORLD, &curRank );
   nLocals = Mesh_GetLocalSize( self->mesh, *dim );
   if( !SpatialTree_Search( self->tree, pnt, &nEls, &els ) )
      return False;

   *el = nLocals;
   elType = Mesh_GetElementType( self->mesh, 0 );
   for( ii = 0; ii < nEls; ii++ ) {
      if( FeMesh_ElementType_ElementHasPoint( elType, els[ii], pnt, &curDim, &curEl ) ) {
         if( curEl >= nLocals ) {
            owner = Mesh_GetOwner( self->mesh, curDim, curEl - nLocals );
            owner = Comm_RankLocalToGlobal( self->mesh->topo->comm, owner );
            if( owner <= curRank ) {
               curRank = owner;
               *el = curEl;
            }
         } else if( self->rank <= curRank && curEl < *el ) {
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


