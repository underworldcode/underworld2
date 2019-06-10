/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_Class_h__
#define __StGermain_Base_Foundation_Class_h__

	/* Textual name of this class */
	extern const Type Stg_Class_Type;

	/* Forward declaring Stream */
	struct Stream;
	struct PtrMap;
	
	/* Child classes must define these abstract functions */
	typedef void (Stg_Class_DeleteFunction)	( void* _class );
	typedef void (Stg_Class_PrintFunction)	( void* _class, struct Stream* stream );
	typedef void* (Stg_Class_CopyFunction)	( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/** \def __Stg_Class information */
	#define __Stg_Class \
		/* General info */ 					\
		SizeT				_sizeOfSelf;		/**< Size of the final class type */ \
		Bool				_deleteSelf;		/**< True it is to be deallocated in Delete */ \
		Type				type;			/**< Global const char* to string of class' name */ \
		unsigned			nRefs;			/* Reference counting. */ \
        Bool                locked;    /* set this flag to prevent deletions/destruction.  useful for when object is created from python, as python will deal with object lifetime */ \
									\
		/* Virtual info */ 					\
		Stg_Class_DeleteFunction*	_delete;		/**< Virtual function for deleting a class. */ \
		Stg_Class_PrintFunction*	_print;			/**< Virtual function for printing a class. */ \
		Stg_Class_CopyFunction*		_copy;			/**< Virtual funciton for copying a class. */
	
	struct _Stg_Class { __Stg_Class };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	#define STG_CLASS_DEFARGS					\
		SizeT				_sizeOfSelf,		\
		Type				type,			\
		Stg_Class_DeleteFunction*	_delete,		\
		Stg_Class_PrintFunction*	 _print,		\
		Stg_Class_CopyFunction*		  _copy

	#define STG_CLASS_PASSARGS					\
		_sizeOfSelf, type, _delete, _print, _copy
    
	
	/** Constructor interface. */
	Stg_Class* _Stg_Class_New( SizeT _sizeOfSelf, 
			Type type, 
			Stg_Class_DeleteFunction*	_delete, 
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy );
	
	/** Init interface. */
	void _Stg_Class_Init( Stg_Class* self );
	
	
	/** Deletes any instance of Stg_Class or a sub-class. */
	void Stg_Class_Delete( void* _class );
	
	/** Prints any instance of Stg_Class or a sub-class. */
	void Stg_Class_Print( void* _class, struct Stream* stream );

	/** Prints any instance of Stg_Class or a sub-class. */
/*
	#define Print Stg_Class_Print
*/
	
	/** Generic copy function caller */
	void* Stg_Generic_Copy( 
		Stg_Class_CopyFunction* copyFunc,
		void* _class, 
		void* dest, 
		Bool deep, 
		Name nameExt, 
		struct PtrMap* ptrMap );
	
	/** Copy a class. */
	void* Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/** Delete interface. */
	void _Stg_Class_Delete( void* _class );
	
	/** Print interface. */
	void _Stg_Class_Print( void* _class, struct Stream* stream );
	
	/** Coy interface. */
	void* _Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Get the class type */
	#define Stg_Class_GetTypeMacro( _class ) \
		((_class)->type)
	Type Stg_Class_GetTypeFunc( void* _class ); 
	#ifdef MACRO_AS_FUNC
		/** Get the class type */
		#define Stg_Class_GetType Stg_Class_GetTypeFunc
	#else
		/** Get the class type */
		#define Stg_Class_GetType Stg_Class_GetTypeMacro
	#endif

	void Stg_Class_AddRef( void* _class );
	void Stg_Class_RemoveRef( void* _class );

    void Stg_Class_Lock( void* _class );
    void Stg_Class_Unlock( void* _class );

	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __StGermain_Base_Foundation_Class_h__ */
