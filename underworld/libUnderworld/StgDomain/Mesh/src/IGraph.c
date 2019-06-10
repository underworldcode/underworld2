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
#include "MeshTopology.h"
#include "IGraph.h"


const Type IGraph_Type = "IGraph";


void IGraph_PickleIncidenceInit( IGraph* self, int dim, int nEls, int* els, int* nBytes );
void IGraph_PickleIncidence( IGraph* self, int dim, int nEls, int* els, stgByte* bytes );
void IGraph_UnpickleIncidence( IGraph* self, int dim, int nBytes, stgByte* bytes );
int IGraph_Cmp( const void* l, const void* r );


IGraph* IGraph_New() {
    IGraph* self;
    SizeT _sizeOfSelf = sizeof(IGraph);
    Type type = IGraph_Type;
    Stg_Class_DeleteFunction* _delete = _IGraph_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;
    MeshTopology_SetCommFunc* setCommFunc = IGraph_SetComm;
    MeshTopology_SetNumDimsFunc* setNumDimsFunc = _IGraph_SetNumDims;
    MeshTopology_SetShadowDepthFunc* setShadowDepthFunc = _IGraph_SetShadowDepth;
    MeshTopology_GetNumCellsFunc* getNumCellsFunc = NULL;
    MeshTopology_GetIncidenceFunc* getIncidenceFunc = _IGraph_GetIncidence;

    self = _IGraph_New( IGRAPH_PASSARGS );
    return self;
}

IGraph* _IGraph_New( IGRAPH_DEFARGS ) {
    IGraph* self;

    self = (IGraph*)_MeshTopology_New( MESHTOPOLOGY_PASSARGS );
    _IGraph_Init( self );
    return self;
}

void IGraph_Init( IGraph* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _MeshTopology_Init( self );
    _IGraph_Init( self );
}

void _IGraph_Init( void* _self ) {
    IGraph* self = (IGraph*)_self;

    self->nDims = 0;
    self->nTDims = 0;
    self->shadDepth = 0;
    self->comm = NULL;
    self->locals = NULL;
    self->remotes = NULL;
    self->nBndEls = NULL;
    self->bndEls = NULL;
    self->nIncEls = NULL;
    self->incEls = NULL;
}

void IGraph_Destruct( IGraph* self ) {
    IGraph_Clear( self );
}

void _IGraph_Delete( void* _self ) {
    IGraph* self = (IGraph*)_self;

    IGraph_Destruct( self );
    _MeshTopology_Delete( self );
}

void IGraph_Copy( void* _self, const void* op ) {
    /*IGraph* self = (IGraph*)_self;*/

    assert( 0 );
    /* TODO */
}

/*
SizeT _IGraph_CalcMem( const void* _self, PtrMap* ptrs ) {
    IGraph* self = (IGraph*)_self;
    SizeT mem;
    int d_i;

    if( PtrMap_Find( ptrs, (void*)self ) )
        return 0;
    mem = _MeshTopology_CalcMem( self, ptrs );

    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->locals )
            mem += NewClass_CalcMem( self->locals[d_i], ptrs );
        if( self->remotes )
            mem += NewClass_CalcMem( self->remotes[d_i], ptrs );
    }
    return mem;
}
*/

void _IGraph_SetNumDims( void* _self, int nDims ) {
    IGraph* self = (IGraph*)_self;
    int d_i;

    IGraph_ClearDims( self );
    _MeshTopology_SetNumDims( self, nDims );

    self->locals = AllocArray( Decomp*, self->nTDims );
    self->remotes = AllocArray( Sync*, self->nTDims );
    self->nIncEls = AllocArray2D( int*, self->nTDims, self->nTDims );
    self->incEls = AllocArray2D( int**, self->nTDims, self->nTDims );

    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        self->locals[d_i] = Decomp_New();
        Stg_Class_AddRef( self->locals[d_i] );
        self->remotes[d_i] = Sync_New();
        Stg_Class_AddRef( self->remotes[d_i] );

        if( self->comm )
            Decomp_SetMPIComm( self->locals[d_i], Comm_GetMPIComm( self->comm ) );
        Sync_SetDecomp( self->remotes[d_i], self->locals[d_i] );

        if( self->comm )
            Sync_SetComm( self->remotes[d_i], self->comm );

        memset( self->nIncEls[d_i], 0, sizeof(int**) * self->nTDims );
        memset( self->incEls[d_i], 0, sizeof(int***) * self->nTDims );
    }
}

void IGraph_SetComm( void* _self, const Comm* comm ) {
    IGraph* self = (IGraph*)_self;
    const Comm* curComm;
    int d_i;

    assert( self );

    IGraph_ClearElements( self );
    _MeshTopology_SetComm( self, comm );

    if( comm ) {
        for( d_i = 0; d_i < self->nTDims; d_i++ ) {
            curComm = Sync_GetComm( self->remotes[d_i] );

            if( curComm != comm ) {
                Sync_SetComm( self->remotes[d_i], self->comm );
                Decomp_SetMPIComm( self->locals[d_i], Comm_GetMPIComm( self->comm ) );
            }
        }
    }
}

