/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_ObjectAdaptor_h__
#define __StGermain_Base_Automation_ObjectAdaptor_h__
	
	/** Delete/Print/Copy function prototypes for attached pointers (not StGermain classes) */
	/** *Note*: this function should only delete extra memory allocated by the object, _not
	the object itself_ - the ObjectAdaptor wrapper takes responsibility for this */
	typedef void  (Stg_ObjectAdaptor_DeletePointerFunction)	( void* ptr );
	typedef void  (Stg_ObjectAdaptor_PrintPointerFunction)	( void* ptr, struct Stream* stream );
	typedef void* (Stg_ObjectAdaptor_CopyPointerFunction)	( 
									void*					ptr, 
									void*					dest,
									Bool					deep,
									Name					nameExt, 
									struct PtrMap*				ptrMap );
	
	extern const Type Stg_ObjectAdaptor_Type;
	
	/** Stg_ObjectAdaptor class */
	#define __Stg_ObjectAdaptor \
		__Stg_Object \
		\
		void*						dataPtr; \
		Bool						iOwn; \
		Bool						isGlobal; \
		Bool						isStgClass; \
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete; \
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint; \
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy;
	
	/** Defines key information about a Stg_ObjectAdaptor - see ObjectAdaptor.h. */
	struct _Stg_ObjectAdaptor{ __Stg_ObjectAdaptor };	
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new Stg_ObjectAdaptor, when the the object is a StGermain object. */ 
	Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfClass( void* dataPtr, Name name, Bool iOwn, Bool isGlobal );
	
	/** Initialise a Stg_ObjectAdaptor, when the the object is a StGermain object. */
	void Stg_ObjectAdaptor_InitOfClass( Stg_ObjectAdaptor* self, void* objPtr, Name name, Bool iOwn, Bool isGlobal );
	
	
	/** Create a new Stg_ObjectAdaptor, when the the object is not a StGermain object.
		See note before Stg_ObjectAdaptor_DeletePointerFunction declaration 
		about what this function has to be responsible for */ 
	Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfPointer( 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	/** Initialise a Stg_ObjectAdaptor, when the the object is not a StGermain object. 
		See note before Stg_ObjectAdaptor_DeletePointerFunction declaration 
		about what this function has to be responsible for */ 
	void Stg_ObjectAdaptor_InitOfPointer( 
		Stg_ObjectAdaptor* 				self, 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STG_OBJECTADAPTOR_DEFARGS \
                STG_OBJECT_DEFARGS, \
                void*                                        dataPtr, \
                Bool                                            iOwn, \
                Bool                                        isGlobal, \
                Bool                                      isStgClass, \
                Stg_ObjectAdaptor_DeletePointerFunction*   ptrDelete, \
                Stg_ObjectAdaptor_PrintPointerFunction*     ptrPrint, \
                Stg_ObjectAdaptor_CopyPointerFunction*       ptrCopy

	#define STG_OBJECTADAPTOR_PASSARGS \
                STG_OBJECT_PASSARGS, \
	        dataPtr,    \
	        iOwn,       \
	        isGlobal,   \
	        isStgClass, \
	        ptrDelete,  \
	        ptrPrint,   \
	        ptrCopy   

	Stg_ObjectAdaptor* _Stg_ObjectAdaptor_New(  STG_OBJECTADAPTOR_DEFARGS  );
	
	/* Initialisation implementation */
	void _Stg_ObjectAdaptor_Init( 
		Stg_ObjectAdaptor*				self, 
		void*						dataPtr, 
		Bool						iOwn,
		Bool						isGlobal,
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	
	/* Stg_Class_Delete() implementation */
	void _Stg_ObjectAdaptor_Delete( void* objectAdaptor );
	
	/* Stg_Class_Print() implementation */
	void _Stg_ObjectAdaptor_Print( void* objectAdaptor, struct Stream* stream );
	
	/* Stg_Class_Copy() implementation */
	void* _Stg_ObjectAdaptor_Copy( void* objectAdaptor, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
		

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Obtain the object of the tuple. */
	#define Stg_ObjectAdaptor_ObjectMacro( self ) \
		( (self)->dataPtr )
	void* Stg_ObjectAdaptor_ObjectFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Obtain the object of the tuple. */
		#define Stg_ObjectAdaptor_Object Stg_ObjectAdaptor_ObjectFunc
	#else
		/** Obtain the object of the tuple. */
		#define Stg_ObjectAdaptor_Object Stg_ObjectAdaptor_ObjectMacro
	#endif
	
	/* Do I own the object (i.e. will I attempt to delete it on my deletion)? */
	#define Stg_ObjectAdaptor_IOwnMacro( self ) \
		( (self)->iOwn )
	Bool Stg_ObjectAdaptor_IOwnFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Do I own the object (i.e. will I attempt to delete it on my deletion)? */
		#define Stg_ObjectAdaptor_IOwn Stg_ObjectAdaptor_IOwnFunc
	#else
		/** Do I own the object (i.e. will I attempt to delete it on my deletion)? */
		#define Stg_ObjectAdaptor_IOwn Stg_ObjectAdaptor_IOwnMacro
	#endif
	
	/* Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
	#define Stg_ObjectAdaptor_IsGlobalMacro( self ) \
		( (self)->iOwn )
	Bool Stg_ObjectAdaptor_IsGlobalFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
		#define Stg_ObjectAdaptor_IsGlobal Stg_ObjectAdaptor_IsGlobalFunc
	#else
		/** Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
		#define Stg_ObjectAdaptor_IsGlobal Stg_ObjectAdaptor_IsGlobalMacro
	#endif
	
	/* Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
	   Stg_Class_Delete on it where appropriate)? */
	#define Stg_ObjectAdaptor_IsStgClassMacro( self ) \
		( (self)->isStgClass )
	Bool Stg_ObjectAdaptor_IsStgClassFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
		   Stg_Class_Delete on it where appropriate)? */
		#define Stg_ObjectAdaptor_IsStgClass Stg_ObjectAdaptor_IsStgClassFunc
	#else
		/** Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
		   Stg_Class_Delete on it where appropriate)? */
		#define Stg_ObjectAdaptor_IsStgClass Stg_ObjectAdaptor_IsStgClassMacro
	#endif
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __StGermain_Base_Automation_ObjectAdaptor_h__ */

