/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <assert.h>
#include "Base/Foundation/Foundation.h"
#include "types.h"
#include "Iter.h"
#include "IMap.h"
#include "IMapIter.h"

const Type IMapIter_Type = "IMapIter";

IMapIter* IMapIter_New() {
   IMapIter* self;
   SizeT _sizeOfSelf = sizeof(IMapIter);
   Type type = IMapIter_Type;
   Stg_Class_DeleteFunction* _delete = _Iter_Delete;
   Stg_Class_PrintFunction* _print = NULL;
   Stg_Class_CopyFunction* _copy = NULL;
   Iter_NextFunc* nextFunc = IMapIter_Next;

   self = _IMapIter_New( IMAPITER_PASSARGS );
   return self;
}

IMapIter* _IMapIter_New( IMAPITER_DEFARGS ) {
   IMapIter* self;

   self = (IMapIter*)_Iter_New( ITER_PASSARGS );
   _IMapIter_Init( self );
   return self;
}

void IMapIter_Init( IMapIter* self ) {
   _Stg_Class_Init( (Stg_Class*)self );
   _Iter_Init( (Iter*)self );
   _IMapIter_Init( self );
}

void _IMapIter_Init( void* _self ) {
   IMapIter* self = (IMapIter*)_self;

   self->imap = NULL;
   self->tblInd = 0;
   self->cur = NULL;
}

void IMapIter_Destruct( IMapIter* self ) {
}

void IMapIter_Copy( void* _self, const void* _op ) {
   IMapIter* self = (IMapIter*)_self;
   const IMapIter* op = (const IMapIter*)_op;

   Iter_Copy( self, op );
   self->imap = op->imap;
   self->tblInd = op->tblInd;
   self->cur = op->cur;
}

void IMapIter_Next( void* _self ) {
   IMapIter* self = (IMapIter*)_self;
   int i_i;

   assert( self );
   assert( self->tblInd < self->imap->tblSize && 
         self->imap->used[self->tblInd] );
   assert( self->cur );
   assert( self->valid );
   if( !self->cur->next ) {
      for( i_i = self->tblInd + 1; i_i < self->imap->tblSize; i_i++ ) {
         if( self->imap->used[i_i] )
            break;
      }
      if( i_i < self->imap->tblSize ) {
         self->tblInd = i_i;
         self->cur = self->imap->tbl + i_i;
      }
      else
         self->valid = False;
   }
   else
      self->cur = self->cur->next;
}

int IMapIter_GetKey( const void* self ) {
   assert( self );
   assert( ((IMapIter*)self)->valid && ((IMapIter*)self)->cur );
   return ((IMapIter*)self)->cur->key;
}

int IMapIter_GetValue( const void* self ) {
   assert( self );
   assert( ((IMapIter*)self)->valid && ((IMapIter*)self)->cur );
   return ((IMapIter*)self)->cur->val;
}
