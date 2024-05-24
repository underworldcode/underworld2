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
#include "StGermain/Base/Automation/src/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "Module.h"


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#ifndef NOSHARED
#include <dlfcn.h>
#else
extern int stg_num_modules;
extern char **stg_module_names;
extern int *stg_num_module_syms;
extern char ***stg_module_syms;
extern void ***stg_module_funcs;

Bool Module_CheckStaticName(Module *self);
void* Module_MapStaticSymbol(Module *self, const char *sym);
#endif

#include <limits.h>

const Type Module_Type = "Module";

static const char* MODULE_SUFFIX = "module";
static const char* MODULE_GETDEPS_SUFFIX = "_GetDeps";

static const char* MODULE_FILENAME = "fileName";
static const char* MODULE_SYMBOLNAME = "symbolName";

#ifdef NOSHARED
int stg_num_modules = 0;
char **stg_module_names = NULL;
int *stg_num_module_syms = NULL;
char ***stg_module_syms = NULL;
void ***stg_module_funcs = NULL;
#endif

Module* _Module_New( MODULE_DEFARGS ) {
   Module* self;

   assert( _sizeOfSelf >= sizeof(Module) );

   /* 
    * The following terms are parameters that have been passed into this function but
    * are being set before being passed onto the parent. 
    * This means that any values of these parameters that are passed into this function are
    * not passed onto the parent function and so should be set to ZERO in any children of this class. 
    */
   nameAllocationType = NON_GLOBAL;

   self = (Module*)_Stg_Object_New( STG_OBJECT_PASSARGS );
   
   _Module_Init( self, MangleName, directories );

   return self;
}
   
void _Module_Init( Module* self, Module_MangleNameFunction MangleName, Stg_ObjectList* directories ) {
   char*   fileName = NULL;
   char*   fullPathName = NULL;
   Stream* stream;
   Stream* debug;
   Stream* error;
   Bool    found = False;
   #ifndef NOSHARED
   int     fullPathLength = 0;
   int     length;
   Index   dir_i;
   #endif
   
   stream = Journal_Register( Info_Type, self->type );
   debug = Journal_Register( Debug_Type, self->type );
   error = Journal_Register( Error_Type, self->type );
   
   self->MangleName = MangleName;
   
   Journal_Printf( debug, "Finding module: \"%s\"... ", self->name );

   self->mangledName = self->MangleName( self->name );

   #ifndef NOSHARED
   fileName = Memory_Alloc_Array( char, strlen(self->mangledName) + strlen(MODULE_SUFFIX) + strlen(MODULE_EXT) + 1,
      MODULE_FILENAME );
   sprintf( fileName, "%s%s%s", self->mangledName, MODULE_SUFFIX, MODULE_EXT );
     
   /* Look for the library from the directories. */
   for( dir_i = 0; dir_i < directories->count; ++dir_i ) {
      length = strlen( (char*)Stg_ObjectList_ObjectAt( directories, dir_i ) ) + 1 + strlen( fileName ) + 1;

      if( fullPathLength < length ) {
         fullPathLength = length;
         fullPathName = Memory_Realloc_Array( fullPathName, char, fullPathLength );
      }
      PathJoin( fullPathName, 2, Stg_ObjectList_ObjectAt( directories, dir_i ), fileName );
      self->dllPtr = dlopen( fullPathName, RTLD_LAZY | RTLD_GLOBAL );

      if( self->dllPtr ) {
         found = True;
         break;
      } else {
         Journal_RPrintf( debug, "\n%s \"%s\" NOT found using %s\n", self->type, self->name, fullPathName );
         Journal_RPrintf( debug, "   dlopen returned the error: %s\n\n", dlerror() );
      }
   }

   /* If it failed alltogether, print a error message. */
   if( !found ) {
      Journal_Printf( debug, "Failed to find in any of directories above, or had error.\n" );
   }

   #else
   /* 
    * This is a static build, so we just need to make sure we can find the name
    * of the module in the module list. 
    */
   self->dllPtr = (DLL_Handle)Module_CheckStaticName( self );
   #endif

   /* Load the symbols */
   if( self->dllPtr ) {
      self->GetDeps = (Module_GetDepsFunction*)Module_LoadSymbol( self, MODULE_GETDEPS_SUFFIX );
   }
   
   if( fullPathName ) {
      Memory_Free( fullPathName );
   }
   Memory_Free( fileName );
}
   
void _Module_Delete( void* module ) {
   Module* self = (Module*)module;

   Module_UnLoad( self );
   Memory_Free( self->mangledName );
   
   /* Delete parent */
   _Stg_Object_Delete( self );
}
   
void _Module_Print( void* module, Stream* stream ) {
   Module*     self = (Module*)module;

   Journal_RPrintf( stream, "Module: %s\n", self->name );
   Stream_Indent( stream );
      
   /* Print parent */
   _Stg_Object_Print( self, stream );
   
   Stream_UnIndent( stream );
}

void* Module_LoadSymbol( void* module, const char* suffix ) {
   Module* self = (Module*)module;
   char*   mangledName;
   char*   symbolText;
   void*   result;
   
   mangledName = self->MangleName( self->name );
   symbolText = Memory_Alloc_Array( char, strlen( mangledName ) + strlen( suffix ) + 3, MODULE_SYMBOLNAME );

   sprintf( symbolText, "%s%s",  mangledName, suffix );

#ifndef NOSHARED
   result = dlsym( self->dllPtr, symbolText );
#else
   /* Try to map the symbol. */
   result = Module_MapStaticSymbol(self, symbolText);
#endif

   if( result == NULL ) {
      /* Try with a leading "_"... this is because on macx the dlcompat library can work either placing
         this "_" for you and without and there is no easy way to know */
      sprintf( symbolText, "_%s%s", mangledName, suffix );
#ifndef NOSHARED
      result = dlsym( self->dllPtr, symbolText );
#else
      result = Module_MapStaticSymbol(self, symbolText);
#endif
   }

   Memory_Free( symbolText );
   Memory_Free( mangledName );

   return result;
}

void Module_UnLoad( void* module ) {
   Module* self = (Module*)module;
   
#ifndef NOSHARED
   if( self->dllPtr ) {
      dlclose( self->dllPtr );
   }
#endif
   self->dllPtr = 0;
}

char* Module_MangledName( void* module ) {
   Module* self = (Module*)module;
   
   return self->MangleName( self->name );
}

#ifdef NOSHARED
Bool Module_CheckStaticName(Module *self) {
   int ii;

   for( ii = 0; ii < stg_num_modules; ii++ ) {
      if( !strcmp( self->mangledName, stg_module_names[ii] ) )
         break;
   }
   if( ii == stg_num_modules )
      return False;
   return True;
}

void* Module_MapStaticSymbol(Module *self, const char *sym) {
   int ii, jj;

   for( ii = 0; ii < stg_num_modules; ii++ ) {
      if( !strcmp(self->mangledName, stg_module_names[ii] ) )
         break;
   }
   if(ii == stg_num_modules)
      return NULL;

   for( jj = 0; jj < stg_num_module_syms[ii]; jj++ ) {
      if( !strcmp(sym, stg_module_syms[ii][jj] ) )
         break;
   }
   if( jj == stg_num_module_syms[ii] )
      return NULL;

   return stg_module_funcs[ii][jj];
}
#endif




