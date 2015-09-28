/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"
#include "DomainContext.h"
#include "DofLayout.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


const Type DofLayout_Type = "DofLayout";


/*--------------------------------------------------------------------------------------------------------------------------
** Private function declarations
*/

/** Add a new Variable to the group that may be used as dofs, where varIndex is the index of the variable
in the variable register. */
Dof_Index	_DofLayout_AddVariable_ByIndex(void* dofLayout, Variable_Index varIndex);

/** Add a new Variable to the group that may be used as dofs - by name. */
Dof_Index	_DofLayout_AddVariable_ByName(void* dofLayout, Name varName);


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

DofLayout* DofLayout_New( Name name, DomainContext* context, Variable_Register* variableRegister, Index numItemsInLayout, void* mesh ) {
	DofLayout* self = _DofLayout_DefaultNew( name );

	self->isConstructed = True;
	_DofLayout_Init( self, context, variableRegister, numItemsInLayout, 0, NULL, mesh );

	return self;
}

DofLayout* _DofLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(DofLayout);
	Type                                                      type = DofLayout_Type;
	Stg_Class_DeleteFunction*                              _delete = _DofLayout_Delete;
	Stg_Class_PrintFunction*                                _print = _DofLayout_Print;
	Stg_Class_CopyFunction*                                  _copy = _DofLayout_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_DofLayout_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _DofLayout_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _DofLayout_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _DofLayout_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _DofLayout_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _DofLayout_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return _DofLayout_New(  DOFLAYOUT_PASSARGS  );
}