void IGraph_SetDomain( void* _self, int dim, Sync* sync ) {
    IGraph* self = (IGraph*)_self;

    assert( self && dim < self->nTDims );
    Stg_Class_RemoveRef( self->locals[dim] );
    Stg_Class_RemoveRef( self->remotes[dim] );
    self->remotes[dim] = sync;
    Stg_Class_AddRef( sync );

    if( sync ) {
        self->locals[dim] = (Decomp*)Sync_GetDecomp( sync );
        Stg_Class_AddRef( self->locals[dim] );
    }
}

void IGraph_SetElements( void* _self, int dim, int nEls, const int* globals ) {
    IGraph* self = (IGraph*)_self;
    int rank;
    int nNbrs;
    const int *nbrs;
    int nSubEls;
    const int *subEls;
    int rem, netRem;
    int *nNbrEls, **nbrEls;
    IArray** isects;
    ISet localsObj, *locals = &localsObj;
    ISet remotesObj, *remotes = &remotesObj;
    int nCurEls, *curEls;
    MPI_Comm mpiComm;
    int n_i, e_i;

    assert( self && dim < self->nTDims );
    assert( !nEls || globals );
    assert( self->comm );

    Comm_GetNeighbours( self->comm, &nNbrs, &nbrs );
    if( !nNbrs ) {
        IGraph_SetLocalElements( self, dim, nEls, globals );
        return;
    }

    ISet_Init( locals );
    mpiComm = Comm_GetMPIComm( self->comm );
    insist( MPI_Comm_rank( mpiComm, &rank ), == MPI_SUCCESS );
    isects = AllocArray( IArray*, nNbrs );
    for( n_i = 0; n_i < nNbrs; n_i++ )
        isects[n_i] = IArray_New();

    ISet_UseArray( locals, nEls, globals );
    nSubEls = (nEls < 1000) ? nEls : 1000;
    rem = nEls;
    subEls = globals;
    nNbrEls = AllocArray( int, nNbrs );
    nbrEls = AllocArray( int*, nNbrs );

    do {
        Comm_AllgatherInit( self->comm, nSubEls, nNbrEls, sizeof(int) );

        for( n_i = 0; n_i < nNbrs; n_i++ )
            nbrEls[n_i] = AllocArray( int, nNbrEls[n_i] );

        Comm_AllgatherBegin( self->comm, subEls, (void**)nbrEls );
        Comm_AllgatherEnd( self->comm );

        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            for( e_i = 0; e_i < nNbrEls[n_i]; e_i++ ) {
                if( ISet_Has( locals, nbrEls[n_i][e_i] ) )
                    IArray_Append( isects[n_i], nbrEls[n_i][e_i] );
            }
            FreeArray( nbrEls[n_i] );
        }

        subEls += nSubEls;
        rem -= nSubEls;
        nSubEls = (rem < 1000) ? rem : 1000;
        insist( MPI_Allreduce( &rem, &netRem, 1, MPI_INT, MPI_SUM, mpiComm ), == MPI_SUCCESS );
    } while( netRem );
    FreeArray( nNbrEls );
    FreeArray( nbrEls );

    ISet_Init( remotes );
    ISet_SetMaxSize( remotes, nEls );
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        IArray_GetArray( isects[n_i], &nCurEls, (const int**)&curEls );
        if( nbrs[n_i] < rank ) {
            for( e_i = 0; e_i < nCurEls; e_i++ ) {
                ISet_TryRemove( locals, curEls[e_i] );
                ISet_TryInsert( remotes, curEls[e_i] );
            }
        }
        Stg_Class_Delete( isects[n_i] );
    }
    FreeArray( isects );

    nCurEls = ISet_GetSize( locals );
    curEls = AllocArray( int, nCurEls );
    ISet_GetArray( locals, curEls );
    ISet_Destruct( locals );
    qsort( curEls, nCurEls, sizeof(int), IGraph_Cmp );
    IGraph_SetLocalElements( self, dim, nCurEls, curEls );
    FreeArray( curEls );
    nCurEls = ISet_GetSize( remotes );
    curEls = AllocArray( int, nCurEls );
    ISet_GetArray( remotes, curEls );
    ISet_Destruct( remotes );
    qsort( curEls, nCurEls, sizeof(int), IGraph_Cmp );
    IGraph_SetRemoteElements( self, dim, nCurEls, curEls );
    FreeArray( curEls );
}

void IGraph_SetLocalElements( void* _self, int dim, int nEls, const int* globals ) {
    IGraph* self = (IGraph*)_self;
    int d_i, e_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || globals );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        for( e_i = 0; e_i < Sync_GetNumDomains( self->remotes[dim] ); d_i++ )
            FreeArray( self->incEls[dim][d_i][e_i] );
        FreeArray( self->incEls[dim][d_i] );
        FreeArray( self->nIncEls[dim][d_i] );
        self->incEls[dim][d_i] = NULL;
        self->nIncEls[dim][d_i] = NULL;
    }
    Decomp_SetLocals( self->locals[dim], nEls, globals );
    Sync_SetDecomp( self->remotes[dim], self->locals[dim] );
}

void IGraph_AddLocalElements( void* _self, int dim, int nEls, const int* globals ) {
    IGraph* self = (IGraph*)_self;

    assert( 0 );
    /* TODO */

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || globals );
    Decomp_AddLocals( self->locals[dim], nEls, globals );
    /* TODO: Expand this dimensions incidence if some incidence already set. */
}

