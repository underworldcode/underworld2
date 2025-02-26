/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_EntryPoint_Register_h__
#define __StGermain_Base_Extensibility_EntryPoint_Register_h__
	

	/* Textual name of this class */
	extern const Type EntryPoint_Register_Type;
	
	/* EntryPoint_Register info */
	#define __EntryPoint_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* EntryPoint_Register info */ \
		EntryPoint_Index			count; \
		SizeT					_size; \
		SizeT					_delta; \
		EntryPoint**				entryPoint;
	struct EntryPoint_Register { __EntryPoint_Register };
	
	
	/* Create a new EntryPoint_Register */
	EntryPoint_Register* EntryPoint_Register_New( void );
	
	/* Initialise a EntryPoint_Register */
	void EntryPoint_Register_Init( void* entryPoint_Register );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ENTRYPOINT_REGISTER_DEFARGS \
                STG_CLASS_DEFARGS

	#define ENTRYPOINT_REGISTER_PASSARGS \
                STG_CLASS_PASSARGS

	EntryPoint_Register* _EntryPoint_Register_New(  ENTRYPOINT_REGISTER_DEFARGS  );
	
	/* Initialisation implementation */
	void _EntryPoint_Register_Init( void* entryPoint_Register );
	
	
	/* Stg_Class_Delete implementation */
	void _EntryPoint_Register_Delete( void* entryPoint_Register );
	
	/* Print implementation */
	void _EntryPoint_Register_Print( void* entryPoint_Register, Stream* stream );
	
	
	/* Add a new elementType */
	EntryPoint_Index EntryPoint_Register_Add( void* entryPoint_Register, void* entryPoint );
	
	/* Get the handle to an elementType */
	EntryPoint_Index EntryPoint_Register_GetHandle( void* entryPoint_Register, Type type );
	
	/* Get an element type from the register... SLOW */
	#define EntryPoint_Register_At( entryPoint_Register, handle )		((entryPoint_Register)->entryPoint[(handle)] )
	EntryPoint* _EntryPoint_Register_At( void* entryPoint_Register, EntryPoint_Index handle );
	
	/* If 'entryPoint' is in the register, return its index */
	EntryPoint_Index EntryPoint_Register_Find( void* entryPoint_Register, void* entryPoint );
	
#endif /* __StGermain_Base_Extensibility_EntryPoint_Register_h__ */

