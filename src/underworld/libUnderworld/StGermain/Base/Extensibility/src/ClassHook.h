/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_ClassHook_h__
#define __StGermain_Base_Extensibility_ClassHook_h__
	

	/** Textual name of this class */
	extern const Type ClassHook_Type;

	/** ClassHook struct- stores a func pointer plus info */
	#define __ClassHook \
		__Hook \
		\
		void*		reference;		/**< Reference to the instance this member function belongs to */
	
	struct ClassHook { __ClassHook };
	
	/* Create a new ClassHook */
	ClassHook* ClassHook_New( Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/* Initialise an ClassHook */
	void ClassHook_Init( void* hook, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CLASSHOOK_DEFARGS \
                STG_OBJECT_DEFARGS, \
                Func_Ptr    funcPtr, \
                char*       addedBy, \
                void*     reference

	#define CLASSHOOK_PASSARGS \
                STG_OBJECT_PASSARGS, \
	        funcPtr,   \
	        addedBy,   \
	        reference

	ClassHook* _ClassHook_New(  CLASSHOOK_DEFARGS  );
	
	/* Initialisation implementation */
	void _ClassHook_Init( ClassHook* self, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	
	/* Stg_Class_Delete implementation */
	void _ClassHook_Delete( void* hook );
	
	/* Print implementation */
	void _ClassHook_Print( void* hook, Stream* stream );
	
	/* Copy */
	#define ClassHook_Copy( self ) \
		(ClassHook*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ClassHook_DeepCopy( self ) \
		(ClassHook*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ClassHook_Copy( void* hook, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
#endif /* __StGermain_Base_Extensibility_ClassHook_h__ */

