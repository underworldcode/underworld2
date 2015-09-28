/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_DofLayout_h__
#define __StgDomain_Utils_DofLayout_h__
	
	/** Textual name of this class */
	extern const Type DofLayout_Type;
	
	/** DofLayout contents */
	#define __DofLayout \
		/* General info */ \
		__Stg_Component \
		\
		DomainContext*			context; \
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		/** Variable register containing the variables referenced in this layout. */ \
		Variable_Register*	_variableRegister; \
		\
		/** The number of items that are controlled by this dof layout, hence number of entries in each set. */ \
		Index						_numItemsInLayout; \
		/** The total number of variables referred to across all the indices in the dof layout. */ \
		Index						_totalVarCount; \
		/** Array of sets, 1 per variable, which record the items in the layout that have that variable as a dof. */ \
		IndexSet**				_variableEnabledSets; \
		/** Table which maps local storage indices of variables to indices into the Variable_Register. */ \
		Variable_Index*		_varIndicesMapping; \
		\
		/** Array containing number of dofs at each index (e.g. at each node in a mesh) */ \
		Dof_Index*				dofCounts; \
		/** 2D Array: for each index (e.g. each node), stores an array (of size dofCounts[i]) containing
		the indexes into the DofLayout::_variableRegister of the Variable s at that index. */ \
		Variable_Index**		varIndices; \
		\
		Mesh*						mesh;	\
		unsigned					nBaseVariables; \
		Variable**				baseVariables;


	/** Allows the user to lay out which Variables exist at each index in a structure (eg nodes of a mesh) - see
	 DofLayout.h for details. */
	struct DofLayout { __DofLayout };
	
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	DofLayout* _DofLayout_DefaultNew( Name name );
	
	DofLayout* DofLayout_New( Name name, DomainContext* context, Variable_Register* variableRegister, Index numItemsInLayout, void* mesh );
	
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DOFLAYOUT_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define DOFLAYOUT_PASSARGS \
                STG_COMPONENT_PASSARGS

	DofLayout* _DofLayout_New(  DOFLAYOUT_DEFARGS  ); 

	void _DofLayout_Init(
		void*						dofLayout,
		DomainContext*			context,
		Variable_Register*	variableRegister,
		Index						numItemsInLayout,
		Variable_Index			baseVariableCount,
		Variable**				baseVariableArray,
		void*						mesh );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void	_DofLayout_Delete(void* dofLayout);
	
	/** Stg_Class_Print() implementation */
	void	_DofLayout_Print(void* dofLayout, Stream* stream);
	
	/* Copy */
	#define DofLayout_Copy( self ) \
		(DofLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define DofLayout_DeepCopy( self ) \
		(DofLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _DofLayout_Copy( void* dofLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Stg_Component_AssignFromXML() implementation */
	void _DofLayout_AssignFromXML( void* dofLayout, Stg_ComponentFactory* cf, void* data );
	
	/** Stg_Component_Build() implementation */
	void _DofLayout_Build( void* dofLayout, void* data );
	
	/** Stg_Component_Initialise() implementation */
	void _DofLayout_Initialise( void* dofLayout, void* data );
	
	/* Stg_Component_Execute() implementation */
	void _DofLayout_Execute( void* dofLayout, void* data );
	
	/* Stg_Component_Destroy() implementation */
	void _DofLayout_Destroy( void* dofLayout, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/** Adds a new Dof, by specifying the Variable index (into the Variable_Register) the dof is an instantiation of,
	and the index in the client's structure it applies to. An example might be (var index 0 ("vx"), node 100). */
	void	DofLayout_AddDof_ByVarIndex(void* dofLayout, Variable_Index varIndex, Index index);
	
	/** Adds a new Dof, by specifying the Variable name the dof is an instantiation of, and the index into the
	client's structure it applies to. An example might be (variable "vx", node 100).  */
	void	DofLayout_AddDof_ByVarName(void* dofLayout, Name varName, Index index);

	/** Gets a ptr to the Variable that lives at a particular index for a Dof */
	Variable* DofLayout_GetVariable(void* dofLayout, Index index, Dof_Index dofAtItemIndex );

	/** Shortcut macro to set a value of a particular dof, at a given index, without having to worry
	about the underlying variable interface. (Only have a "double" version as that's all we need so far */
	#define DofLayout_SetValueDouble( dofLayout, index, dofAtItemIndex, value ) \
		( Variable_SetValueDouble( DofLayout_GetVariable( dofLayout, index, dofAtItemIndex ), index, value ) )
	
	/** Shortcut macro to get a value of a particular dof, at a given index, without having to worry
	about the underlying variable interface. (Only have a "double" version as that's all we need so far */
	#define DofLayout_GetValueDouble( dofLayout, index, dofAtItemIndex ) \
		( Variable_GetValueDouble( DofLayout_GetVariable( dofLayout, index, dofAtItemIndex ), index ) )
	
	/** Utility function to set every dof's value to zero. */
	void DofLayout_SetAllToZero( void* dofLayout );
	
	/** Copies values from one dofLayout to another. Note this function requires the destination and
	source dofLayouts to have the same "shape". */
	void DofLayout_CopyValues( void* dofLayout, void* destDofLayout );
	
	/* Map the dofLayout to another set of indices */
	void DofLayout_Remap( void* dofLayout, Index newIndexCount, IndexMap* map );

	/** Adds each variable in this array to each item in the dof layout */
	void DofLayout_AddAllFromVariableArray( void* dofLayout, Variable_Index variableCount, Variable** variableArray ) ;

	/** Saves all variables used by this dofLayout to files */
	void DofLayout_SaveAllVariablesToFiles( void* dofLayout, char* prefixString, unsigned rank );

	/** Saves all variables used by this dofLayout to files */
	void DofLayout_LoadAllVariablesFromFiles( void* dofLayout, char* prefixString, unsigned rank );

#endif /* __StgDomain_Utils_DofLayout_h__ */