void IGraph_RemoveLocalElements( void* _self, int dim, int nEls, const int* globals, IMap* map ) {
    /*IGraph* self = (IGraph*)_self;*/

    assert( 0 );
    /* TODO: Method body goes here */
}

void IGraph_SetRemoteElements( void* _self, int dim, int nEls, const int* globals ) {
    IGraph* self = (IGraph*)_self;
    int nDoms;
    int d_i, e_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || globals );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->nIncEls[dim][d_i] ) {
            for( e_i = Decomp_GetNumLocals( self->locals[dim] ); e_i < Sync_GetNumDomains( self->remotes[dim] ); e_i++ ) {
                FreeArray( self->incEls[dim][d_i][e_i] );
            }
        }
    }

    Sync_SetRemotes( self->remotes[dim], nEls, globals );

    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->nIncEls[dim][d_i] ) {
            nDoms = Sync_GetNumDomains( self->remotes[dim] );
            self->nIncEls[dim][d_i] = ReallocArray( self->incEls[dim][d_i], int, nDoms );
            self->incEls[dim][d_i] = ReallocArray( self->incEls[dim][d_i], int*, nDoms );

            for( e_i = Decomp_GetNumLocals( self->locals[dim] ); e_i < nDoms; e_i++ ) {
                self->nIncEls[dim][d_i][e_i] = 0;
                self->incEls[dim][d_i][e_i] = NULL;
            }
        }
    }
}

void IGraph_AddRemoteElements( void* _self, int dim, int nEls, const int* globals ) {
    IGraph* self = (IGraph*)_self;
    int nOldDoms, nDoms;
    int d_i, e_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || globals );
    nOldDoms = Sync_GetNumDomains( self->remotes[dim] );
    Sync_AddRemotes( self->remotes[dim], nEls, globals );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->nIncEls[dim][d_i] ) {
            nDoms = Sync_GetNumDomains( self->remotes[dim] );
            self->nIncEls[dim][d_i] = ReallocArray( self->nIncEls[dim][d_i], int, nDoms );
            self->incEls[dim][d_i] = ReallocArray( self->incEls[dim][d_i], int*, nDoms );

            for( e_i = nOldDoms; e_i < nDoms; e_i++ ) {
                self->nIncEls[dim][d_i][e_i] = 0;
                self->incEls[dim][d_i][e_i] = NULL;
            }
        }
    }
}

void IGraph_RemoveRemoteElements( void* _self, int dim, int nEls, const int* globals, IMap* map ) {
    /*IGraph* self = (IGraph*)_self;*/

    assert( 0 );
    /* TODO: Method body goes here */
}

void IGraph_SetBoundaryElements( void* _self, int dim, int nEls, const int* els ) {
    IGraph* self = (IGraph*)_self;

    assert( !nEls || els );
    assert( dim < self->nTDims );

    if( !self->nBndEls ) {
        self->nBndEls = AllocArray( int, self->nTDims );
        memset( self->nBndEls, 0, sizeof(int) * self->nTDims );
    }
    if( !self->bndEls ) {
        self->bndEls = AllocArray( int*, self->nTDims );
        memset( self->bndEls, 0, sizeof(int*) * self->nTDims );
    }
    self->nBndEls[dim] = nEls;
    self->bndEls[dim] = ReallocArray( self->bndEls[dim], int, nEls );
    memcpy( self->bndEls[dim], els, sizeof(int) * nEls );
}

void IGraph_SetIncidence( void* _self, int fromDim, int fromEl, int toDim, int nIncEls, const int* incEls  ) {
    IGraph* self = (IGraph*)_self;
    int nDoms;

    assert( self );
    assert( fromDim < self->nTDims && toDim < self->nTDims );
    assert( self->locals[fromDim] );
    if( !self->nIncEls[fromDim][toDim] ) {
        nDoms = Sync_GetNumDomains( self->remotes[fromDim] );
        self->nIncEls[fromDim][toDim] = AllocArray( int, nDoms );
        self->incEls[fromDim][toDim] = AllocArray( int*, nDoms );
        memset( self->incEls[fromDim][toDim], 0, sizeof(int*) * nDoms );
    }

    self->nIncEls[fromDim][toDim][fromEl] = nIncEls;
    self->incEls[fromDim][toDim][fromEl] = ReallocArray( self->incEls[fromDim][toDim][fromEl], int, nIncEls );
    memcpy( self->incEls[fromDim][toDim][fromEl], incEls, nIncEls * sizeof(int) );
}

void IGraph_RemoveIncidence( void* _self, int fromDim, int toDim ) {
    IGraph* self = (IGraph*)_self;
    int nEls;
    int e_i;

    assert( self );
    assert( fromDim < self->nTDims );
    assert( toDim < self->nTDims );

    nEls = Sync_GetNumDomains( self->remotes[fromDim] );
    for( e_i = 0; e_i < nEls; e_i++ )
        FreeArray( self->incEls[fromDim][toDim][e_i] );
    FreeArray( self->incEls[fromDim][toDim] );
    FreeArray( self->nIncEls[fromDim][toDim] );
    self->incEls[fromDim][toDim] = NULL;
    self->nIncEls[fromDim][toDim] = NULL;
}

