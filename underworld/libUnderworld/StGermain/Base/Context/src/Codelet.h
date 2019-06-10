/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_Codelet_h__
#define __StGermain_Base_Context_Codelet_h__
   
   /* Textual name of this class */
   extern const Type Codelet_Type;

   /* Codelets info */
   #define __Codelet \
      __Stg_Component \
      \
      AbstractContext* context;
      
   struct Codelet { __Codelet };

   void* Codelet_New(
      Type                                      type,
      Stg_Component_DefaultConstructorFunction* _defaultConstructor,
      Stg_Component_ConstructFunction*          _construct,
      Stg_Component_BuildFunction*              _build,
      Stg_Component_InitialiseFunction*         _initialise,
      Stg_Component_ExecuteFunction*            _execute,
      Stg_Component_DestroyFunction*            _destroy,
      Name                                      name );
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define CODELET_DEFARGS \
      STG_COMPONENT_DEFARGS

   #define CODELET_PASSARGS \
      STG_COMPONENT_PASSARGS

   void* _Codelet_New( CODELET_DEFARGS );

   void _Codelet_Delete( void* codelet );

   void _Codelet_Print( void* codelet, Stream* stream );

   void* _Codelet_Copy( void* codelet, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

   void _Codelet_Build( void* codelet, void* data );

   void _Codelet_Initialise( void* codelet, void* data );

   void _Codelet_Execute( void* codelet, void* data );

   void _Codelet_Destroy( void* codelet, void* data );

   Dictionary* Codelet_GetPluginDictionary( void* codelet, Dictionary* rootDict );

#endif

