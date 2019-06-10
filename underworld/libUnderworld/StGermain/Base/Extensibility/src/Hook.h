/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_Hook_h__
#define __StGermain_Base_Extensibility_Hook_h__
   
   /** Textual name of this class */
   extern const Type Hook_Type;

   /** Hook struct- stores a func pointer plus info */
   #define __Hook \
      __Stg_Object \
      \
      Func_Ptr     funcPtr; \
      Hook_AddedBy addedBy; \
   
   struct Hook { __Hook };
   
   /* Create a new Hook */
   Hook* Hook_New( Name name, Func_Ptr funcPtr, char* addedBy );
   
   /* Initialise an Hook */
   void Hook_Init( void* hook, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /* Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define HOOK_DEFARGS \
      STG_OBJECT_DEFARGS, \
      Func_Ptr  funcPtr, \
      char*     addedBy

   #define HOOK_PASSARGS \
      STG_OBJECT_PASSARGS, \
      funcPtr, \
      addedBy

   Hook* _Hook_New( HOOK_DEFARGS );
   
   /* Initialisation implementation */
   void _Hook_Init( Hook* self, Func_Ptr funcPtr, char* addedBy );
   
   /* Stg_Class_Delete implementation */
   void _Hook_Delete( void* hook );
   
   /* Print implementation */
   void _Hook_Print( void* hook, Stream* stream );
   
   /* Copy */
   #define Hook_Copy( self ) \
      (Hook*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
   #define Hook_DeepCopy( self ) \
      (Hook*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
   
   void* _Hook_Copy( void* hook, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
   
#endif /* __StGermain_Base_Extensibility_Hook_h__ */

