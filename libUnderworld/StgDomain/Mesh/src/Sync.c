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
#include "StGermain/StGermain.h"
#include "types.h"
#include "Decomp.h"
#include "Sync.h"


const Type Sync_Type = "Sync";


void Sync_UpdateTables( Sync* self );
void Sync_UpdateShared( Sync* self );
void Sync_UpdateOwners( Sync* self );
void Sync_ClearTables( Sync* self );
void Sync_ClearShared( Sync* self );
void Sync_ClearOwners( Sync* self );


Sync* Sync_New() {
    Sync* self;
    SizeT _sizeOfSelf = sizeof(Sync);
    Type type = Sync_Type;
    Stg_Class_DeleteFunction* _delete = _Sync_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _Sync_New( SYNC_PASSARGS );
    return self;
}

Sync* _Sync_New( SYNC_DEFARGS ) {
    Sync* self;

    self = (Sync*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _Sync_Init( self );
    return self;
}

void Sync_Init( Sync* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _Sync_Init( self );
}

void _Sync_Init( void* _self ) {
   Sync* self = (Sync*)_self;

   self->decomp = NULL;
   self->comm = NULL;
   self->nDomains = 0;
   self->remotes = &self->remotesObj;
   IArray_Init( self->remotes );
   self->owners = NULL;
   self->nShared = 0;
   self->shared = NULL;
   self->nSharers = NULL;
   self->sharers = NULL;
   self->gr = &self->grObj;
   IMap_Init( self->gr );
   self->ls = &self->lsObj;
   IMap_Init( self->ls );
   self->nSrcs = NULL;
   self->srcs = NULL;
   self->nSnks = NULL;
   self->snks = NULL;
}

void Sync_Destruct( Sync* self ) {
   Sync_Clear( self );
   IArray_Destruct( self->remotes );
   IMap_Destruct( self->gr );
   IMap_Destruct( self->ls );
}

void _Sync_Delete( void* _self ) {
   Sync* self = (Sync*)_self;

   Sync_Destruct( self );
   _Stg_Class_Delete( self );
}

void Sync_Copy( void* _self, const void* op ) {
   /*Sync* self = (Sync*)_self;*/

   /* TODO: Method body goes here */
   assert( 0 );
}

/*
SizeT _Sync_CalcMem( const void* _self, PtrMap* ptrs ) {
   const Sync* self = (const Sync*)_self;
   SizeT mem;

   if( PtrMap_Find( ptrs, (void*)self ) )
      return 0;
   mem = _NewClass_CalcMem( self, ptrs );
   if( self->decomp )
      mem += NewClass_CalcMem( self->decomp, ptrs );
   mem += NewClass_CalcMem( self->remotes, ptrs );
   mem += NewClass_CalcMem( self->gr, ptrs );
   mem += NewClass_CalcMem( self->ls, ptrs );
   return mem;
}
*/

void Sync_SetDecomp( void* _self, const Decomp* decomp ) {
   Sync* self = (Sync*)_self;

   Sync_ClearDecomp( self );
   self->decomp = (Decomp*)decomp;
   if( self->decomp ) {
      Stg_Class_AddRef( self->decomp );
      self->nDomains = Decomp_GetNumLocals( self->decomp );
      Sync_UpdateTables( self );
      Sync_UpdateShared( self );
      Sync_UpdateOwners( self );
   }
}

void Sync_FindRemotes( void* _self, int nRemotes, const int* remotes ) {
   Sync *self = (Sync*)_self;
   int *owners;
   ISet nbrSetObj, *nbrSet = &nbrSetObj;
   Comm* comm;
   int nNbrs, *nbrs;
   int nRanks;
   Bool *sendFlags, *recvFlags;
   MPI_Comm mpiComm;
   int r_i;

   assert( !nRemotes || remotes );

   mpiComm = Decomp_GetMPIComm( self->decomp );
   insist( MPI_Comm_size( mpiComm, &nRanks ), == MPI_SUCCESS );

   owners = AllocArray( int, nRemotes );
   Decomp_FindOwners( self->decomp, nRemotes, remotes, owners );
   ISet_Init( nbrSet );
   ISet_SetMaxSize( nbrSet, nRemotes );
   for( r_i = 0; r_i < nRemotes; r_i++ )
      ISet_TryInsert( nbrSet, owners[r_i] );
   FreeArray( owners );

   sendFlags = AllocArray( Bool, nRanks );
   recvFlags = AllocArray( Bool, nRanks );
   for( r_i = 0; r_i < nRanks; r_i++ )
      sendFlags[r_i] = ISet_Has( nbrSet, r_i );
   insist( MPI_Alltoall( sendFlags, 1, MPI_INT, 
			 recvFlags, 1, MPI_INT, 
			 mpiComm ), == MPI_SUCCESS );
   FreeArray( sendFlags );
   for( r_i = 0; r_i < nRanks; r_i++ ) {
      if( recvFlags[r_i] )
	 ISet_TryInsert( nbrSet, r_i );
   }
   FreeArray( recvFlags );

   nNbrs = ISet_GetSize( nbrSet );
   nbrs = AllocArray( int, nNbrs );
   ISet_GetArray( nbrSet, nbrs );
   ISet_Destruct( nbrSet );

   comm = Comm_New();
   Comm_SetMPIComm( comm, Decomp_GetMPIComm( self->decomp ) );
   Comm_SetNeighbours( comm, nNbrs, nbrs );
   FreeArray( nbrs );
   Sync_SetComm( self, comm );

   Sync_SetRemotes( self, nRemotes, remotes );
}

void Sync_SetComm( void* _self, const Comm* comm ) {
   Sync* self = (Sync*)_self;

   Sync_ClearComm( self );
   self->comm = (Comm*)comm;
   if( self->comm ) {
      Stg_Class_AddRef( self->comm );
      Sync_UpdateTables( self );
      Sync_UpdateShared( self );
      Sync_UpdateOwners( self );
   }
}

void Sync_SetRemotes( void* _self, int nRemotes, const int* remotes ) {
   Sync* self = (Sync*)_self;

   Sync_ClearRemotes( self );
   Sync_AddRemotes( self, nRemotes, remotes );
}

void Sync_AddRemotes( void* _self, int nRemotes, const int* remotes ) {
   Sync* self = (Sync*)_self;
   int nOldRems;
   int r_i;

   assert( self && self->decomp && self->comm );
   nOldRems = IArray_GetSize( self->remotes );
   IMap_SetMaxSize( self->gr, IArray_GetSize( self->remotes ) + nRemotes );
   IArray_Add( self->remotes, nRemotes, remotes );
   for( r_i = 0; r_i < nRemotes; r_i++ )
      IMap_Insert( self->gr, remotes[r_i], nOldRems + r_i );
   Sync_UpdateTables( self );
   Sync_UpdateShared( self );
   Sync_UpdateOwners( self );
   self->nDomains += nRemotes;
}

void Sync_RemoveRemotes( void* _self, int nRemotes, const int* remotes, IMap* map ) {
   Sync* self = (Sync*)_self;

   assert( self && self->decomp && self->comm );
   IArray_Remove( self->remotes, nRemotes, remotes, map );
   Sync_UpdateTables( self );
   Sync_UpdateShared( self );
   Sync_UpdateOwners( self );
   self->nDomains -= nRemotes;
}

void Sync_Clear( void* _self ) {
   Sync* self = (Sync*)_self;

   Sync_ClearRemotes( self );
   Stg_Class_RemoveRef( self->decomp );
   self->decomp = NULL;
   Stg_Class_RemoveRef( self->comm );
   self->comm = NULL;
   self->nDomains = 0;
}

void Sync_ClearDecomp( void* _self ) {
   Sync* self = (Sync*)_self;

   Sync_ClearRemotes( self );
   Stg_Class_RemoveRef( self->decomp );
   self->decomp = NULL;
   self->nDomains = 0;
}

void Sync_ClearComm( void* _self ) {
   Sync* self = (Sync*)_self;

   Sync_ClearRemotes( self );
   Stg_Class_RemoveRef( self->comm );
   self->comm = NULL;
   if( self->decomp )
      self->nDomains = Decomp_GetNumLocals( self->decomp );
   else
      self->nDomains = 0;
}

void Sync_ClearRemotes( void* _self ) {
   Sync* self = (Sync*)_self;
   int s_i;

   // decrement reference counter
   if( self->nRefs > 1 ) {
      return;
   }

   Sync_ClearTables( self );
   IArray_Clear( self->remotes );
   for( s_i = 0; s_i < self->nShared; s_i++ )
      FreeArray( self->sharers[s_i] );
   FreeArray( self->shared );
   FreeArray( self->nSharers );
   FreeArray( self->sharers );
   FreeArray( self->owners );
   IMap_Clear( self->gr );
   IMap_Clear( self->ls );
   if( self->decomp )
      self->nDomains = Decomp_GetNumLocals( self->decomp );
   else
      self->nDomains = 0;
   self->nShared = 0;
   self->shared = NULL;
   self->nSharers = NULL;
   self->sharers = NULL;
   self->owners = NULL;
}

const Decomp* Sync_GetDecomp( const void* self ) {
   assert( self );
   return ((Sync*)self)->decomp;
}

const Comm* Sync_GetComm( const void* self ) {
   assert( self );
   return ((Sync*)self)->comm;
}

int Sync_GetNumRemotes( const void* self ) {
   assert( self );
   return IArray_GetSize( ((Sync*)self)->remotes );
}

int Sync_GetNumDomains( const void* self ) {
   assert( self );
   return ((Sync*)self)->nDomains;
}

int Sync_GetNumShared( const void* self ) {
   assert( self );
   return ((Sync*)self)->nShared;
}

int Sync_GetNumSharers( const void* self, int shared ) {
   assert( self && shared < ((Sync*)self)->nShared );
   assert( ((Sync*)self)->nSharers );
   return ((Sync*)self)->nSharers[shared];
}

void Sync_GetRemotes( const void* self, int* nRemotes, const int** remotes ) {
   assert( self );
   *nRemotes = IArray_GetSize( ((Sync*)self)->remotes );
   *remotes = IArray_GetPtr( ((Sync*)self)->remotes );
}

int Sync_GetOwner( const void* self, int remote ) {
   assert( self && remote < IArray_GetSize( ((Sync*)self)->remotes ) );
   assert( ((Sync*)self)->owners );
   return ((Sync*)self)->owners[remote];
}

void Sync_GetShared( const void* self, int* nShared, const int** shared ) {
   assert( self );
   assert( !((Sync*)self)->nShared || shared );
   if( nShared )
      *nShared = ((Sync*)self)->nShared;
   *shared = ((Sync*)self)->shared;
}

void Sync_GetSharers( const void* self, int shared, int* nSharers, const int** sharers ) {
   assert( self && shared < ((Sync*)self)->nShared );
   assert( ((Sync*)self)->nSharers && sharers );
   if( nSharers )
      *nSharers = ((Sync*)self)->nSharers[shared];
   *sharers = ((Sync*)self)->sharers[shared];
}

int Sync_RemoteToGlobal( const void* self, int remote ) {
   assert( self && remote < IArray_GetSize( ((Sync*)self)->remotes ) );
   return IArray_GetPtr( ((Sync*)self)->remotes )[remote];
}

int Sync_GlobalToRemote( const void* self, int global ) {
   assert( self && global < Decomp_GetNumGlobals( ((Sync*)self)->decomp ) );
   return IMap_Map( ((Sync*)self)->gr, global );
}

Bool Sync_TryGlobalToRemote( const void* self, int global, int* remote ) {
   assert( self && remote );
   assert( global < Decomp_GetNumGlobals( ((Sync*)self)->decomp ) );
   return IMap_TryMap( ((Sync*)self)->gr, global, remote );
}

int Sync_DomainToGlobal( const void* self, int domain ) {
   assert( self );
   assert( domain < Decomp_GetNumLocals( ((Sync*)self)->decomp ) + 
	   IArray_GetSize( ((Sync*)self)->remotes ) );
   if( domain < Decomp_GetNumLocals( ((Sync*)self)->decomp ) )
      return Decomp_LocalToGlobal( ((Sync*)self)->decomp, domain );
   else {
      return IArray_GetPtr( ((Sync*)self)->remotes )
	 [domain - Decomp_GetNumLocals( ((Sync*)self)->decomp )];
   }
}

int Sync_GlobalToDomain( const void* self, int global ) {
  int domain;

   assert( self );
   assert( global < Decomp_GetNumGlobals( ((Sync*)self)->decomp ) );
   if( !Decomp_TryGlobalToLocal( ((Sync*)self)->decomp, global, &domain ) ) {
      if( !IMap_Has( ((Sync*)self)->gr, global ) )
	 abort();
      return IMap_Map( ((Sync*)self)->gr, global ) + 
	 Decomp_GetNumLocals( ((Sync*)self)->decomp );
   }
   else
      return domain;
}

Bool Sync_TryGlobalToDomain( const void* self, int global, int* domain ) {
   assert( self && domain );
   assert( global < Decomp_GetNumGlobals( ((Sync*)self)->decomp ) );
   if( !Decomp_TryGlobalToLocal( ((Sync*)self)->decomp, global, domain ) ) {
      if( IMap_TryMap( ((Sync*)self)->gr, global, domain ) ) {
	 *domain += Decomp_GetNumLocals( ((Sync*)self)->decomp );
	 return True;
      }
      else
	 return False;
   }
   else
      return True;
}

int Sync_SharedToLocal( const void* self, int shared ) {
   assert( self && shared < ((Sync*)self)->nShared );
   return ((Sync*)self)->shared[shared];
}

int Sync_LocalToShared( const void* self, int local ) {
   assert( self && local < Decomp_GetNumLocals( ((Sync*)self)->decomp ) );
   return IMap_Map( ((Sync*)self)->ls, local );
}

Bool Sync_TryLocalToShared( const void* self, int local, int* shared ) {
   assert( self && local < Decomp_GetNumLocals( ((Sync*)self)->decomp ) );
   assert( shared );
   return IMap_TryMap( ((Sync*)self)->ls, local, shared );
}

void Sync_SyncArray( const void* _self,
		     const void* local, size_t localStride, 
		     const void* remote, size_t remoteStride, 
		     size_t itmSize )
{
   Sync* self = (Sync*)_self;
   int nNbrs;
   int* nSrcs;
   stgByte **srcs, **snks;
   int n_i, s_i;

   assert( self );
   nNbrs = Comm_GetNumNeighbours( self->comm );
   snks = AllocArray( stgByte*, nNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      snks[n_i] = AllocArray( stgByte, self->nSnks[n_i] * itmSize );
      for( s_i = 0; s_i < self->nSnks[n_i]; s_i++ ) {
	 memcpy( snks[n_i] + s_i * itmSize, 
		 (stgByte*)local + self->snks[n_i][s_i] * localStride, 
		 itmSize );
      }
   }

   nSrcs = AllocArray( int, nNbrs );
   Comm_AlltoallInit( self->comm, self->nSnks, nSrcs, itmSize );
   srcs = AllocArray( stgByte*, nNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      srcs[n_i] = AllocArray( stgByte, nSrcs[n_i] * itmSize );
   Comm_AlltoallBegin( self->comm, (const void**)snks, (void**)srcs );
   Comm_AlltoallEnd( self->comm );
   FreeArray( nSrcs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      FreeArray( snks[n_i] );
   FreeArray( snks );

   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSrcs[n_i]; s_i++ ) {
	 memcpy( (stgByte*)remote + self->srcs[n_i][s_i] * remoteStride, 
		 srcs[n_i] + s_i * itmSize, 
		 itmSize );
      }
      FreeArray( srcs[n_i] );
   }
   FreeArray( srcs );
}

void Sync_UpdateTables( Sync* self ) {
   int nNbrs;
   ISet theirRemsObj, *theirRems = &theirRemsObj;
   ISet myLocsObj, *myLocs = &myLocsObj;
   int nLocals, nRems;
   const int *locals, *rems;
   int* nAllRems, **allRems;
   int* nFnds, **fnds;
   int n_i, s_i;

   Sync_ClearTables( self );
   if( !self->comm || !self->decomp )
      return;

   nNbrs = Comm_GetNumNeighbours( self->comm );
   ISet_Init( theirRems );
   ISet_Init( myLocs );
   Decomp_GetLocals( self->decomp, &nLocals, &locals );
   ISet_UseArray( myLocs, nLocals, locals );
   nRems = IArray_GetSize( self->remotes );
   rems = IArray_GetPtr( self->remotes );

   nAllRems = AllocArray( int, nNbrs );
   allRems = AllocArray( int*, nNbrs );
   Comm_AllgatherInit( self->comm, nRems, nAllRems, sizeof(int) );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      allRems[n_i] = AllocArray( int, nAllRems[n_i] );
   Comm_AllgatherBegin( self->comm, rems, (void**)allRems );
   Comm_AllgatherEnd( self->comm );

   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      ISet_UseArray( theirRems, nAllRems[n_i], allRems[n_i] );
      ISet_Isect( theirRems, myLocs );
      nAllRems[n_i] = ISet_GetSize( theirRems );
      allRems[n_i] = ReallocArray( allRems[n_i], int, nAllRems[n_i] );
      ISet_GetArray( theirRems, allRems[n_i] );
   }
   ISet_Destruct( theirRems );
   ISet_Destruct( myLocs );

   nFnds = AllocArray( int, nNbrs );
   fnds = AllocArray( int*, nNbrs );
   Comm_AlltoallInit( self->comm, nAllRems, nFnds, sizeof(int) );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      fnds[n_i] = AllocArray( int, nFnds[n_i] );
   Comm_AlltoallBegin( self->comm, (const void**)allRems, (void**)fnds );
   Comm_AlltoallEnd( self->comm );

   self->nSrcs = nFnds;
   self->srcs = fnds;
   self->nSnks = nAllRems;
   self->snks = allRems;

   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSrcs[n_i]; s_i++ ) {
	 self->srcs[n_i][s_i] = Sync_GlobalToRemote( self, 
						     self->srcs[n_i][s_i] ); 
      }
      for( s_i = 0; s_i < self->nSnks[n_i]; s_i++ ) {
	 self->snks[n_i][s_i] = Decomp_GlobalToLocal( self->decomp, 
						      self->snks[n_i][s_i] );
      }
   }
}