DofLayout* _DofLayout_New(  DOFLAYOUT_DEFARGS  ) {
	DofLayout* self;
	
	/* Allocate memory/General info */
	assert( _sizeOfSelf >= sizeof(DofLayout) );
	self = (DofLayout*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* Virtual info */
	self->_build = _build;
	self->_initialise = _initialise;
	
	/* Stg_Class info */
	
	return self;
}

void _DofLayout_Init(
	void*						dofLayout,
	DomainContext*			context,
	Variable_Register*	variableRegister, 
	Index						numItemsInLayout,
	Variable_Index			baseVariableCount,
	Variable**				baseVariableArray,
	void*						_mesh )
{
	DofLayout*	self = (DofLayout*)dofLayout;
    self->context = context;
	self->mesh = (Mesh*)_mesh;
	
    if ( variableRegister )
        self->_variableRegister = variableRegister;
    else
        self->_variableRegister = Variable_Register_New();

	self->_numItemsInLayout = numItemsInLayout;
	self->_variableEnabledSets = NULL;
	self->_totalVarCount = 0;
	self->_varIndicesMapping = NULL;
	
	self->dofCounts = NULL;

	self->nBaseVariables = baseVariableCount;
	self->baseVariables = baseVariableArray;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _DofLayout_Delete(void* dofLayout) {
	DofLayout* self = (DofLayout*)dofLayout;
	
    /* we create our own var register if no context, so must delete */
    if (!self->context)
        Stg_Class_Delete(self->_variableRegister);
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _DofLayout_Print(void* dofLayout, Stream* stream) {
	DofLayout*	self = (DofLayout*)dofLayout;
	
	/* Set the Journal for printing informations */
	Stream* compositeVCStream = stream;
	
	/* General info */
	Journal_Printf( compositeVCStream, "DofLayout (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	
	/* Print parent */
	_Stg_Component_Print( self, compositeVCStream );
}


void* _DofLayout_Copy( void* dofLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	DofLayout*	self = (DofLayout*)dofLayout;
	DofLayout*	newDofLayout;
	PtrMap*		map = ptrMap;
	Bool			ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newDofLayout = (DofLayout*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	newDofLayout->_variableRegister = self->_variableRegister;
	newDofLayout->_numItemsInLayout = self->_numItemsInLayout;
	newDofLayout->_totalVarCount = self->_totalVarCount;
	
	if( deep ) {
		if( (newDofLayout->_variableEnabledSets = PtrMap_Find( map, self->_variableEnabledSets )) == NULL && self->_variableEnabledSets ) {
			Index	set_I;
			
			newDofLayout->_variableEnabledSets = Memory_Alloc_Array( IndexSet*, newDofLayout->_totalVarCount, "DofLayout->_variableEnabledSets" );
			for( set_I = 0; set_I < newDofLayout->_totalVarCount; set_I++ ) {
				newDofLayout->_variableEnabledSets[set_I] = (IndexSet*)Stg_Class_Copy( self->_variableEnabledSets[set_I], NULL, deep, nameExt, map );
			}
			PtrMap_Append( map, self->_variableEnabledSets, newDofLayout->_variableEnabledSets );
		}
		
		if( (newDofLayout->_varIndicesMapping = PtrMap_Find( map, self->_varIndicesMapping )) == NULL && self->_varIndicesMapping ) {
			newDofLayout->_varIndicesMapping = Memory_Alloc_Array( Variable_Index, newDofLayout->_totalVarCount, "DofLayout->_varIndicesMapping" );
			memcpy( newDofLayout->_varIndicesMapping, self->_varIndicesMapping, sizeof(Variable_Index) * newDofLayout->_totalVarCount );
			PtrMap_Append( map, self->_varIndicesMapping, newDofLayout->_varIndicesMapping );
		}
		
		if( (newDofLayout->dofCounts = PtrMap_Find( map, self->dofCounts )) == NULL && self->dofCounts ) {
			newDofLayout->dofCounts = Memory_Alloc_Array( Index, newDofLayout->_numItemsInLayout, "DofLayout->dofCounts" );
			memcpy( newDofLayout->dofCounts, self->dofCounts, sizeof(Index) * newDofLayout->_numItemsInLayout );
			PtrMap_Append( map, self->dofCounts, newDofLayout->dofCounts );
		}
		
		if( (newDofLayout->varIndices = PtrMap_Find( map, self->varIndices )) == NULL && self->varIndices ) {
			Index	idx_I;
			
			newDofLayout->varIndices = Memory_Alloc_2DComplex( Variable_Index, newDofLayout->_numItemsInLayout, self->dofCounts, "DofLayout->varIndices" );
			for( idx_I = 0; idx_I < newDofLayout->_numItemsInLayout; idx_I++ ) {
				memcpy( newDofLayout->varIndices[idx_I], self->varIndices[idx_I], sizeof(Variable_Index) * newDofLayout->dofCounts[idx_I] );
			}
			PtrMap_Append( map, self->varIndices, newDofLayout->varIndices );
		}
	}
	else {
		newDofLayout->_variableEnabledSets = self->_variableEnabledSets;
		newDofLayout->_varIndicesMapping = self->_varIndicesMapping;
		newDofLayout->dofCounts = self->dofCounts;
		newDofLayout->varIndices = self->varIndices;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newDofLayout;
}

void _DofLayout_AssignFromXML( void* dofLayout, Stg_ComponentFactory* cf, void* data ) {
   DofLayout *             self = (DofLayout*)dofLayout;
   Dictionary*             thisComponentDict = NULL;
   void*                   variableRegister = NULL;
   Dictionary_Entry_Value* list;
   Variable_Index          baseVariableCount = 0;
   Variable**              baseVariableList = NULL;
   Mesh*							mesh;
   DomainContext*				context=NULL;
   MeshVariable*           meshVar=NULL;
   int v_i;

   /* Get component's dictionary setup */
   assert( cf->componentDict );
   thisComponentDict = Dictionary_GetDictionary( cf->componentDict, self->name );
   assert( thisComponentDict );

   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", DomainContext, False, data );
   if( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, False, data  );
    
   if ( context )
       variableRegister = context->variable_Register;
   else
       variableRegister = Variable_Register_New();


   meshVar = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"MeshVariable", MeshVariable, False, data );

   /* DofLayout XML definitions can have 2 styles: new clean way or old dirty
    * way.
    * The clean way is via a MeshVariable dependency.
    * The old way, left in for backwards compatibility relies on a mesh and be
    * BaseVariables
    */
   if( meshVar ) {
      /* new way */
      mesh = meshVar->mesh;
      baseVariableCount = meshVar->subVariablesCount;
      baseVariableList = Memory_Alloc_Array( Variable*, baseVariableCount, "baseVariableList" );

      /* setup up baseVariableCount & baseVariableList from the meshVar 
       * and give those data structure to the init function */
      if( baseVariableCount == 1 ){
         baseVariableList[0] = meshVar;
         Variable_Register_Add(variableRegister, baseVariableList[0]);
      } else {
         for ( v_i = 0 ; v_i < baseVariableCount ; v_i++ ) {
             baseVariableList[v_i] = meshVar->components[v_i];
             Variable_Register_Add(variableRegister, baseVariableList[v_i]);
         }
      }
   } else {
      /* old way */
      /* Get the mesh. */
      mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"mesh", Mesh, True, data );

      if (( list = Dictionary_Get( thisComponentDict, (Dictionary_Entry_Key)"BaseVariables" ) ) ) {
         Variable_Index	baseVariable_I    = 0;
         Name				variableName;

         baseVariableCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"BaseVariableCount", Dictionary_Entry_Value_GetCount( list ) );
         Journal_Firewall(
            baseVariableCount <= Dictionary_Entry_Value_GetCount( list  ),
            Journal_Register( Error_Type, (Name)self->type  ),
            "BaseVariableCount %u is too large for list given.\n", baseVariableCount );

         baseVariableList = Memory_Alloc_Array( Variable*, baseVariableCount, "baseVariableList" );
         
         for ( baseVariable_I = 0 ; baseVariable_I < baseVariableCount ; baseVariable_I++ ) {
            variableName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( list, baseVariable_I ) );

            Journal_PrintfL( cf->infoStream, 2, "Looking for Variable '%s' in Variable_Register\n", variableName );
            baseVariableList[ baseVariable_I ] = Variable_Register_GetByName( variableRegister, variableName );

            if ( !baseVariableList[ baseVariable_I ] )
               baseVariableList[ baseVariable_I ] = Stg_ComponentFactory_ConstructByName( cf, (Name)variableName, Variable, True, data  );
         }
      }
   }

   _DofLayout_Init( self, context, variableRegister, 0, baseVariableCount, baseVariableList, mesh );
}

void _DofLayout_Build( void* dofLayout, void* data ) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Index			indexCount;
	Index*		indices;
	Index			set_I, i, pos;

	assert( self );

	/* Build mesh and extract domain size, if required. */
	if( self->mesh ) {
		Stg_Component_Build( self->mesh, data, False );
		self->_numItemsInLayout = Mesh_GetDomainSize( self->mesh, MT_VERTEX );
	}

	/* Adds each variable in this list as a base degree of freedom to each item in DofLayout */
	DofLayout_AddAllFromVariableArray( self, self->nBaseVariables, self->baseVariables );

	/* ensure variables are built */
	for( i = 0; i < self->_totalVarCount; i++ )
		Stg_Component_Build( Variable_Register_GetByIndex( self->_variableRegister, self->_varIndicesMapping[i] ), data, False );
	
	self->dofCounts = Memory_Alloc_Array( Index, self->_numItemsInLayout, "DofLayout->dofCounts" );
	memset(self->dofCounts, 0, sizeof(Index)*self->_numItemsInLayout);
	
	for (set_I = 0; set_I < self->_totalVarCount; set_I++) {
		IndexSet_GetMembers(self->_variableEnabledSets[set_I], &indexCount, &indices);

		for (i = 0; i < indexCount; i++) {
			self->dofCounts[indices[i]]++;
		}
			
		if (indices)
			Memory_Free(indices);
	}
	
	self->varIndices = Memory_Alloc_2DComplex( Variable_Index, self->_numItemsInLayout, self->dofCounts, "DofLayout->varIndices" );
	for (i = 0; i < self->_numItemsInLayout; i++) {
		pos = 0;

		for (set_I = 0; set_I < self->_totalVarCount; set_I++) {
			if (IndexSet_IsMember(self->_variableEnabledSets[set_I], i))
				self->varIndices[i][pos++] = self->_varIndicesMapping[set_I];
		}
	}
}


void _DofLayout_Initialise( void* dofLayout, void* data ) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Index			var_I;

	/* Initialise all the Variables used - in some cases they don't allocate themselves properly until
		this is done */
	for( var_I = 0; var_I < self->_totalVarCount; var_I++ ) {
		Stg_Component_Initialise( Variable_Register_GetByIndex( self->_variableRegister, self->_varIndicesMapping[var_I] ),
			data, False );
	}	
}