void IGraph_InvertIncidence( void* _self, int fromDim, int toDim ) {
    IGraph* self = (IGraph*)_self;
    int fromSize, toSize;
    int *nInvIncEls, **invIncEls;
    int *nIncEls, **incEls;
    int elInd;
    int e_i, inc_i;

    assert( self );

    // find the overall number of 'fromSize' topo elements
    fromSize = Sync_GetNumDomains( self->remotes[fromDim] );
    // find the overall number of 'toSize' topo elements
    toSize = Sync_GetNumDomains( self->remotes[toDim] );

    // get the original mapping fromDim->toDim
    nInvIncEls = self->nIncEls[toDim][fromDim];
    invIncEls = self->incEls[toDim][fromDim];

    // build to counts array first
    nIncEls = AllocArray( int, fromSize );
    memset( nIncEls, 0, fromSize * sizeof(int) );
    for( e_i = 0; e_i < toSize; e_i++ ) {
        for( inc_i = 0; inc_i < nInvIncEls[e_i]; inc_i++ )
            nIncEls[invIncEls[e_i][inc_i]]++;
    }

    // allocate the memory for storage
    incEls = AllocArray( int*, fromSize );
    for( e_i = 0; e_i < fromSize; e_i++ )
        incEls[e_i] = AllocArray( int, nIncEls[e_i] );
    memset( nIncEls, 0, fromSize * sizeof(unsigned) ); // re-initialise counts

    // consturct inverse mapping 
    for( e_i = 0; e_i < toSize; e_i++ ) {
        for( inc_i = 0; inc_i < nInvIncEls[e_i]; inc_i++ ) {
            elInd = invIncEls[e_i][inc_i];
            incEls[elInd][nIncEls[elInd]++] = e_i;
        }
    }

    for( e_i = 0; e_i < fromSize; e_i++ ) {
        IGraph_SetIncidence( self, fromDim, e_i, toDim, nIncEls[e_i], incEls[e_i] );
        FreeArray( incEls[e_i] );
    }
    FreeArray( nIncEls );
    FreeArray( incEls );
}

void IGraph_ExpandIncidence( void* _self, int dim ) {
    IGraph* self = (IGraph*)_self;
    ISet nbrSetObj, *nbrSet = &nbrSetObj;
    int nEls;
    int nCurNbrs, maxNbrs;
    int nIncEls, *incEls;
    int nUpEls, *upEls;
    int e_i, inc_i, inc_j;

    assert( self );
    assert( dim < self->nTDims );
    assert( dim > 0 );
    assert( self->nIncEls[dim][0] );

    nEls = Sync_GetNumDomains( self->remotes[dim] );
    maxNbrs = 0;
    for( e_i = 0; e_i < nEls; e_i++ ) {
        nCurNbrs = 0;
        nIncEls = self->nIncEls[dim][0][e_i];
        incEls = self->incEls[dim][0][e_i];
        for( inc_i = 0; inc_i < nIncEls; inc_i++ )
            nCurNbrs += self->nIncEls[0][dim][incEls[inc_i]];
        if( nCurNbrs > maxNbrs )
            maxNbrs = nCurNbrs;
    }

    ISet_Init( nbrSet );
    ISet_SetMaxSize( nbrSet, maxNbrs );
    if( !self->nIncEls[dim][dim] ) {
        self->nIncEls[dim][dim] = AllocArray( int, nEls );
        memset( self->nIncEls[dim][dim], 0, nEls * sizeof(int) );
    }
    if( !self->incEls[dim][dim] ) {
        self->incEls[dim][dim] = AllocArray( int*, nEls );
        memset( self->incEls[dim][dim], 0, nEls * sizeof(int*) );
    }
    for( e_i = 0; e_i < nEls; e_i++ ) {
        nIncEls = self->nIncEls[dim][0][e_i];
        incEls = self->incEls[dim][0][e_i];
        for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
            nUpEls = self->nIncEls[0][dim][incEls[inc_i]];
            upEls = self->incEls[0][dim][incEls[inc_i]];
            for( inc_j = 0; inc_j < nUpEls; inc_j++ ) {
                if( upEls[inc_j] == e_i )
                    continue;
                ISet_TryInsert( nbrSet, upEls[inc_j] );
            }
        }
        self->nIncEls[dim][dim][e_i] = ISet_GetSize( nbrSet );
        self->incEls[dim][dim][e_i] = ReallocArray( self->incEls[dim][dim][e_i], int, self->nIncEls[dim][dim][e_i] );
        ISet_GetArray( nbrSet, self->incEls[dim][dim][e_i] );
        ISet_Clear( nbrSet );
    }
    ISet_Destruct( nbrSet );
}

