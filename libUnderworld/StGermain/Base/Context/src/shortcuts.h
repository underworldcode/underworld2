/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_shortcuts_h__
#define __StGermain_Base_Context_shortcuts_h__
	
	#define Context_PrintConcise		AbstractContext_PrintConcise
	#define Context_AddEntryPoint		AbstractContext_AddEntryPoint
	#define Context_GetEntryPoint		AbstractContext_GetEntryPoint
	#define Context_Call			AbstractContext_Call
	#define Context_KeyCall			AbstractContext_KeyCall
	#define Context_KeyHandle		AbstractContext_KeyHandle
	#define Context_Dt			AbstractContext_Dt
	#define Context_Step			AbstractContext_Step
	#define Context_WarnIfNoHooks		AbstractContext_WarnIfNoHooks
	#define Context_ErrorIfNoHooks		AbstractContext_ErrorIfNoHooks
	
	#define KeyCall				Context_KeyCall
	#define KeyHandle			Context_KeyHandle

	/* Shortcuts to other short cut Entry Point functions. */

	#define ContextEP_Prepend( context, epName, func ) \
		EP_Prepend( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Prepend_AlwaysFirst( context, epName, func ) \
		EP_Prepend_AlwaysFirst( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Append( context, epName, func ) \
		EP_Append( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Append_AlwaysLast( context, epName, func ) \
		EP_Append_AlwaysLast( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_InsertBefore( context, epName, funcToInsertBefore, func ) \
		EP_InsertBefore( \
			Context_GetEntryPoint( context, epName ), \
			funcToInsertBefore, \
			func )
	
	#define ContextEP_InsertAfter( context, epName, funcToInsertAfter, func ) \
		EP_InsertAfter( \
			Context_GetEntryPoint( context, epName ), \
			funcToInsertAfter, \
			func )

	#define ContextEP_Remove( context, epName, reference )\
		EP_Remove( \
			Context_GetEntryPoint( context, epName ), \
			reference )
	
	#define ContextEP_Replace( context, epName, funcToReplace, func ) \
		EP_Replace( \
			Context_GetEntryPoint( context, epName ), \
			funcToReplace, \
			func )

	#define ContextEP_ReplaceAll( context, epName, func ) \
		EP_ReplaceAll( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Purge( context, epName ) \
		EP_Purge( Context_GetEntryPoint( context, epName ) )

#endif /* __StGermain_Base_Context_shortcuts_h__ */
