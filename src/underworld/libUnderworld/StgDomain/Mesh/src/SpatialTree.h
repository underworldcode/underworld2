/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StgDomain_Mesh_SpatialTree_h__
#define __StgDomain_Mesh_SpatialTree_h__

extern const Type SpatialTree_Type;
        
#define __SpatialTree                           \
    __Stg_Class                                 \
    Mesh* mesh;                                 \
    int nDims;                                  \
    double* min;                                \
    double* max;                                \
    int nChld;                                  \
    void* root;                                 \
    int tol;                                    \
    int nNodes;

struct SpatialTree { __SpatialTree };

#ifndef ZERO
#define ZERO 0
#endif

#define SPATIALTREE_DEFARGS                     \
    STG_CLASS_DEFARGS

#define SPATIALTREE_PASSARGS                    \
    STG_CLASS_PASSARGS

SpatialTree* SpatialTree_New();

SpatialTree* _SpatialTree_New( SPATIALTREE_DEFARGS );

void SpatialTree_Init( SpatialTree* self );

void _SpatialTree_Init( void* self );

void SpatialTree_Destruct( SpatialTree* self );

void _SpatialTree_Delete( void* self );

void SpatialTree_Copy( void* self, const void* op );

void SpatialTree_SetMesh( void* _self, void* mesh );

void SpatialTree_Rebuild( void* _self );

Bool SpatialTree_Search( void* _self, const double* pnt, int* nEls, int** els );

void SpatialTree_Clear( void* _self );

#endif /* __StgDomain_Mesh_SpatialTree_h__ */
