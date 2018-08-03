/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "ConditionFunction.h"

#include <stdio.h>
#include <assert.h>


const Type ConditionFunction_Type = "ConditionFunction";


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

ConditionFunction* ConditionFunction_New( ConditionFunction_ApplyFunc* apply, Name name, void* data ) {
   /* Variables set in this function */
   SizeT                     _sizeOfSelf = sizeof(ConditionFunction);
   Type                             type = ConditionFunction_Type;
   Stg_Class_DeleteFunction*     _delete = _ConditionFunction_Delete;
   Stg_Class_PrintFunction*       _print = _ConditionFunction_Print;
   Stg_Class_CopyFunction*         _copy = NULL;
   ConditionFunction_InitFunc*      init = NULL;

   return _ConditionFunction_New( CONDITIONFUNCTION_PASSARGS );
}

ConditionFunction* ConditionFunction_New2( ConditionFunction_ApplyFunc* apply, ConditionFunction_InitFunc* init, Name name, void* data ) {
   /* Variables set in this function */
   SizeT                     _sizeOfSelf = sizeof(ConditionFunction);
   Type                             type = ConditionFunction_Type;
   Stg_Class_DeleteFunction*     _delete = _ConditionFunction_Delete;
   Stg_Class_PrintFunction*       _print = _ConditionFunction_Print;
   Stg_Class_CopyFunction*         _copy = NULL;

   return _ConditionFunction_New( CONDITIONFUNCTION_PASSARGS );
}

ConditionFunction* _ConditionFunction_New( CONDITIONFUNCTION_DEFARGS ) {
   ConditionFunction* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(ConditionFunction) );
   self = (ConditionFunction*)_Stg_Class_New( STG_CLASS_PASSARGS );

   self->data = NULL;
   
   /* General info */
   
   /* Virtual functions */
   
   /* Stg_Class info */
   _ConditionFunction_Init(self, apply, init, name, data);
   
   return self;
}

void _ConditionFunction_Init( void* conditionFunction, ConditionFunction_ApplyFunc* apply, ConditionFunction_InitFunc* init, Name name, void* data) {
   ConditionFunction* self = (ConditionFunction*)conditionFunction;
   
   self->apply = apply;
   self->init  = init;
   self->name  = name;
   self->data  = data;
}

/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _ConditionFunction_Delete( void* conditionFunction ) {
   ConditionFunction* self = (ConditionFunction*)conditionFunction;
   
   /* Stg_Class_Delete parent class */
   _Stg_Class_Delete(self);
}

void _ConditionFunction_Print( void* conditionFunction, Stream* stream ) {
   ConditionFunction* self = (ConditionFunction*)conditionFunction;
   
   /* Set the Journal for printing informations */
   Stream* conditionFunctionStream = stream;
   
   /* General info */
   Journal_Printf( conditionFunctionStream, "ConditionFunction (ptr): %p\n", self );
   
   /* Virtual info */
   
   /* Stg_Class info */
   Journal_Printf( conditionFunctionStream, "\tapply (func ptr): %p\n", self->apply );
   Journal_Printf( conditionFunctionStream, "\tname (ptr): %p\n", self->name );

   if (self->name)
      Journal_Printf( conditionFunctionStream, "\t\tname: %s\n", self->name );
   
   /* Print parent class */
   _Stg_Class_Print( self, conditionFunctionStream );
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


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

void _ConditionFunction_Apply(
   void*          conditionFunction,
   Index          index,
   StgVariable_Index var_I,
   void*          context,
   void*          result )
{
   ConditionFunction* self = (ConditionFunction*)conditionFunction;
   
   ConditionFunction_Apply( self, index, var_I, context, result );
}


