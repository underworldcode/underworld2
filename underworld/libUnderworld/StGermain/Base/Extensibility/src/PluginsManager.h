/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_PluginsManager_h__
#define __StGermain_Base_Extensibility_PluginsManager_h__
	

	/* Textual name of this class */
	extern const Type PluginsManager_Type;

	
	/* Plugins info */
	#define __PluginsManager \
		/* General info */ \
		__ModulesManager \
		\
		/* Virtual info */ \
		\
		/* Plugins info */
		
	struct PluginsManager { __PluginsManager };
	
    /** Define a global list of plugin directories*/
     extern Stg_ObjectList*  pluginDirectories;	

	/* Create a new Plugins */
	PluginsManager* PluginsManager_New( void );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PLUGINSMANAGER_DEFARGS \
                MODULESMANAGER_DEFARGS

	#define PLUGINSMANAGER_PASSARGS \
                MODULESMANAGER_PASSARGS

	PluginsManager* _PluginsManager_New(  PLUGINSMANAGER_DEFARGS  );
	
	/* Initialisation implementation */
	void _PluginsManager_Init( void* pluginsManager );
	
	/* Stg_Class_Delete implementation */
	void _PluginsManager_Delete( void* pluginsManager );
	
	/* Print implementation */
	void _PluginsManager_Print( void* pluginsManager, Stream* stream );
	
	/** Get the plugins list from the dictionary */
	Dictionary_Entry_Value* _PluginsManager_GetPluginsList( void* pluginsManager, void* dictionary );

	/** Exactly what to do to load the plugin */
	Bool _PluginsManager_LoadPlugin( void* pluginsManager, Module* plugin );

	/** Exactly what to do to unload the plugin */
	Bool _PluginsManager_UnloadPlugin( void* pluginsManager, Module* plugin );

   /** unload all plugins, this includes dlclosing when dynamic libs are used */
   Bool PluginsManager_UnloadAll( void* pluginsManager );

	/** Remove all plugins from register */
   void PluginsManager_RemoveAllFromComponentRegister( void* pluginsManager );
	
	Bool _PluginsManager_CheckContext( void* pluginsManager, Dictionary_Entry_Value* modulesVal, unsigned int entry_I, Name contextName );

	Name _PluginsManager_GetModuleName( void* pluginsManager, Dictionary_Entry_Value* moduleVal, unsigned int entry_I );

	#define PluginsManager_Submit ModulesManager_Submit
	
#endif /* __StGermain_Base_Extensibility_PluginsManager_h__ */

