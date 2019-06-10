/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Decomp_Sync_Array_h__
#define __StgDomain_Mesh_Decomp_Sync_Array_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Array_Type;

	/** Virtual function types */

	/** Mesh class contents */
	#define __Decomp_Sync_Array		\
		/* General info */		\
		__Stg_Class			\
						\
		/* Virtual info */		\
						\
		/* Decomp_Sync_Array info */	\
		Decomp_Sync*	sync;		\
						\
		void*		snkArray;	\
		unsigned	snkStride;	\
		unsigned*	snkDisps;	\
		unsigned*	snkSizes;	\
		unsigned*	snkOffs;	\
						\
		void*		srcArray;	\
		unsigned	srcStride;	\
		unsigned*	srcDisps;	\
		unsigned*	srcSizes;	\
		unsigned*	srcOffs;	\
						\
		size_t		itemSize;

	struct Decomp_Sync_Array { __Decomp_Sync_Array };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_ARRAY_DEFARGS	\
		STG_CLASS_DEFARGS

	#define DECOMP_SYNC_ARRAY_PASSARGS	\
		STG_CLASS_PASSARGS

	Decomp_Sync_Array* Decomp_Sync_Array_New();
	Decomp_Sync_Array* _Decomp_Sync_Array_New( DECOMP_SYNC_ARRAY_DEFARGS );
	void _Decomp_Sync_Array_Init( Decomp_Sync_Array* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Array_Delete( void* array );
	void _Decomp_Sync_Array_Print( void* array, Stream* stream );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Decomp_Sync_Array_SetSync( void* array, Decomp_Sync* sync );
	void Decomp_Sync_Array_SetMemory( void* array, 
					  void* localArray, void* remoteArray, 
					  size_t localStride, size_t remoteStride, 
					  size_t itemSize );
	void Decomp_Sync_Array_Sync( void* array );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_Array_BuildArray( Decomp_Sync_Array* self );
	void Decomp_Sync_Array_Destruct( Decomp_Sync_Array* self );

#endif /* __StgDomain_Mesh_Decomp_Sync_Array_h__ */
