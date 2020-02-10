/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Decomp_Sync_Claim_h__
#define __StgDomain_Mesh_Decomp_Sync_Claim_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Claim_Type;

	/** Virtual function types */
	typedef void (Decomp_Sync_Claim_SelectFunc)( void* claim, Decomp_Sync* sync, unsigned nRequired, unsigned* required, 
						     unsigned* nLocals, unsigned** locals );

	/** Mesh class contents */
	#define __Decomp_Sync_Claim				\
		/* General info */				\
		__Stg_Component					\
								\
		AbstractContext*		context;	\
		/* Virtual info */				\
		Decomp_Sync_Claim_SelectFunc*	selectFunc;	\
								\
		/* Decomp_Sync_Claim info */

	struct Decomp_Sync_Claim { __Decomp_Sync_Claim };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_CLAIM_DEFARGS			\
		STG_COMPONENT_DEFARGS,				\
		Decomp_Sync_Claim_SelectFunc*	selectFunc

	#define DECOMP_SYNC_CLAIM_PASSARGS		\
		STG_COMPONENT_PASSARGS, selectFunc

	Decomp_Sync_Claim* Decomp_Sync_Claim_New( Name name );
	Decomp_Sync_Claim* _Decomp_Sync_Claim_New( DECOMP_SYNC_CLAIM_DEFARGS );
	void _Decomp_Sync_Claim_Init( Decomp_Sync_Claim* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Claim_Delete( void* claim );
	void _Decomp_Sync_Claim_Print( void* claim, Stream* stream );

	#define Decomp_Sync_Claim_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_Sync_Claim_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Sync_Claim_Copy( void* claim, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Sync_Claim_AssignFromXML( void* claim, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Sync_Claim_Build( void* claim, void* data );
	void _Decomp_Sync_Claim_Initialise( void* claim, void* data );
	void _Decomp_Sync_Claim_Execute( void* claim, void* data );
	void _Decomp_Sync_Claim_Destroy( void* claim, void* data );

	void Decomp_Sync_Claim_Select( void* claim, Decomp_Sync* sync, unsigned nRequired, unsigned* required, 
				       unsigned* nLocals, unsigned** locals );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_Claim_ClaimOwnership( Decomp_Sync_Claim* self, CommTopology* topo, unsigned nRequired, unsigned* required, 
					       RangeSet* lSet, RangeSet** isects, 
					       unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes );
	void Decomp_Sync_Claim_BuildIndices( Decomp_Sync_Claim* self, unsigned nRequired, unsigned* required, RangeSet* claimed, 
					     unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes );

#endif /* __StgDomain_Mesh_Decomp_Sync_Claim_h__ */