void _IGraph_SetShadowDepth( void* _self, int depth ) {
    IGraph* self = (IGraph*)_self;
    int nNbrs, nDims;
    Sync* vertSync;
    ISet ghostSetObj, *ghostSet = &ghostSetObj;
    ISet mySetObj, *mySet = &mySetObj;
    IArray* isects;
    int nGhosts, *ghosts;
    int nLocals, nRemotes;
    const int* locals, *remotes;
    int *nNbrGhosts, **nbrGhosts;
    int *nBytes, *nRecvBytes;
    stgByte **bytes, **recvBytes;
    int nIncEls, *incEls;
    int **nLowEls, ***lowEls;
    int *nShdEls, **shdEls;
    int el, dom;
    int n_i, s_i, l_i, inc_i;
    int v_i, g_i, e_i, d_i;

    assert( self && depth >= 0 );
    assert( self->comm && self->nTDims );

    _MeshTopology_SetShadowDepth( self, depth );

    /* Build ghost set. */
    nDims = self->nDims;
    nNbrs = Comm_GetNumNeighbours( self->comm );
    vertSync = self->remotes[0];
    nGhosts = 0;
    for( n_i = 0; n_i < nNbrs; n_i++ )
        nGhosts += vertSync->nSnks[n_i] + vertSync->nSrcs[n_i];
    ISet_Init( ghostSet );
    ISet_SetMaxSize( ghostSet, nGhosts );
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        for( s_i = 0; s_i < vertSync->nSnks[n_i]; s_i++ ) {
            el = Decomp_LocalToGlobal( self->locals[0], vertSync->snks[n_i][s_i] );
            ISet_TryInsert( ghostSet, el );
        }
        for( s_i = 0; s_i < vertSync->nSrcs[n_i]; s_i++ ) {
            el = Sync_RemoteToGlobal( vertSync, vertSync->srcs[n_i][s_i] );
            ISet_TryInsert( ghostSet, el );
        }
    }
    nGhosts = ISet_GetSize( ghostSet );
    ghosts = AllocArray( int, nGhosts );
    ISet_GetArray( ghostSet, ghosts );
    ISet_Destruct( ghostSet );

    /* Gather neighbouring ghost sets. */
    nNbrGhosts = AllocArray( int, nNbrs );
    nbrGhosts = AllocArray( int*, nNbrs );
    Comm_AllgatherInit( self->comm, nGhosts, nNbrGhosts, sizeof(int) );
    for( n_i = 0; n_i < nNbrs; n_i++ )
        nbrGhosts[n_i] = AllocArray( int, nNbrGhosts[n_i] );
    Comm_AllgatherBegin( self->comm, ghosts, (void**)nbrGhosts );
    Comm_AllgatherEnd( self->comm );
    FreeArray( ghosts );

    /* Build intersections. */
    ISet_Init( mySet );
    ISet_SetMaxSize( mySet, Sync_GetNumDomains( vertSync ) );
    Decomp_GetLocals( self->locals[0], &nLocals, &locals );
    for( l_i = 0; l_i < nLocals; l_i++ )
        ISet_Insert( mySet, locals[l_i] );
    Sync_GetRemotes( self->remotes[0], &nRemotes, &remotes );
    for( l_i = 0; l_i < nRemotes; l_i++ )
        ISet_Insert( mySet, remotes[l_i] );
    isects = AllocArray( IArray, nNbrs );
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        IArray_Init( isects + n_i );
        for( g_i = 0; g_i < nNbrGhosts[n_i]; g_i++ ) {
            if( ISet_Has( mySet, nbrGhosts[n_i][g_i] ) )
                IArray_Append( isects + n_i, nbrGhosts[n_i][g_i] );
        }
        FreeArray( nbrGhosts[n_i] );
    }

    /* Convert vertices to shadowed elements. */
    ISet_Clear( mySet );
    ISet_SetMaxSize( mySet, Decomp_GetNumLocals( self->locals[self->nDims] ) );
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        IArray_GetArray( isects + n_i, nNbrGhosts + n_i, (const int**)(nbrGhosts + n_i) );

        for( v_i = 0; v_i < nNbrGhosts[n_i]; v_i++ ) {
            dom = Sync_GlobalToDomain( vertSync, nbrGhosts[n_i][v_i] );

            for( inc_i = 0; inc_i < self->nIncEls[0][nDims][dom]; inc_i++ ) {
                el = self->incEls[0][nDims][dom][inc_i];

                if( el < Decomp_GetNumLocals( self->locals[nDims] ) ) {
                    el = Decomp_LocalToGlobal( self->locals[nDims], el );
                    ISet_TryInsert( mySet, el );
                }
            }
        }
        IArray_Destruct( isects + n_i );
        nNbrGhosts[n_i] = ISet_GetSize( mySet );
        nbrGhosts[n_i] = AllocArray( int, nNbrGhosts[n_i] );
        ISet_GetArray( mySet, nbrGhosts[n_i] );
        ISet_Clear( mySet );
    }
    FreeArray( isects );
    ISet_SetMaxSize( mySet, 0 );

    /* Transfer elements. */
    nShdEls = AllocArray( int, nNbrs );
    shdEls = AllocArray( int*, nNbrs );
    Comm_AlltoallInit( self->comm, nNbrGhosts, nShdEls, sizeof(int) );
    for( n_i = 0; n_i < nNbrs; n_i++ )
        shdEls[n_i] = AllocArray( int, nShdEls[n_i] );
    Comm_AlltoallBegin( self->comm, (const void**)nbrGhosts, (void**)shdEls );
    Comm_AlltoallEnd( self->comm );
    dom = 0;
    for( n_i = 0; n_i < nNbrs; n_i++ )
        dom += nShdEls[n_i];
    ISet_SetMaxSize( mySet, dom );
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        for( e_i = 0; e_i < nShdEls[n_i]; e_i++ )
            ISet_Insert( mySet, shdEls[n_i][e_i] );
        FreeArray( shdEls[n_i] );
    }
    nShdEls[0] = ISet_GetSize( mySet );
    shdEls[0] = AllocArray( int, nShdEls[0] );
    ISet_GetArray( mySet, shdEls[0] );
    ISet_Clear( mySet );
    ISet_SetMaxSize( mySet, 0 );
    qsort( shdEls[0], nShdEls[0], sizeof(int), IGraph_Cmp );
    IGraph_AddRemoteElements( self, nDims, nShdEls[0], shdEls[0] );
    FreeArray( shdEls[0] );

    /* Transfer lower level shadowed elements. */
    for( n_i = 0; n_i < nNbrs; n_i++ ) {
        for( e_i = 0; e_i < nNbrGhosts[n_i]; e_i++ ) {
            nbrGhosts[n_i][e_i] = Decomp_GlobalToLocal( self->locals[nDims], nbrGhosts[n_i][e_i] );
        }
    }
    nLowEls = AllocArray( int*, self->nTDims );
    lowEls = AllocArray( int**, self->nTDims );
    for( d_i = nDims - 1; d_i >= 0; d_i-- ) {
        if( !self->nIncEls[nDims][d_i] ) {
            nLowEls[d_i] = NULL;
            lowEls[d_i] = NULL;
            continue;
        }
        nLowEls[d_i] = AllocArray( int, nNbrs );
        lowEls[d_i] = AllocArray( int*, nNbrs );
        ISet_SetMaxSize( mySet, Decomp_GetNumLocals( self->locals[d_i] ) );

        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            for( e_i = 0; e_i < nNbrGhosts[n_i]; e_i++ ) {
                nIncEls = self->nIncEls[nDims][d_i][nbrGhosts[n_i][e_i]];
                incEls = self->incEls[nDims][d_i][nbrGhosts[n_i][e_i]];

                for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
                    if( incEls[inc_i] >= Decomp_GetNumLocals( self->locals[d_i] ) )
                        continue;
                    el = Decomp_LocalToGlobal( self->locals[d_i], incEls[inc_i] );
                    ISet_TryInsert( mySet, el );
                }
            }
            nLowEls[d_i][n_i] = ISet_GetSize( mySet );
            lowEls[d_i][n_i] = AllocArray( int, nLowEls[d_i][n_i] );
            ISet_GetArray( mySet, lowEls[d_i][n_i] );
            ISet_Clear( mySet );
        }

        Comm_AlltoallInit( self->comm, nLowEls[d_i], nShdEls, sizeof(int) );
        for( n_i = 0; n_i < nNbrs; n_i++ )
            shdEls[n_i] = AllocArray( int, nShdEls[n_i] );
        Comm_AlltoallBegin( self->comm, (const void**)lowEls[d_i], (void**)shdEls );
        Comm_AlltoallEnd( self->comm );

        dom = 0;
        for( n_i = 0; n_i < nNbrs; n_i++ )
            dom += nShdEls[n_i];
        ISet_SetMaxSize( mySet, dom );
        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            for( s_i = 0; s_i < nShdEls[n_i]; s_i++ ) {
                if( !Sync_TryGlobalToDomain( self->remotes[d_i], shdEls[n_i][s_i], &el ) ) {
                    ISet_Insert( mySet, shdEls[n_i][s_i] );
                }
            }
            FreeArray( shdEls[n_i] );
        }
        nShdEls[0] = ISet_GetSize( mySet );
        shdEls[0] = AllocArray( int, nShdEls[0] );
        ISet_GetArray( mySet, shdEls[0] );
        ISet_Clear( mySet );
        qsort( shdEls[0], nShdEls[0], sizeof(int), IGraph_Cmp );
        IGraph_AddRemoteElements( self, d_i, nShdEls[0], shdEls[0] );
        FreeArray( shdEls[0] );
    }
    ISet_Destruct( mySet );
    FreeArray( shdEls );
    FreeArray( nShdEls );

    /* Transfer shadowed incidence. */
    nBytes = AllocArray( int, nNbrs );
    bytes = AllocArray( stgByte*, nNbrs );
    nRecvBytes = AllocArray( int, nNbrs );
    recvBytes = AllocArray( stgByte*, nNbrs );
    nLowEls[nDims] = nNbrGhosts;
    lowEls[nDims] = nbrGhosts;
    for( d_i = 0; d_i < nDims; d_i++ ) {
        if( !nLowEls[d_i] )
            continue;
        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            for( e_i = 0; e_i < nLowEls[d_i][n_i]; e_i++ ) {
                lowEls[d_i][n_i][e_i] = Decomp_GlobalToLocal( self->locals[d_i], lowEls[d_i][n_i][e_i] );
            }
        }
    }
    for( d_i = nDims; d_i >= 0; d_i-- ) {
        if( !nLowEls[d_i] )
            continue;
        if( d_i == 0 ) {
            for( n_i = 0; n_i < nNbrs; n_i++ )
                FreeArray( lowEls[0][n_i] );
            FreeArray( lowEls[0] );
            FreeArray( nLowEls[0] );
            continue;
        }

        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            IGraph_PickleIncidenceInit( self, d_i, nLowEls[d_i][n_i], lowEls[d_i][n_i], nBytes + n_i );
            bytes[n_i] = AllocArray( stgByte, nBytes[n_i] );
            IGraph_PickleIncidence( self, d_i, nLowEls[d_i][n_i], lowEls[d_i][n_i], bytes[n_i] );
            FreeArray( lowEls[d_i][n_i] );
        }
        FreeArray( nLowEls[d_i] );
        FreeArray( lowEls[d_i] );

        Comm_AlltoallInit( self->comm, nBytes, nRecvBytes, sizeof(stgByte) );
        for( n_i = 0; n_i < nNbrs; n_i++ )
            recvBytes[n_i] = AllocArray( stgByte, nRecvBytes[n_i] );
        Comm_AlltoallBegin( self->comm, (const void**)bytes, (void**)recvBytes );
        Comm_AlltoallEnd( self->comm );
        for( n_i = 0; n_i < nNbrs; n_i++ )
            FreeArray( bytes[n_i] );

        for( n_i = 0; n_i < nNbrs; n_i++ ) {
            IGraph_UnpickleIncidence( self, d_i, nRecvBytes[n_i], recvBytes[n_i] );
            FreeArray( recvBytes[n_i] );
        }
    }
    FreeArray( nBytes );
    FreeArray( bytes );
    FreeArray( lowEls );
    FreeArray( nLowEls );
    FreeArray( recvBytes );
    FreeArray( nRecvBytes );
}

