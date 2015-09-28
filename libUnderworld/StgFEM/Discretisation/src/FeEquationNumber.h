/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**  Role:
**	This class implements the feEquationNumber feMesh to matrix functionality
**
** Assumptions:
**	feMesh partitioned by node, no shadow nodes, L2G tables built.
**	The Variable's DofLayout is indexed by local node numbers, not global.
**	(These are verified by the FiniteElement at construction.)
**
** Comments:
**	Is now (as of 12 OCtober 2004) also responsible for determining how
**	the eqNums should be decomposed between processors in the matrices and
**	vectors.
**
**	If Mesh connectivity or which nodes BCs are applied to changes,
**	this component needs to be rebuilt. TODO: provide an interface 
**	for this:- maybe using the loop we are up to to make sure it doesn't 
**	happen multiple times? A variable on the mesh and bcs could be 
**	set when they change connectivity/BC dofs.
**
**	Qtn to verify- maybe the exchange of locally known set end
**	CPs is pointless? they only turn out to be the interfaces in
**	non-shared, non-shadowing cases i.e. pressure mesh with no
**	shadowing activated. However, I'm pretty sure I worked out
**	they were necessary in some cases to ensure all the procs
**	got matching values in the end.
**
**	TODO: add a test case with all BCs on one of the processors:
**	Just in case it causes problems with the lowestLocalEqNum 
**	calculations.
**
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StgFEM_Discretisation_FeEquationNumber_h__
#define __StgFEM_Discretisation_FeEquationNumber_h__
	#include <mpi.h>
	
	typedef void (FeEquationNumber_BuildFunction) ( void* feEquationNumber, void *data );
	typedef void (FeEquationNumber_InitialiseFunction)	( void* feEquationNumber, void *data );
	
	
	extern const Type FeEquationNumber_Type;

	extern MPI_Datatype MPI_critPointInfoType;

	typedef struct RemappedNodeInfo {
		Node_RemappedGlobalIndex	remappedGlobal;
		Node_DomainIndex				domain;
	} RemappedNodeInfo;
	
	/** FeEquationNumber class */
	#define __FeEquationNumber \
		/* General info */ \
		__Stg_Component \
		\
		DomainContext*				context; \
		/* FeEquationNumber info */ \
		Stream*						debug; \
		/** Stream for debugging LM. (usually only need I.D.) */ \
		Stream*						debugLM; \
		Stream*						warning; \
		/** attached feMesh */ \
		FeMesh*						feMesh; \
		/** DofLayout describing the discretisation of an FeVariable over the mesh */ \
		DofLayout*					dofLayout; \
		/** LinkedEquationInfo - information on which dofs are linked together */ \
		LinkedDofInfo*				linkedDofInfo; \
		/** BCs applied to this mesh */ \
		VariableCondition*		bcs; \
		/** (possibly remapped for efficiency) table of domain&global node indexes */ \
		RemappedNodeInfo*			remappedNodeInfos; \
		/** map of (domain node, nodeLocalDof) -> global eq num */ \
		Dof_EquationNumber**		destinationArray; \
		/** Used to calculate globalSumUnconstrainedDofs */ \
		Dof_EquationNumber		_highestLocalEqNum;\
		/** Used to determine which procs hold which numbers */ \
		Dof_EquationNumber		_lowestLocalEqNum;\
		Dof_EquationNumber*		_lowestGlobalEqNums;\
		unsigned int				_eqNumsPerProcDivisor; \
		unsigned int				_eqNumsRemainder; \
		Dof_EquationNumber		_remNotAddedChangeover; \
		STreeMap*					ownedMap; \
		Dof_EquationNumber		firstOwnedEqNum; \
		Dof_EquationNumber		lastOwnedEqNum; \
		Dof_EquationNumber		localEqNumsOwnedCount; \
		/** number of unconstrained dofs */ \
		unsigned int				globalSumUnconstrainedDofs; \
		/** map of (domain element, elementLocalNode,  nodeLocalDof) -> global eq. num */ \
		Dof_EquationNumber***	locationMatrix; \
		/** Records whether someone has tried to build the eqNum table yet */ \
		/** Bool to make sure LM only gets built once */ \
		Bool							locationMatrixBuilt; \
		/** Bool to determine whether remapping is activated */ \
		Bool							remappingActivated; \
		\
		/** Some vars to deal with removing BCs from mats and vecs. */ \
		Bool							removeBCs; \
                Bool    periodic[3];                                                    \
		STree*						bcEqNums;  \
		/** nDomainEls is required during the destroy phase */ \
		Index                   nDomainEls;

	struct FeEquationNumber { __FeEquationNumber };


	
	/* +++ Constructors / Destructors +++ */
	
	/** Create a feEquationNumber feMesh */
	void* FeEquationNumber_DefaultNew( Name name );
	
	FeEquationNumber* FeEquationNumber_New( 
		Name						name,
		DomainContext*			context,
		void*						feMesh,
		DofLayout*				dofLayout,
		VariableCondition*	bcs,
		LinkedDofInfo*			linkedDofInfo );
	
	/** Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FEEQUATIONNUMBER_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define FEEQUATIONNUMBER_PASSARGS \
                STG_COMPONENT_PASSARGS

	FeEquationNumber* _FeEquationNumber_New(  FEEQUATIONNUMBER_DEFARGS  );
	
	/** Initialisation implementation functions */
	void _FeEquationNumber_Init(
		FeEquationNumber*		self, 
		DomainContext*			context,
		void*						feMesh,
		DofLayout*				dofLayout,
		VariableCondition*	bcs,
		LinkedDofInfo*			linkedDofInfo );
	
	
	/** Stg_Class_Delete implementation: frees the destination array, and the 
	global location Matrix (if it exists) */
	void _FeEquationNumber_Delete( void* feEquationNumber );
	
	/* +++ Virtual Function Interfaces & Implementations +++ */
	
	/** Print implementation */
	void _FeEquationNumber_Print( void* feEquationNumber, Stream* stream );
	
	void _FeEquationNumber_AssignFromXML( void* feEquationNumber, Stg_ComponentFactory *cf, void* data );
	
	void _FeEquationNumber_Execute( void* feEquationNumber, void *data );
	
	void _FeEquationNumber_Destroy( void* feEquationNumber, void *data );
	
	/* Copy */
	#define FeEquationNumber_Copy( self ) \
		(FeEquationNumber*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define FeEquationNumber_DeepCopy( self ) \
		(FeEquationNumber*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _FeEquationNumber_Copy( void* feEquationNumber, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/** The "Build" phase for the equation number information. Calculates the 
	domain and global Dof and unconstrained Dof totals, and builds the
	destination array. *Doesn't* build the full Location Matrix - this is 
	optional as some users prefer to only calculate this element-by-element as
	required.
	*/
	
	/** Build the FeEquationNumber. \see FeEquationNumber_Build() */
	void _FeEquationNumber_Build( void* feEquationNumber, void* data );
	
	/** Initialise implementation. Currently does nothing. */
	void _FeEquationNumber_Initialise( void* feEquationNumber, void* data );
	
	/* +++ Public Functions +++ */
	
	/** Sets up MPI datatype handle for exchanging arrays of CritPointInfo 
	structs efficiently. Must be called in initialisation stage 
	(\see FiniteElement_Init() ). 
	void FeEquationNumber_Create_CritPointInfo_MPI_Datatype( void ); */

	/* Next few lines are a function, in case the alg. for calculating boundaries changes once we start
	trying ghosting again */
	Partition_Index FeEquationNumber_CalculateOwningProcessorOfEqNum( void* self, Dof_EquationNumber eqNum );

	/** build the processor's location matrix mapping elements, element node, dof -> eq num */
	void FeEquationNumber_BuildLocationMatrix( void* feEquationNumber );
	
	/** Build an element's local location matrix */
	Dof_EquationNumber** FeEquationNumber_BuildOneElementLocationMatrix( void* feEquationNumber, Element_LocalIndex lElement_I );
	
	/** Calculates the total number of active (i.e. Non BC) dofs at a given node */
	Index FeEquationNumber_CalculateActiveEqCountAtNode(
		void*						feEquationNumber,
		Node_DomainIndex		dNode_I,
		Dof_EquationNumber*	lowestActiveEqNumAtNodePtr );
	
	/** Prints only the destination array */
	void FeEquationNumber_PrintDestinationArray( void* feEquationNumber, Stream* stream );
	void FeEquationNumber_PrintDestinationArrayBox( void* feFeEquationNumber, Stream* stream ) ;

	/** Prints only the location matrix */
	void FeEquationNumber_PrintLocationMatrix( void* feEquationNumber, Stream* stream );
	
	/* +++ Private Functions +++ */
	
	#if 0
	/** Calculates the total number of unconstrained (ie no Variable Condition
	being applied) degrees of freedom. Must be called *after* the Destination
	Array is built as it grabs values from it directly. */
	void _FeEquationNumber_CalculateGlobalUnconstrainedDofTotal( FeEquationNumber* self );

	/** Calculate the minimum and maximum parts that my processor is responsible for holding.
	When remapping is active, the decomposition will be optimally aligned with the mesh. */

	void _FeEquationNumber_CalculateEqNumsDecomposition( FeEquationNumber* self );

	/** Build the processor's ID (destination) array. Has been designed to
	handle parallel irregular meshes with any decomposition. The algorithm
	is fully parallel, and communication has been minimised to 2 sets
	of global communications. */
	void _FeEquationNumber_BuildDestinationArray( FeEquationNumber* self );
	#endif

	void FeEquationNumber_BuildWithTopology( FeEquationNumber* self );

	/* void FeEquationNumber_BuildRegular( FeEquationNumber* self ); */

	void FeEquationNumber_BuildWithDave( FeEquationNumber* self );

#endif /* __StgFEM_Discretisation_FeEquationNumber_h__ */

