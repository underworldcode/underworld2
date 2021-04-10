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

const Type Iter_Type = "Iter";

Iter* _Iter_New( ITER_DEFARGS ) {
   Iter* self;

   self = (Iter*)_Stg_Class_New( STG_CLASS_PASSARGS );
   self->nextFunc = nextFunc;
   _Iter_Init( self );
   return self;
}

void _Iter_Init( void* self ) {
   ((Iter*)self)->valid = False;
}

void _Iter_Delete( void* self ) {
   _Stg_Class_Delete( self );
}

void Iter_Copy( void* self, const void* op ) {
   assert( self && op );
   ((Iter*)self)->valid = ((Iter*)op)->valid;
}

Bool Iter_IsValid( void* self ) {
   assert( self );
   return ((Iter*)self)->valid;
}
