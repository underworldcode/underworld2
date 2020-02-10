/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StgDomain_Mesh_IGraph_h__
#define __StgDomain_Mesh_IGraph_h__

extern const Type IGraph_Type;
        
#define __IGraph                                \
    __MeshTopology                              \
    Decomp** locals;                            \
    Sync** remotes;                             \
    int* nBndEls;                               \
    int** bndEls;                               \
    int*** nIncEls;                             \
    int**** incEls;

struct IGraph { __IGraph };

#ifndef ZERO
#define ZERO 0
#endif

#define IGRAPH_DEFARGS                          \
    MESHTOPOLOGY_DEFARGS

#define IGRAPH_PASSARGS                         \
    MESHTOPOLOGY_PASSARGS

IGraph* IGraph_New();

IGraph* _IGraph_New( IGRAPH_DEFARGS );

void IGraph_Init( IGraph* self );

void _IGraph_Init( void* self );

void IGraph_Destruct( IGraph* self );

void _IGraph_Delete( void* self );

void _IGraph_SetNumDims( void* _self, int nDims );

void IGraph_SetComm( void* _self, const Comm* comm );

void IGraph_SetDomain( void* _self, int dim, Sync* sync );

void IGraph_SetElements( void* _self, int dim, int nEls, const int* globals );

void IGraph_SetLocalElements( void* _self, int dim, int nEls, const int* globals );

void IGraph_AddLocalElements( void* _self, int dim, int nEls, const int* globals );

void IGraph_RemoveLocalElements( void* _self, int dim, int nEls, const int* globals, IMap* map );

void IGraph_SetRemoteElements( void* _self, int dim, int nEls, const int* globals );

void IGraph_AddRemoteElements( void* _self, int dim, int nEls, const int* globals );

void IGraph_RemoveRemoteElements( void* _self, int dim, int nEls, const int* globals, IMap* map );

void IGraph_SetBoundaryElements( void* _self, int dim, int nEls, const int* els );

void IGraph_SetIncidence( void* _self, int fromDim, int fromEl, int toDim, int nIncEls, const int* incEls  );

void IGraph_RemoveIncidence( void* _self, int fromDim, int toDim );

void IGraph_InvertIncidence( void* _self, int fromDim, int toDim );

void IGraph_ExpandIncidence( void* _self, int dim );

void _IGraph_SetShadowDepth( void* _self, int depth );

void IGraph_Clear( void* self );

void IGraph_ClearDims( void* _self );

void IGraph_ClearElements( void* _self );

void IGraph_ClearIncidence( void* _self );

int IGraph_GetNumDims( const void* self );

const Comm* IGraph_GetComm( const void* self );

Bool IGraph_HasDomain( const void* self, int dim );

const Sync* IGraph_GetDomain( const void* self, int dim );

void IGraph_GetBoundaryElements( const void* self, int dim, int* nEls, const int** els );

Bool IGraph_HasIncidence( const void* self, int fromDim, int toDim );

int IGraph_GetIncidenceSize( const void* self, int fromDim, int fromEl, int toDim );

void _IGraph_GetIncidence( void* self, int fromDim, int fromEl, int toDim, IArray* inc );

void IGraph_PrintIncidence( const void* _self, int fromDim, int toDim );

#endif /* __StgDomain_Mesh_IGraph_h__ */
