/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_UIntMap_h__
#define __StGermain_Base_Container_UIntMap_h__

	/** Textual name of this class */
	extern const Type UIntMap_Type;

	/** Virtual function types */

	/** Mesh class contents */
	#define __UIntMap				\
		/* General info */			\
		__Stg_Class				\
							\
		/* Virtual info */			\
							\
		/* UIntMap info */			\
		BTree*		btree;			\
		size_t		size;

	struct UIntMap { __UIntMap };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define UINTMAP_DEFARGS \
                STG_CLASS_DEFARGS

	#define UINTMAP_PASSARGS \
                STG_CLASS_PASSARGS

	UIntMap* UIntMap_New();
	UIntMap* _UIntMap_New(  UINTMAP_DEFARGS  );
	void _UIntMap_Init( UIntMap* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _UIntMap_Delete( void* map );
	void _UIntMap_Print( void* map, Stream* stream );

	#define UIntMap_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define UIntMap_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _UIntMap_Copy( void* map, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void UIntMap_Insert( void* map, unsigned key, unsigned value );
	void UIntMap_Remove( void* map, unsigned key );
	void UIntMap_Clear( void* map );
	Bool UIntMap_Map( void* map, unsigned key, unsigned* value );
	void UIntMap_GetItems( void* map, unsigned* nItems, unsigned** keys, unsigned** values );
	unsigned UIntMap_GetSize( void* map );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	int UIntMap_DataCompare( void* left, void* right );
	void UIntMap_DataCopy( void** dstData, void* data, SizeT size );
	void UIntMap_DataDelete( void* data );
	void UIntMap_ParseNode( void* data, void* _parseStruct );

#endif /* __StGermain_Base_Container_UIntMap_h__ */