void _DofLayout_Execute( void* dofLayout, void* data ) {
}

void _DofLayout_Destroy( void* dofLayout, void* data ) {
	DofLayout*	self = (DofLayout*)dofLayout;

	if( self->baseVariables )
		Memory_Free( self->baseVariables );

	if( self->dofCounts )
		Memory_Free( self->dofCounts );

   if( self->_varIndicesMapping ) Memory_Free( self->_varIndicesMapping );

	if( self->varIndices )
		Memory_Free( self->varIndices );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Private functions
*/

Dof_Index _DofLayout_AddVariable_ByIndex(void* dofLayout, Variable_Index varIndex) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Dof_Index	dof_I;
	
	for (dof_I = 0; dof_I < self->_totalVarCount; dof_I++)
		if (varIndex == self->_varIndicesMapping[dof_I])
			return dof_I;
	
	self->_totalVarCount++;
	
	/* Do an Alloc if array does not exist to register stats in memory module. Other times, just Realloc */
	if ( self->_varIndicesMapping ) {
		self->_varIndicesMapping = Memory_Realloc_Array( self->_varIndicesMapping, Variable_Index, self->_totalVarCount );
	}
	else {
		self->_varIndicesMapping = Memory_Alloc_Array( Variable_Index, self->_totalVarCount,
			"DofLayout->_varIndicesMapping" );
	}
	self->_varIndicesMapping[self->_totalVarCount - 1] = varIndex;

	if ( self->_variableEnabledSets ) {
		self->_variableEnabledSets = Memory_Realloc_Array( self->_variableEnabledSets, IndexSet*, self->_totalVarCount );
	}
	else {
		self->_variableEnabledSets = Memory_Alloc_Array( IndexSet*, self->_totalVarCount,
			"DofLayout->_variableEnabledSets" );
	}
	self->_variableEnabledSets[self->_totalVarCount - 1] = IndexSet_New(self->_numItemsInLayout);
	
	return self->_totalVarCount - 1;
}


