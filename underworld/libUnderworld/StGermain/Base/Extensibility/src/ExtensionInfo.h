/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_ExtensionInfo_h__
#define __StGermain_Base_Extensibility_ExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type ExtensionInfo_Type;

	typedef void* (ExtensionInfo_DataCopyFunction)( 
		void* extensionInfo, 
		void* source, 
		void* dest, 
		Bool deep, 
		Name nameExt,
		PtrMap* ptrMap );
	
	/** ExtensionInfo struct- stores a func pointer plus info */
	#define __ExtensionInfo \
		__Stg_Object \
		ExtensionInfo_DataCopyFunction*	_dataCopy;		/**< Virtual func which copies the data of extension. */ \
		Name				key;			/**< Pointer to const key */ \
		SizeT				originalSize;		/**< How big the extension is (as provided). */\
		SizeT				size;			/**< How big the extension is (word aligned). */\
		Bool				isRegistered;		/**< Is this ExtensionInfo registered with a manager. */\
		SizeT				itemSize;		/**< If array, the size of each item. */\
		Index				count;			/**< If array, the number of items. */\
		SizeT				offset;			/**< The extension's offset in memory. */\
		ExtensionManager*		extensionManager;	/**< The extension manager that this is part of. */\
		ExtensionInfo_Index		handle;			/**< The extension handle in the manager. */ \
		void*				data;			/**< If extending extended array, the data of extension.*/
		
	/** Defines key information about an object/struct extension - see ExtensionInfo.h. */
	struct ExtensionInfo { __ExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define EXTENSIONINFO_DEFARGS \
                STG_OBJECT_DEFARGS, \
                ExtensionInfo_DataCopyFunction*  _dataCopy, \
                SizeT                                 size, \
                Index                                count

	#define EXTENSIONINFO_PASSARGS \
                STG_OBJECT_PASSARGS, \
	        _dataCopy, \
	        size,      \
	        count    

	ExtensionInfo* _ExtensionInfo_New(  EXTENSIONINFO_DEFARGS  );
	
	/* Initialisation implementation */
	void _ExtensionInfo_Init( ExtensionInfo* self, Name name, SizeT size, Index count );
	
	/** Class_Delete() implementation: derivatives should call this in their implementation */
	void _ExtensionInfo_Delete( void* extensionInfo );
	
	/** Class_Print() implementation: derivatives should call this in their implementation */
	void _ExtensionInfo_Print( void* extensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _ExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/** Copyies the extended dats from source to dest */
	void* ExtensionInfo_DataCopy( void* extensionInfo, void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* Obtain the original (provided) size of the extension */
	#define ExtensionInfo_OriginalSizeMacro( self ) \
		( (self)->originalSize )
	SizeT ExtensionInfo_OriginalSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the original (provided) size of the extension */
		#define ExtensionInfo_OriginalSize ExtensionInfo_OriginalSizeFunc
	#else
		/** Obtain the original (provided) size of the extension */
		#define ExtensionInfo_OriginalSize ExtensionInfo_OriginalSizeMacro
	#endif
	
	/* Obtain the size (word alignment compliant) of the extension */
	#define ExtensionInfo_SizeMacro( self ) \
		( (self)->size )
	SizeT ExtensionInfo_SizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the size (word alignment compliant) of the extension */
		#define ExtensionInfo_Size ExtensionInfo_SizeFunc
	#else
		/** Obtain the size (word alignment compliant) of the extension */
		#define ExtensionInfo_Size ExtensionInfo_SizeMacro
	#endif
	
	/* Obtain the size per item for extension which are arrays */
	#define ExtensionInfo_ItemSizeMacro( self ) \
		( (self)->itemSize )
	SizeT ExtensionInfo_ItemSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the size per item for extension which are arrays */
		#define ExtensionInfo_ItemSize ExtensionInfo_ItemSizeFunc
	#else
		/** Obtain the size per item for extension which are arrays */
		#define ExtensionInfo_ItemSize ExtensionInfo_ItemSizeMacro
	#endif
	
	/* Obtain the count of items for extension which are arrays */
	#define ExtensionInfo_CountMacro( self ) \
		( (self)->count )
	SizeT ExtensionInfo_ItemSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the count of items for extension which are arrays */
		#define ExtensionInfo_Count ExtensionInfo_CountFunc
	#else
		/** Obtain the count of items for extension which are arrays */
		#define ExtensionInfo_Count ExtensionInfo_CountMacro
	#endif
	
	/* Obtain the offset of the extension */
	#define ExtensionInfo_OffsetMacro( self ) \
		( (self)->offset )
	SizeT ExtensionInfo_OffsetFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the offset of the extension */
		#define ExtensionInfo_Offset ExtensionInfo_OffsetFunc
	#else
		/** Obtain the offset of the extension */
		#define ExtensionInfo_Offset ExtensionInfo_OffsetMacro
	#endif
	
	/* Has this extension info been registered with a manager? This object is useless until this occours. */
	#define ExtensionInfo_IsRegisteredMacro( self ) \
		( (self)->isRegistered )
	Bool ExtensionInfo_IsRegisteredFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Has this extension info been registered with a manager? This object is useless until this occours. */
		#define ExtensionInfo_IsRegistered ExtensionInfo_IsRegisteredFunc
	#else
		/** Has this extension info been registered with a manager? This object is useless until this occours. */
		#define ExtensionInfo_IsRegistered ExtensionInfo_IsRegisteredMacro
	#endif
	
	
	/* This function is a friend of ExtensionManager. It is called by ithe manager to register its info with this extension.
	   Registration is done by the manager on an Add. An ExtensionInfo is useless without being registered. */
	void ExtensionInfo_Register( 
		void*				extensionInfo,
		SizeT				offset,
		ExtensionManager*		extensionManager,
		ExtensionInfo_Index		handle,
		void*				data );
	
#endif /* __StGermain_Base_Extensibility_ExtensionInfo_h__ */

