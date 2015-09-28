/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_ClassPtrExtensionInfo_h__
#define __StGermain_Base_Extensibility_ClassPtrExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type ClassPtrExtensionInfo_Type;

	typedef struct {
		void* ptr;
	} ClassPtrExtensionInfo_PtrClass;

	/** ClassPtrExtensionInfo struct */
	#define __ClassPtrExtensionInfo \
		__ExtensionInfo \
		Stg_Class_CopyFunction* copyFunc;
		
	/** Defines key information about an object/struct extension - see ClassPtrExtensionInfo.h. */
	struct ClassPtrExtensionInfo { __ClassPtrExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new ClassPtrExtensionInfo */
	ClassPtrExtensionInfo* ClassPtrExtensionInfo_New( const Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/** Initialise an ClassPtrExtensionInfo */
	void ClassPtrExtensionInfo_Init( void* arrayExtensionInfo, const Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CLASSPTREXTENSIONINFO_DEFARGS \
                EXTENSIONINFO_DEFARGS, \
                Stg_Class_CopyFunction*  copyFunc

	#define CLASSPTREXTENSIONINFO_PASSARGS \
                EXTENSIONINFO_PASSARGS, \
	        copyFunc

	ClassPtrExtensionInfo* _ClassPtrExtensionInfo_New(  CLASSPTREXTENSIONINFO_DEFARGS  );
	
	/* Initialisation implementation */
	void _ClassPtrExtensionInfo_Init( ClassPtrExtensionInfo* self, Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/* Class_Delete() implementation */
	void _ClassPtrExtensionInfo_Delete( void* arrayExtensionInfo );
	
	/* Class_Print() implementation */
	void _ClassPtrExtensionInfo_Print( void* arrayExtensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _ClassPtrExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	

	void* _ClassPtrExtensionInfo_DataCopy(
		void* extensionInfo,
		void* source,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap );

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __StGermain_Base_Extensibility_ClassPtrExtensionInfo_h__ */

