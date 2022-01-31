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
#include "Toolbox.h"
#include "ModulesManager.h"
#include "ToolboxesManager.h"

#include <stdlib.h>
#include <string.h>

/* Textual name of this class */
const Type ToolboxesManager_Type = "ToolboxesManager";

ToolboxesManager* ToolboxesManager_New( int* argc, char*** argv ) {
   /* Variables set in this function */
   SizeT                                       _sizeOfSelf = sizeof(ToolboxesManager);
   Type                                               type = ToolboxesManager_Type;
   Stg_Class_DeleteFunction*                       _delete = _ToolboxesManager_Delete;
   Stg_Class_PrintFunction*                         _print = _ToolboxesManager_Print;
   Stg_Class_CopyFunction*                           _copy = NULL;
   ModulesManager_GetModulesListFunction*  _getModulesList = _ToolboxesManager_GetToolboxesList;
   ModulesManager_LoadModuleFunction*          _loadModule = _ToolboxesManager_LoadToolbox;
   ModulesManager_UnloadModuleFunction*      _unloadModule = _ToolboxesManager_UnloadToolbox;
   ModulesManager_ModuleFactoryFunction*    _moduleFactory = Toolbox_Factory;
   ModulesManager_CheckContextFunction*      _checkContext = _ToolboxesManager_CheckContext;
   ModulesManager_GetModuleNameFunction*    _getModuleName = _ToolboxesManager_GetModuleName;

   return _ToolboxesManager_New( TOOLBOXESMANAGER_PASSARGS );
}

ToolboxesManager* _ToolboxesManager_New( TOOLBOXESMANAGER_DEFARGS ) {
   ToolboxesManager* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(ToolboxesManager) );
   self = (ToolboxesManager*)_ModulesManager_New( MODULESMANAGER_PASSARGS );
   
   /* General info */
   
   /* Virtual info */
   
   _ToolboxesManager_Init( self, argc, argv );
   
   return self;
}

void _ToolboxesManager_Init( void* toolboxesManager, int* argc, char*** argv ) {
   ToolboxesManager* self = (ToolboxesManager*)toolboxesManager;

   self->initTB = Stg_ObjectList_New();
   self->argc = argc;
   self->argv = argv;
}

void _ToolboxesManager_Delete( void* toolboxesManager ) {
   ToolboxesManager* self = (ToolboxesManager*)toolboxesManager;

   Stg_ObjectList_DeleteAllObjects( self->codelets );
   Stg_Class_Delete( self->codelets );
   ModulesManager_Unload( self );  /* this will unload all toolboxes implicitly */
   Stg_Class_Delete( self->modules );
   
   /* Delete parent */
   _Stg_Class_Delete( self );
}

void _ToolboxesManager_Print( void* toolboxesManager, Stream* stream ) {
   ToolboxesManager* self = (ToolboxesManager*)toolboxesManager;
   
   /* General info */
   Journal_Printf( (void*) stream, "Toolboxes (ptr): %p\n", self );
   
   if( Stg_ObjectList_Count(self->initTB) > 0 ) {
      Index i;
      
      Journal_Printf( stream, "Initialised Modules:\n" );
      Stream_Indent( stream );
      for( i = 0; i < Stg_ObjectList_Count(self->initTB) ; ++i ) {
         Journal_Printf( stream, "%s\n", self->initTB->data[i]->name );
      }
      Stream_UnIndent( stream );
   }
   
   /* Print parent */
   _ModulesManager_Print( self, stream );
}

Dictionary_Entry_Value* _ToolboxesManager_GetToolboxesList( void* toolboxesManager, void* _dictionary ) {
   /*ToolboxesManager*     self = (ToolboxesManager*)toolboxesManager;*/
   Dictionary*             dictionary = (Dictionary*)_dictionary;
   Dictionary_Entry_Value* pluginsList = NULL;
   
   pluginsList = Dictionary_Get( dictionary, "import" );

   return pluginsList;
}

Bool _ToolboxesManager_LoadToolbox( void* toolboxesManager, Module* toolbox ) {
   ToolboxesManager* self = (ToolboxesManager*)toolboxesManager;
   
   /* if not Loaded call the Initialise() and Register() */
   if( !Stg_ObjectList_Get( self->initTB, toolbox->name ) ) {

      ((Toolbox*)toolbox)->Initialise( self, self->argc, self->argv );
      ((Toolbox*)toolbox)->Register( self );

      Stg_ObjectList_Append( self->initTB, toolbox );
   }
   return True;
}

Bool _ToolboxesManager_UnloadToolbox( void* toolboxesManager, Module* toolbox ) {
   ToolboxesManager* self = (ToolboxesManager*)toolboxesManager;

   if( Stg_ObjectList_Get( self->initTB, toolbox->name ) ) {
      ((Toolbox*)toolbox)->Finalise( self );

      /* remove the toolbox from the initTB list, but don't actually Delete it's memory */
      Stg_ObjectList_Remove( self->initTB, toolbox->name, KEEP );
   }

   if( self->initTB && self->initTB->count == 0 ) {
      Stg_Class_Delete( self->initTB );
      self->initTB = NULL;
   }

   return True;
}

/* toolboxes do not need to be associated with contexts, so just return true */
Bool _ToolboxesManager_CheckContext( void* toolboxesManager, Dictionary_Entry_Value* modulesVal, unsigned int entry_I, Name contextName ) {
   return True;
}

Name _ToolboxesManager_GetModuleName( void* toolboxesManager, Dictionary_Entry_Value* moduleVal, unsigned int entry_I ) {
   return Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( moduleVal, entry_I ) );
}

Bool ToolboxesManager_IsInitialised( void* initRegister, char* label ) {
   ToolboxesManager* self = (ToolboxesManager*)initRegister;

   if( Stg_ObjectList_Get( self->initTB, label ) )
      return True;
   else
      return False;
}