Dof_Index _DofLayout_AddVariable_ByName(void* dofLayout, Name varName) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Dof_Index	dof_I;
	
	dof_I = Variable_Register_GetIndex( self->_variableRegister, varName );
	Journal_Firewall( 
		dof_I != (unsigned)-1,
		Journal_Register( Error_Type, (Name)DofLayout_Type  ),
		"Attempting to name a variable as a DOF that is not in the variable registry!\n" );
	return _DofLayout_AddVariable_ByIndex( self, dof_I );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public functions
*/

void DofLayout_AddDof_ByVarIndex(void* dofLayout, Variable_Index varIndex, Index index) {
	DofLayout*	self = (DofLayout*)dofLayout;
	
	IndexSet_Add(self->_variableEnabledSets[varIndex], index);
}


void DofLayout_AddDof_ByVarName(void* dofLayout, Name varName, Index index) {
	DofLayout*	self = (DofLayout*)dofLayout;
	
	DofLayout_AddDof_ByVarIndex(self, _DofLayout_AddVariable_ByName(self, varName), index);
}

Variable* DofLayout_GetVariable(void* dofLayout, Index index, Dof_Index dofAtItemIndex ) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Variable_Index	var_I = self->varIndices[index][dofAtItemIndex];
	
	return Variable_Register_GetByIndex( self->_variableRegister, var_I );
}


