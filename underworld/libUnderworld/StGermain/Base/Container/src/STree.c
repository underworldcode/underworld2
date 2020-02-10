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
#include <math.h>
#include <assert.h>
#include <StGermain/Base/Foundation/Foundation.h>
#include "types.h"
#include "STree.h"


#define INV_LOG2 1.442695041


const Type STree_Type = "STree";


STreeNode* STree_Rebalance( void* _self, STreeNode *root, int nNodes );
void STree_Flatten( STreeNode *pseudo );
void STree_Grow( STreeNode *pseudo, int nNodes );
void STree_Compression( STreeNode *pseudo, int nSpineNodes );
void STreeNode_Insert( STreeNode *self, STreeNode *node, STree *tree, STreeNode **par );
void STreeNode_Destroy( STreeNode *self, STree *tree );
int STree_IntCmp( const void* left, const void* right );
void STree_IntDel( void* itm );


int log2i( int x ) {
    int e = 0;
    while((x >> e) != 1 )
	e++;
    return e;
}


STree* STree_New() {
    STree* self;
    SizeT _sizeOfSelf = sizeof(STree);
    Type type = STree_Type;
    Stg_Class_DeleteFunction* _delete = _STree_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _STree_New( STREE_PASSARGS );
    return self;
}

STree* _STree_New( STREE_DEFARGS ) {
    STree* self;

    self = (STree*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _STree_Init( self );
    return self;
}

void STree_Init( STree* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _STree_Init( self );
}

void _STree_Init( void* _self ) {
   STree* self = (STree*)_self;

   self->root = NULL;
   self->nNodes = 0;
   self->maxNodes = 0;
   self->alpha = 0.65;
   self->invAlpha = 1.0 / 0.65;
   self->curDepth = 0;
   self->curSize = 0;
   self->flip= 0;
}

void STree_Destruct( STree* self ) {
   if( self->root )
      STreeNode_Destroy( self->root, self );
}

void _STree_Delete( void* _self ) {
   STree* self = (STree*)_self;

   if( self->root )
      STreeNode_Destroy( self->root, self );
   _Stg_Class_Delete( self );
}

void STree_Copy( void* _self, const void* _op ) {
   abort();
}

void STree_SetCallbacks( void* _self, STree_CompareCB* cmp, STree_DeleteCB* del ) {
   STree* self = (STree*)_self;

   STree_Clear( self );
   self->cmp = cmp;
   self->del = del;
}

void STree_SetIntCallbacks( void* _self ) {
   STree* self = (STree*)_self;

   STree_Clear( self );
   self->cmp = STree_IntCmp;
   self->del = STree_IntDel;
}

void STree_SetItemSize( void* _self, int itmSize ) {
   STree* self = (STree*)_self;

   STree_Clear( self );
   self->itmSize = itmSize;
}

void STree_SetAlpha( void* _self, float alpha ) {
   STree* self = (STree*)_self;

   assert( alpha >= 0.5 && alpha <= 1.0 );
   self->alpha = alpha;
   self->invAlpha = 1.0 / alpha;
}

void STree_Insert( void* _self, const void* itm ) {
   STree* self = (STree*)_self;
   STreeNode* node;

   assert( itm );
   node = AllocArray( STreeNode, 1 );
   node->left = NULL;
   node->right = NULL;
   node->data = AllocArray( stgByte, self->itmSize );
   memcpy( node->data, itm, self->itmSize );
   if ( self->root ) {
      STreeNode_Insert( self->root, node, self, &self->root );
      self->curDepth = 0;
   }
   else
      self->root = node;
   if ( ++self->nNodes > self->maxNodes )
      self->maxNodes = self->nNodes;
}

void STree_Remove( void* _self, const void* itm ) {
   STree* self = (STree*)_self;
   STreeNode *cur = self->root, **pre = &self->root;
   int res;

   assert( itm );
   assert( self->cmp );
   while( (res = self->cmp( itm, cur->data )) ) {
      if ( res < 0 ) {
	 pre = &cur->left;
	 cur = cur->left;
      }
      else {
	 pre = &cur->right;
	 cur = cur->right;
      }
   }
   assert( cur );
   if ( !cur->left )
      *pre = cur->right;
   else if ( !cur->right )
      *pre = cur->left;
   else if ( !cur->left->right ) {
      *pre = cur->left;
      cur->left->right = cur->right;
   }
   else if ( !cur->right->left ) {
      *pre = cur->right;
      cur->right->left = cur->left;
   }
   else if ( self->flip ) {
      STreeNode *last = cur->left, *preLast;
      while ( last->right ) {
	 preLast = last;
	 last = last->right;
      }
      preLast->right = last->left;
      last->left = cur->left;
      last->right = cur->right;
      *pre = last;
      self->flip = 0;
   }
   else {
      STreeNode *last = cur->right, *preLast;
      while ( last->left ) {
	 preLast = last;
	 last = last->left;
      }
      preLast->left = last->right;
      last->right = cur->right;
      last->left = cur->left;
      *pre = last;
      self->flip = 1;
   }
   self->del( cur->data );
   FreeArray( cur->data );
   FreeArray( cur );
   if ( --self->nNodes <= self->maxNodes / 2 ) {
      self->root = STree_Rebalance( self, self->root, self->nNodes );
      self->maxNodes = self->nNodes;
   }
}

void STree_Clear( void* _self ) {
   STree* self = (STree*)_self;

   if ( self->root ) {
      STreeNode_Destroy( self->root, self );
      self->root = NULL;
      self->nNodes = 0;
      self->maxNodes = 0;
      self->curDepth = 0;
      self->curSize = 0;
      self->flip = 0;
   }
}

int STree_GetSize( const void* _self ) {
   const STree* self = (const STree*)_self;

   return self->nNodes;
}

const STreeNode* STree_GetRoot( const void* _self ) {
   const STree* self = (const STree*)_self;

   return self->root;
}

Bool STree_Has( const void* _self, const void* itm ) {
   STree* self = (STree*)_self;
   STreeNode *cur = self->root;
   int res;

   assert( self->cmp );
   while ( cur && (res = self->cmp( itm, cur->data )) )
      cur = (res < 0) ? cur->left : cur->right;

   return cur ? True : False;
}

int STree_Size( const STreeNode *node ) {
   if ( node )
      return STree_Size( node->left ) + STree_Size( node->right ) + 1;
   else
      return 0;
}

STreeNode* STree_Rebalance( void* _self, STreeNode *root, int nNodes ) {
   //STree* self = (STree*)_self;
   STreeNode *pseudo, *tmp;

   pseudo = AllocArray( STreeNode, 1 );
   pseudo->left = NULL;
   pseudo->right = root;
   pseudo->data = NULL;
   STree_Flatten( pseudo );
   STree_Grow( pseudo, nNodes );
   tmp = pseudo->right;
   FreeArray( pseudo );

   return tmp;
}

void STree_Flatten( STreeNode *pseudo ) {
   STreeNode *tail, *rem;

   assert( pseudo );
   tail = pseudo;
   rem = tail->right;
   while ( rem ) {
      if ( rem->left ) {
	 STreeNode *tmp = rem->left;
	 rem->left = tmp->right;
	 tmp->right = rem;
	 rem = tmp;
	 tail->right = tmp;
      }
      else {
	 tail = rem;
	 rem = rem->right;
      }
   }
}

void STree_Grow( STreeNode *pseudo, int nNodes ) {
   int nLeaves, nSpineNodes;

   nLeaves = nNodes + 1 - ( 1 << log2i( nNodes + 1 ) );
   STree_Compression( pseudo, nLeaves );
   nSpineNodes = nNodes - nLeaves;
   while ( nSpineNodes > 1 ) {
      nSpineNodes /= 2;
      STree_Compression( pseudo, nSpineNodes );
   }
}

void STree_Compression( STreeNode *pseudo, int nSpineNodes ) {
   STreeNode *scan = pseudo;
   int n_i;

   for ( n_i = 0; n_i < nSpineNodes; n_i++ ) {
      STreeNode *child = scan->right;
      scan->right = child->right;
      scan = scan->right;
      child->right = scan->left;
      scan->left = child;
   }
}

void STreeNode_Insert( STreeNode *self, STreeNode *node, STree *tree, STreeNode **par ) {
   STreeNode **child;
   int res;

   assert( tree->cmp );
   res = tree->cmp( node->data, self->data );
   assert( res );
   tree->curDepth++;
   child = (res < 0) ? &self->left : &self->right;
   if ( *child )
      STreeNode_Insert( *child, node, tree, child );
   else {
      int height;
      *child = node;
      height = (int)((log( (float)(tree->nNodes + 1) ) * INV_LOG2) /
                     (log( tree->invAlpha ) * INV_LOG2));
      if ( tree->curDepth > height )
	 tree->curSize = 1;
   }
   if ( tree->curSize ) {
      STreeNode **bro = (res < 0) ? &self->right : &self->left;
      int broSize = STree_Size( *bro );
      int nodeSize = tree->curSize + broSize + 1;
      float weight = tree->alpha * ( float )nodeSize;

      if (( float )tree->curSize > weight ||
	  ( float )broSize > weight )
      {
	 *par = STree_Rebalance( tree, self, nodeSize );
	 tree->curSize = 0;
      }
      else
	 tree->curSize = nodeSize;
   }
}

void STreeNode_Destroy( STreeNode *self, STree *tree ) {
   if ( self->left )
      STreeNode_Destroy( self->left, tree );
   if ( self->right )
      STreeNode_Destroy( self->right, tree );
   tree->del( self->data );
   FreeArray( self->data );
   FreeArray( self );
}

int STree_IntCmp( const void* left, const void* right ) {
   return (*((int*)left) < *((int*)right)) ? -1 : 
      (*((int*)left) > *((int*)right)) ? 1 : 0;
}

void STree_IntDel( void* itm ) {
}


