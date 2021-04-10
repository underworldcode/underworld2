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
#include <StgDomain/StgDomain.h>
#include "types.h"

#include "LinkedDofInfo.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/** Textual name of this class */
const Type LinkedDofInfo_Type = "LinkedDofInfo";

LinkedDofInfo* LinkedDofInfo_New( Name name, DomainContext* context, void* mesh, DofLayout* dofLayout, Dictionary* dictionary ) {
	LinkedDofInfo* self = _LinkedDofInfo_DefaultNew( name );

	self->isConstructed = True;	
	_LinkedDofInfo_Init( self, context, mesh, dofLayout, dictionary );

	return self;
}

void* _LinkedDofInfo_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(LinkedDofInfo);
	Type                                                      type = LinkedDofInfo_Type;
	Stg_Class_DeleteFunction*                              _delete = _LinkedDofInfo_Delete;
	Stg_Class_PrintFunction*                                _print = _LinkedDofInfo_Print;
	Stg_Class_CopyFunction*                                  _copy = _LinkedDofInfo_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _LinkedDofInfo_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _LinkedDofInfo_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _LinkedDofInfo_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _LinkedDofInfo_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _LinkedDofInfo_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _LinkedDofInfo_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return (void*)
		_LinkedDofInfo_New(  LINKEDDOFINFO_PASSARGS  );
}

LinkedDofInfo* _LinkedDofInfo_New(  LINKEDDOFINFO_DEFARGS  ) {
	LinkedDofInfo* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LinkedDofInfo) );
	self = (LinkedDofInfo*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	self->_build = _build;
	self->_initialise = _initialise;
	
	/* member info */
	
	return self;
}
	
void _LinkedDofInfo_Init( void* linkedDofInfo, DomainContext* context, void* mesh, DofLayout* dofLayout, Dictionary* dictionary ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;

	self->context = context;
	self->dofLayout = dofLayout;
	self->mesh = (Mesh*)mesh;
	self->dictionary = dictionary;

	/* Initially no sets */
	self->linkedDofSetsCount = 0;
	self->linkedDofSetsDelta = 4;
	self->linkedDofSetsSize = self->linkedDofSetsDelta;
	self->eqNumsOfLinkedDofs = Memory_Alloc_Array( int, self->linkedDofSetsSize, "linkedDofInfo->eqNumsOfLinkedDofs" );
}

void _LinkedDofInfo_Delete( void* linkedDofInfo ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;

	_Stg_Component_Delete( self );
}

void _LinkedDofInfo_Print( void* linkedDofInfo, Stream* stream ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	Index				dofSet_I = 0;
	Node_Index		node_I = 0;
	Dof_Index		nodeLocalDof_I = 0;
	
	/* General info */
	Journal_Printf( stream, "LinkedDofInfo (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );

	Journal_Printf( stream, "%d Linked dof sets active:\n", self->linkedDofSetsCount );
	Stream_Indent( stream );

	for ( dofSet_I = 0; dofSet_I < self->linkedDofSetsCount; dofSet_I++ ) {
		Journal_Printf( stream, "Set %u: has eqNum %d\n", dofSet_I, self->eqNumsOfLinkedDofs[dofSet_I] );
	}
	Stream_UnIndent( stream );

	Journal_Printf( stream, "Linked dof sets table:\n", self->linkedDofSetsCount );
	Stream_Indent( stream );

	for ( node_I = 0; node_I < self->dofLayout->_numItemsInLayout; node_I++ ) {
		Journal_Printf( stream, "Node %d: ", node_I );
		for ( nodeLocalDof_I = 0; nodeLocalDof_I < self->dofLayout->dofCounts[node_I]; nodeLocalDof_I++ ) {
			Journal_Printf( stream, "%d, ", self->linkedDofTbl[node_I][nodeLocalDof_I] );
		}
		Journal_Printf( stream, "\n" );
	}
	Stream_UnIndent( stream );
}

void _LinkedDofInfo_AssignFromXML( void* linkedDofInfo, Stg_ComponentFactory *cf, void* data ){
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	Dictionary*		dictionary;
	Mesh*				mesh = NULL;
	DofLayout*		dofLayout = NULL;
	DomainContext*	context;

	dictionary = Dictionary_GetDictionary( cf->componentDict, self->name );
	
	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", DomainContext, False, data );
	if( !context  ) 
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, True, data  );

	mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Mesh", Mesh, True, data  );
	dofLayout = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)DofLayout_Type, DofLayout, True, data  );

	_LinkedDofInfo_Init( self, context, mesh, dofLayout, dictionary ); 
}
	