void Sync_UpdateShared( Sync* self ) {
   ISet sharedSetObj, *sharedSet = &sharedSetObj;
   int shared, nNbrs;
   int n_i, s_i;

   assert( self );
   Sync_ClearShared( self );
   if( !self->comm || !self->decomp )
      return;

   nNbrs = Comm_GetNumNeighbours( self->comm );
   ISet_Init( sharedSet );
   ISet_SetMaxSize( sharedSet, Decomp_GetNumLocals( self->decomp ) );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSnks[n_i]; s_i++ )
	 ISet_TryInsert( sharedSet, self->snks[n_i][s_i] );
   }
   self->nShared = ISet_GetSize( sharedSet );
   self->shared = ReallocArray( self->shared, int, self->nShared );
   ISet_GetArray( sharedSet, self->shared );
   ISet_Destruct( sharedSet );

   IMap_Clear( self->ls );
   IMap_SetMaxSize( self->ls, self->nShared );
   for( s_i = 0; s_i < self->nShared; s_i++ )
      IMap_Insert( self->ls, self->shared[s_i], s_i );

   self->nSharers = ReallocArray( self->nSharers, int, self->nShared );
   memset( self->nSharers, 0, self->nShared * sizeof(int) );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSnks[n_i]; s_i++ ) {
	 shared = IMap_Map( self->ls, self->snks[n_i][s_i] );
	 self->nSharers[shared]++;
      }
   }
   self->sharers = ReallocArray( self->sharers, int*, self->nShared );
   for( s_i = 0; s_i < self->nShared; s_i++ )
      self->sharers[s_i] = AllocArray( int, self->nSharers[s_i] );
   memset( self->nSharers, 0, self->nShared * sizeof(int) );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSnks[n_i]; s_i++ ) {
	 shared = IMap_Map( self->ls, self->snks[n_i][s_i] );
	 self->sharers[shared][self->nSharers[shared]++] = n_i;
      }
   }
}

