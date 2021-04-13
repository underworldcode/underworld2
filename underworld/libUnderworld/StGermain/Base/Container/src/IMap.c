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
#include <assert.h>
#include <StGermain/pcu/src/pcu.h>
#include <StGermain/Base/Foundation/src/Foundation.h>
#include "types.h"
#include "Iter.h"
#include "IMapIter.h"
#include "IMap.h"


const Type IMap_Type = "IMap";
const double IMap_TableFactor = 1.18;


IMap* IMap_New() {
    IMap* self;
    SizeT _sizeOfSelf = sizeof(IMap);
    Type type = IMap_Type;
    Stg_Class_DeleteFunction* _delete = _IMap_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _IMap_New( IMAP_PASSARGS );
    return self;
}

IMap* _IMap_New( IMAP_DEFARGS ) {
    IMap* self;

    self = (IMap*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _IMap_Init( self );
    return self;
}

void IMap_Init( IMap* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _IMap_Init( self );
}

void _IMap_Init( void* _self ) {
   IMap* self = (IMap*)_self;

   self->maxSize = 0;
   self->curSize = 0;
   self->tblSize = 0;
   self->tbl = NULL;
   self->used = NULL;
   IMap_SetMaxSize( self, 0 );
}

void IMap_Destruct( IMap* self ) {
   IMap_Clear( self );
   FreeArray( ((IMap*)self)->tbl );
   FreeArray( ((IMap*)self)->used );
}

void _IMap_Delete( void* self ) {
   IMap_Destruct( (IMap*)self );
   _Stg_Class_Delete( self );
}

void IMap_Copy( void* _self, const void* _op ) {
   IMap* self = (IMap*)_self;
   const IMap* op = (const IMap*)_op;
   IMapIter iter;

   pcu_assert( self && op );
   IMap_Clear( self );
   IMap_SetMaxSize( self, op->maxSize );
   IMapIter_Init( &iter );
   for( IMap_First( op, &iter ); Iter_IsValid( &iter ); IMapIter_Next( &iter ) )
      IMap_Insert( self, IMapIter_GetKey( &iter ), IMapIter_GetValue( &iter ) );
   IMapIter_Destruct( &iter );
}

void IMap_SetMaxSize( void* _self, int maxSize ) {
   IMap* self = (IMap*)_self;
   int nOldItms, *keys, *vals;
   IMapIter iterObj, *iter = &iterObj;
   IMapItem* itm;
   int i_i;

   pcu_assert( self );
   nOldItms = self->curSize;
   keys = AllocArray( int, self->curSize );
   vals = AllocArray( int, self->curSize );
   IMapIter_Init( iter );
   for( i_i = 0, IMap_First( self, iter );
	Iter_IsValid( iter );
	i_i++, IMapIter_Next( iter ) )
   {
      keys[i_i] = IMapIter_GetKey( iter );
      vals[i_i] = IMapIter_GetValue( iter );
   }
   IMapIter_Destruct( iter );

   IMap_Clear( self );
   self->maxSize = maxSize;
   self->curSize = 0;
   self->tblSize = (int)((double)maxSize * IMap_TableFactor);
   self->tblSize += (self->tblSize % 2) ? 0 : 1;
   self->tbl = (IMapItem*)ReallocArray( self->tbl, IMapItem, self->tblSize );
   for( i_i = 0; i_i < self->tblSize; i_i++ ) {
      itm = self->tbl + i_i;
      itm->key = 0;
      itm->val = 0;
      itm->next = NULL;
   }
   self->used = (Bool*)ReallocArray( self->used, Bool, self->tblSize );
   memset( self->used, 0, self->tblSize* sizeof(Bool) );

   for( i_i = 0; i_i < nOldItms; i_i++ )
      IMap_Insert( self, keys[i_i], vals[i_i] );
   FreeArray( keys );
   FreeArray( vals );
}

void IMap_Insert( void* _self, int key, int val ) {
   IMap* self = (IMap*)_self;
   IMapItem *itm, *cur;
   int ind;

   pcu_assert( self );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   itm = self->tbl + ind;
   if( !self->used[ind] ) {
      itm->key = key;
      itm->val = val;
      itm->next = NULL;
      self->used[ind] = True;
   }
   else {
#ifndef NDEBUG
      cur = itm;
      do {
	 pcu_assert( cur->key != key );
	 cur = cur->next;
      } while( cur );
#endif
      cur = itm->next;
      itm->next = AllocArray( IMapItem, 1 );
      itm->next->key = key;
      itm->next->val = val;
      itm->next->next = cur;
   }
   insist( ++self->curSize, <= self->maxSize );
}

void IMap_SetValue( void* _self, int key, int val ) {
   IMap* self = (IMap*)_self;
   IMapItem *itm;
   int ind;

   pcu_assert( self );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   pcu_assert( self->used[ind] );
   itm = self->tbl + ind;
   do {
      if( itm->key == key )
	 break;
      itm = itm->next;
   } while( itm );
   pcu_assert( itm );
   itm->val = val;
}

void IMap_Remove( void* _self, int key ) {
   IMap* self = (IMap*)_self;
   IMapItem *itm, *prev, *toDel;
   int ind;

   pcu_assert( self );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   pcu_assert( self->used[ind] );
   itm = self->tbl + ind;
   if( itm->key == key ) {
      toDel = itm->next;
      if( toDel ) {
	 itm->key = toDel->key;
	 itm->val = toDel->val;
	 itm->next = toDel->next;
      }
      else
	 self->used[ind] = False;
   }
   else {
      prev = itm;
      toDel = itm->next;
      while( toDel ) {
	 if( toDel->key == key ) {
	    prev->next = toDel->next;
	    break;
	 }
	 prev = toDel;
	 toDel = toDel->next;
      }
      pcu_assert( toDel );
   }
   if( toDel )
      FreeArray( toDel );
   self->curSize--;
}

void IMap_Clear( void* _self ) {
   IMap* self = (IMap*)_self;
   IMapItem *itm, *cur, *nxt;
   int i_i;

   pcu_assert( self );
   for( i_i = 0; i_i < self->tblSize; i_i++ ) {
      self->used[i_i] = False;
      itm = self->tbl + i_i;
      cur = itm->next;
      while( cur ) {
	 nxt = cur->next;
	 FreeArray( cur );
	 cur = nxt;
      }
      itm->next = NULL;
   }
   self->curSize = 0;
}

int IMap_GetSize( const void* self ) {
   pcu_assert( self );
   return ((IMap*)self)->curSize;
}

int IMap_GetMaxSize( const void* self ) {
   pcu_assert( self );
   return ((IMap*)self)->maxSize;
}

int IMap_Map( const void* _self, int key ) {
   const IMap* self = (const IMap*)_self;
   IMapItem* itm;
   int ind;

   pcu_assert( self );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   pcu_assert( self->used[ind] );
   itm = self->tbl + ind;
   do {
      if( itm->key == key )
	 break;
      itm = itm->next;
   } while( itm );
   pcu_assert( itm );
   return itm->val;
}

Bool IMap_TryMap( const void* _self, int key, int* val ) {
   const IMap* self = (const IMap*)_self;
   IMapItem* itm;
   int ind;

   pcu_assert( self && val );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   if( !self->used[ind] )
      return False;
   itm = self->tbl + ind;
   do {
      if( itm->key == key )
	 break;
      itm = itm->next;
   } while( itm );
   if( !itm )
      return False;
   *val = itm->val;
   return True;
}

Bool IMap_Has( const void* _self, int key ) {
   const IMap* self = (const IMap*)_self;
   IMapItem* itm;
   int ind;

   pcu_assert( self );
   ind = IMap_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   if( !self->used[ind] )
      return False;
   itm = self->tbl + ind;
   if( itm->key != key ) {
      while( (itm = itm->next) ) {
	 if( itm->key == key )
	    break;
      }
   }
   return itm ? True : False;
}

int IMap_Hash( const void* self, int key ) {
   return key % ((IMap*)self)->tblSize;
}

void IMap_First( const void* _self, IMapIter* iter ) {
   const IMap* self = (IMap*)_self;
   int i_i;

   pcu_assert( self && iter );
   for( i_i = 0; i_i < self->tblSize; i_i++ ) {
      if( self->used[i_i] ) {
	 iter->imap = (IMap*)self;
	 iter->tblInd = i_i;
	 iter->cur = self->tbl + i_i;
	 iter->valid = True;
	 return;
      }
   }
   iter->valid = False;
}


