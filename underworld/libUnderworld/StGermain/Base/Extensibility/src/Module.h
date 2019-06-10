/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_Module_h__
#define __StGermain_Base_Extensibility_Module_h__
   
   /* The prototype for the virtual functions in a module */
   typedef char*       (Module_MangleNameFunction)  ( char* name );

   typedef const char* (Module_GetDepsFunction)     ( void );
   
   /** Textual name of this class */
   extern const Type Module_Type;

   /* Modules info */
   #define __Module \
      /* General info */ \
      __Stg_Object \
      \
      /* Virtual info */ \
      Module_MangleNameFunction*  MangleName; \
      \
      /* Module info */ \
      DLL_Handle                  dllPtr; \
      Module_GetDepsFunction*     GetDeps;    \
      char*                       mangledName;
      
   struct Module { __Module };

   /* Creation implementation / Virtual constructor */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define MODULE_DEFARGS \
      STG_OBJECT_DEFARGS, \
      Module_MangleNameFunction* MangleName, \
      Stg_ObjectList*            directories

   #define MODULE_PASSARGS \
      STG_OBJECT_PASSARGS, \
      MangleName,  \
      directories

   Module* _Module_New( MODULE_DEFARGS );
   
   /* Initialisation implementation */
   void _Module_Init(
      Module*                   self,
      Module_MangleNameFunction MangleName,
      Stg_ObjectList*           directories );
   
   /* Stg_Class_Delete implementation */
   void _Module_Delete( void* plugin );
   
   /* Print implementation */
   void _Module_Print( void* plugin, Stream* stream );

   /** Return the mangled (symbol and file) name. Note: result needs to be freed */
   char* Module_MangledName( void* module );

   /** Load a specific symbol of the module, where the symbol is prefixed by the module name */
   void* Module_LoadSymbol( void* module, const char* suffix );

   /** Un load the module */
   void Module_UnLoad( void* module );
   
#endif /* __StGermain_Base_Extensibility_Module_h__ */

