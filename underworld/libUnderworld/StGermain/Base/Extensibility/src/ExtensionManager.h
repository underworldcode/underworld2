/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Role:
**	Facilitates and manages the run-time extension of structures by the user.
**
** Assumptions/Invariants:
**
** Comments:
**	ExtensionManager_Get() should be called right before the extended data is used ... and in
**	particular ExtensionManager_Add() should not be called between calling ExtensionManager_Get() and
**	using an extended part of the structure.
**
**	For extensions to existing objects, they will initially be allocated right after
**	the object they are extending...but won't necessarily be if additions are made.
**
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Base_Extensibility_ExtensionManager_h__
#define __StGermain_Base_Extensibility_ExtensionManager_h__
	

	/* Textual name of this class */
	extern const Type ExtensionManager_Type;
	
	/** Extension class contents */
	#define __ExtensionManager \
		/* General info */ \
		__Stg_Object \
		\
		/* Virtual info */ \
		\
		/* Extension info */ \
		SizeT			initialSize;		/**< Records the initial size of the structure to be extended. */ \
		ExtensionInfoList*	extInfos;		/**< list of information on each extension */ \
		SizeT			finalSize;		/**< final size of extended object */ \
		Stg_Class_CopyFunction* dataCopyFunc;		/**< Function for copying the original data */ \
		HashTable*		objToExtensionMapper;	/**< Maps an obj ptr to a hashtable */ \
		\
		/* Extension for existing object variables */ \
		void*			_existingObject; 	/**< Ptr to existing object */ \
		void*			_extensionsToExisting;	/**< Ptr to extensions to existing object. */ \
		\
		/* Extension for array variable */ \
		void*			_array;			/**< Ptr to existing array */ \
		SizeT			itemSize;		/**< Size in bytes of each item in array */ \
		ExtensionManager*	em;			/**< ExtensionManager if any which extended the items in array */ \
		Index			count;			/**< Number of items in array */ \
		Bool                    lockDown;               /**< If true, all additional extension adds are firewalled */

	/** ExtensionManager class */
	struct ExtensionManager { __ExtensionManager };
	
	/** Create a new Extension manager, to manage a class (i.e. an object that isn't yet allocated). */
	ExtensionManager* ExtensionManager_New_OfStruct( 
		Name				name, 
		SizeT				initialSize );
	
	/** Create a new Extension manager for an existing, already allocated object (instantiated class). */
	ExtensionManager* ExtensionManager_New_OfExistingObject( 
		Name				name, 
		void*				_existingObject );

	ExtensionManager* ExtensionManager_New_OfArray(
		Name				name,
		void*				_array,
		SizeT				itemSize,
		Index				count );

	ExtensionManager* ExtensionManager_New_OfExtendedArray(
		Name				name,
		void*				_array,
		ExtensionManager*		em,
		Index				count );
	
	/** Initialise a Extension */
	void ExtensionManager_Init_OfStruct( 
		void*				extension,
		Name				name, 
		SizeT				initialSize );
	
	void ExtensionManager_Init_OfExistingObject( 
		void*				extension,
		Name				name, 
		void*				_existingObject );
	
	void ExtensionManager_Init_OfArray(
		void*				extension,
		Name				name,
		void*				_array,
		SizeT				itemSize,
		Index				count );

	void ExtensionManager_Init_OfExtendedArray(
		void*				extension,
		Name				name,
		void*				_array,
		ExtensionManager*		em,
		Index				count );
	
	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define EXTENSIONMANAGER_DEFARGS \
                STG_OBJECT_DEFARGS, \
                SizeT                  initialSize, \
                void*              _existingObject, \
                void*                       _array, \
                SizeT                     itemSize, \
                ExtensionManager*               em, \
                Index                        count

	#define EXTENSIONMANAGER_PASSARGS \
                STG_OBJECT_PASSARGS, \
	        initialSize,     \
	        _existingObject, \
	        _array,          \
	        itemSize,        \
	        em,              \
	        count          

	ExtensionManager* _ExtensionManager_New(  EXTENSIONMANAGER_DEFARGS  );
	
	/* Initialisation implementation */
	void _ExtensionManager_Init(
		void*				extension,
		SizeT				initialSize,
		void*				_existingObject,
		void*				_array,
		SizeT				itemSize,
		ExtensionManager*		em,
		Index				count );
	
	
	/** Stg_Class_Delete() implementation */
	void _ExtensionManager_Delete( void* extension );
	
	/** Stg_Class_Print() implementation */
	void _ExtensionManager_Print( void* extension, Stream* stream );
	
	/* Copy */
	#define ExtensionManager_Copy( self ) \
		(ExtensionManager*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ExtensionManager_DeepCopy( self ) \
		(ExtensionManager*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ExtensionManager_Copy( void* extensionManager, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/** For original data which was a class or or complex struct which requires its own copy function */
	void ExtensionManager_SetDataCopyFunc( void* extensionManager, Stg_Class_CopyFunction* dataCopyFunc );

	/** Notify the extension manager of a new extension. In the case of extensions
	to an existing object, the extension manager will allocate the new space 
	required. */
	ExtensionInfo_Index ExtensionManager_AddExtensionInfo( void* extensionManager, void* extensionInfo );

	ExtensionInfo_Index ExtensionManager_AddArray( 
		void* extension, 
		const Name offsetName, 
		SizeT size, 
		Index count );

	#define ExtensionManager_Add( extension, offsetName, size ) \
		ExtensionManager_AddArray( extension, offsetName, size, 1 )
	
	ExtensionInfo_Index ExtensionManager_AddClassPtrArray(
		void* extension, 
		const Name offsetName, 
		Stg_Class_CopyFunction* copyFunc,
		Index count );

	#define ExtensionManager_AddClassPtr( extension, offsetName, copyFunc ) \
		ExtensionManager_AddClassPtrArray( extension, offsetName, copyFunc, 1 )
	
	
	/** Get the handle of a desired particular extension... slow since uses a O(n) * strcmp algorithm. */
	ExtensionInfo_Index ExtensionManager_GetHandle( void* extension, Name offsetName );
	
	
	/** Are we extending an object? */
	#define ExtensionManager_OfExistingMacro( self ) \
		( (self)->_existingObject ? 1 : 0 )

	/** Are we extending an object? */
	Bool ExtensionManager_OfExistingFunc( void* extension );

	/** Are we extending an object? */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_OfExisting ExtensionManager_OfExistingFunc
	#else
		#define ExtensionManager_OfExisting ExtensionManager_OfExistingMacro
	#endif
	
	/** Are we extending an array?  */
	#define ExtensionManager_OfArrayMacro( self ) \
		( (self)->_array ? 1 : 0 )

	/** Are we extending an array? */
	Bool ExtensionManager_OfArrayFunc( void* extension );

	/** Are we extending an object? */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_OfArray ExtensionManager_OfArrayFunc
	#else
		#define ExtensionManager_OfArray ExtensionManager_OfArrayMacro
	#endif
	
	/** Are we extending an array of extended structs?  */
	#define ExtensionManager_OfExtendedArrayMacro( self ) \
		( (self)->em ? 1 : 0 )

	/** Are we extending an array of extended structs?  */
	Bool ExtensionManager_OfExtendedArrayFunc( void* extension );

	/** Are we extending an array of extended structs?  */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_OfExtendedArray ExtensionManager_OfExtendedArrayFunc
	#else
		#define ExtensionManager_OfExtendedArray ExtensionManager_OfExtendedArrayMacro
	#endif
	

	/** What is the final size of the class/object? */
	#define ExtensionManager_GetFinalSizeMacro( self ) \
		( (self)->finalSize )

	/** Final size of object extension / struct size / base item size */
	SizeT ExtensionManager_GetFinalSizeFunc( void* extension );

	/** Final size of object extension / struct size / base item size. Macro form only.. address of variable is often needed. */
	#define ExtensionManager_GetFinalSize ExtensionManager_GetFinalSizeMacro
	

	/** Get the pointer to the handle specified object extension. 
	 *  Faster than ExtensionManager_Get, but assumes object (not class) extension */
	#define ExtensionManager_GetOMacro( self, ptr, handle ) \
		( (void*)( (ArithPointer)(self)->_extensionsToExisting + ExtensionInfo_At( (self)->extInfos, (handle) )->offset ) )
	
	/** Get the pointer to the handle specified object extension. 
	 *  Faster than ExtensionManager_Get, but assumes object (not class) extension */
	void* ExtensionManager_GetOFunc( void* extension, void* ptr, ExtensionInfo_Index handle );

	/** Get the pointer to the handle specified object extension. 
	 *  Faster than ExtensionManager_Get, but assumes object (not class) extension */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_GetO ExtensionManager_GetOFunc
	#else
		#define ExtensionManager_GetO ExtensionManager_GetOMacro
	#endif
	
	
	/** Get the pointer to the handle specified class extension. 
	 *  Faster than ExtensionManager_Get, but assumes class (not object) extension */
	#define ExtensionManager_GetCMacro( self, ptr, handle ) \
		( (void*)( (ArithPointer)(ptr) + ExtensionInfo_At( (self)->extInfos, (handle) )->offset ) )
	
	/** Get the pointer to the handle specified class extension. 
	 *  Faster than ExtensionManager_Get, but assumes class (not object) extension */
	void* ExtensionManager_GetCFunc( void* extension, void* ptr, ExtensionInfo_Index handle );

/** Get the pointer to the handle specified class extension. 
	 *  Faster than ExtensionManager_Get, but assumes class (not object) extension */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_GetC ExtensionManager_GetCFunc
	#else
		#define ExtensionManager_GetC ExtensionManager_GetCMacro
	#endif
	
	/** Get the pointer to the handle specified array extension. 
	 *  Assumes array of non-extended items */
	#define ExtensionManager_GetAMacro( self, itemPtr, handle ) \
		( (void*)((ArithPointer)ExtensionInfo_At( (self)->extInfos, (handle) )->data + \
						( (ArithPointer)ExtensionInfo_At( (self)->extInfos, (handle) )->size * \
						(((ArithPointer)(itemPtr) - (ArithPointer)((self)->_array)) / (self)->itemSize) )))

	/** Get the pointer to the handle specified array extension. 
	 *  Assumes array of non-extended items */
	void* ExtensionManager_GetAFunc( void* extension, void* itemPtr, ExtensionInfo_Index handle );

	/** Get the pointer to the handle specified array extension. 
	 *  Assumes array of non-extended items */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_GetA ExtensionManager_GetAFunc
	#else
		#define ExtensionManager_GetA ExtensionManager_GetAMacro
	#endif
	
	
	/** Get the pointer to the handle specified array extension.
	 *  Assumes array of extended structs */
	#define ExtensionManager_GetExtendedAMacro( self, itemPtr, handle ) \
		( (handle) >= (self)->em->extInfos->count ? \
			ExtensionManager_GetA( self, itemPtr, (handle) - (self)->em->extInfos->count ) : \
			ExtensionManager_GetC( (self)->em, itemPtr, handle ) )
		
	/** Get the pointer to the handle specified array extension.
	 *  Assumes array of extended structs */
	void* ExtensionManager_GetExtendedAFunc( void* extension, void* itemPtr, ExtensionInfo_Index handle );

	/** Get the pointer to the handle specified array extension.
	 *  Assumes array of extended structs */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_GetExtendedA ExtensionManager_GetExtendedAFunc
	#else
		#define ExtensionManager_GetExtendedA ExtensionManager_GetExtendedAMacro
	#endif
	
	/** Get the pointer to the handle specified extension. Works out if extension is of object or class type. */ 
	#define ExtensionManager_GetMacro( self, ptr, handle ) \
		( ExtensionManager_OfExisting( (self) ) ? \
			ExtensionManager_GetO( self, ptr, handle ) : \
			( !ExtensionManager_OfArray( (self) ) ? \
				ExtensionManager_GetC( self, ptr, handle ) : \
				( !ExtensionManager_OfExtendedArray( (self) ) ? \
					ExtensionManager_GetA( self, ptr, handle ) : \
					ExtensionManager_GetExtendedA( self, ptr, handle ) ) ) )
	

	/** Get the pointer to the handle specified extension. Works out if extension is of object or class type. */ 
	void* ExtensionManager_GetFunc( void* extension, void* ptr, ExtensionInfo_Index handle );

	/** Get the pointer to the handle specified extension. Works out if extension is of object or class type. */ 
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_Get ExtensionManager_GetFunc
	#else
		#define ExtensionManager_Get ExtensionManager_GetMacro
	#endif

	void* ExtensionManager_HashGet( void* extension, void* ptr, Name key );
	

	/** Calculate the word-aligned version of provided data size. */
	#define ExtensionManager_AlignMacro( size ) \
		( (size) % sizeof(Stg_Word) ? ( (size)/sizeof(Stg_Word) + 1 ) * sizeof(Stg_Word) : (size) )

	
	/** Calculate the word-aligned version of provided data size. */
	SizeT ExtensionManager_AlignFunc( SizeT size );

	/** Calculate the word-aligned version of provided data size. */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_Align ExtensionManager_AlignFunc
	#else
		#define ExtensionManager_Align ExtensionManager_AlignMacro
	#endif

	
	/* The actual calculation for ExtensionManager_SizeMacro */
	#define _ExtensionManager_CalcSize( self ) \
		( ExtensionInfo_At( (self)->extInfos, (self)->extInfos->count - 1 )->offset + \
		  ExtensionInfo_At( (self)->extInfos, (self)->extInfos->count - 1 )->size )
	
	/** Get the current size of the type. That is, including the extensions added to it. */
	#define ExtensionManager_SizeMacro( self ) \
		ExtensionManager_Align( (self)->extInfos->count ? _ExtensionManager_CalcSize( (self) ) : (self)->initialSize )

	/** Get the current size of the type. That is, including the extensions added to it. */
	SizeT ExtensionManager_SizeFunc( void* extension );

	/** Get the current size of the type. That is, including the extensions added to it. */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_Size ExtensionManager_SizeFunc
	#else
		#define ExtensionManager_Size ExtensionManager_SizeMacro
	#endif


	/** Create a new instance (or array of) of the data type. Relevant to "OfStruct" extensions only (returns 0 otherwise). */
	void* ExtensionManager_Malloc( void* extension, Index count );

	/** Copy an allocation made by ExtensionManager_Malloc. Only for OfStruct! */
	void* ExtensionManager_CopyAllocation(
		void* extension,
		void* array,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap,
		Index count );
		

	/** Memset an instance (or array of) of the data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	#define ExtensionManager_MemsetMacro( self, ptr, value, cnt ) \
		( ExtensionManager_OfExisting( (self) ) ? 0 : memset( (ptr), (value), ExtensionManager_GetFinalSize( (self) ) * (cnt) ) )
		
	/** Memset an instance (or array of) of the data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	void* ExtensionManager_MemsetFunc( void* extension, void* ptr, int value, Index count );

	/** Memset an instance (or array of) of the data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_Memset ExtensionManager_MemsetFunc
	#else
		#define ExtensionManager_Memset ExtensionManager_MemsetMacro
	#endif
	
	
	/** Free data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	#define ExtensionManager_FreeMacro( self, ptr )	( ExtensionManager_OfExisting( (self) ) ? (void)0 : Memory_Free( (ptr) ) )
	
	/** Free data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	void ExtensionManager_FreeFunc( void* extension, void* ptr );

	/** Free data type. Relevant to "OfStruct" extensions only (does nothing otherwise). */
	#ifdef CAUTOUS
		#define ExtensionManager_Free ExtensionManager_FreeFunc
	#else
		#define ExtensionManager_Free ExtensionManager_FreeMacro
	#endif
	
	
	/* The actual calculation for ExtensionManager_SizeMacro */
	#define _ExtensionManager_AtCalc( self, ptr, index ) \
		( (void*)( (ArithPointer)(ptr) + (index) * (ArithPointer)( ExtensionManager_GetFinalSize( (self) ) ) ) )

	/** Given an array of an extended class, obtain the pointer to the extended class at the desired index. Relevant to
	    "OfStruct" extensions only (returns 0 otherwise). */
	#define ExtensionManager_AtMacro( self, ptr, index ) \
		( ExtensionManager_OfExisting( (self) ) ? 0 : _ExtensionManager_AtCalc( (self), (ptr), (index) ) )
	
	/** Given an array of an extended class, obtain the pointer to the extended class at the desired index. Relevant to
	    "OfStruct" extensions only (returns 0 otherwise). */
	void* ExtensionManager_AtFunc( void* extension, void* ptr, Index index );
	#ifdef MACRO_AS_FUNC
		#define ExtensionManager_At ExtensionManager_AtFunc
	#else
		#define ExtensionManager_At ExtensionManager_AtMacro
	#endif

	/* Property Set/Get for lockdown */
	void ExtensionManager_SetLockDown( void* extension, Bool lockDown );
	Bool ExtensionManager_GetLockDown( void* extension );
#endif /* __StGermain_Base_Extensibility_ExtensionManager_h__ */

