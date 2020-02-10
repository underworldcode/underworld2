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
#include <mpi.h>
#include "StGermain/StGermain.h"
#include "types.h"
#include "Decomp.h"


void Decomp_Update( Decomp* self );
void Decomp_UpdateOwnerMap( Decomp* self );


int stgCmpIntNE( const void* l, const void* r ) {
   assert( *(int*)l != *(int*)r );
   return (*(int*)l < *(int*)r) ? -1 : 1;
}


const Type Decomp_Type = "Decomp";


Decomp* Decomp_New() {
    Decomp* self;
    SizeT _sizeOfSelf = sizeof(Decomp);
    Type type = Decomp_Type;
    Stg_Class_DeleteFunction* _delete = _Decomp_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _Decomp_New( DECOMP_PASSARGS );
    return self;
}

Decomp* _Decomp_New( DECOMP_DEFARGS ) {
    Decomp* self;

    self = (Decomp*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _Decomp_Init( self );
    return self;
}

void Decomp_Init( Decomp* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _Decomp_Init( self );
}

void _Decomp_Init( void* _self ) {
   Decomp* self = (Decomp*)_self;

   self->mpiComm = MPI_COMM_WORLD;
   self->nGlobals = 0;
   self->locals = &self->localsObj;
   IArray_Init( self->locals );
   self->inv = &self->invObj;
   IMap_Init( self->inv );
   self->rngBegin = 0;
   self->rngEnd = 0;
   self->owners = &self->ownersObj;
   IMap_Init( self->owners );
}

void Decomp_Destruct( Decomp* _self ) {
   Decomp* self = (Decomp*)_self;

   Decomp_Clear( self );
   IArray_Destruct( self->locals );
   IMap_Destruct( self->inv );
   IMap_Destruct( self->owners );
}

void _Decomp_Delete( void* _self ) {
   Decomp* self = (Decomp*)_self;

   Decomp_Destruct( self );
   _Stg_Class_Delete( self );
}

void Decomp_Copy( void* _self, const void* _op ) {
   Decomp* self = (Decomp*)_self;
   const Decomp* op = (const Decomp*)_op;

   assert( self && op );
   self->mpiComm = op->mpiComm;
   self->nGlobals = op->nGlobals;
   IArray_Copy( self->locals, op->locals );
   IMap_Copy( self->inv, op->inv );
}

/*
SizeT _Decomp_CalcMem( const void* _self, PtrMap* ptrs ) {
   const Decomp* self = (const Decomp*)_self;
   SizeT mem;

   if( PtrMap_Find( ptrs, (void*)self ) )
      return 0;
   mem = _NewClass_CalcMem( self, ptrs );
   mem += NewClass_CalcMem( self->locals, ptrs );
   mem += NewClass_CalcMem( self->inv, ptrs );
   return mem;
}
*/

void Decomp_SetMPIComm( void* _self, MPI_Comm mpiComm ) {
   Decomp* self = (Decomp*)_self;

   assert( self );
   self->mpiComm = mpiComm;
}

void Decomp_SetLocals( void* _self, int nLocals, const int* locals ) {
   Decomp* self = (Decomp*)_self;
   int l_i;

   assert( self && (!nLocals || locals) );
   IArray_Set( self->locals, nLocals, locals );
   Decomp_Update( self );
   IMap_Clear( self->inv );
   IMap_SetMaxSize( self->inv, nLocals );
   for( l_i = 0; l_i < nLocals; l_i++ )
      IMap_Insert( self->inv, locals[l_i], l_i );
}

void Decomp_AddLocals( void* _self, int nLocals, const int* locals ) {
   Decomp* self = (Decomp*)_self;
   int l_i;

   assert( self && (!nLocals || locals) );
   IMap_SetMaxSize( self->inv, IArray_GetSize( self->locals ) + nLocals );
   IArray_Add( self->locals, nLocals, locals );
   Decomp_Update( self );
   for( l_i = 0; l_i < nLocals; l_i++ )
      IMap_Insert( self->inv, locals[l_i], l_i );
}

void Decomp_RemoveLocals( void* _self, int nLocals, const int* locals, IMap* map ) {
   Decomp* self = (Decomp*)_self;
   int oldSize;
   int l_i;

   assert( self && (!nLocals || locals) );
   oldSize = IArray_GetSize( self->locals );
   IArray_Remove( self->locals, nLocals, locals, map );
   Decomp_Update( self );
   for( l_i = 0; l_i < nLocals; l_i++ )
      IMap_Remove( self->inv, locals[l_i] );
   IMap_SetMaxSize( self->inv, oldSize - nLocals );
}

void Decomp_Clear( void* _self ) {
   Decomp* self = (Decomp*)_self;

   Decomp_ClearLocals( self );
   self->mpiComm = MPI_COMM_WORLD;
   self->nGlobals = 0;
}

void Decomp_ClearLocals( void* _self ) {
   Decomp* self = (Decomp*)_self;

   assert( self );
   IArray_Clear( self->locals );
   IMap_Clear( self->inv );
   self->rngBegin = 0;
   self->rngEnd = 0;
   IMap_Clear( self->owners );
}

MPI_Comm Decomp_GetComm( const void* self ) {
   assert( self );
   return ((Decomp*)self)->mpiComm;
}

MPI_Comm Decomp_GetMPIComm( const void* self ) {
   return ((Decomp*)self)->mpiComm;
}

int Decomp_GetNumGlobals( const void* self ) {
   assert( self );
   return ((Decomp*)self)->nGlobals;
}

int Decomp_GetNumLocals( const void* self ) {
   assert( self );
   return IArray_GetSize( ((Decomp*)self)->locals );
}

void Decomp_GetLocals( const void* self, int* nLocals, const int** locals ) {
   assert( self );
   *nLocals = IArray_GetSize( ((Decomp*)self)->locals );
   *locals = IArray_GetPtr( ((Decomp*)self)->locals );
}

int Decomp_LocalToGlobal( const void* self, int local ) {
   assert( self && local < IArray_GetSize( ((Decomp*)self)->locals ) );
   return IArray_GetPtr( ((Decomp*)self)->locals )[local];
}

int Decomp_GlobalToLocal( const void* self, int global ) {
   assert( self && global < ((Decomp*)self)->nGlobals );
   return IMap_Map( ((Decomp*)self)->inv, global );
}

Bool Decomp_TryGlobalToLocal( const void* self, int global, int* local ) {
   assert( self && global < ((Decomp*)self)->nGlobals );
   return IMap_TryMap( ((Decomp*)self)->inv, global, local );
}

void Decomp_FindOwners( const void* _self, int nGlobals, const int* globals, 
		       int* ranks )
{
   const Decomp* self = (Decomp*)_self;
   IMap ordMapObj, *ordMap = &ordMapObj;
   int rangeSize;
   int *ord;
   int **ptrs, *sizes;
   int begin, end, len, pos, ind;
   int *recvSizes, **recvArrays;
   int *remSizes, **remRanks;
   int nRanks, rank;
   int g_i, r_i, i_i;

   assert( !nGlobals || globals );
   assert( !nGlobals || ranks );

   insist( MPI_Comm_size( self->mpiComm, &nRanks ), == MPI_SUCCESS );
   insist( MPI_Comm_rank( self->mpiComm, &rank ), == MPI_SUCCESS );

   IMap_Init( ordMap );
   IMap_SetMaxSize( ordMap, nGlobals );
   for( g_i = 0; g_i < nGlobals; g_i++ )
      IMap_Insert( ordMap, globals[g_i], g_i );
   ord = AllocArray( int, nGlobals );
   memcpy( ord, globals, nGlobals * sizeof(int) );
   qsort( ord, nGlobals, sizeof(int), stgCmpIntNE );

   ptrs = AllocArray( int*, nRanks );
   sizes = AllocArray( int, nRanks );

   rangeSize = self->nGlobals / nRanks;
   if( rank < self->nGlobals % nRanks )
      rangeSize++;

   pos = 0;
   begin = end = 0;
   for( r_i = 0; r_i < nRanks; r_i++ ) {
      end += self->nGlobals / nRanks;
      if( r_i < self->nGlobals % nRanks )
	 end++;

      if( pos < nGlobals && ord[pos] >= begin && ord[pos] < end ) {
	 len = 0;
	 while( pos + len < nGlobals && ord[pos + len] < end ) {
	    if( r_i == rank ) {
	       ind = IMap_Map( ordMap, ord[pos + len] );
	       ranks[ind] = IMap_Map( self->owners, ord[pos + len] );
	    }
	    len++;
	 }
	 if( r_i != rank ) {
	    ptrs[r_i] = ord + pos;
	    sizes[r_i] = len;
	 }
	 else {
	    ptrs[r_i] = NULL;
	    sizes[r_i] = 0;
	 }
	 pos += len;
      }
      else {
	 ptrs[r_i] = NULL;
	 sizes[r_i] = 0;
      }

      begin = end;
   }

   MPIArray_Alltoall( (unsigned*)sizes, (void**)ptrs, 
		      (unsigned**)&recvSizes, (void***)&recvArrays, 
		      sizeof(int), self->mpiComm );

   for( r_i = 0; r_i < nRanks; r_i++ ) {
      for( i_i = 0; i_i < recvSizes[r_i]; i_i++ )
	 recvArrays[r_i][i_i] = IMap_Map( self->owners, recvArrays[r_i][i_i] );
   }

   MPIArray_Alltoall( (unsigned*)recvSizes, (void**)recvArrays, 
		      (unsigned**)&remSizes, (void***)&remRanks, 
		      sizeof(int), self->mpiComm );
   MemFree( recvSizes );
   for( r_i = 0; r_i < nRanks; r_i++ )
      MemFree( recvArrays[r_i] );
   MemFree( recvArrays );

   for( r_i = 0; r_i < nRanks; r_i++ ) {
      for( i_i = 0; i_i < remSizes[r_i]; i_i++ ) {
	 ind = IMap_Map( ordMap, ptrs[r_i][i_i] );
	 ranks[ind] = remRanks[r_i][i_i];
      }
      MemFree( remRanks[r_i] );
   }
   IMap_Destruct( ordMap );
   MemFree( remRanks );
   MemFree( remSizes );
   FreeArray( sizes );
   FreeArray( ptrs );
   FreeArray( ord );
}

void Decomp_Update( Decomp* self ) {
   int nLocals;

   assert( self );
   if( self->mpiComm ) {
      nLocals = IArray_GetSize( self->locals );
      insist( MPI_Allreduce( &nLocals, &self->nGlobals, 1, MPI_INT, MPI_SUM, 
			     self->mpiComm ), == MPI_SUCCESS );
   }
   else
      self->nGlobals = 0;

   Decomp_UpdateOwnerMap( self );
}

void Decomp_UpdateOwnerMap( Decomp* self ) {
   int rangeSize;
   int nLocals;
   const int *locals;
   int *ord;
   int **ptrs, *sizes;
   int begin, end, len, pos;
   int *recvSizes, **recvArrays;
   int nRanks, rank;
   int r_i, i_i;

   insist( MPI_Comm_size( self->mpiComm, &nRanks ), == MPI_SUCCESS );
   insist( MPI_Comm_rank( self->mpiComm, &rank ), == MPI_SUCCESS );

   IArray_GetArray( self->locals, &nLocals, &locals );
   ord = AllocArray( int, nLocals );
   memcpy( ord, locals, nLocals * sizeof(int) );
   qsort( ord, nLocals, sizeof(int), stgCmpIntNE );

   ptrs = AllocArray( int*, nRanks );
   sizes = AllocArray( int, nRanks );

   rangeSize = self->nGlobals / nRanks;
   if( rank < self->nGlobals % nRanks )
      rangeSize++;
   IMap_Clear( self->owners );
   IMap_SetMaxSize( self->owners, rangeSize );

   pos = 0;
   begin = end = 0;
   for( r_i = 0; r_i < nRanks; r_i++ ) {
      end += self->nGlobals / nRanks;
      if( r_i < self->nGlobals % nRanks )
	 end++;
      if( r_i == rank ) {
	 self->rngBegin = begin;
	 self->rngEnd = end;
      }

      if( pos < nLocals && ord[pos] >= begin && ord[pos] < end ) {
	 len = 0;
	 while( pos + len < nLocals && ord[pos + len] < end ) {
	    if( r_i == rank )
	       IMap_Insert( self->owners, ord[pos + len], r_i );
	    len++;
	 }
	 if( r_i != rank ) {
	    ptrs[r_i] = ord + pos;
	    sizes[r_i] = len;
	 }
	 else {
	    ptrs[r_i] = NULL;
	    sizes[r_i] = 0;
	 }
	 pos += len;
      }
      else {
	 ptrs[r_i] = NULL;
	 sizes[r_i] = 0;
      }

      begin = end;
   }

   MPIArray_Alltoall( (unsigned*)sizes, (void**)ptrs, 
		      (unsigned**)&recvSizes, (void***)&recvArrays, 
		      sizeof(int), self->mpiComm );
   FreeArray( sizes );
   FreeArray( ptrs );
   FreeArray( ord );

   for( r_i = 0; r_i < nRanks; r_i++ ) {
      for( i_i = 0; i_i < recvSizes[r_i]; i_i++ )
	 IMap_Insert( self->owners, recvArrays[r_i][i_i], r_i );
      MemFree( recvArrays[r_i] );
   }
   MemFree( recvSizes );
   MemFree( recvArrays );
}


