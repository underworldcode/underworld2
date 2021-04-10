/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_Object_h__
#define __StGermain_Base_Foundation_Object_h__

	/** Textual name for Stg_Object class. */
	extern const Type Stg_Object_Type;

	/** \def __Stg_Object See Object */
	#define __Stg_Object \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Object info */ \
		Name				name; \
		AllocationType			nameAllocationType;
	
	struct _Stg_Object { __Stg_Object };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STG_OBJECT_DEFARGS \
                STG_CLASS_DEFARGS, \
                Name                          name, \
                AllocationType  nameAllocationType

	#define STG_OBJECT_PASSARGS \
                STG_CLASS_PASSARGS, \
	        name,               \
	        nameAllocationType

	Stg_Object* _Stg_Object_New(  STG_OBJECT_DEFARGS  );
	
	/** Init interface. */
	void _Stg_Object_Init( Stg_Object* self, Name name, AllocationType nameAllocationType );
	
	/** Delete interface. */
	void _Stg_Object_Delete( void* object );
	
	/** Print interface. */
	void _Stg_Object_Print( void* object, struct Stream* stream );
	
	/** Copy interface. */
	void* _Stg_Object_Copy( void* object, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Sets the name of the object. */
	void Stg_Object_SetName( void* object, Name name );
	
	
	/** \internal Get the object name. */
	#define _Stg_Object_GetNameMacro( object ) ( (object)->name )
	Name _Stg_Object_GetNameFunc( void* object );
	#ifdef MACRO_AS_FUNC
		/** Get the object name. */
		#define Stg_Object_GetName _Stg_Object_GetNameFunc
	#else
		#define Stg_Object_GetName _Stg_Object_GetNameMacro
	#endif
	
	/** Function which simply tacks on a suffix to an object's name of form "ObjectName-Suffix" 
	 *  Pointer returned must be free'd */
	Name Stg_Object_AppendSuffix( void* object, Name suffix ) ;
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __StGermain_Base_Foundation_Object_h__ */

