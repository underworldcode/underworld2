/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_LinkedDofInfo_h__
#define __StgFEM_Discretisation_LinkedDofInfo_h__

	extern const Type LinkedDofInfo_Type;
	
	#define __LinkedDofInfo \
		__Stg_Component \
		\
		DomainContext*	context; \
		Dictionary*		dictionary; \
		/** A mesh that this indo is based around */ \
		Mesh*				mesh; \
		/** A DofLayout that this info is based around */ \
		DofLayout*		dofLayout; \
		/** an array of ints, specifying for each local dof whether it is linked, and if so where to */ \
		int**				linkedDofTbl; \
		/** count of how many linked dof sets are active */ \
		Index				linkedDofSetsCount; \
		Index				linkedDofSetsSize; \
		Index				linkedDofSetsDelta; \
		/** For each linked dof set, records the eq num they all map to */ \
		int*				eqNumsOfLinkedDofs; \

	struct LinkedDofInfo { __LinkedDofInfo };



	/* +++ Constructors / Destructors +++ */
	
	/** Create a linkedDofInfo */
	void* _LinkedDofInfo_DefaultNew( Name name );
	
	LinkedDofInfo* LinkedDofInfo_New( 
		Name				name,
		DomainContext*	context,
		void*				mesh,
		DofLayout*		dofLayout,
		Dictionary*		dictionary );
	
	/** Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LINKEDDOFINFO_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define LINKEDDOFINFO_PASSARGS \
                STG_COMPONENT_PASSARGS

	LinkedDofInfo* _LinkedDofInfo_New(  LINKEDDOFINFO_DEFARGS  );
	
	/** Initialisation implementation functions */
	void _LinkedDofInfo_Init( void* linkedDofInfo, DomainContext* context, void* mesh, DofLayout* dofLayout, Dictionary* dictionary );
	
	/** Stg_Class_Delete implementation. */
	void _LinkedDofInfo_Delete( void* linkedDofInfo );
	
	/* +++ Virtual Function Interfaces & Implementations +++ */
	
	/** Print implementation */
	void _LinkedDofInfo_Print( void* linkedDofInfo, Stream* stream );
	
	void _LinkedDofInfo_AssignFromXML( void* linkedDofInfo, Stg_ComponentFactory *cf, void* data );
	
	void _LinkedDofInfo_Execute( void* linkedDofInfo, void *data );
	
	void _LinkedDofInfo_Destroy( void* linkedDofInfo, void *data );

	/* Copy */
	#define LinkedDofInfo_Copy( self ) \
		(LinkedDofInfo*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define LinkedDofInfo_DeepCopy( self ) \
		(LinkedDofInfo*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _LinkedDofInfo_Copy( void* linkedDofInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _LinkedDofInfo_Build( void* linkedDofInfo, void* data );

	void _LinkedDofInfo_Initialise( void* linkedDofInfo, void* data );

	Index LinkedDofInfo_AddDofSet( void* linkedDofInfo );
	
	void LinkedDofInfo_AddDofToSet( void* linkedDofInfo, Index linkedDofSet_I, Node_Index node_I, Dof_Index nodeLocalDof_I );
	
	void LinkedDofInfo_AddDofsToSet_FromIndexSet( void* linkedDofInfo, Index linkedDofSet_I, IndexSet* nodeSet, Dof_Index nodeLocalDof_I );
		
#endif

