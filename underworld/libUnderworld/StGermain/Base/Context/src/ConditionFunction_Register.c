/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"

#include "types.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


const Type ConditionFunction_Register_Type = "ConditionFunction_Register";

ConditionFunction_Register* condFunc_Register = 0;

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

ConditionFunction_Register* ConditionFunction_Register_New( void ) {
   /* Variables set in this function */
   SizeT                     _sizeOfSelf = sizeof( ConditionFunction_Register );
   Type                             type = ConditionFunction_Register_Type;
   Stg_Class_DeleteFunction*     _delete = _ConditionFunction_Register_Delete;
   Stg_Class_PrintFunction*       _print = _ConditionFunction_Register_Print;
   Stg_Class_CopyFunction*         _copy = NULL;

   return _ConditionFunction_Register_New( CONDITIONFUNCTION_REGISTER_PASSARGS );
}

void ConditionFunction_Register_Init( ConditionFunction_Register* self ) { 
   /* General info */
   self->type = ConditionFunction_Register_Type;
   self->_sizeOfSelf = sizeof( ConditionFunction_Register );
   self->_deleteSelf = False;
   
   /* Virtual info */
   self->_delete = _ConditionFunction_Register_Delete;
   self->_print = _ConditionFunction_Register_Print;
   self->_copy = NULL;
   
   _Stg_Class_Init((Stg_Class*)self );
   
   /* Stg_Class info */
   _ConditionFunction_Register_Init( self );
}

ConditionFunction_Register* _ConditionFunction_Register_New( CONDITIONFUNCTION_REGISTER_DEFARGS ) {
   ConditionFunction_Register* self;
   
   /* Allocate memory */
   self = (ConditionFunction_Register*)_Stg_Class_New( STG_CLASS_PASSARGS );
   
   /* Virtual info */
   
   /* ConditionFunction_Register info */
   _ConditionFunction_Register_Init( self );
   
   return self;
}

void _ConditionFunction_Register_Init( void* conditionFunction_Register ) { 
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   
   self->count = 0;
   self->_size = 8;
   self->_delta = 8;
   self->_cf = Memory_Alloc_Array( ConditionFunction*, self->_size, "ConditionFunction_Register->_cf" );
   memset( self->_cf, 0, sizeof( ConditionFunction*)*self->_size );
}

/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _ConditionFunction_Register_Delete( void* conditionFunction_Register ) {
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   
   if(self->_cf ) Memory_Free( self->_cf );
      
   /* Stg_Class_Delete parent class */
   _Stg_Class_Delete( self );
}

void _ConditionFunction_Register_Print( void* conditionFunction_Register, Stream* stream ) {
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   
   /* Set the Journal for printing informations */
   Stream* conditionFunction_RegisterStream = stream;
   
   /* General info */
   Journal_Printf( conditionFunction_RegisterStream, "ConditionFunction_Register (ptr ): %p\n", self );
   
   /* Virtual info */
   
   /* ConditionFunction_Register info */
   Journal_Printf( conditionFunction_RegisterStream, "\tcount: %u\n", self->count );
   Journal_Printf( conditionFunction_RegisterStream, "\t_size: %lu\n", self->_size );
   Journal_Printf( conditionFunction_RegisterStream, "\t_delta: %lu\n", self->_delta );
   Journal_Printf( conditionFunction_RegisterStream, "\t_cf (ptr ): %p\n", self->_cf );

   if( self->_cf ) {
      ConditionFunction_Index cf_I;
      
      for( cf_I = 0; cf_I < self->count; cf_I++ )
         Journal_Printf( conditionFunction_RegisterStream, "\t\t_cf[%u]: %p\n", cf_I, self->_cf[cf_I]);
   }
   
   /* Parent class info */
   _Stg_Class_Print( self, conditionFunction_RegisterStream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/

ConditionFunction_Index ConditionFunction_Register_Add( void* conditionFunction_Register, ConditionFunction* cf ) {
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   StgVariable_Index              handle;
   
   if( self->count >= self->_size ) {
      SizeT currentSize = self->_size;
      
      /* Note: why is realloc not used here? */
      self->_size += self->_delta;
      
      self->_cf = Memory_Realloc_Array( self->_cf, ConditionFunction*, self->_size );
      memset(
         (Pointer )((ArithPointer )self->_cf + (sizeof( ConditionFunction*) * currentSize ) ),
         0,
         sizeof( ConditionFunction* ) * (self->_size - currentSize ) );
   }
   
   handle = self->count++;
   self->_cf[handle] = cf;
   
   return handle;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

ConditionFunction_Index ConditionFunction_Register_GetIndex( void* conditionFunction_Register, Name name ) {
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   ConditionFunction_Index     cf_I;
   
   for( cf_I = 0; cf_I < self->count; cf_I++ ) {
      if( self->_cf[cf_I]->name && !strcmp( name, self->_cf[cf_I]->name ))
         return cf_I;
   }
   
   return (ConditionFunction_Index )-1;
}

void ConditionFunction_Register_PrintNameOfEachFunc( void* conditionFunction_Register, Stream* stream ) {
   ConditionFunction_Register* self = (ConditionFunction_Register*)conditionFunction_Register;
   ConditionFunction_Index     cf_I;
   
   for( cf_I = 0; cf_I < self->count; cf_I++ ) {
      Journal_Printf( stream, "\"%s\", ", self->_cf[cf_I]->name );
   }
}


