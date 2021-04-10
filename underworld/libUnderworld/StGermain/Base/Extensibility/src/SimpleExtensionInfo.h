/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_SimpleExtensionInfo_h__
#define __StGermain_Base_Extensibility_SimpleExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type SimpleExtensionInfo_Type;
	
	/** SimpleExtensionInfo struct */
	#define __SimpleExtensionInfo \
		__ExtensionInfo
		
	/** Defines key information about an object/struct extension - see SimpleExtensionInfo.h. */
	struct SimpleExtensionInfo { __SimpleExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new SimpleExtensionInfo */
	SimpleExtensionInfo* SimpleExtensionInfo_New( const Name name, SizeT size, Index count );
	
	/** Initialise an SimpleExtensionInfo */
	void SimpleExtensionInfo_Init( void* simpleExtensionInfo, const Name name, SizeT size, Index count );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SIMPLEEXTENSIONINFO_DEFARGS \
                EXTENSIONINFO_DEFARGS

	#define SIMPLEEXTENSIONINFO_PASSARGS \
                EXTENSIONINFO_PASSARGS

	SimpleExtensionInfo* _SimpleExtensionInfo_New(  SIMPLEEXTENSIONINFO_DEFARGS  );
	
	/* Initialisation implementation */
	void _SimpleExtensionInfo_Init( SimpleExtensionInfo* self, Name name, SizeT size, Index count );
	
	/* Class_Delete() implementation */
	void _SimpleExtensionInfo_Delete( void* simpleExtensionInfo );
	
	/* Class_Print() implementation */
	void _SimpleExtensionInfo_Print( void* simpleExtensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _SimpleExtensionInfo_Copy( void* simpleExtensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _SimpleExtensionInfo_DataCopy( 
		void* extensionInfo, 
		void* source, 
		void* dest, 
		Bool deep, 
		Name nameExt, 
		PtrMap* ptrMap );

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __StGermain_Base_Extensibility_SimpleExtensionInfo_h__ */

