/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Decomposer_h__
#define __StgDomain_Mesh_Decomposer_h__

	/** Textual name of this class */
	extern const Type	Decomposer_Type;

	/** Virtual function types */
	typedef void (Decomposer_DecomposeFunc)( void* decomposer, unsigned nDomains, unsigned* domains, 
						 CommTopology** commTopology, Decomp** decomp, Decomp_Sync** sync );

	/** Class contents */
	#define __Decomposer					\
		/* General info */				\
		__Stg_Class					\
								\
		/* Virtual info */				\
		Decomposer_DecomposeFunc*	decomposeFunc;	\
								\
		/* Decomposer info */				\
		MPI_Comm		comm;

	struct Decomposer { __Decomposer };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMPOSER_DEFARGS				\
		STG_CLASS_DEFARGS,				\
		Decomposer_DecomposeFunc*	decomposeFunc

	#define DECOMPOSER_PASSARGS			\
		STG_CLASS_PASSARGS, decomposeFunc

	Decomposer* Decomposer_New();
	Decomposer* _Decomposer_New( DECOMPOSER_DEFARGS );
	void _Decomposer_Init( Decomposer* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomposer_Delete( void* decomposer );
	void _Decomposer_Print( void* decomposer, Stream* stream );

	void _Decomposer_Decompose( void* decomposer, unsigned nDomains, unsigned* domains, 
				    CommTopology** commTopology, Decomp** decomp, Decomp_Sync** sync );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Decomposer_SetComm( void* decomposer, MPI_Comm comm );

	#define Decomposer_Decompose( decomposer, nDomains, domains, commTopo, decomp, sync )				\
		(assert( (decomposer) && ((Decomposer*)decomposer)->decomposeFunc ),					\
		 ((Decomposer*)decomposer)->decomposeFunc( decomposer, nDomains, domains, commTopo, decomp, sync ))

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomposer_BuildCommTopology( Decomposer* self, unsigned nDomains, unsigned* domains, 
					   CommTopology** commTopo, RangeSet*** isects );
	void Decomposer_BuildLocalIntersections( Decomposer* self, unsigned nDomains, unsigned* domains, 
						 CommTopology* commTopo, RangeSet*** isects );
	void Decomposer_Claim( Decomposer* self, CommTopology* topo, RangeSet** isects, 
			       unsigned nDomains, unsigned* domains, 
			       Decomp** decomp, Decomp_Sync** sync );
	void Decomposer_BuildIndices( Decomposer* self, unsigned nDomains, unsigned* domains, RangeSet* claimed, 
				      CommTopology* commTopo, Decomp** decomp, Decomp_Sync** sync );

#endif /* __StgDomain_Mesh_Decomposer_h__ */