void IGraph_Clear( void* self ) {
    IGraph_ClearDims( self );
    if( ((IGraph*)self)->comm )
        Stg_Class_RemoveRef( ((IGraph*)self)->comm );
    ((IGraph*)self)->comm = NULL;
}

void IGraph_ClearDims( void* _self ) {
    IGraph* self = (IGraph*)_self;
    int d_i;

    IGraph_ClearElements( self );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->locals && self->locals[d_i] )
            Stg_Class_RemoveRef( self->locals[d_i] );
        if( self->remotes && self->remotes[d_i] )
            Stg_Class_RemoveRef( self->remotes[d_i] );
    }
    FreeArray( self->locals );
    FreeArray( self->remotes );
    FreeArray( self->nIncEls );
    FreeArray( self->incEls );

    self->nDims = 0;
    self->nTDims = 0;
    self->shadDepth = 0;
    self->locals = NULL;
    self->remotes = NULL;
    self->nIncEls = NULL;
    self->incEls = NULL;
}

void IGraph_ClearElements( void* _self ) {
    IGraph* self = (IGraph*)_self;
    int d_i;

    IGraph_ClearIncidence( self );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        if( self->locals )
            Decomp_ClearLocals( self->locals[d_i] );
        if( self->remotes && self->remotes[d_i] )
            Sync_ClearRemotes( self->remotes[d_i] );
        if( self->bndEls )
            FreeArray( self->bndEls[d_i] );
    }
    FreeArray( self->bndEls );
    FreeArray( self->nBndEls );
    self->bndEls = NULL;
    self->nBndEls = NULL;
}

