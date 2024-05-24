/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <assert.h>
#include <mpi.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "types.h"
#include "Comm.h"


const Type Comm_Type = "Comm";


Comm* Comm_New() {
    Comm* self;
    SizeT _sizeOfSelf = sizeof(Comm);
    Type type = Comm_Type;
    Stg_Class_DeleteFunction* _delete = _Comm_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _Comm_New( COMM_PASSARGS );
    return self;
}

Comm* _Comm_New( COMM_DEFARGS ) {
    Comm* self;

    self = (Comm*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _Comm_Init( self );
    return self;
}

void Comm_Init( Comm* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _Comm_Init( self );
}

void _Comm_Init( void* _self ) {
   Comm* self = (Comm*)_self;

   self->mpiComm = MPI_COMM_WORLD;
   IArray_Init( &self->nbrs );
   IMap_Init( &self->inv );
   self->recvs = NULL;
   self->sends = NULL;
   self->stats = NULL;
   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSize = 0;
   self->srcSizes = NULL;
}

void Comm_Destruct( Comm* self ) {
   IArray_Destruct( &self->nbrs );
   IMap_Destruct( &self->inv );
   FreeArray( self->recvs );
   FreeArray( self->sends );
   FreeArray( self->stats );
}

void _Comm_Delete( void* _self ) {
   Comm* self = (Comm*)_self;

   Comm_Destruct( self );
   _Stg_Class_Delete( self );
}

void Comm_Copy( void* _self, const void* _op ) {
   Comm* self = (Comm*)_self;
   const Comm* op = (const Comm*)_self;
   int nNbrs;

   self->mpiComm = op->mpiComm;
   IArray_Copy( &self->nbrs, &op->nbrs );
   IMap_Copy( &self->inv, &op->inv );
   nNbrs = IArray_GetSize( &self->nbrs );
   self->recvs = (MPI_Request*)ReallocArray( self->recvs, MPI_Request, nNbrs );
   self->sends = (MPI_Request*)ReallocArray( self->sends, MPI_Request, nNbrs );
   self->stats = (MPI_Status*)ReallocArray( self->stats, MPI_Status, nNbrs );
}

/*
SizeT _Comm_CalcMem( const void* _self, PtrMap* ptrs ) {
   const Comm* self = (const Comm*)_self;
   SizeT mem;

   if( PtrMap_Find( ptrs, (void*)self ) )
      return 0;
   mem = _NewClass_CalcMem( self, ptrs );
   mem += NewClass_CalcMem( &self->nbrs, ptrs );
   mem += NewClass_CalcMem( &self->inv, ptrs );
   return mem;
}
*/

void Comm_SetMPIComm( void* _self, MPI_Comm mpiComm ) {
   Comm* self = (Comm*)_self;

   self->mpiComm = mpiComm;
   IArray_Set( &self->nbrs, 0, NULL );
   IMap_Clear( &self->inv );
   FreeArray( self->recvs );
   self->recvs = NULL;
   FreeArray( self->sends );
   self->sends = NULL;
   FreeArray( self->stats );
   self->stats = NULL;
}

void Comm_SetNeighbours( void* _self, int nNbrs, const int* nbrs ) {
   Comm* self = (Comm*)_self;
   int n_i;

   IArray_Set( &self->nbrs, nNbrs, nbrs );
   IMap_Clear( &self->inv );
   IMap_SetMaxSize( &self->inv, nNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      IMap_Insert( &self->inv, nbrs[n_i], n_i );
   self->recvs = (MPI_Request*)ReallocArray( self->recvs, MPI_Request, nNbrs );
   self->sends = (MPI_Request*)ReallocArray( self->sends, MPI_Request, nNbrs );
   self->stats = (MPI_Status*)ReallocArray( self->stats, MPI_Status, nNbrs );
}

void Comm_AddNeighbours( void* _self, int nNbrs, const int* nbrs ) {
   Comm* self = (Comm*)_self;
   int netNbrs;
   int n_i;

   IArray_Add( &self->nbrs, nNbrs, nbrs );
   netNbrs = IArray_GetSize( &self->nbrs );
   IMap_SetMaxSize( &self->inv, netNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      IMap_Insert( &self->inv, nbrs[n_i], netNbrs + n_i );
   self->recvs = (MPI_Request*)ReallocArray( self->recvs, MPI_Request, netNbrs );
   self->sends = (MPI_Request*)ReallocArray( self->sends, MPI_Request, netNbrs );
   self->stats = (MPI_Status*)ReallocArray( self->stats, MPI_Status, netNbrs );
}


/** Note: nbrLocalIndices is _not_ the ranks of neighbours to remove, but local indices of local
 * neighbours in the nbrs array */
void Comm_RemoveNeighbours( void* _self, int nNbrs, const int* nbrLocalIndices, IMap* map ) {
   Comm* self = (Comm*)_self;
   int netNbrs;
   int n_i;
   int globalIndex;

   /* First, we need to map the local index to a global, so we know what to remove from the inverse
    * mapping array next */
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      globalIndex = Comm_RankLocalToGlobal( self, nbrLocalIndices[n_i] );
      IMap_Remove( &self->inv, globalIndex );
   }
   /* Now remove the local to global listings */
   IArray_Remove( &self->nbrs, nNbrs, nbrLocalIndices, map );
   /* Finally, update sizes */
   netNbrs = IArray_GetSize( &self->nbrs );
   IMap_SetMaxSize( &self->inv, netNbrs );
   self->recvs = (MPI_Request*)ReallocArray( self->recvs, MPI_Request, netNbrs );
   self->sends = (MPI_Request*)ReallocArray( self->sends, MPI_Request, netNbrs );
   self->stats = (MPI_Status*)ReallocArray( self->stats, MPI_Status, netNbrs );
}

MPI_Comm Comm_GetMPIComm( const void* self ) {
   return ((Comm*)self)->mpiComm;
}

int Comm_GetNumNeighbours( const void* self ) {
   return IArray_GetSize( &((Comm*)self)->nbrs );
}

void Comm_GetNeighbours( const void* self, int* nNbrs, const int** nbrs ) {
   IArray_GetArray( &((Comm*)self)->nbrs, nNbrs, nbrs );
}

int Comm_RankLocalToGlobal( const void* self, int local ) {
   assert( local < IArray_GetSize( &((Comm*)self)->nbrs ) );
   return IArray_GetPtr( &((Comm*)self)->nbrs )[local];
}

Bool Comm_RankGlobalToLocal( const void* self, int global, int* local ) {
   return IMap_TryMap( &((Comm*)self)->inv, global, local );
}

void Comm_AllgatherInit( const void* _self, int srcSize, 
			 int* dstSizes, int itmSize )
{
   Comm* self = (Comm*)_self;
   const int sizeTag = 1001;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( itmSize );
   assert( !self->srcSize && !self->recvSizes );

   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstSizes + n_i, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( &srcSize, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = dstSizes;
   self->itmSize = itmSize;
   self->srcSize = srcSize;
}

void Comm_AllgatherBegin( const void* _self, const void* srcArray, 
			  void** dstArrays )
{
   Comm* self = (Comm*)_self;
   const int dataTag = 2002;
   int nNbrs;
   const int* nbrs;
   int n_i;

   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstArrays[n_i], self->recvSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)srcArray, self->srcSize * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
}

void Comm_AllgatherEnd( const void* _self ) {
   Comm* self = (Comm*)_self;
   int nNbrs;
   int n_i;

   nNbrs = IArray_GetSize( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSize = 0;
}

void Comm_AlltoallInit( const void* _self, const int* srcSizes, 
			int* dstSizes, int itmSize )
{
   Comm* self = (Comm*)_self;
   const int sizeTag = 1001;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( self && itmSize );
   assert( !self->srcSizes && !self->recvSizes );
   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstSizes + n_i, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)(srcSizes + n_i), 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = dstSizes;
   self->itmSize = itmSize;
   self->srcSizes = (int*)srcSizes;
}

void Comm_AlltoallBegin( const void* _self, const void** srcArrays, 
			 void** dstArrays )
{
   Comm* self = (Comm*)_self;
   const int dataTag = 2002;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( self );
   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstArrays[n_i], self->recvSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)(srcArrays[n_i]), self->srcSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
}

void Comm_AlltoallEnd( const void* _self ) {
   Comm* self = (Comm*)_self;
   int nNbrs;
   int n_i;

   assert( self );
   nNbrs = IArray_GetSize( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSizes = NULL;
}


