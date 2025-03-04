/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_ExtensionManager_Register_h__
#define __StGermain_Base_Extensibility_ExtensionManager_Register_h__
	

	/** Textual name of this class */
	extern const Type ExtensionManager_Register_Type;

	/* global default instantiation of the register (created in Init.c) */
	extern ExtensionManager_Register* extensionMgr_Register;
	
	/** ExtensionManager_Register class contents */
	#define __ExtensionManager_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* ExtensionManager_Register info */ \
		ExtensionList*				extensions;

	/** Keeps track of all the Extended data managers in use by a program - see ExtensionRegister.h */
	struct ExtensionManager_Register { __ExtensionManager_Register };
	
	/** Create a new ExtensionManager_Register */
	ExtensionManager_Register* ExtensionManager_Register_New( void );
	
	/** Initialise an ExtensionManager_Register */
	void ExtensionManager_Register_Init( void* extension_Register );
	
	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define EXTENSIONMANAGER_REGISTER_DEFARGS \
                STG_CLASS_DEFARGS

	#define EXTENSIONMANAGER_REGISTER_PASSARGS \
                STG_CLASS_PASSARGS

	ExtensionManager_Register* _ExtensionManager_Register_New(  EXTENSIONMANAGER_REGISTER_DEFARGS  );
	
	/** Initialisation implementation */
	void _ExtensionManager_Register_Init( void* extension_Register );
	
	/** Stg_Class_Delete() implementation. Note that this also deletes the extension managers themselves ... so the user
	shouldn't manually delete them. */
	void _ExtensionManager_Register_Delete( void* extension_Register );
	
	/** Stg_Class_Print() implementation. */
	void _ExtensionManager_Register_Print( void* extension_Register, Stream* stream );
	
	/** Add a new Extension to the register. */
	ExtensionManager_Index ExtensionManager_Register_Add( void* extension_Register, void* extension );

   /** Removes an Extension from the register */
   ExtensionManager_Index ExtensionManager_Register_Remove( void* extensionManager_Register, void* extension );
	
	/** Get the handle of an extension in the register. */
	ExtensionManager_Index ExtensionManager_Register_GetExtensionHandle( void* extension_Register, Name toGet );
	
	/* Get an extension manager from the register. */
	#define ExtensionManager_Register_GetExtension( extension_Register, index ) \
		( ExtensionManager_FromList( (extension_Register)->extensions, index ) )
	
#endif /* __StGermain_Base_Extensibility_ExtensionManager_Register_h__ */

