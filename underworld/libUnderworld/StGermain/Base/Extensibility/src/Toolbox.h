/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_Toolbox_h__
#define __StGermain_Base_Extensibility_Toolbox_h__
	
	/** The prototype for the Register function in a toolbox */
	typedef Index (Toolbox_RegisterFunction)   ( void* toolboxesManager );
	typedef Index (Toolbox_InitialiseFunction) ( void* toolboxesManager, int* argc, char*** argv );
	typedef Index (Toolbox_FinaliseFunction)   ( void* toolboxesManager );

	/* Textual name of this class */
	extern const Type Toolbox_Type;

	/* Toolboxs info */
	#define __Toolbox \
		/* General info */ \
		__Module \
		\
		/* Virtual info */ \
		\
		/* Toolbox info */ \
		Toolbox_RegisterFunction*    Register; \
		Toolbox_InitialiseFunction*  Initialise; \
		Toolbox_FinaliseFunction*    Finalise;
		
	struct Toolbox { __Toolbox };


	/* Create a new Toolbox */
	Toolbox* Toolbox_New( Name name, Stg_ObjectList* directories );
	Module* Toolbox_Factory( Name name, Stg_ObjectList* directories );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TOOLBOX_DEFARGS \
                MODULE_DEFARGS

	#define TOOLBOX_PASSARGS \
                MODULE_PASSARGS

	Toolbox* _Toolbox_New(  TOOLBOX_DEFARGS  );
	
	/* Initialisation implementation */
	void _Toolbox_Init( Toolbox* self );
	
	/* Delete implementation */
	void _Toolbox_Delete( void* toolbox );
	
	/* Print implementation */
	void _Toolbox_Print( void* toolbox, Stream* stream );

	/* MangleName implementation */
	char* _Toolbox_MangleName( char* name );

	/** Get the function pointer the to the toolbox's register function */
	Toolbox_RegisterFunction* Toolbox_GetRegisterFunc( void* toolbox );
	
	/** Get the function pointer the to the toolbox's register function */
	Toolbox_InitialiseFunction* Toolbox_GetInitialiseFunc( void* toolbox );
	
	/** Get the function pointer the to the toolbox's register function */
	Toolbox_FinaliseFunction* Toolbox_GetFinaliseFunc( void* toolbox );
	
#endif /* __StGermain_Base_Extensibility_Toolbox_h__ */