void _LinkedDofInfo_Execute( void* linkedDofInfo, void *data ){
}
	
void _LinkedDofInfo_Destroy( void* linkedDofInfo, void *data ){
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	
	Memory_Free( self->linkedDofTbl );
	Memory_Free( self->eqNumsOfLinkedDofs );
}

void* _LinkedDofInfo_Copy( void* linkedDofInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	LinkedDofInfo*	newLinkedDofInfo;
	PtrMap*			map = ptrMap;
	Bool				ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newLinkedDofInfo = _Stg_Class_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newLinkedDofInfo->linkedDofSetsCount = self->linkedDofSetsCount;
	newLinkedDofInfo->linkedDofSetsSize = self->linkedDofSetsSize;
	newLinkedDofInfo->linkedDofSetsDelta = self->linkedDofSetsDelta;
	
	if ( deep ) {
		newLinkedDofInfo->dofLayout = (DofLayout*)Stg_Class_Copy( self->dofLayout, NULL, deep, nameExt, map );
		newLinkedDofInfo->mesh = (Mesh*)Stg_Class_Copy( self->mesh, NULL, deep, nameExt, map );

		if ( (newLinkedDofInfo->linkedDofTbl = PtrMap_Find( map, self->linkedDofTbl )) == NULL && self->linkedDofTbl ) {
			Node_Index	node_I;
			Dof_Index	dof_I;
			newLinkedDofInfo->linkedDofTbl = Memory_Alloc_2DComplex( int, self->dofLayout->_numItemsInLayout, self->dofLayout->dofCounts, "linkedDofInfo->linkedDofTbl" );
			for ( node_I = 0; node_I < self->dofLayout->_numItemsInLayout; node_I++ ) {
				for ( dof_I = 0; dof_I < self->dofLayout->dofCounts[node_I]; dof_I++ ) {
					newLinkedDofInfo->linkedDofTbl[node_I][dof_I] =
						self->linkedDofTbl[node_I][dof_I];
				}
			}
			PtrMap_Append( map, self->linkedDofTbl, newLinkedDofInfo->linkedDofTbl );
		}
		if ( (newLinkedDofInfo->eqNumsOfLinkedDofs = PtrMap_Find( map, self->eqNumsOfLinkedDofs )) == NULL && self->eqNumsOfLinkedDofs ) {	
			newLinkedDofInfo->eqNumsOfLinkedDofs = Memory_Alloc_Array( int, self->linkedDofSetsCount, "linkedDofInfo->eqNumsOfLinkedDofs" );
			memcpy( newLinkedDofInfo->eqNumsOfLinkedDofs, self->eqNumsOfLinkedDofs, self->linkedDofSetsCount * sizeof(int) );
			PtrMap_Append( map, self->eqNumsOfLinkedDofs, newLinkedDofInfo->eqNumsOfLinkedDofs );
		}
	}
	else {
		newLinkedDofInfo->dofLayout = self->dofLayout;
		newLinkedDofInfo->mesh = self->mesh;
		newLinkedDofInfo->linkedDofTbl = self->linkedDofTbl;
		newLinkedDofInfo->eqNumsOfLinkedDofs = self->eqNumsOfLinkedDofs;
	}

	if( ownMap ) {
		Stg_Class_Delete( map );
	}

	return (void*)newLinkedDofInfo;
}

