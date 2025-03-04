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
#include <StGermain/Base/Foundation/src/Foundation.h>
#include "types.h"
#include "STree.h"
#include "STreeMap.h"


const Type STreeMap_Type = "STreeMap";


STreeMap* STreeMap_New() {
    STreeMap* self;
    SizeT _sizeOfSelf = sizeof(STreeMap);
    Type type = STreeMap_Type;
    Stg_Class_DeleteFunction* _delete = _STreeMap_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _STreeMap_New( STREEMAP_PASSARGS );
    return self;
}

STreeMap* _STreeMap_New( STREEMAP_DEFARGS ) {
    STreeMap* self;

    self = (STreeMap*)_STree_New( STREE_PASSARGS );
    _STreeMap_Init( self );
    return self;
}

void STreeMap_Init( STreeMap* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _STree_Init( self );
    _STreeMap_Init( self );
}

void _STreeMap_Init( void* _self ) {
   STreeMap* self = (STreeMap*)_self;

   self->keySize = 0;
   self->valSize = 0;
}

void STreeMap_Destruct( STreeMap* self ) {
}

void _STreeMap_Delete( void* self ) {
   _STree_Delete( self );
}

void STreeMap_SetItemSize( void* _self, int keySize, int valSize ) {
   STreeMap* self = (STreeMap*)_self;

   STree_SetItemSize( self, keySize + valSize );
   self->keySize = keySize;
   self->valSize = valSize;
}

void STreeMap_Insert( void* _self, const void* key, const void* val ) {
   STreeMap* self = (STreeMap*)_self;
   stgByte* tmp;

   tmp = (stgByte*)malloc( self->keySize + self->valSize );
   assert( key );
   memcpy( tmp, key, self->keySize );
   if( val )
      memcpy( tmp + self->keySize, val, self->valSize );
   else
      memset( tmp + self->keySize, 0, self->valSize );
   STree_Insert( self, tmp );
   if( tmp )
     free( tmp );
}

void STreeMap_Remove( void* _self, const void* key ) {
   STreeMap* self = (STreeMap*)_self;

   STree_Remove( self, key );
}

void* STreeMap_Map( const void* _self, const void* key ) {
   STreeMap* self = (STreeMap*)_self;
   const STreeNode* node;
   int res;

   assert( self->root );
   node = self->root;
   while( node && (res = self->cmp( key, node->data )) )
     node = (res < 0) ? node->left : node->right;
   assert( node );

   return node->data + self->keySize;
}

Bool STreeMap_HasKey( const void* _self, const void* key ) {
   STreeMap* self = (STreeMap*)_self;

   return STree_Has( self, key );
}


