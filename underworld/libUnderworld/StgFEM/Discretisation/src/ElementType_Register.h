/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_ElementType_Register_h__
#define __StgFEM_Discretisation_ElementType_Register_h__
	
	
	/* Textual name of this class */
	extern const Type ElementType_Register_Type;

	/* global default instantiation of this register (created in Init.c) */
	extern ElementType_Register* elementType_Register;
	
	/* ElementType_Register info */
	#define __ElementType_Register \
		/* General info */ \
		__Stg_Component \
		\
		DomainContext*			context; \
		/* Virtual info */ \
		\
		/* ElementType_Register info */ \
		Stream*					debug; \
		ElementType_Index		count; \
		SizeT						_size; \
		SizeT						_delta; \
		ElementType**			elementType;

	struct ElementType_Register { __ElementType_Register };

	/* Create a new ElementType_Register */
	void* ElementType_Register_DefaultNew( Name name );
	
	ElementType_Register* ElementType_Register_New( Name name );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ELEMENTTYPE_REGISTER_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define ELEMENTTYPE_REGISTER_PASSARGS \
                STG_COMPONENT_PASSARGS

	ElementType_Register* _ElementType_Register_New(  ELEMENTTYPE_REGISTER_DEFARGS  );
	
	/* Initialisation implementation */
	void _ElementType_Register_Init( void* elementType_Register );
	
	/* Stg_Class_Delete implementation */
	void _ElementType_Register_Delete( void* elementType_Register );
	
	/* Print implementation */
	void _ElementType_Register_Print( void* elementType_Register, Stream* stream );
	
	void _ElementType_Register_AssignFromXML( void* elementType_Register, Stg_ComponentFactory *cf, void* data );
	
	void _ElementType_Register_Build( void* elementType_Register, void *data );
	
	void _ElementType_Register_Initialise( void* elementType_Register, void *data );
	
	void _ElementType_Register_Execute( void* elementType_Register, void *data );

	void _ElementType_Register_Destroy( void* elementType_Register, void *data );
	
	
	/* Add a new elementType */
	ElementType_Index ElementType_Register_Add( void* elementType_Register, void* elementType );
	
	/* Get the handle to an elementType */
	ElementType_Index ElementType_Register_GetIndex( void* elementType_Register, Type type );
	
	/* Get an element type from the register */
	#define ElementType_Register_At( elementType_Register, handle )		((elementType_Register)->elementType[(handle)] )
	ElementType* _ElementType_Register_At( void* elementType_Register, ElementType_Index handle );
	
#endif /* __StgFEM_Discretisation_ElementType_Register_h__ */

