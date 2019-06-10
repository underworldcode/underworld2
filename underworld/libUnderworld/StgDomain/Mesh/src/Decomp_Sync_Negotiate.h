/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Decomp_Sync_Negotiate_h__
#define __StgDomain_Mesh_Decomp_Sync_Negotiate_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Negotiate_Type;

	/** Virtual function types */
	typedef void (Decomp_Sync_Negotiate_SelectFunc)( void* negotiate, Decomp_Sync* decomp );

	/** Mesh class contents */
	#define __Decomp_Sync_Negotiate					\
		/* General info */					\
		__Stg_Component						\
									\
		AbstractContext*			context;	\
		/* Virtual info */					\
		Decomp_Sync_Negotiate_SelectFunc*	selectFunc;	\
									\
		/* Decomp_Sync_Negotiate info */

	struct Decomp_Sync_Negotiate { __Decomp_Sync_Negotiate };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_NEGOTIATE_DEFARGS				\
		STG_COMPONENT_DEFARGS,					\
		Decomp_Sync_Negotiate_SelectFunc*	selectFunc

	#define DECOMP_SYNC_NEGOTIATE_PASSARGS		\
		STG_COMPONENT_PASSARGS, selectFunc

	Decomp_Sync_Negotiate* Decomp_Sync_Negotiate_New( Name name );
	Decomp_Sync_Negotiate* _Decomp_Sync_Negotiate_New( DECOMP_SYNC_NEGOTIATE_DEFARGS );
	void _Decomp_Sync_Negotiate_Init( Decomp_Sync_Negotiate* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Negotiate_Delete( void* negotiate );
	void _Decomp_Sync_Negotiate_Print( void* negotiate, Stream* stream );

	#define Decomp_Sync_Negotiate_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_Sync_Negotiate_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Sync_Negotiate_Copy( void* negotiate, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Sync_Negotiate_AssignFromXML( void* negotiate, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Sync_Negotiate_Build( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Initialise( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Execute( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Destroy( void* negotiate, void* data );

	void Decomp_Sync_Negotiate_Select( void* negotiate, Decomp_Sync* decomp );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_Negotiate_RemoteSearch( Decomp_Sync_Negotiate* self, Decomp_Sync* decomp, 
						 unsigned** nRemFound, unsigned*** remFound );

#endif /* __StgDomain_Mesh_Decomp_Sync_Negotiate_h__ */