void _LinkedDofInfo_Build( void* linkedDofInfo, void* data ) {
	LinkedDofInfo*				self = (LinkedDofInfo*)linkedDofInfo;
	Dictionary_Entry_Value*	linkSpecs = NULL;
	Node_Index					node_I;
	Node_Index					nodeLocalDof_I;
	DofLayout*					dofLayout;

	dofLayout = self->dofLayout;
	Stg_Component_Build( self->dofLayout, data, False );
	
	self->linkedDofTbl = Memory_Alloc_2DComplex( int, dofLayout->_numItemsInLayout,
		dofLayout->dofCounts, "linkedDofInfo->linkedDofTbl" );

	/* Initially, set all dofs as not belonging to a set */
	for ( node_I = 0 ; node_I < dofLayout->_numItemsInLayout; node_I++ ) {
		for ( nodeLocalDof_I = 0; nodeLocalDof_I < dofLayout->dofCounts[node_I]; nodeLocalDof_I++ ) {
			self->linkedDofTbl[node_I][nodeLocalDof_I] = -1;
		}
	}
	
	linkSpecs = Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"linkSpecifications" );

	/* Note that the dictionary entry is optional - user may prefer to specify the sets
	using the AddDofToSet and AddDofsToSet_FromIndexSet functions */
	
	if ( linkSpecs ) {
		Index							numSpecs = 0;
		Index							linkSpec_I;
		Dictionary_Entry_Value*	linkSpec = NULL;
		Dictionary*					linkSpecDict;
		Index							dofSet = 0;
		Dictionary_Entry_Value*	wallVal;
		Dictionary_Entry_Value*	shapeVal;
		Dictionary_Entry_Value*	wallPairVal;
		Index							nodalDof;

		numSpecs = Dictionary_Entry_Value_GetCount( linkSpecs );
		
		for ( linkSpec_I = 0; linkSpec_I < numSpecs; linkSpec_I++  ) {
			linkSpec = Dictionary_Entry_Value_GetElement( linkSpecs, linkSpec_I );
			linkSpecDict = Dictionary_Entry_Value_AsDictionary( linkSpec );
			nodalDof = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"dof" ) );
			
			if ( nodalDof >= self->dofLayout->dofCounts[0]  ) {
				Stream*  warningStr = Journal_Register( Error_Type, (Name)self->type  );

				Journal_DPrintf( warningStr, "Warning- in %s: User requested a periodic BC on "
					"dof %d, but only %d dofs are active. Ignoring.\n", __func__, nodalDof,
					self->dofLayout->dofCounts[0] );
				
				continue;
			}
			
			if ( ( wallVal = Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"wall" ) ) ) {
				IndexSet*	indexSet = NULL;
				char*		wallStr = Dictionary_Entry_Value_AsString( wallVal  );
				
				dofSet = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"dofSet" )  );
				
				if (      !strcasecmp(wallStr,"left")   || !strcasecmp(wallStr,"MinI") ) {
					indexSet = RegularMeshUtils_CreateGlobalMinISet( self->mesh );
				}
				else if ( !strcasecmp(wallStr,"right")  || !strcasecmp(wallStr,"MaxI") ) {
					indexSet = RegularMeshUtils_CreateGlobalMaxISet( self->mesh );
				}
				else if ( !strcasecmp(wallStr,"bottom") || !strcasecmp(wallStr,"MinJ") ) {
					indexSet = RegularMeshUtils_CreateGlobalMinJSet( self->mesh );
				}
				else if ( !strcasecmp(wallStr,"top")    || !strcasecmp(wallStr,"MaxJ") ) {
					indexSet = RegularMeshUtils_CreateGlobalMaxJSet( self->mesh );
				}
				else if ( !strcasecmp(wallStr,"front")  || !strcasecmp(wallStr,"MaxK") ) {
					indexSet = RegularMeshUtils_CreateGlobalMaxKSet( self->mesh );
				}
				else if ( !strcasecmp(wallStr,"back")   || !strcasecmp(wallStr,"MinK") ) {
					indexSet = RegularMeshUtils_CreateGlobalMinKSet( self->mesh );
				}
				else {
					Stream*	errorStr = Journal_Register( Error_Type, (Name)self->type  );
					Journal_Printf( errorStr, "Error in %s: unknown wall string \"%s\" given. "
						"Exiting.\n", __func__, wallStr );
					exit(EXIT_FAILURE);
				}

				while ( dofSet >= self->linkedDofSetsCount ) { 
					LinkedDofInfo_AddDofSet( linkedDofInfo );
				}
				
				LinkedDofInfo_AddDofsToSet_FromIndexSet( self, dofSet, indexSet, nodalDof );
			}
			else if ( ( shapeVal = Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"shape" ) ) ) {
				char*             shapeStr = Dictionary_Entry_Value_AsString( shapeVal );
				Stg_Shape*        shape = NULL;
				Node_LocalIndex   lNode_I = 0;
				AbstractContext*  context = (AbstractContext* ) data;
				Stream*           errorStr = Journal_Register( Error_Type, (Name)self->type  );

				Journal_Firewall( context && Stg_Class_IsInstance( context, AbstractContext_Type ),
					errorStr, "Error - in %s(): you've asked to create a Shape linked dof "
					"specification, but haven't passed in the context as the \"data\" parameter "
					"to %s - hence the shape can't be found in the component factory.\n",
					__func__, __func__ );

				Journal_Firewall( context->CF != NULL, errorStr,
					"Error - in %s(): you've asked to create a Shape linked dof "
					"specification, but the context passed as the \"data\" parameter "
					"doesn't have a valid component factory to find the shape in.\n",
					__func__, __func__ );

				dofSet = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"dofSet" ) );
				while ( dofSet >= self->linkedDofSetsCount ) { 
					LinkedDofInfo_AddDofSet( linkedDofInfo  );
				}
				
				shape = Stg_ComponentFactory_ConstructByName( context->CF, (Name)shapeStr, Stg_Shape, True, data  );
				
				for ( lNode_I = 0; lNode_I < Mesh_GetLocalSize( self->mesh, MT_VERTEX ); lNode_I++ ) {
					if ( Stg_Shape_IsCoordInside( shape, Mesh_GetVertex( self->mesh, lNode_I ) ) ) {
						LinkedDofInfo_AddDofToSet( self, dofSet, lNode_I, nodalDof );
					}
				}	
			}
			else if ( ( wallPairVal = Dictionary_Get( linkSpecDict, (Dictionary_Entry_Key)"wallPair" ) ) ) {
				char*		wallPairStr = Dictionary_Entry_Value_AsString( wallPairVal  );
				Index		dofSetStart_I = self->linkedDofSetsCount;
				Index		dofSet_I = 0;
				Index		numWallNodes;
				Index		wallSetNode_I = 0;
				Index		firstSetNode_I = 0;
				Index		secondSetNode_I = 0;
				IndexSet*	firstSet = NULL;
				IndexSet*	secondSet = NULL;
				Index		numExtraDofSetsNeeded = 0;
				Index		globallyKnownExtraDofSetsNeeded = 0;

				if (      !strcasecmp(wallPairStr,"left-right") || !strcasecmp(wallPairStr,"I_AXIS") ) {
					firstSet = RegularMeshUtils_CreateGlobalMinISet( self->mesh );
					secondSet = RegularMeshUtils_CreateGlobalMaxISet( self->mesh );
				}	
				else if ( !strcasecmp(wallPairStr,"back-front") || !strcasecmp(wallPairStr,"K_AXIS") ) {
					firstSet = RegularMeshUtils_CreateGlobalMinKSet( self->mesh );
					secondSet = RegularMeshUtils_CreateGlobalMaxKSet( self->mesh );
				}
				else if ( !strcasecmp(wallPairStr,"bottom-top") || !strcasecmp(wallPairStr,"J_AXIS") ) {
					firstSet = RegularMeshUtils_CreateGlobalMinJSet( self->mesh );
					secondSet = RegularMeshUtils_CreateGlobalMaxJSet( self->mesh );
				}
				else {
					Stream*	errorStr = Journal_Register( Error_Type, (Name)self->type  );
					Journal_Printf( errorStr, "Error in %s: unknown wall pair string \"%s\" given. "
						"Exiting.\n", __func__, wallPairStr );
					exit(EXIT_FAILURE);
				}
		
				numExtraDofSetsNeeded = IndexSet_UpdateMembersCount( firstSet );
				numWallNodes = IndexSet_UpdateMembersCount( secondSet );
				if ( numWallNodes > numExtraDofSetsNeeded ) {
					numExtraDofSetsNeeded = numWallNodes;
				}	
				/* In parallel, there may be several processors in the middle who aren't holding the
				 * walls, but need to know they exist. Hence use an MPI_Allreduce to get the count */
				(void)MPI_Allreduce( &numExtraDofSetsNeeded, &globallyKnownExtraDofSetsNeeded, 1, MPI_UNSIGNED,
					       MPI_MAX, Mesh_GetCommTopology( self->mesh, MT_VERTEX )->mpiComm );

				for( dofSet_I = 0; dofSet_I < globallyKnownExtraDofSetsNeeded; dofSet_I++ ) {	
					LinkedDofInfo_AddDofSet( linkedDofInfo );
				}	

				/* The below for loops are 2 separate ones, since in the parallel case we don't
				know that the current proc contains both walls */
				numWallNodes = IndexSet_UpdateMembersCount( firstSet );
				dofSet_I = dofSetStart_I;
				for ( wallSetNode_I = 0; wallSetNode_I < numWallNodes; wallSetNode_I++ ) {
					firstSetNode_I = IndexSet_GetIndexOfNthMember( firstSet, wallSetNode_I );
					LinkedDofInfo_AddDofToSet( linkedDofInfo, dofSet_I, firstSetNode_I, nodalDof );
					dofSet_I++;
				}

				numWallNodes = IndexSet_UpdateMembersCount( secondSet );
				dofSet_I = dofSetStart_I;
				for ( wallSetNode_I = 0; wallSetNode_I < numWallNodes; wallSetNode_I++ ) {
					secondSetNode_I = IndexSet_GetIndexOfNthMember( secondSet, wallSetNode_I );
					LinkedDofInfo_AddDofToSet( linkedDofInfo, dofSet_I, secondSetNode_I, nodalDof );
					dofSet_I++;
				}
			}
		}
	}
}

