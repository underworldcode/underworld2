/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_Plugin_h__
#define __StGermain_Base_Extensibility_Plugin_h__
	
	/** The prototype for the Register function in a plugin */
	typedef Index (Plugin_RegisterFunction) ( void* pluginsManager );

	/* Textual name of this class */
	extern const Type Plugin_Type;

	/* Plugins info */
	#define __Plugin \
		/* General info */ \
		__Module \
		\
		/* Virtual info */ \
		\
		/* Plugin info */ \
		Plugin_RegisterFunction*    Register;
		
	struct Plugin { __Plugin };


	/* Create a new Plugin */
	Plugin* Plugin_New( Name name, Stg_ObjectList* directories );
	Module* Plugin_Factory( Name name, Stg_ObjectList* directories );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PLUGIN_DEFARGS \
                MODULE_DEFARGS

	#define PLUGIN_PASSARGS \
                MODULE_PASSARGS

	Plugin* _Plugin_New(  PLUGIN_DEFARGS  );
	
	/* Initialisation implementation */
	void _Plugin_Init( Plugin* self );
	
	/* Delete implementation */
	void _Plugin_Delete( void* plugin );
	
	/* Print implementation */
	void _Plugin_Print( void* plugin, Stream* stream );

	/* MangleName implementation */
	char* _Plugin_MangleName( char* name );

	/** Get the function pointer the to the plugin's register function */
	Plugin_RegisterFunction* Plugin_GetRegisterFunc( void* plugin );
	
#endif /* __StGermain_Base_Extensibility_Plugin_h__ */