void DofLayout_CopyValues( void* dofLayout, void* destDofLayout ) {
	DofLayout*    self = (DofLayout*)dofLayout;
	DofLayout*    dest = (DofLayout*)destDofLayout;
	Index         ii = 0;
	Dof_Index     dof_I = 0;
	Stream*       error = Journal_Register( Error_Type, (Name)self->type );
	
	Journal_Firewall( (self->_numItemsInLayout == dest->_numItemsInLayout ), error,
		"Error: Number of items in source dof layout (%d) not equal to number of "
		"items in destination dof layout (%d).\n",
		self->_numItemsInLayout, dest->_numItemsInLayout );

	for ( ii=0; ii < self->_numItemsInLayout; ii++ ) {
		#ifdef CAUTIOUS
		Journal_Firewall( (self->dofCounts[ii] == dest->dofCounts[ii] ), error,
			"Error: Number of dofs in source dof layout (item %d, %d dofs) not equal to "
			"number of dofs in destination dof layout at same entry (%d).\n",
			self->_numItemsInLayout, self->dofCounts[ii], dest->dofCounts[ii] );
		#endif	

		for ( dof_I = 0; dof_I < self->dofCounts[ii]; dof_I++ ) {
			DofLayout_SetValueDouble( dest, ii, dof_I,
				DofLayout_GetValueDouble( self, ii, dof_I ) );
		}
	}
}


void DofLayout_SetAllToZero( void* dofLayout ) {
	DofLayout*    self = (DofLayout*)dofLayout;
	Index         ii = 0;
	Dof_Index     dof_I = 0;
	
	for ( ii=0; ii < self->_numItemsInLayout; ii++ ) {

		for ( dof_I = 0; dof_I < self->dofCounts[ii]; dof_I++ ) {
			DofLayout_SetValueDouble( self, ii, dof_I, 0 );
		}
	}
}


void DofLayout_Remap( void* dofLayout, Index newIndexCount, IndexMap* map ) {
	DofLayout*	self = (DofLayout*)dofLayout;
	Index		item_I;
	
	assert( newIndexCount && map ); /* TODO: change this to Journal commands */
	
	/*
	** NOTE: it is possible that a remapping will eliminate some of the variables accessed
	**	 by the DofLayout.  This will require a bit more effort to account for, but for
	**	 the moment it shouldn't matter.
	*/
	
	if( self->_variableEnabledSets ) {
		IndexSet**	variableEnabledSets;
		Index		set_I;
		
		variableEnabledSets = Memory_Alloc_Array( IndexSet*, self->_totalVarCount, "DofLayout->_variableEnabledSets" );
		for( set_I = 0; set_I < self->_totalVarCount; set_I++ ) {
			Index	itemCount;
			Index*	items;
			
			variableEnabledSets[set_I] = IndexSet_New( newIndexCount );
			
			IndexSet_GetMembers( self->_variableEnabledSets[set_I], &itemCount, &items );
			for( item_I = 0; item_I < itemCount; item_I++ ) {
				Index	mappedValue = IndexMap_Find( map, items[item_I] );
				
				if( mappedValue < self->_numItemsInLayout ) {
					IndexSet_Add( variableEnabledSets[set_I], mappedValue );
				}
			}
			
			Memory_Free( items );
			Stg_Class_Delete( self->_variableEnabledSets[set_I] );
		}
		Memory_Free( self->_variableEnabledSets );
		self->_variableEnabledSets = variableEnabledSets;
	}
	
	/* Map dofCounts */
	if( self->dofCounts ) {
		Dof_Index*	dofCounts;
		
		dofCounts = Memory_Alloc_Array( Dof_Index, newIndexCount, "DofLayout->dofCounts" );
		for( item_I = 0; item_I < self->_numItemsInLayout; item_I++ ) {
			Index	mappedValue = IndexMap_Find( map, item_I );
			
			if( mappedValue < self->_numItemsInLayout ) {
				dofCounts[mappedValue] = self->dofCounts[item_I];
			}
		}
		Memory_Free( self->dofCounts );
		self->dofCounts = dofCounts;
	}
	
	/* Map varIndices */
	if( self->varIndices ) {
		Variable_Index**	varIndices;
		
		varIndices = Memory_Alloc_2DComplex( Variable_Index, newIndexCount, self->dofCounts, "DofLayout->varIndices" );
		for( item_I = 0; item_I < self->_numItemsInLayout; item_I++ ) {
			Index	mappedValue = IndexMap_Find( map, item_I );
			
			if( mappedValue < self->_numItemsInLayout ) {
				memcpy( varIndices[mappedValue], 
					self->varIndices[item_I], 
					sizeof(Variable_Index) * self->dofCounts[mappedValue] );
			}
		}
		Memory_Free( self->varIndices );
		self->varIndices = varIndices;
	}
	
	/* Overwrite old values */
	self->_numItemsInLayout = newIndexCount;
}