void _LinkedDofInfo_Initialise( void* linkedDofInfo, void* data ) {
}

Index LinkedDofInfo_AddDofSet( void* linkedDofInfo ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	Index				indexIntoArray;

	if ( self->linkedDofSetsCount == self->linkedDofSetsSize ) {
		self->linkedDofSetsSize += self->linkedDofSetsDelta;
		self->eqNumsOfLinkedDofs = Memory_Realloc_Array( self->eqNumsOfLinkedDofs, int, self->linkedDofSetsSize );
	}
	/* Set the eq num to -1 since uninitialised */
	self->eqNumsOfLinkedDofs[self->linkedDofSetsCount] = -1;
	indexIntoArray = self->linkedDofSetsCount++;

	return indexIntoArray;
}

void LinkedDofInfo_AddDofToSet( void* linkedDofInfo, Index linkedDofSet_I, Node_Index node_I, Dof_Index nodeLocalDof_I ) {
	LinkedDofInfo*	self = (LinkedDofInfo*)linkedDofInfo;
	Stream*			errorStr = Journal_Register( Error_Type, (Name)self->type  );
	
	Journal_Firewall( node_I < self->dofLayout->_numItemsInLayout, errorStr, "Error- in %s: tried to add a dof at a "
		"node that doesn't exist.\n", __func__ );
	Journal_Firewall( nodeLocalDof_I < self->dofLayout->dofCounts[node_I], errorStr, "Error- in %s: tried to add a "
		"dof %d that doesn't exist at current node %d (max index is %d).\n", __func__, nodeLocalDof_I, node_I, 
		self->dofLayout->dofCounts[node_I]-1 );
	
	self->linkedDofTbl[node_I][nodeLocalDof_I] = linkedDofSet_I;
}

void LinkedDofInfo_AddDofsToSet_FromIndexSet( void* linkedDofInfo, Index linkedDofSet_I, IndexSet* nodeSet, Dof_Index nodeLocalDof_I ) {
	LinkedDofInfo*		self = (LinkedDofInfo*)linkedDofInfo;
	Node_DomainIndex	node_I;

	for ( node_I = 0; node_I < nodeSet->size; node_I++ ) {
		if ( IndexSet_IsMember( nodeSet, node_I ) ) {
			self->linkedDofTbl[node_I][nodeLocalDof_I] = linkedDofSet_I;
		}
	}	
}



