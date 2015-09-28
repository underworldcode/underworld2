/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include <string.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "Module.h"
#include "Plugin.h"
#include "ModulesManager.h"
#include "PluginsManager.h"


/* Textual name of this class */
const Type PluginsManager_Type = "PluginsManager";


PluginsManager* PluginsManager_New( void ) {
	/* Variables set in this function */
	SizeT                                       _sizeOfSelf = sizeof(PluginsManager);
	Type                                               type = PluginsManager_Type;
	Stg_Class_DeleteFunction*                       _delete = _PluginsManager_Delete;
	Stg_Class_PrintFunction*                         _print = _PluginsManager_Print;
	Stg_Class_CopyFunction*                           _copy = NULL;
	ModulesManager_GetModulesListFunction*  _getModulesList = _PluginsManager_GetPluginsList;
	ModulesManager_LoadModuleFunction*          _loadModule = _PluginsManager_LoadPlugin;
	ModulesManager_UnloadModuleFunction*      _unloadModule = _PluginsManager_UnloadPlugin;
	ModulesManager_ModuleFactoryFunction*    _moduleFactory = Plugin_Factory;
	ModulesManager_CheckContextFunction*      _checkContext = _PluginsManager_CheckContext;
	ModulesManager_GetModuleNameFunction*    _getModuleName = _PluginsManager_GetModuleName;

	return _PluginsManager_New(  PLUGINSMANAGER_PASSARGS  );
}

PluginsManager* _PluginsManager_New(  PLUGINSMANAGER_DEFARGS  )
{
	PluginsManager* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PluginsManager) );
	self = (PluginsManager*)_ModulesManager_New(  MODULESMANAGER_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	_PluginsManager_Init( self );
	
	return self;
}

void _PluginsManager_Init( void* pluginsManager ) {
}


void _PluginsManager_Delete( void* pluginsManager ) {
	PluginsManager*         self = (PluginsManager*)pluginsManager;

	/* Delete parent */
	_ModulesManager_Delete( self );
}

void _PluginsManager_Print( void* pluginsManager, Stream* stream ) {
	PluginsManager* self = (PluginsManager*)pluginsManager;
	
	/* General info */
	Journal_Printf( (void*) stream, "Plugins (ptr): %p\n", self );
	
	/* Print parent */
	_ModulesManager_Print( self, stream );
}


Dictionary_Entry_Value* _PluginsManager_GetPluginsList( void* pluginsManager, void* _dictionary ) {
	/*PluginsManager*			self = (PluginsManager*)pluginsManager;*/
	Dictionary*			dictionary = (Dictionary*)_dictionary;
	Dictionary_Entry_Value*		pluginsList = NULL;
	
	pluginsList = Dictionary_Get( dictionary, "plugins" );

	return pluginsList;
}

Bool _PluginsManager_LoadPlugin( void* pluginsManager, Module* plugin ) {
	PluginsManager* self = (PluginsManager*)pluginsManager;
	
	((Plugin*)plugin)->Register( self );
    
	return True;
}

Bool PluginsManager_UnloadAll( void* pluginsManager ) {
	PluginsManager* self = (PluginsManager*)pluginsManager;
   Module* module = NULL;

   while( self->modules->count ) {
      module = (Module*)Stg_ObjectList_At( self->modules, self->modules->count - 1 ); /* reverse order deletion */
      _PluginsManager_UnloadPlugin( self, module );
   }

   return True;
}
Bool _PluginsManager_UnloadPlugin( void* pluginsManager, Module* plugin ) {
	PluginsManager* self = (PluginsManager*)pluginsManager;

   Module_UnLoad( plugin );
   self->modules->count--;

	return True;
}

void PluginsManager_RemoveAllFromComponentRegister( void* pluginsManager ) {
	PluginsManager*			self = (PluginsManager*)pluginsManager;
   Stg_ComponentRegister*	componentRegister = Stg_ComponentRegister_Get_ComponentRegister();
   Index							i;

   for (i=0; i<Stg_ObjectList_Count(self->codelets); i++) {
      Stg_Object *codelet = self->codelets->data[i];
      Stg_ComponentRegister_RemoveEntry(componentRegister, codelet->type, "0");
   }
}

Bool _PluginsManager_CheckContext( void* pluginsManager, Dictionary_Entry_Value* modulesVal, unsigned int entry_I, Name contextName ) {
	PluginsManager*			self;
	Dictionary_Entry_Value*	pluginDEV = Dictionary_Entry_Value_GetElement( modulesVal, entry_I );
	Dictionary*					pluginDict;
	Name							componentName;

	self = (PluginsManager*)pluginsManager;

	pluginDict = Dictionary_Entry_Value_AsDictionary( pluginDEV );
	if( !pluginDict )
		return False;

	componentName = Dictionary_GetString_WithDefault( pluginDict, "Context", "context" );

	if( !strcmp( componentName, contextName ) )
		return True;

	return False;
}

Name _PluginsManager_GetModuleName( void* pluginsManager, Dictionary_Entry_Value* moduleVal, unsigned int entry_I ) {
	PluginsManager*			self;
	Dictionary_Entry_Value*	pluginDEV = Dictionary_Entry_Value_GetElement( moduleVal, entry_I );
	Dictionary*					pluginDict = Dictionary_Entry_Value_AsDictionary( pluginDEV );
	Name							pluginName = Dictionary_GetString( pluginDict, "Type" );

	self = (PluginsManager*)pluginsManager;

	return pluginName;	
}




