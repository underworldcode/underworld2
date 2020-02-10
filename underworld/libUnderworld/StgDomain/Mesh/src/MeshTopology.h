/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StgDomain_Mesh_MeshTopology_h__
#define __StgDomain_Mesh_MeshTopology_h__

extern const Type MeshTopology_Type;

typedef void (MeshTopology_SetCommFunc)( void* self, const Comm* comm );
typedef void (MeshTopology_SetNumDimsFunc)( void* self, int nDims );
typedef void (MeshTopology_SetShadowDepthFunc)( void* self, int depth );
typedef int (MeshTopology_GetNumCellsFunc)( void* self, int dim );
typedef void (MeshTopology_GetIncidenceFunc)( void* self, int fromDim, int fromEl, int toDim, IArray* inc );
        
#define __MeshTopology                                          \
    __Stg_Class                                                 \
    MeshTopology_SetCommFunc* setCommFunc;                      \
    MeshTopology_SetNumDimsFunc* setNumDimsFunc;                \
    MeshTopology_SetShadowDepthFunc* setShadowDepthFunc;        \
    MeshTopology_GetNumCellsFunc* getNumCellsFunc;              \
    MeshTopology_GetIncidenceFunc* getIncidenceFunc;            \
    int nDims;                                                  \
    int nTDims;                                                 \
    int shadDepth;                                              \
    Comm* comm;

struct MeshTopology { __MeshTopology };

#ifndef ZERO
#define ZERO 0
#endif

#define MESHTOPOLOGY_DEFARGS                                    \
    STG_CLASS_DEFARGS,                                          \
        MeshTopology_SetCommFunc* setCommFunc,                  \
        MeshTopology_SetNumDimsFunc* setNumDimsFunc,            \
        MeshTopology_SetShadowDepthFunc* setShadowDepthFunc,    \
        MeshTopology_GetNumCellsFunc* getNumCellsFunc,          \
        MeshTopology_GetIncidenceFunc* getIncidenceFunc

#define MESHTOPOLOGY_PASSARGS                                   \
    STG_CLASS_PASSARGS,                                         \
        setCommFunc, setNumDimsFunc, setShadowDepthFunc,        \
        getNumCellsFunc, getIncidenceFunc

MeshTopology* _MeshTopology_New( MESHTOPOLOGY_DEFARGS );

void _MeshTopology_Init( void* self );

void MeshTopology_Destruct( MeshTopology* self );

void _MeshTopology_Delete( void* self );

void MeshTopology_Copy( void* self, const void* op );

void _MeshTopology_SetNumDims( void* _self, int nDims );

void _MeshTopology_SetComm( void* _self, const Comm* comm );

void _MeshTopology_SetShadowDepth( void* _self, int depth );

int MeshTopology_GetNumDims( const void* self );

const Comm* MeshTopology_GetComm( const void* self );

#define MeshTopology_SetShadowDepth( self, depth )      \
    (self)->setShadowDepthFunc( self, depth )

#define MeshTopology_GetIncidence( self, fromDim, fromEl, toDim, inc )  \
    (self)->getIncidenceFunc( self, fromDim, fromEl, toDim, inc )

#define MeshTopology_SetNumDims( self, nDims )  \
    (self)->setNumDimsFunc( self, nDims )

#define MeshTopology_SetComm( self, comm )      \
    (self)->setCommFunc( self, comm )

#define MeshTopology_GetNumCells( self, dim )   \
    (self)->getNumCellsFunc( self, dim )

#endif /* __StgDomain_Mesh_MeshTopology_h__ */
