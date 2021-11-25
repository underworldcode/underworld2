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
#include "EntryPoint.h"
#include "EntryPoint_Register.h"
#include "ExtensionInfo.h"
#include "ClassPtrExtensionInfo.h"
#include "ExtensionManager.h"
#include "ExtensionManager_Register.h"
#include "Hook.h"
#include "ClassHook.h"
#include "Module.h"
#include "Plugin.h"
#include "Toolbox.h"
#include "ModulesManager.h"
#include "PluginsManager.h"
#include "ToolboxesManager.h"
#include "SimpleExtensionInfo.h"
#include "Init.h"
#include "Finalise.h"

#include <stdio.h>



ToolboxesManager* stgToolboxesManager = 0;

Bool BaseExtensibility_Init( int* argc, char** argv[] ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 

	extensionMgr_Register = ExtensionManager_Register_New();

	RegisterParent( Hook_Type,                      Stg_Object_Type );
	RegisterParent( ClassHook_Type,                 Hook_Type );

	RegisterParent( EntryPoint_Type,                Stg_Object_Type );
	RegisterParent( EntryPoint_Register_Type,       Stg_Class_Type );
	
	RegisterParent( ExtensionManager_Type,          Stg_Object_Type );
	RegisterParent( ExtensionManager_Register_Type, Stg_Class_Type );

	RegisterParent( ExtensionInfo_Type,             Stg_Object_Type );
	RegisterParent( SimpleExtensionInfo_Type,       ExtensionInfo_Type );
	RegisterParent( ClassPtrExtensionInfo_Type,     ExtensionInfo_Type );

	RegisterParent( Module_Type,                    Stg_Object_Type );
	RegisterParent( Plugin_Type,                    Module_Type );
	RegisterParent( Toolbox_Type,                   Module_Type );
	RegisterParent( ModulesManager_Type,            Stg_Class_Type );
	RegisterParent( PluginsManager_Type,            ModulesManager_Type );
	RegisterParent( ToolboxesManager_Type,          ModulesManager_Type );
	
	stgToolboxesManager = ToolboxesManager_New( argc, argv );
		
	return True;
}

ToolboxesManager* GetToolboxManagerInstance(){ return stgToolboxesManager;}