void IGraph_ClearIncidence( void* _self ) {
    IGraph* self = (IGraph*)_self;
    int d_i, d_j, e_i;

    assert( self );
    for( d_i = 0; d_i < self->nTDims; d_i++ ) {
        for( d_j = 0; d_j < self->nTDims; d_j++ ) {
            if( self->nIncEls[d_i][d_j] ) {
                for( e_i = 0; e_i < Sync_GetNumDomains( self->remotes[d_i] ); e_i++ )
                    FreeArray( self->incEls[d_i][d_j][e_i] );
                FreeArray( self->incEls[d_i][d_j] );
                FreeArray( self->nIncEls[d_i][d_j] );
                self->nIncEls[d_i][d_j] = NULL;
                self->incEls[d_i][d_j] = NULL;
            }
        }
    }
}

int IGraph_GetNumDims( const void* self ) {
    assert( self );
    return ((IGraph*)self)->nDims;
}

const Comm* IGraph_GetComm( const void* self ) {
    assert( self );
    return ((IGraph*)self)->comm;
}

Bool IGraph_HasDomain( const void* self, int dim ) {
    assert( self && dim < ((IGraph*)self)->nTDims );
    return Sync_GetNumDomains( ((IGraph*)self)->remotes[dim] ) ? 
        True : False;
}

const Sync* IGraph_GetDomain( const void* self, int dim ) {
    assert( self && dim < ((IGraph*)self)->nTDims );
    return ((IGraph*)self)->remotes[dim];
}

void IGraph_GetBoundaryElements( const void* self, int dim, int* nEls, const int** els ) {
    assert( dim < ((IGraph*)self)->nTDims );
    assert( nEls );
    assert( els );

    *nEls = ((IGraph*)self)->nBndEls ? ((IGraph*)self)->nBndEls[dim] : 0;
    *els = ((IGraph*)self)->bndEls ? ((IGraph*)self)->bndEls[dim] : NULL;
}

Bool IGraph_HasIncidence( const void* self, int fromDim, int toDim ) {
    assert( self );
    assert( fromDim < ((IGraph*)self)->nTDims );
    assert( toDim < ((IGraph*)self)->nTDims );
    return ((IGraph*)self)->nIncEls[fromDim][toDim] ? True : False;
}

int IGraph_GetIncidenceSize( const void* self, int fromDim, int fromEl, int toDim ) {
    assert( self );
    assert( fromDim < ((IGraph*)self)->nTDims );
    assert( toDim < ((IGraph*)self)->nTDims );
    assert( fromEl < Sync_GetNumDomains( ((IGraph*)self)->remotes[fromDim] ) );
    return ((IGraph*)self)->nIncEls[fromDim][toDim][fromEl];
}

