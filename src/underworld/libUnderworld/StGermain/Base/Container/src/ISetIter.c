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
#include <StGermain/Base/Foundation/src/Foundation.h>
#include "types.h"
#include "Iter.h"
#include "ISet.h"
#include "ISetIter.h"


const Type ISetIter_Type = "ISetIter";


ISetIter* ISetIter_New() {
    ISetIter* self;
    SizeT _sizeOfSelf = sizeof(ISetIter);
    Type type = ISetIter_Type;
    Stg_Class_DeleteFunction* _delete = _Iter_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;
    Iter_NextFunc* nextFunc = ISetIter_Next;

    self = _ISetIter_New( ISETITER_PASSARGS );
    return self;
}

ISetIter* _ISetIter_New( ISETITER_DEFARGS ) {
    ISetIter* self;

    self = (ISetIter*)_Iter_New( ITER_PASSARGS );
    _ISetIter_Init( self );
    return self;
}

void ISetIter_Init( ISetIter* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _Iter_Init( (Iter*)self );
    _ISetIter_Init( self );
}

void _ISetIter_Init( void* _self ) {
   ISetIter* self = (ISetIter*)_self;

   self->iset = NULL;
   self->tblInd = 0;
   self->cur = NULL;
}

void ISetIter_Destruct( ISetIter* self ) {
}

void ISetIter_Copy( void* _self, const void* _op ) {
   ISetIter* self = (ISetIter*)_self;
   const ISetIter* op = (const ISetIter*)_op;

   Iter_Copy( self, op );
   self->iset = op->iset;
   self->tblInd = op->tblInd;
   self->cur = op->cur;
}

void ISetIter_Next( void* _self ) {
   ISetIter* self = (ISetIter*)_self;
   int i_i;

   assert( self );
   assert( self->tblInd < self->iset->tblSize && 
	   self->iset->used[self->tblInd] );
   assert( self->cur );
   assert( self->valid );
   if( !self->cur->next ) {
      for( i_i = self->tblInd + 1; i_i < self->iset->tblSize; i_i++ ) {
	 if( self->iset->used[i_i] )
	    break;
      }
      if( i_i < self->iset->tblSize ) {
	self->tblInd = i_i;
	self->cur = self->iset->tbl + i_i;
      }
      else
	self->valid = False;
   }
   else
      self->cur = self->cur->next;
}

int ISetIter_GetKey( const void* self ) {
   assert( self );
   assert( ((ISetIter*)self)->valid && ((ISetIter*)self)->cur );
   return ((ISetIter*)self)->cur->key;
}


