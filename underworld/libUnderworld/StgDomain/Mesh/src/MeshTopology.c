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
#include "StGermain/libStGermain/src/StGermain.h"
#include "types.h"
#include "Decomp.h"
#include "Sync.h"
#include "MeshTopology.h"


const Type MeshTopology_Type = "MeshTopology";


MeshTopology* _MeshTopology_New( MESHTOPOLOGY_DEFARGS ) {
    MeshTopology* self;

    self = (MeshTopology*)_Stg_Class_New( STG_CLASS_PASSARGS );
    self->setCommFunc = setCommFunc;
    self->setNumDimsFunc = setNumDimsFunc;
    self->setShadowDepthFunc = setShadowDepthFunc;
    self->getNumCellsFunc = getNumCellsFunc;
    self->getIncidenceFunc = getIncidenceFunc;
    _MeshTopology_Init( self );
    return self;
}

void _MeshTopology_Init( void* _self ) {
   MeshTopology* self = (MeshTopology*)_self;

   self->nDims = 0;
   self->nTDims = 0;
   self->shadDepth = 0;
   self->comm = NULL;
}

void MeshTopology_Destruct( MeshTopology* self ) {
}

void _MeshTopology_Delete( void* _self ) {
   MeshTopology* self = (MeshTopology*)_self;

   _Stg_Class_Delete( self );
}

void MeshTopology_Copy( void* _self, const void* op ) {
   /*MeshTopology* self = (MeshTopology*)_self;*/

   assert( 0 );
   /* TODO */
}

/*
SizeT _MeshTopology_CalcMem( const void* _self, PtrMap* ptrs ) {
   MeshTopology* self = (MeshTopology*)_self;
   SizeT mem;

   if( PtrMap_Find( ptrs, (void*)self ) )
      return 0;
   mem = _NewClass_CalcMem( self, ptrs );
   return mem;
}
*/

void _MeshTopology_SetNumDims( void* _self, int nDims ) {
   MeshTopology* self = (MeshTopology*)_self;

   self->nDims = nDims;
   if( self->nDims > 0 )
      self->nTDims = nDims + 1;
}

void _MeshTopology_SetComm( void* _self, const Comm* comm ) {
   MeshTopology* self = (MeshTopology*)_self;

   Stg_Class_RemoveRef( self->comm );
   self->comm = (Comm*)comm;
   if( comm )
      Stg_Class_AddRef( (Comm*)comm );
}

void _MeshTopology_SetShadowDepth( void* _self, int depth ) {
   MeshTopology* self = (MeshTopology*)_self;

   assert( self && depth >= 0 );
   assert( self->comm && self->nTDims );

   self->shadDepth = depth;
}

int MeshTopology_GetNumDims( const void* self ) {
   assert( self );
   return ((MeshTopology*)self)->nDims;
}

const Comm* MeshTopology_GetComm( const void* self ) {
   assert( self );
   return ((MeshTopology*)self)->comm;
}


