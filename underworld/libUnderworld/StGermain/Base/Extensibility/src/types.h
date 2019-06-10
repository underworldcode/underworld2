/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_types_h__
#define __StGermain_Base_Extensibility_types_h__
	
	/* Generic types */
	/* Base types/classes */
	typedef struct Hook				Hook;
	typedef struct ClassHook			ClassHook;
	typedef struct EntryPoint 			EntryPoint;
	typedef struct ExtensionInfo			ExtensionInfo;
	typedef struct SimpleExtensionInfo		SimpleExtensionInfo;
	typedef struct ClassPtrExtensionInfo		ClassPtrExtensionInfo;
	typedef struct ExtensionManager			ExtensionManager;
	typedef struct ExtensionManager_Register	ExtensionManager_Register;
	typedef struct EntryPoint_Register		EntryPoint_Register;
	typedef struct Module				Module;
	typedef struct Plugin				Plugin;
	typedef struct Toolbox				Toolbox;
	typedef struct ModulesManager			ModulesManager;
	typedef struct PluginsManager			PluginsManager;
	typedef struct ToolboxesManager			ToolboxesManager;

	/* Extension types ... for readability */
	typedef Stg_ObjectList			ExtensionList;
	typedef Index				ExtensionManager_Index;
	typedef Stg_ObjectList			ExtensionInfoList;
	typedef Index				ExtensionInfo_Index;
	
	/* EntryPoint types ... for readability */
	typedef Stg_ObjectList			EntryPointList;
	typedef unsigned int			EntryPoint_Index;

	/* Hook types ... for readability */
	typedef void*				Func_Ptr;
	typedef Name				Hook_Name;
	typedef char*				Hook_AddedBy;
	typedef Hook*				Hook_Ptr;
	typedef Stg_ObjectList			HookList;
	typedef Index				Hook_Index;
	
	/* plugin types ... for readability */
	typedef void*				DLL_Handle;
	typedef DLL_Handle*			DLL_Handle_List;

#endif /* __StGermain_Base_Extensibility_types_h__ */