void Sync_UpdateOwners( Sync* self ) {
   int nRemotes, nNbrs;
   int n_i, s_i;

   assert( self );
   Sync_ClearOwners( self );
   if( !self->comm || !self->decomp )
      return;

   nNbrs = Comm_GetNumNeighbours( self->comm );
   nRemotes = IArray_GetSize( self->remotes );
   self->owners = ReallocArray( self->owners, int, nRemotes );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      for( s_i = 0; s_i < self->nSrcs[n_i]; s_i++ )
	 self->owners[self->srcs[n_i][s_i]] = n_i;
   }
}

void Sync_ClearTables( Sync* self ) {
   int n_i;

   assert( self );
   if( self->decomp ) {
      if( self->comm ) {
	 for( n_i = 0; n_i < Comm_GetNumNeighbours( self->comm ); n_i++ ) {
	    if( self->srcs )
	       FreeArray( self->srcs[n_i] );
	    if( self->snks )
	       FreeArray( self->snks[n_i] );
	 }
      }
   }
   FreeArray( self->nSrcs );
   FreeArray( self->srcs );
   FreeArray( self->nSnks );
   FreeArray( self->snks );
   self->nSrcs = NULL;
   self->srcs = NULL;
   self->nSnks = NULL;
   self->snks = NULL;
}

void Sync_ClearShared( Sync* self ) {
   int s_i;

   assert( self );
   for( s_i = 0; s_i < self->nShared; s_i++ )
      FreeArray( self->sharers[s_i] );
   FreeArray( self->shared );
   FreeArray( self->nSharers );
   FreeArray( self->sharers );
   self->nShared = 0;
   self->shared = NULL;
   self->nSharers = NULL;
   self->sharers = NULL;
}

void Sync_ClearOwners( Sync* self ) {
   assert( self );
   FreeArray( self->owners );
   self->owners = NULL;
}


