/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_shortcuts_h__
#define __StGermain_Base_Extensibility_shortcuts_h__

	/** Macro to grab an extension from a named Stg_ObjectList */
	#define ExtensionManager_FromList( extList, index ) \
		((ExtensionManager*) (extList)->data[index])

	/** Macro to grab an extension info from a Stg_ObjectList */
	#define ExtensionInfo_At( extInfos, index ) \
		((ExtensionInfo*) (extInfos)->data[index])

	/* Shortcuts to Entry Point functions. Assume that the build system is 
	passing a value of CURR_MODULE_NAME (ie -DCURR_MODULE_NAME=\"Snark\" ).
	Also assumes the user wants to call the E.P. by the name of the function. */

	#define EP_Prepend( entryPoint, func ) \
		EntryPoint_Prepend( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )


	#define EP_PrependClassHook( entryPoint, func, reference ) \
		EntryPoint_PrependClassHook( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )

	#define EP_Prepend_AlwaysFirst( entryPoint, func ) \
		EntryPoint_Prepend_AlwaysFirst( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_PrependClassHook_AlwaysFirst( entryPoint, func, reference ) \
		EntryPoint_PrependClassHook_AlwaysFirst( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )

	#define EP_Append( entryPoint, func ) \
		EntryPoint_Append( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_AppendClassHook( entryPoint, func, reference ) \
		EntryPoint_AppendClassHook( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )

	#define EP_Append_AlwaysLast( entryPoint, func ) \
		EntryPoint_Append_AlwaysLast( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_AppendClassHook_AlwaysLast( entryPoint, func, reference ) \
		EntryPoint_AppendClassHook_AlwaysLast( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )

	#define EP_InsertBefore( entryPoint, funcToInsertBefore, func ) \
		EntryPoint_InsertBefore( \
			entryPoint, \
			funcToInsertBefore, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_InsertClassHookBefore( entryPoint, funcToInsertBefore, func, reference ) \
		EntryPoint_InsertClassHookBefore( \
			entryPoint, \
			funcToInsertBefore, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )
	
	#define EP_InsertAfter( entryPoint, funcToInsertAfter, func ) \
		EntryPoint_InsertAfter( \
			entryPoint, \
			funcToInsertAfter, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_InsertClassHookAfter( entryPoint, funcToInsertAfter, func, reference ) \
		EntryPoint_InsertClassHookAfter( \
			entryPoint, \
			funcToInsertAfter, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )

	#define EP_Remove          EntryPoint_Remove
	#define EP_RemoveClassHook EntryPoint_Remove
	
	#define EP_Replace( entryPoint, funcToReplace, func ) \
		EntryPoint_Replace( \
			entryPoint, \
			funcToReplace, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_ReplaceClassHook( entryPoint, funcToReplace, func, reference ) \
		EntryPoint_ReplaceClassHook( \
			entryPoint, \
			funcToReplace, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )				

	#define EP_ReplaceAll( entryPoint, func ) \
		EntryPoint_ReplaceAll( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			CURR_MODULE_NAME )

	#define EP_ReplaceAllClassHook( entryPoint, func, reference ) \
		EntryPoint_ReplaceAllClassHook( \
			entryPoint, \
			#func, \
			(Func_Ptr)func, \
			((Stg_Class*)reference)->type, \
			reference )		

	#define EP_Purge( entryPoint ) \
		EntryPoint_Purge( entryPoint )

#endif /* __StGermain_Base_Extensibility_shortcuts_h__ */	

