/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_STree_h__
#define __StGermain_Base_Container_STree_h__

extern const Type STree_Type;

#define __STree                                 \
    __Stg_Class                                 \
    STree_CompareCB* cmp;                       \
    STree_DeleteCB* del;                        \
    int itmSize;                                \
    STreeNode* root;                            \
    int nNodes;                                 \
    int maxNodes;                               \
    float alpha;                                \
    float invAlpha;                             \
    int curDepth;                               \
    int curSize;                                \
    int flip;

struct STree { __STree };

#ifndef ZERO
#define ZERO 0
#endif

#define STREE_DEFARGS                           \
    STG_CLASS_DEFARGS

#define STREE_PASSARGS                          \
    STG_CLASS_PASSARGS

STree* STree_New();

STree* _STree_New( STREE_DEFARGS );

void STree_Init( STree* self );

void _STree_Init( void* self );

void STree_Destruct( STree* self );

void _STree_Delete( void* self );

void STree_Copy( void* self, const void* op );

void STree_SetCallbacks( void* _self, STree_CompareCB* cmp, STree_DeleteCB* del );

void STree_SetIntCallbacks( void* _self );

void STree_SetItemSize( void* _self, int itmSize );

void STree_SetAlpha( void* _self, float alpha );

void STree_Insert( void* _self, const void* itm );

void STree_Remove( void* _self, const void* itm );

void STree_Clear( void* _self );

int STree_GetSize( const void* _self );

const STreeNode* STree_GetRoot( const void* _self );

Bool STree_Has( const void* _self, const void* itm );

int STree_Size( const STreeNode *node );

int STree_IntCmp( const void* left, const void* right );

void STree_IntDel( void* itm );

#endif /* __StGermain_Base_Container_STree_h__ */