void DofLayout_AddAllFromVariableArray( void* dofLayout, Variable_Index variableCount, Variable** variableArray ) {
	DofLayout*      self        = (DofLayout*) dofLayout;
	Index           item_I;
	Index           itemCount   = self->_numItemsInLayout;
	Variable_Index  variable_I;
	Stream*         errorStream = Journal_Register( Error_Type, (Name)self->type );

	for( variable_I = 0; variable_I < variableCount ; variable_I++  ) {
		Journal_Firewall( variableArray[variable_I] != NULL, errorStream,
				"In func %s for %s '%s' - Variable %d in array is NULL.\n", __func__, self->type, self->name, variable_I);
		for( item_I = 0; item_I < itemCount ; item_I++ ) {
			DofLayout_AddDof_ByVarName( self, variableArray[variable_I]->name, item_I );
		}
	}
}


void DofLayout_SaveAllVariablesToFiles( void* dofLayout, char* prefixString, unsigned rank ) {
	DofLayout*  self = (DofLayout*) dofLayout;
	Index       ownedVar_I, var_I;
	Variable*   variable = NULL;
	char*       varFileName;
	int         prefixStringLength=0;

	for ( ownedVar_I = 0; ownedVar_I < self->_totalVarCount; ownedVar_I++ ) {
		var_I = self->_varIndicesMapping[ownedVar_I];
		variable = Variable_Register_GetByIndex( self->_variableRegister, var_I );

		if ( prefixString ) {
			prefixStringLength = strlen(prefixString);
		}
		else {
			prefixStringLength = 0;
		}

		varFileName = Memory_Alloc_Array_Unnamed( char, prefixStringLength + strlen( variable->name ) + 5 + 4 + 1 );
		if ( prefixString ) {
			sprintf( varFileName, "%s.%s.dat.%.4d", prefixString, variable->name, rank );
		}
		else {
			sprintf( varFileName, "%s.dat.%.4d", variable->name, rank );
		}
		
		Variable_SaveToFileAsDoubles( variable, varFileName );
		Memory_Free( varFileName );
	}
}


void DofLayout_LoadAllVariablesFromFiles( void* dofLayout, char* prefixString, unsigned rank ) {
	DofLayout*  self = (DofLayout*) dofLayout;
	Index       ownedVar_I, var_I;
	Variable*   variable = NULL;
	char*       varFileName;
	int         prefixStringLength=0;

	for ( ownedVar_I = 0; ownedVar_I < self->_totalVarCount; ownedVar_I++ ) {
		var_I = self->_varIndicesMapping[ownedVar_I];
		variable = Variable_Register_GetByIndex( self->_variableRegister, var_I );

		if ( prefixString ) {
			prefixStringLength = strlen(prefixString);
		}
		else {
			prefixStringLength = 0;
		}

		varFileName = Memory_Alloc_Array_Unnamed( char, prefixStringLength + strlen( variable->name ) + 6 + 4 + 1 );
		if ( prefixString ) {
			sprintf( varFileName, "%s.%s.dat.%.4d", prefixString, variable->name, rank );
		}
		else {
			sprintf( varFileName, "%s.dat.%.4d", variable->name, rank );
		}
		
		Variable_ReadFromFileAsDoubles( variable, varFileName );
		Memory_Free( varFileName );
	}
}



