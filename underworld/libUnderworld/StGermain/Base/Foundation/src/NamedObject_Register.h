/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_NamedObject_Register_h__
#define __StGermain_Base_Foundation_NamedObject_Register_h__ 

   extern const Type NamedObject_Register_Type;
   
   #define __NamedObject_Register \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      \
      /* Stg_Class info */ \
      Stg_ObjectList* objects;

   struct NamedObject_Register { __NamedObject_Register };
   
   /* Stg_Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   NamedObject_Register* NamedObject_Register_New( void );
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define NAMEDOBJECT_REGISTER_DEFARGS \
      STG_CLASS_DEFARGS

   #define NAMEDOBJECT_REGISTER_PASSARGS \
      STG_CLASS_PASSARGS

   NamedObject_Register* _NamedObject_Register_New( NAMEDOBJECT_REGISTER_DEFARGS );
   
   void _NamedObject_Register_Init( NamedObject_Register* self );
   
   void _NamedObject_Register_Delete( void* nameObjectRegister );
   
   void _NamedObject_Register_Print( void* nameObjectRegister, struct Stream* stream );
   
   void* _NamedObject_Register_Copy( void* namedObjectRegister, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
   
   /* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /* Deletes all elements from register and then
      deletes the register */
   void NamedObject_Register_DeleteAll( void* reg );
   
   #define NamedObject_Register_Add( self, nameObject ) \
      ( Stg_ObjectList_Append( (self)->objects, nameObject ) )

   #define NamedObject_Register_GetIndex( self, name ) \
      ( Stg_ObjectList_GetIndex( (self)->objects, name ) )

   #define NamedObject_Register_GetByName( self, name ) \
      ( Stg_ObjectList_Get( (self)->objects, name ) )

   #define NamedObject_Register_GetByIndex( self, index ) \
      ( (self)->objects->data[(index)] )
   
   #define NamedObject_Register_PrintAllEntryNames( self, stream ) \
      ( Stg_ObjectList_PrintAllEntryNames( (self)->objects, stream ) )
   
   /* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif /* __StGermain_Base_Foundation_NamedObject_Register_h__ */

