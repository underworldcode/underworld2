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
#include "StGermain/pcu/src/pcu.h"
#include <StGermain/Base/Foundation/src/Foundation.h>
#include "types.h"
#include "IArray.h"
#include "Iter.h"
#include "ISetIter.h"
#include "ISet.h"


const Type ISet_Type = "ISet";
const double ISet_TableFactor = 1.18;


ISet* ISet_New() {
    ISet* self;
    SizeT _sizeOfSelf = sizeof(ISet);
    Type type = ISet_Type;
    Stg_Class_DeleteFunction* _delete = _ISet_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _ISet_New( ISET_PASSARGS );
    return self;
}

ISet* _ISet_New( ISET_DEFARGS ) {
    ISet* self;

    self = (ISet*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _ISet_Init( self );
    return self;
}

void ISet_Init( ISet* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _ISet_Init( self );
}

void _ISet_Init( void* _self ) {
   ISet* self = (ISet*)_self;

   self->maxSize = 0;
   self->curSize = 0;
   self->tblSize = 0;
   self->tbl = NULL;
   self->used = NULL;
   ISet_SetMaxSize( self, 0 );
}

void ISet_Destruct( ISet* self ) {
    ISet_Clear( self );
    FreeArray( ((ISet*)self)->tbl );
    FreeArray( ((ISet*)self)->used );
}

void _ISet_Delete( void* self ) {
    ISet_Destruct( (ISet*)self );
    _Stg_Class_Delete( self );
}

void ISet_Copy( void* _self, const void* _op ) {
   ISet* self = (ISet*)_self;
   const ISet* op = (const ISet*)_op;
   ISetIter iterObj, *iter = &iterObj;

   pcu_assert( self && op );
   ISet_Clear( self );
   ISet_SetMaxSize( self, op->maxSize );
   ISetIter_Init( iter );
   for( ISet_First( op, iter ); Iter_IsValid( iter ); ISetIter_Next( iter ) )
      ISet_Insert( self, ISetIter_GetKey( iter ) );
   ISetIter_Destruct( iter );
}

void ISet_UseArray( void* _self, int size, const int* array ) {
   ISet* self = (ISet*)_self;
   int i_i;

   ISet_Clear( self );
   ISet_SetMaxSize( self, size );
   for( i_i = 0; i_i < size; i_i++ )
      ISet_Insert( self, array[i_i] );
}

void ISet_SetMaxSize( void* _self, int maxSize ) {
   ISet* self = (ISet*)_self;
   int nOldItms, *keys;
   ISetItem* itm;
   int i_i;

   pcu_assert( self );
   nOldItms = self->curSize;
   keys = AllocArray( int, self->curSize );
   ISet_GetArray( self, keys );

   ISet_Clear( self );
   self->maxSize = maxSize;
   self->curSize = 0;
   self->tblSize = (int)((double)maxSize * ISet_TableFactor);
   self->tblSize += (self->tblSize % 2) ? 0 : 1;
   self->tbl = (ISetItem*)ReallocArray( self->tbl, ISetItem, self->tblSize );
   for( i_i = 0; i_i < self->tblSize; i_i++ ) {
      itm = self->tbl + i_i;
      itm->key = 0;
      itm->next = NULL;
   }
   self->used = (Bool*)ReallocArray( self->used, Bool, self->tblSize );
   memset( self->used, 0, self->tblSize* sizeof(Bool) );

   for( i_i = 0; i_i < nOldItms; i_i++ )
      ISet_Insert( self, keys[i_i] );
   FreeArray( keys );
}

void ISet_Insert( void* _self, int key ) {
   ISet* self = (ISet*)_self;
   ISetItem *itm, *cur;
   int ind;

   pcu_assert( self );
   ind = ISet_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   itm = self->tbl + ind;
   if( !self->used[ind] ) {
      itm->key = key;
      itm->next = NULL;
      self->used[ind] = True;
   }
   else {
#ifndef NDEBUG
      cur = itm;
      do {
	 if( cur->key == key ) {
	    pcu_assert( cur->key != key );
	 }
	 cur = cur->next;
      } while( cur );
#endif
      cur = itm->next;
      itm->next = AllocArray( ISetItem, 1 );
      itm->next->key = key;
      itm->next->next = cur;
   }
   insist( ++self->curSize, <= self->maxSize );
}

Bool ISet_TryInsert( void* _self, int key ) {
   ISet* self = (ISet*)_self;
   ISetItem *itm, *cur;
   int ind;

   pcu_assert( self );
   ind = ISet_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   itm = self->tbl + ind;
   if( !self->used[ind] ) {
      itm->key = key;
      itm->next = NULL;
      self->used[ind] = True;
   }
   else {
      cur = itm;
      do {
	 if( cur->key == key )
	    return False;
	 cur = cur->next;
      } while( cur );
      cur = itm->next;
      itm->next = AllocArray( ISetItem, 1 );
      itm->next->key = key;
      itm->next->next = cur;
   }
   insist( ++self->curSize, <= self->maxSize );
   return True;
}

void ISet_Remove( void* _self, int key ) {
   ISet* self = (ISet*)_self;
   ISetItem *itm, *prev, *toDel;
   int ind;

   pcu_assert( self );
   ind = ISet_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   pcu_assert( self->used[ind] );
   itm = self->tbl + ind;
   if( itm->key == key ) {
      toDel = itm->next;
      if( toDel ) {
	 itm->key = toDel->key;
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

Bool ISet_TryRemove( void* _self, int key ) {
   ISet* self = (ISet*)_self;
   ISetItem *itm, *prev, *toDel;
   int ind;

   pcu_assert( self );
   ind = ISet_Hash( self, key );
   pcu_assert( ind < self->tblSize );
   if( !self->used[ind] )
      return False;
   itm = self->tbl + ind;
   if( itm->key == key ) {
      toDel = itm->next;
      if( toDel ) {
	 itm->key = toDel->key;
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
      if( !toDel )
	 return False;
   }
   if( toDel )
      FreeArray( toDel );
   self->curSize--;
   return True;
}

void ISet_Clear( void* _self ) {
   ISet* self = (ISet*)_self;
   ISetItem *itm, *cur, *nxt;
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

void ISet_Union( void* _self, const void* _op ) {
   ISet* self = (ISet*)_self;
   const ISet* op = (const ISet*)_op;
   ISetIter iterObj, *iter = &iterObj;
   IArray arrayObj, *array = &arrayObj;
   int arrSize, key;
   const int* arrPtr;
   int i_i;

   pcu_assert( self && op );
   ISetIter_Init( iter );
   IArray_Init( array );
   for( ISet_First( op, iter ); Iter_IsValid( iter ); ISetIter_Next( iter ) ) {
      key = ISetIter_GetKey( iter );
      if( !ISet_Has( self, key ) )
	 IArray_Append( array, key );
   }
   ISetIter_Destruct( iter );

   arrSize = IArray_GetSize( array );
   arrPtr = IArray_GetPtr( array );
   ISet_SetMaxSize( self, self->curSize + arrSize );
   for( i_i = 0; i_i < arrSize; i_i++ )
      ISet_Insert( self, arrPtr[i_i] );
   IArray_Destruct( array );
}

void ISet_Isect( void* _self, const void* _op ) {
   ISet* self = (ISet*)_self;
   const ISet* op = (const ISet*)_op;
   ISetIter iterObj, *iter = &iterObj;
   IArray arrayObj, *array = &arrayObj;
   int arrSize, key;
   const int* arrPtr;
   int i_i;

   pcu_assert( self && op );
   ISetIter_Init( iter );
   IArray_Init( array );
   for( ISet_First( op, iter ); Iter_IsValid( iter ); ISetIter_Next( iter ) ) {
      key = ISetIter_GetKey( iter );
      if( ISet_Has( self, key ) )
	 IArray_Append( array, key );
   }
   ISetIter_Destruct( iter );

   arrSize = IArray_GetSize( array );
   arrPtr = IArray_GetPtr( array );
   ISet_Clear( self );
   ISet_SetMaxSize( self, arrSize );
   for( i_i = 0; i_i < arrSize; i_i++ )
      ISet_Insert( self, arrPtr[i_i] );
   IArray_Destruct( array );
}

void ISet_Subtr( void* _self, const void* _op ) {
   ISet* self = (ISet*)_self;
   const ISet* op = (const ISet*)_op;
   ISetIter iterObj, *iter = &iterObj;
   IArray arrayObj, *array = &arrayObj;
   int arrSize, key;
   const int* arrPtr;
   int i_i;

   pcu_assert( self && op );
   ISetIter_Init( iter );
   IArray_Init( array );
   for( ISet_First( self, iter ); Iter_IsValid( iter ); ISetIter_Next( iter ) ) {
      key = ISetIter_GetKey( iter );
      if( !ISet_Has( op, key ) )
	 IArray_Append( array, key );
   }
   ISetIter_Destruct( iter );

   arrSize = IArray_GetSize( array );
   arrPtr = IArray_GetPtr( array );
   ISet_Clear( self );
   ISet_SetMaxSize( self, arrSize );
   for( i_i = 0; i_i < arrSize; i_i++ )
      ISet_Insert( self, arrPtr[i_i] );
   IArray_Destruct( array );
}

int ISet_GetMaxSize( const void* self ) {
   pcu_assert( self );
   return ((ISet*)self)->maxSize;
}

int ISet_GetSize( const void* self ) {
   pcu_assert( self );
   return ((ISet*)self)->curSize;
}

void ISet_GetArray( const void* _self, int* keys ) {
   const ISet* self = (const ISet*)_self;
   ISetIter iterObj, *iter = &iterObj;
   int i_i;

   pcu_assert( self );
   ISetIter_Init( iter );
   for( i_i = 0, ISet_First( self, iter );
	Iter_IsValid( iter );
	i_i++, ISetIter_Next( iter ) )
   {
      keys[i_i] = ISetIter_GetKey( iter );
   }
   ISetIter_Destruct( iter );
}

Bool ISet_Has( const void* _self, int key ) {
   const ISet* self = (const ISet*)_self;
   ISetItem* itm;
   int ind;

   pcu_assert( self );
   ind = ISet_Hash( self, key );
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

int ISet_Hash( const void* self, int key ) {
   return key % ((ISet*)self)->tblSize;
}

void ISet_First( const void* _self, ISetIter* iter ) {
   const ISet* self = (ISet*)_self;
   int i_i;

   pcu_assert( self && iter );
   for( i_i = 0; i_i < self->tblSize; i_i++ ) {
      if( self->used[i_i] ) {
	 iter->iset = (ISet*)self;
	 iter->tblInd = i_i;
	 iter->cur = self->tbl + i_i;
	 iter->valid = True;
	 return;
      }
   }
   iter->valid = False;
}


