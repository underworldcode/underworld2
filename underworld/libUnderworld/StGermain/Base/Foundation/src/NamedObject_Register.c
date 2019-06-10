/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "ObjectAdaptor.h"
#include "ObjectList.h"
#include "NamedObject_Register.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Stg_Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type NamedObject_Register_Type = "NamedObject_Register";

NamedObject_Register* NamedObject_Register_New( void ) {
   /* Variables set in this function */
   SizeT                     _sizeOfSelf = sizeof(NamedObject_Register);
   Type                             type = NamedObject_Register_Type;
   Stg_Class_DeleteFunction*     _delete = _NamedObject_Register_Delete;
   Stg_Class_PrintFunction*       _print = _NamedObject_Register_Print;
   Stg_Class_CopyFunction*         _copy = _NamedObject_Register_Copy;

   return _NamedObject_Register_New( NAMEDOBJECT_REGISTER_PASSARGS );
}

NamedObject_Register* _NamedObject_Register_New( NAMEDOBJECT_REGISTER_DEFARGS ) {
   NamedObject_Register* self;
   
   /* Allocate memory/General info */
   assert(_sizeOfSelf >= sizeof(NamedObject_Register));
   self = (NamedObject_Register*)_Stg_Class_New( STG_CLASS_PASSARGS );
   
   /* Virtual info */
   
   /* Stg_Class info */
   _NamedObject_Register_Init( self );   

   return self;
}
   
void _NamedObject_Register_Init( NamedObject_Register* self ) {
   self->objects = Stg_ObjectList_New();
}

void _NamedObject_Register_Delete( void* namedObjectRegister ) {
   NamedObject_Register* self = (NamedObject_Register*)namedObjectRegister;
   
   Journal_DPrintf(
      Journal_Register( Debug_Type, NamedObject_Register_Type ),
      "In: %s()\n",
      __func__ );
   
   Stg_Class_Delete( self->objects ); 

   /* Stg_Class_Delete parent */
   _Stg_Class_Delete( self );
}
   
void NamedObject_Register_DeleteAll( void* reg ) {
   /* 
    * Deletes all elements from register and then
    * deletes the register.
    */
   NamedObject_Register* self = (NamedObject_Register*)reg;

   Stg_ObjectList_DeleteAllObjects( self->objects );

   /* Stg_Class_Delete parent */
   Stg_Class_Delete( self );
}
void _NamedObject_Register_Print( void* namedObjectRegister, struct Stream* stream ) {
   NamedObject_Register* self = (NamedObject_Register*)namedObjectRegister;

   /* General info */
   Journal_Printf( stream, "NamedObject_Register (ptr): %p\n", self);
   Stream_Indent( stream );
   
   /* Use parent print */
   _Stg_Class_Print( self, stream );
   
   /* Print the list of registered objects */
   Stg_Class_Print( self->objects, stream );   
   
   Stream_UnIndent( stream );
}

void* _NamedObject_Register_Copy(
   void*          namedObjectRegister,
   void*          dest,
   Bool           deep,
   Name           nameExt,
   struct PtrMap* ptrMap ) {
   NamedObject_Register* self = (NamedObject_Register*)namedObjectRegister;
   NamedObject_Register* newNamedObjectRegister;
   
   newNamedObjectRegister = (NamedObject_Register*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
   
   Journal_Firewall( 
      deep, 
      Journal_Register( Error_Type, NamedObject_Register_Type ), 
      "Shallow copy not yet implemented\n" );

   if( deep ) {
      newNamedObjectRegister->objects = (Stg_ObjectList*)Stg_Class_Copy( self->objects, NULL, deep, nameExt, ptrMap );
   }
   
   return newNamedObjectRegister;
}

/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