void _IGraph_GetIncidence( void* self, int fromDim, int fromEl, int toDim, IArray* inc ) {
    assert( self );
    assert( fromDim < ((IGraph*)self)->nTDims );
    assert( toDim < ((IGraph*)self)->nTDims );
    assert( fromEl < Sync_GetNumDomains( ((IGraph*)self)->remotes[fromDim] ) );
    assert( inc );

    IArray_SoftResize( inc, ((IGraph*)self)->nIncEls[fromDim][toDim][fromEl] );
    memcpy( inc->ptr, ((IGraph*)self)->incEls[fromDim][toDim][fromEl], IArray_GetSize( inc ) * sizeof(int) );
}

void IGraph_PrintIncidence( const void* _self, int fromDim, int toDim ) {
    IGraph* self = (IGraph*)_self;
    int nEls, global;
    int nIncEls, *incEls;
    int e_i, inc_i;

    assert( self );
    assert( toDim < self->nTDims );
    assert( fromDim < self->nTDims );

    nEls = Sync_GetNumDomains( self->remotes[fromDim] );
    printf( "Printing incidence for %d elements:\n", nEls );
    for( e_i = 0; e_i < nEls; e_i++ ) {
        global = Sync_DomainToGlobal( self->remotes[fromDim], e_i );
        nIncEls = self->nIncEls[fromDim][toDim][e_i];
        incEls = self->incEls[fromDim][toDim][e_i];
        printf( "   %d, %d incident elements:\n", global, nIncEls );
        for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
            printf( "      %d\n", incEls[inc_i] );
        }
    }
}

void IGraph_PickleIncidenceInit( IGraph* self, int dim, int nEls, int* els, int* nBytes ) {
    int size;
    int d_i, e_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || els );
    assert( nBytes );

    size = 1;
    for( e_i = 0; e_i < nEls; e_i++ ) {
        size += 1;
        for( d_i = 0; d_i < dim; d_i++ ) {
            size += 1;
            if( self->nIncEls[dim][d_i] )
                size += self->nIncEls[dim][d_i][els[e_i]];
        }
    }
    *nBytes = size * sizeof(int);
}

void IGraph_PickleIncidence( IGraph* self, int dim, int nEls, int* els, stgByte* bytes ) {
    Sync* sync;
    int curEntry, *entries;
    int nIncEls, *incEls;
    int inc_i, d_i, e_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( !nEls || els );
    assert( bytes );

    sync = self->remotes[dim];
    entries = (int*)bytes;
    entries[0] = nEls;
    curEntry = 1;
    for( e_i = 0; e_i < nEls; e_i++ ) {
        entries[curEntry++] = Sync_DomainToGlobal( sync, els[e_i] );
        for( d_i = 0; d_i < dim; d_i++ ) {
            if( !self->nIncEls[dim][d_i] ) {
                entries[curEntry++] = 0;
                continue;
            }
            nIncEls = self->nIncEls[dim][d_i][els[e_i]];
            entries[curEntry++] = nIncEls;
            incEls = self->incEls[dim][d_i][els[e_i]];
            for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
                entries[curEntry++] = Sync_DomainToGlobal( self->remotes[d_i], incEls[inc_i] );
            }
        }
    }
}

void IGraph_UnpickleIncidence( IGraph* self, int dim, int nBytes, stgByte* bytes ) {
    Sync* sync;
    int nEls, el;
    int nIncEls, **incEls;
    int curEntry, *entries;
    int inc_i, e_i, d_i;

    assert( self );
    assert( dim < self->nTDims );
    assert( nBytes && bytes );

    sync = self->remotes[dim];
    entries = (int*)bytes;
    nEls = entries[0];
    curEntry = 1;
    for( e_i = 0; e_i < nEls; e_i++ ) {
        el = Sync_GlobalToDomain( sync, entries[curEntry++] );
        for( d_i = 0; d_i < dim; d_i++ ) {
            nIncEls = entries[curEntry++];
            if( !self->nIncEls[dim][d_i] ) {
                if( !nIncEls )
                    continue;
                self->nIncEls[dim][d_i] = AllocArray( int, Sync_GetNumDomains( sync ) );
                memset( self->nIncEls[dim][d_i], 0, sizeof(int) * Sync_GetNumDomains( sync ) );
            }
            self->nIncEls[dim][d_i][el] = nIncEls;
            if( !nIncEls ) {
                if( self->incEls[dim][d_i] )
                    FreeArray( self->incEls[dim][d_i][el] );
                continue;
            }
            if( !self->incEls[dim][d_i] ) {
                self->incEls[dim][d_i] = AllocArray( int*, Sync_GetNumDomains( sync ) );
                memset( self->incEls[dim][d_i], 0, 
                        Sync_GetNumDomains( sync ) * sizeof(int*) );
            }
            incEls = self->incEls[dim][d_i];
            incEls[el] = ReallocArray( incEls[el], int, nIncEls );
            for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
                incEls[el][inc_i] = Sync_GlobalToDomain( self->remotes[d_i], entries[curEntry++] );
            }
        }
    }
}

int IGraph_Cmp( const void* l, const void* r ) {
    assert( *(int*)l != *(int*)r );
    return (*(int*)l < *(int*)r) ? -1 : 1;
}
