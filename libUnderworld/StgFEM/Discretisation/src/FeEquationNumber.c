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

#include "ElementType.h"
#include "ElementType_Register.h"
#include "Element.h"
#include "FeMesh.h"
#include "FeEquationNumber.h"
#include "FeVariable.h"
#include "LinkedDofInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>
#include <petsc.h>
#include <petscvec.h>

int stgCmpInt( const void *l, const void *r ) {
   return *(int*)l - *(int*)r;
}

/*###### Typedefs and Structs ######*/

/** Textual name of this class */
const Type FeEquationNumber_Type = "FeEquationNumber";

/** struct to store sub-totals: what is the equation number up to at a given
    node? These can then be exchanged between processors */
typedef struct CritPointInfo {
      Node_GlobalIndex	index;
      Dof_EquationNumber	eqNum;
} CritPointInfo;


/** An element of linked list of critical point info. Several of the functions
    use this to keep track of key points */
typedef struct AddListEntry {
      CritPointInfo*		critPointInfo;
      struct AddListEntry*	next;
} AddListEntry;

/** Enum to say whetehr values at crit. nodes should be printed */
typedef enum PrintValuesFlag {
   DONT_PRINT_VALUES,
   PRINT_VALUES
} PrintValuesFlag;

/** MPI datatype handle for efficiently exchanging CritPointInfo structures.
	see FeEquationNumber_Create_CritPointInfo_MPI_Datatype() for where this
	handle is defined. */
MPI_Datatype MPI_critPointInfoType;

/*###### Private Function Declarations ######*/


/*###### Function Definitions ######*/

/** Public constructor */

FeEquationNumber* FeEquationNumber_New(
	Name						name,
	DomainContext*			context,
	void*						mesh,
	DofLayout*				dofLayout,
	VariableCondition*	bcs,
	LinkedDofInfo*			linkedDofInfo )
{
   FeEquationNumber* self = FeEquationNumber_DefaultNew( name );

	self->isConstructed = True;
	_FeEquationNumber_Init( self, context, mesh, dofLayout, bcs, linkedDofInfo );

	return self;
}

void* FeEquationNumber_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(FeEquationNumber);
	Type                                                      type = FeEquationNumber_Type;
	Stg_Class_DeleteFunction*                              _delete = _FeEquationNumber_Delete;
	Stg_Class_PrintFunction*                                _print = _FeEquationNumber_Print;
	Stg_Class_CopyFunction*                                  _copy = _FeEquationNumber_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)FeEquationNumber_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _FeEquationNumber_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _FeEquationNumber_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FeEquationNumber_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FeEquationNumber_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FeEquationNumber_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

   return _FeEquationNumber_New(  FEEQUATIONNUMBER_PASSARGS  );
}
/** Constructor implementation. */
FeEquationNumber* _FeEquationNumber_New(  FEEQUATIONNUMBER_DEFARGS  ){
   FeEquationNumber* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(FeEquationNumber) );
   self = (FeEquationNumber*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   /* General info */

   /* Virtual info */
   self->_build = _build;
   self->_initialise = _initialise;

   /* Mesh info */

   return self;
}


/** Constructor variables initialisation. Doesn't allocate any
    memory, just saves arguments and sets counters to 0. */
void _FeEquationNumber_Init(
   FeEquationNumber*		self,
	DomainContext*			context,
   void*						feMesh,
   DofLayout*				dofLayout,
   VariableCondition*	bcs,
   LinkedDofInfo*			linkedDofInfo )
{
   /* General and Virtual info should already be set */

   /* FinteElementMesh info */
	self->context = context;
   self->feMesh = (FeMesh*)feMesh;
   self->globalSumUnconstrainedDofs = 0;
   self->isBuilt = False;
   self->locationMatrixBuilt = False;
   self->_lowestLocalEqNum = -1;
   self->_lowestGlobalEqNums = NULL;
   self->_highestLocalEqNum = -1;
   self->dofLayout = dofLayout;
   self->bcs = bcs;
   self->linkedDofInfo = linkedDofInfo;
   self->remappingActivated = False;
   /* register streams */
   self->debug = Stream_RegisterChild( StgFEM_Discretisation_Debug, FeEquationNumber_Type );
   self->debugLM = Stream_RegisterChild( self->debug, "LM" );
   self->warning = Stream_RegisterChild( StgFEM_Warning, FeEquationNumber_Type );
   self->removeBCs = True;
   self->bcEqNums = STree_New();
   STree_SetIntCallbacks( self->bcEqNums );
   STree_SetItemSize( self->bcEqNums, sizeof(int) );
   self->ownedMap = STreeMap_New();
   STreeMap_SetItemSize( self->ownedMap, sizeof(int), sizeof(int) );
   STree_SetIntCallbacks( self->ownedMap );

   Stream_SetPrintingRank( self->debug, 0 );
}

void _FeEquationNumber_AssignFromXML( void* feEquationNumber, Stg_ComponentFactory *cf, void* data ) {
}

void _FeEquationNumber_Execute( void* feEquationNumber, void *data ){

}

void _FeEquationNumber_Destroy( void* feEquationNumber, void *data ){
   FeEquationNumber* self = (FeEquationNumber*) feEquationNumber;
   Index ii;

   /* free destination array memory */
   Journal_DPrintfL( self->debug, 2, "Freeing I.D. Array\n" );
   FreeArray( self->mapNodeDof2Eq );

   if (self->locationMatrix) {
      Journal_DPrintfL( self->debug, 2, "Freeing Full L.M. Array\n" );
      for( ii = 0; ii < self->nDomainEls; ii++ )
         FreeArray( self->locationMatrix[ii] );

      FreeArray( self->locationMatrix );
   }

   if( self->bcEqNums )
      Stg_Class_Delete( self->bcEqNums );

   if( self->ownedMap )
      Stg_Class_Delete( self->ownedMap );
}

/* Copy */

/** Stg_Class_Delete implementation. */
void _FeEquationNumber_Delete( void* feEquationNumber ) {
   FeEquationNumber* self = (FeEquationNumber*) feEquationNumber;

   Journal_DPrintfL( self->debug, 1, "In %s\n",  __func__ );
   Stream_IndentBranch( StgFEM_Debug );

   /* Stg_Class_Delete parent */
   _Stg_Class_Delete( self );
   Stream_UnIndentBranch( StgFEM_Debug );
}


/** Print implementation */
void _FeEquationNumber_Print( void* mesh, Stream* stream ) {
   FeEquationNumber* self = (FeEquationNumber*)mesh;

   /* General info */
   Journal_Printf( stream, "FeEquationNumber (ptr): %p\n", self );

   /* Print parent */
   _Stg_Class_Print( self, stream );

   /* Virtual info */
   Journal_Printf( stream,  "\t_build (func ptr): %p\n", self->_build );
   Journal_Printf( stream,  "\t_intialise (func ptr): %p\n", self->_initialise );

   /* FeEquationNumber info */
   /* Don't print dofs or bcs as these will be printed by FeVariable */

   if ( self->mapNodeDof2Eq ) {
      FeEquationNumber_PrintmapNodeDof2Eq( self, stream );
      FeEquationNumber_PrintLocationMatrix( self, stream );
   }
   else {
      Journal_Printf( stream, "\tmapNodeDof2Eq: (null)... not built yet\n" );
      Journal_Printf( stream, "\tlocationMatrix: (null)... not built yet\n" );
   }
}


void* _FeEquationNumber_Copy( void* feEquationNumber, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {

   abort();
}

void _FeEquationNumber_Build( void* feEquationNumber, void* data ) {
   FeEquationNumber* self = (FeEquationNumber*) feEquationNumber;

   assert(self);

   Journal_DPrintf( self->debug, "In %s:\n",  __func__ );
   Stream_IndentBranch( StgFEM_Debug );

   Stg_Component_Build( self->feMesh   , data, False );
   Stg_Component_Build( self->dofLayout, data, False );
   if ( self->linkedDofInfo ) Stg_Component_Build( self->dofLayout, data, False );
   if ( self->bcs )           Stg_Component_Build( self->bcs      , data, False );
	/* If we have new mesh topology information, do this differently. */
   /* if( self->feMesh->topo->domains && self->feMesh->topo->domains[MT_VERTEX] ) { */

   if( Mesh_HasExtension( self->feMesh, "vertexGrid" ) )
      FeEquationNumber_BuildWithDave( self );
   else
      FeEquationNumber_BuildWithTopology( self );

   /* If not removing BCs, construct a table of which equation numbers are actually BCs. */
   if( !self->removeBCs ) {
      FeMesh* mesh = self->feMesh;
      DofLayout* dofLayout = self->dofLayout;
      VariableCondition* bcs = self->bcs;
      int nDofs, varInd;
      int ii, jj;

      for( ii = 0; ii < FeMesh_GetNodeLocalSize( mesh ); ii++ ) {
         nDofs = dofLayout->dofCounts[ii];
         for( jj = 0; jj < nDofs; jj++ ) {
            varInd = dofLayout->varIndices[ii][jj];
            if( bcs && VariableCondition_IsCondition( bcs, ii, varInd ) ) {
               if( !STree_Has( self->bcEqNums, self->mapNodeDof2Eq[ii] + jj ) )
                  STree_Insert( self->bcEqNums, self->mapNodeDof2Eq[ii] + jj );
            }
         }
      }
   }

   if ( Stream_IsPrintableLevel( self->debug, 3 ) ) {
      FeEquationNumber_PrintmapNodeDof2Eq( self, self->debug );
   }

   Stream_UnIndentBranch( StgFEM_Debug );
}

/** Initialise implementation. Currently does nothing. */
void _FeEquationNumber_Initialise( void* feEquationNumber, void* data ) {
   FeEquationNumber* self = (FeEquationNumber*) feEquationNumber;

   Stg_Component_Initialise( self->feMesh   , data, False );
   Stg_Component_Initialise( self->dofLayout, data, False );
   if ( self->linkedDofInfo ) Stg_Component_Initialise( self->dofLayout, data, False );
   if ( self->bcs )           Stg_Component_Initialise( self->bcs      , data, False );
}


Index FeEquationNumber_CalculateActiveEqCountAtNode(
   void*			feEquationNumber,
   Node_DomainIndex	dNode_I,
   Dof_EquationNumber*	lowestActiveEqNumAtNodePtr )
{
   FeEquationNumber*	self = (FeEquationNumber*) feEquationNumber;
   Dof_Index		nodalDof_I = 0;
   Index			activeEqsAtCurrRowNode = 0;
   Dof_EquationNumber	currEqNum;
   Bool			foundLowest = False;

   for ( nodalDof_I = 0; nodalDof_I < self->dofLayout->dofCounts[dNode_I]; nodalDof_I++ ) {
      currEqNum = self->mapNodeDof2Eq[dNode_I][nodalDof_I];
      if ( currEqNum != -1 ) {
         activeEqsAtCurrRowNode++;
         if ( False == foundLowest ) {
            (*lowestActiveEqNumAtNodePtr) = currEqNum;
            foundLowest = True;
         }
      }
   }

   return activeEqsAtCurrRowNode;
}


void FeEquationNumber_BuildLocationMatrix( void* feEquationNumber ) {
   FeEquationNumber*	self = (FeEquationNumber*)feEquationNumber;
   FeMesh*			feMesh;
   unsigned		nDims;
   unsigned		nDomainEls;
   unsigned		nLocalNodes;
   unsigned*		nNodalDofs;
   unsigned		nElNodes;
   unsigned*		elNodes;
   int**			dstArray;
   int***			locMat;
   IArray*			inc;
   unsigned		e_i, n_i, dof_i;

   assert( self );

   /* Don't build if already done. */
   if( self->locationMatrixBuilt ) {
      Journal_DPrintf( self->debugLM, "In %s: LM already built, so just returning.\n",  __func__ );
      Stream_UnIndentBranch( StgFEM_Debug );
      return;
   }

   inc = IArray_New();

   /* Shortcuts. */
   feMesh = self->feMesh;
   nDims = Mesh_GetDimSize( feMesh );
   nDomainEls = FeMesh_GetElementDomainSize( feMesh );
   nLocalNodes = FeMesh_GetNodeLocalSize( feMesh );
   nNodalDofs = self->dofLayout->dofCounts;
   dstArray = self->mapNodeDof2Eq;

   /* Allocate for the location matrix. */
   locMat = AllocArray( int**, nDomainEls );
   for( e_i = 0; e_i < nDomainEls; e_i++ ) {
      FeMesh_GetElementNodes( feMesh, e_i, inc );
      nElNodes = IArray_GetSize( inc );
      elNodes = IArray_GetPtr( inc );
      locMat[e_i] = AllocArray( int*, nElNodes );
      for( n_i = 0; n_i < nElNodes; n_i++ )
         locMat[e_i][n_i] = AllocArray( int, nNodalDofs[elNodes[n_i]] );
   }

   /* Build location matrix. */
   for( e_i = 0; e_i < nDomainEls; e_i++ ) {
      FeMesh_GetElementNodes( feMesh, e_i, inc );
      nElNodes = IArray_GetSize( inc );
      elNodes = IArray_GetPtr( inc );
      for( n_i = 0; n_i < nElNodes; n_i++ ) {
         for( dof_i = 0; dof_i < nNodalDofs[elNodes[n_i]]; dof_i++ )
            locMat[e_i][n_i][dof_i] = dstArray[elNodes[n_i]][dof_i];
      }
   }

   Stg_Class_Delete( inc );

   /* Store result. */
   self->locationMatrix = locMat;
}

FeEquationNumber* _FeEquationNumber_Create( void* _self, Bool removeBCs ) {
  /* Build a n equation number for a given feVariable */
  FeVariable* feVar = Stg_CheckType( (FeVariable*)_self, FeVariable) ;
  FeEquationNumber* eqNum=NULL;
  Stg_Component_Build( feVar, NULL, False );
  Stg_Component_Initialise( feVar, NULL, False );
  eqNum = FeEquationNumber_New( "MrMoon",
                                  feVar->context,
                                  feVar->feMesh,
                                  feVar->dofLayout,
                                  feVar->bcs,
                                  NULL );

  eqNum->removeBCs = removeBCs;
  memcpy( eqNum->periodic, feVar->periodic, 3*sizeof(Bool) );
  Stg_Component_Build( eqNum, NULL, False );
  Stg_Component_Initialise( eqNum, NULL, False );
  // feVar->eqNum = self->eqNum; // We do it for now
  return eqNum;
}

void FeEquationNumber_PrintmapNodeDof2Eq( void* feFeEquationNumber, Stream* stream ) {
   FeEquationNumber* self = (FeEquationNumber*) feFeEquationNumber;
   FeMesh*		feMesh = self->feMesh;
   MPI_Comm comm = Comm_GetMPIComm( Mesh_GetCommTopology( feMesh, MT_VERTEX ) );
   unsigned rank;
   Node_GlobalIndex gNode_I;
   Node_GlobalIndex nodeGlobalCount = FeMesh_GetNodeGlobalSize( feMesh );

   MPI_Comm_rank( comm, (int*)&rank );
   Journal_Printf( stream, "%d: *** Printing destination array ***\n", rank );

   for (gNode_I =0; gNode_I < nodeGlobalCount; gNode_I++) {
      Node_DomainIndex dNode_I;

      if ( !Mesh_GlobalToDomain( feMesh, MT_VERTEX, gNode_I, &dNode_I ) ) {
         Journal_Printf( stream, "\tmapNodeDof2Eq[(gnode)%2d]: on another proc\n", gNode_I);
      }
      else {
         Dof_Index currNodeNumDofs = self->dofLayout->dofCounts[ dNode_I ];
         Dof_Index nodeLocalDof_I;

         Journal_Printf( stream, "\tmapNodeDof2Eq[(gnode)%2d][(dof)0-%d]:",gNode_I, currNodeNumDofs );
         for( nodeLocalDof_I = 0; nodeLocalDof_I < currNodeNumDofs; nodeLocalDof_I++ ) {
            Journal_Printf( stream, "%3d, ", self->mapNodeDof2Eq[dNode_I][nodeLocalDof_I] );
         }
         Journal_Printf( stream, "\n" );
      }
   }
}

void FeEquationNumber_PrintmapNodeDof2EqBox( void* feFeEquationNumber, Stream* stream ) {
	FeEquationNumber*   self = (FeEquationNumber*) feFeEquationNumber;
	FeMesh*             feMesh = self->feMesh;
	MPI_Comm            comm = Comm_GetMPIComm( Mesh_GetCommTopology( feMesh, MT_VERTEX ) );
	unsigned            rank;
	Grid*               vGrid;
	int                 ijk[3];
	Element_LocalIndex  lNode_I;
	Node_GlobalIndex    gNode_I;
	Node_Index          sizeI, sizeJ, sizeK;
	Dof_Index           nDofs;
	Dof_Index           dof_I;

	MPI_Comm_rank( comm, (int*)&rank );
	Journal_Printf( stream, "%d: *** Printing destination array ***\n", rank );

	vGrid = *Mesh_GetExtension( feMesh, Grid**,  feMesh->vertGridId );
	nDofs = self->dofLayout->dofCounts[0];
	sizeI = vGrid->sizes[ I_AXIS ];
	sizeJ = vGrid->sizes[ J_AXIS ];
	sizeK = vGrid->sizes[ K_AXIS ] ? vGrid->sizes[ K_AXIS ] : 1;

	for ( ijk[2] = 0 ; ijk[2] < sizeK ; ijk[2]++ ) {
		if ( sizeK != 1 )
			Journal_Printf( stream, "\nk = %d\n", ijk[2] );
		for ( ijk[1] = sizeJ - 1 ; ijk[1] >= 0 ; ijk[1]-- ) {
			Journal_Printf( stream, "%2d - ", ijk[1] );
			for ( ijk[0] = 0 ; ijk[0] < sizeI ; ijk[0]++ ) {
				gNode_I = Grid_Project( vGrid, ijk );
				Journal_Printf( stream, "{ " );
				if ( Mesh_GlobalToDomain( feMesh, MT_VERTEX, gNode_I, &lNode_I ) ) {
					for ( dof_I = 0 ; dof_I < nDofs ; dof_I++ )
						Journal_Printf( stream, "%3d ", self->mapNodeDof2Eq[lNode_I][dof_I] );
				}
				else {
					for ( dof_I = 0 ; dof_I < nDofs ; dof_I++ )
						Journal_Printf( stream, " XX " );
				}
				Journal_Printf( stream, "}" );
			}
			Journal_Printf( stream, "\n" );
		}
		/* Bottom row */
		Journal_Printf( stream, "    " );
		for ( ijk[0] = 0 ; ijk[0] < sizeI ; ijk[0]++ ) {
			Journal_Printf( stream, "    %3d    ", ijk[0] );
			if ( nDofs == 3 )
				Journal_Printf( stream, "    " );
		}
		Journal_Printf( stream, "\n" );
	}
}

void FeEquationNumber_PrintLocationMatrix( void* feFeEquationNumber, Stream* stream ) {
   FeEquationNumber* self = (FeEquationNumber*) feFeEquationNumber;
   FeMesh*		feMesh = self->feMesh;
   MPI_Comm comm = Comm_GetMPIComm( Mesh_GetCommTopology( feMesh, MT_VERTEX ) );
   unsigned rank;
   Element_GlobalIndex gEl_I;
   unsigned nDims = Mesh_GetDimSize( feMesh );
   Element_GlobalIndex elementGlobalCount = FeMesh_GetElementGlobalSize( feMesh );
   unsigned nLocalEls = FeMesh_GetElementLocalSize( feMesh );

   Journal_Printf( stream, "%d: *** Printing location matrix ***\n", rank  );

   MPI_Comm_rank( comm, (int*)&rank );

   for (gEl_I =0; gEl_I < elementGlobalCount; gEl_I++ ) {
      Element_LocalIndex lEl_I;

      if ( !Mesh_GlobalToDomain( feMesh, nDims, gEl_I, &lEl_I ) || lEl_I >= nLocalEls ) {
         Journal_Printf( stream, "\tLM[(g/l el)%2d/XXX]: on another proc\n", gEl_I);
      }
      else {
         Node_LocalIndex numNodesAtElement;
         Node_LocalIndex elLocalNode_I;
         unsigned*	incNodes;
         IArray*		inc;

         inc = IArray_New();
         FeMesh_GetElementNodes( self->feMesh, lEl_I, inc );
         numNodesAtElement = IArray_GetSize( inc );
         incNodes = IArray_GetPtr( inc );

         Journal_Printf( stream, "\tLM[(g/l el)%2d/%2d][(enodes)0-%d]", gEl_I, lEl_I, numNodesAtElement);
         /* print the nodes and dofs */
         for ( elLocalNode_I = 0; elLocalNode_I < numNodesAtElement; elLocalNode_I++ ) {
            /* look up processor local node number. */
            Element_LocalIndex currNode = incNodes[elLocalNode_I == 2 ? 3 :
                                                   elLocalNode_I == 3 ? 2 :
                                                   elLocalNode_I == 6 ? 7 :
                                                   elLocalNode_I == 7 ? 6 :
                                                   elLocalNode_I];
            /* get the number of dofs at current node */
            Dof_Index currNodeNumDofs = self->dofLayout->dofCounts[ currNode ];
            Dof_Index nodeLocalDof_I;

            Journal_Printf( stream, "({%2d}", currNode );
            for( nodeLocalDof_I = 0; nodeLocalDof_I < currNodeNumDofs; nodeLocalDof_I++ ) {
               Journal_Printf( stream, "%3d,", self->mapNodeDof2Eq[currNode][nodeLocalDof_I] );
            }
            Journal_Printf( stream, "), " );
         }

         Journal_Printf( stream, "\n" );

         Stg_Class_Delete( inc );
      }

   }
}

Partition_Index FeEquationNumber_CalculateOwningProcessorOfEqNum( void* feEquationNumber, Dof_EquationNumber eqNum ) {
   FeEquationNumber* self = (FeEquationNumber*)feEquationNumber;
   /* Partition_Index ownerProc = (unsigned int)-1; */
   Comm*	comm = Mesh_GetCommTopology( self->feMesh, MT_VERTEX );
   MPI_Comm	mpiComm = Comm_GetMPIComm( comm );
   unsigned	nProcs;
   unsigned	p_i;

   MPI_Comm_size( mpiComm, (int*)&nProcs );
   for( p_i = 1; p_i < nProcs; p_i++ ) {
      if( eqNum < self->_lowestGlobalEqNums[p_i] )
         break;
   }

   return p_i - 1;

}

void FeEquationNumber_BuildWithTopology( FeEquationNumber* self ) {
   Stream*		stream;
   double		startTime, endTime, time, tmin, tmax;
   FeMesh*		feMesh;
   Sync*		sync;
   Comm*		comm;
   MPI_Comm		mpiComm;
   unsigned		rank, nProcs;
   unsigned		nDims;
   unsigned		nDomainNodes;
   unsigned		nLocalNodes;
   unsigned*		nNodalDofs;
   unsigned		nElNodes, *elNodes;
   int**		dstArray;
   int			*nLocMatDofs, ***locMat;
   unsigned		varInd;
   unsigned		curEqNum;
   unsigned		base;
   unsigned		subTotal;
   MPI_Status		status;
   unsigned		maxDofs;
   unsigned*		tuples;
   LinkedDofInfo*	links;
   unsigned		highest;
   IArray*		inc;
   unsigned             e_i, n_i, dof_i, s_i;
   int			ii;

   assert( self );

   inc = IArray_New();

//   stream = Journal_Register( Info_Type, (Name)self->type  );
//   Stream_SetPrintingRank( stream, 0 );
//
//   Journal_RPrintf( stream, "FeEquationNumber: '%s'\n", self->name );
//   Stream_Indent( stream );
//   Journal_RPrintf( stream, "Generating equation numbers...\n" );
//   Stream_Indent( stream );
//   if( self->removeBCs )
//      Journal_RPrintf( stream, "BCs set to be removed.\n" );
//   else
//      Journal_RPrintf( stream, "BCs will not be removed.\n" );

   startTime = MPI_Wtime();

   /* Shortcuts. */
   feMesh = self->feMesh;
   comm = Mesh_GetCommTopology( feMesh, MT_VERTEX );
   mpiComm = Comm_GetMPIComm( comm );
   MPI_Comm_size( mpiComm, (int*)&nProcs );
   MPI_Comm_rank( mpiComm, (int*)&rank );
   nDims = Mesh_GetDimSize( feMesh );
   nDomainNodes = FeMesh_GetNodeDomainSize( feMesh );
   self->nDomainEls = FeMesh_GetElementDomainSize( feMesh );
   nLocalNodes = FeMesh_GetNodeLocalSize( feMesh );
   nNodalDofs = self->dofLayout->dofCounts;
   links = self->linkedDofInfo;

   /* Allocate for destination array. */
   dstArray = Memory_Alloc_2DComplex( int, nDomainNodes, nNodalDofs,
                                      "FeEquationNumber::mapNodeDof2Eq" );

   /* If needed, allocate for linked equation numbers. */
   if( links ) {
      unsigned	s_i;

      links->eqNumsOfLinkedDofs = ReallocArray( links->eqNumsOfLinkedDofs, int, links->linkedDofSetsCount );
      for( s_i = 0; s_i < links->linkedDofSetsCount; s_i++ )
         links->eqNumsOfLinkedDofs[s_i] = -1;
   }

   /* Allocate for the location matrix. */
   nLocMatDofs = NULL;
   locMat = AllocArray( int**, self->nDomainEls );
   for( e_i = 0; e_i < self->nDomainEls; e_i++ ) {
      FeMesh_GetElementNodes( feMesh, e_i, inc );
      nElNodes = IArray_GetSize( inc );
      elNodes = IArray_GetPtr( inc );
      nLocMatDofs = ReallocArray( nLocMatDofs, int, nElNodes );
      for( n_i = 0; n_i < nElNodes; n_i++ )
         nLocMatDofs[n_i] = nNodalDofs[elNodes[n_i]];
      locMat[e_i] = AllocComplex2D( int, nElNodes, nLocMatDofs );
   }
   FreeArray( nLocMatDofs );

   /* Build initial destination array and store max dofs. */
   curEqNum = 0;
   maxDofs = 0;
   for( n_i = 0; n_i < nLocalNodes; n_i++ ) {
      if( nNodalDofs[n_i] > maxDofs )
         maxDofs = nNodalDofs[n_i];

      for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
         varInd = self->dofLayout->varIndices[n_i][dof_i];
         if( !self->bcs || !VariableCondition_IsCondition( self->bcs, n_i, varInd ) ||
             !self->removeBCs )
         {
            if( links && links->linkedDofTbl[n_i][dof_i] != -1 ) {
               if( rank > 0 ) {
                  dstArray[n_i][dof_i] = -2;
                  continue;
               }
               if( links->eqNumsOfLinkedDofs[links->linkedDofTbl[n_i][dof_i]] == -1 )
                  links->eqNumsOfLinkedDofs[links->linkedDofTbl[n_i][dof_i]] = curEqNum++;
               dstArray[n_i][dof_i] = links->eqNumsOfLinkedDofs[links->linkedDofTbl[n_i][dof_i]];
            }
            else
               dstArray[n_i][dof_i] = curEqNum++;
         }
         else
            dstArray[n_i][dof_i] = -1;
      }
   }

   /* Order the equation numbers based on processor rank; cascade counts forward. */
   base = 0;
   if( rank > 0 )
      (void)MPI_Recv( &base, 1, MPI_UNSIGNED, rank - 1, 6669, mpiComm, &status );
   subTotal = base + curEqNum;
   if( rank < nProcs - 1 )
      (void)MPI_Send( &subTotal, 1, MPI_UNSIGNED, rank + 1, 6669, mpiComm );

   if( links ) {
      /* Reduce to find lowest linked DOFs. */
      for( s_i = 0; s_i < links->linkedDofSetsCount; s_i++ ) {
         if( links->eqNumsOfLinkedDofs[s_i] != -1 )
            links->eqNumsOfLinkedDofs[s_i] += base;
/*
  MPI_Allreduce( links->eqNumsOfLinkedDofs + s_i, &lowest, 1, MPI_UNSIGNED, MPI_MAX, mpiComm );
*/
         MPI_Allreduce( links->eqNumsOfLinkedDofs + s_i, &highest, 1, MPI_INT, MPI_MAX, mpiComm );
/*
  assert( (lowest == (unsigned)-1) ? lowest == highest : 1 );
*/
         links->eqNumsOfLinkedDofs[s_i] = highest;
      }
   }

   /* Modify existing destination array and dump to a tuple array. */
   tuples = AllocArray( unsigned, nDomainNodes * maxDofs );
   for( n_i = 0; n_i < nLocalNodes; n_i++ ) {
      for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
         varInd = self->dofLayout->varIndices[n_i][dof_i];
         if( !self->bcs || !VariableCondition_IsCondition( self->bcs, n_i, varInd ) ||
             !self->removeBCs )
         {
            if( links && links->linkedDofTbl[n_i][dof_i] != -1 ) {
               highest = links->eqNumsOfLinkedDofs[links->linkedDofTbl[n_i][dof_i]];
               dstArray[n_i][dof_i] = highest;
            }
            else
               dstArray[n_i][dof_i] += base;
         }
         tuples[n_i * maxDofs + dof_i] = dstArray[n_i][dof_i];
      }
   }

   /* Update all other procs. */
   sync = Mesh_GetSync( feMesh, MT_VERTEX );
   Sync_SyncArray( sync, tuples, maxDofs * sizeof(unsigned),
                   tuples + nLocalNodes * maxDofs, maxDofs * sizeof(unsigned),
                   maxDofs * sizeof(unsigned) );

   /* Update destination array's domain indices. */
   for( n_i = nLocalNodes; n_i < nDomainNodes; n_i++ ) {
      for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
         varInd = self->dofLayout->varIndices[n_i][dof_i];
         if( !self->bcs || !VariableCondition_IsCondition( self->bcs, n_i, varInd ) ||
             !self->removeBCs )
         {
            dstArray[n_i][dof_i] = tuples[n_i * maxDofs + dof_i];
         }
         else
            dstArray[n_i][dof_i] = -1;
      }
   }

   /* Destroy tuple array. */
   FreeArray( tuples );

   /* Build location matrix. */
   for( e_i = 0; e_i < self->nDomainEls; e_i++ ) {
      FeMesh_GetElementNodes( feMesh, e_i, inc );
      nElNodes = IArray_GetSize( inc );
      elNodes = IArray_GetPtr( inc );
      for( n_i = 0; n_i < nElNodes; n_i++ ) {
         for( dof_i = 0; dof_i < nNodalDofs[elNodes[n_i]]; dof_i++ )
            locMat[e_i][n_i][dof_i] = dstArray[elNodes[n_i]][dof_i];
      }
   }

   /* Store stuff on class. */
   self->mapNodeDof2Eq = dstArray;
   self->locationMatrix = locMat;
   self->locationMatrixBuilt = True;
   self->remappingActivated = False;
   self->localEqNumsOwnedCount = curEqNum;
   self->firstOwnedEqNum = base;
   self->lastOwnedEqNum = subTotal - 1;
   self->_lowestLocalEqNum = self->firstOwnedEqNum;

   /* Setup owned mapping. */
   STree_Clear( self->ownedMap );
   for( ii = self->firstOwnedEqNum; ii <= self->lastOwnedEqNum; ii++ ) {
      int val = ii - self->firstOwnedEqNum;
      STreeMap_Insert( self->ownedMap, &ii, &val );
   }

   /* Bcast global sum from highest rank. */
   if( rank == nProcs - 1 )
      self->globalSumUnconstrainedDofs = self->lastOwnedEqNum + 1;
   (void)MPI_Bcast( &self->globalSumUnconstrainedDofs, 1, MPI_UNSIGNED, nProcs - 1, mpiComm );

   /* Construct lowest global equation number list. */
   self->_lowestGlobalEqNums = AllocArray( int, nProcs );
   (void)MPI_Allgather( &self->firstOwnedEqNum, 1, MPI_UNSIGNED, self->_lowestGlobalEqNums, 1, MPI_UNSIGNED, mpiComm );

//   endTime = MPI_Wtime();

//   Journal_RPrintf( stream, "Assigned %d global equation numbers.\n", self->globalSumUnconstrainedDofs );
//   Journal_Printf( stream, "[%u] Assigned %d local equation numbers, within range %d to %d.\n",
//                   rank, self->lastOwnedEqNum - self->firstOwnedEqNum + 1, self->firstOwnedEqNum, self->lastOwnedEqNum + 1 );
//   Stream_UnIndent( stream );

//   time = endTime - startTime;
//   (void)MPI_Reduce( &time, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, mpiComm );
//   (void)MPI_Reduce( &time, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, mpiComm );
//   Journal_RPrintf( stream, "... Completed in %g [min] / %g [max] seconds.\n", tmin, tmax );
//   Stream_UnIndent( stream );

   Stg_Class_Delete( inc );
}

void FeEquationNumber_BuildWithDave( FeEquationNumber* self ) {
   int nLocals, *locals;
   Grid *vGrid;
   int varInd;
   int nEqNums, **dstArray;
   IArray *inc;
   int nDofs;
   int *periodic;
   int ***locMat;
   int nDims;
   int *elNodes;
   Comm *comm;
   MPI_Comm mpiComm;
   int nRanks, rank;
   Sync *sync;
   Bool isCond;
   int nPeriodicInds[3];
   int *periodicInds[3];
   int inds[3];
   Bool usePeriodic;
   int *tmpArray, nLocalEqNums;
   int lastOwnedEqNum, ind;
   STree *doneSet;
   int ii, jj, kk;

   comm = Mesh_GetCommTopology( self->feMesh, 0 );
   mpiComm = Comm_GetMPIComm( comm );
   MPI_Comm_size( mpiComm, &nRanks );
   MPI_Comm_rank( mpiComm, &rank );

   /* Setup an array containing global indices of all locally owned nodes. */
   nLocals = Mesh_GetLocalSize( self->feMesh, 0 );
   locals = AllocArray( int, nLocals );
   for( ii = 0; ii < nLocals; ii++ )
      locals[ii] = Mesh_DomainToGlobal( self->feMesh, 0, ii );

   /* Allocate for destination array. */
   nDofs = self->dofLayout->dofCounts[0];
   dstArray = AllocArray2D( int, Mesh_GetDomainSize( self->feMesh, 0 ), nDofs );

   /* Get the vertex grid extension and any periodicity. */
   nDims = Mesh_GetDimSize( self->feMesh );
   vGrid = *Mesh_GetExtension( self->feMesh, Grid**,  self->feMesh->vertGridId );
   periodic = Mesh_GetExtension( self->feMesh, int*, self->feMesh->periodicId );

   /* Fill destination array with initial values, setting dirichlet BCs as we go. */
   for( ii = 0; ii < nLocals; ii++ )
   {
/*
      Grid_Lift( vGrid, locals[ii], inds );
      usePeriodic = False;
      for( jj = 0; jj < nDims; jj++ )
      {
         if( (periodic[jj] || self->periodic[jj]) && (inds[jj] == 0 || inds[jj] == vGrid->sizes[jj] - 1) )
         {
            usePeriodic = True;
            break;
         }
      }
*/
      for( jj = 0; jj < nDofs; jj++ )
      {
         varInd = self->dofLayout->varIndices[ii][jj];
         if( self->bcs )
            isCond = VariableCondition_IsCondition( self->bcs, ii, varInd );
         else
            isCond = False;

         if( isCond && self->removeBCs )
            dstArray[ii][jj] = -1;
         else
            dstArray[ii][jj] = 0;
      }
   }

   /* Generate opposing indices for periodicity. */
   for( ii = 0; ii < nDims; ii++ )
   {
      nPeriodicInds[ii] = 0;
      periodicInds[ii] = NULL;
      if( periodic[ii] || self->periodic[ii] )
      {
         periodicInds[ii] = AllocArray( int, nLocals );
         for( jj = 0; jj < nLocals; jj++ )
         {
            Grid_Lift( vGrid, locals[jj], inds );
            if( inds[ii] != vGrid->sizes[ii] - 1 )
               continue;
            /*
            for( kk = 0; kk < nDofs; kk++ )
               if( dstArray[jj][kk] == -1 )
                  break;
               if( kk < nDofs )
                  continue;
            */
            periodicInds[ii][nPeriodicInds[ii]++] = locals[jj];
         }
      }
   }

   /* Call Dave's equation number generation routine. */
   if( nDims == 2 ) {
      GenerateEquationNumbering( vGrid->sizes[0], vGrid->sizes[1], 1,
				 nLocals, locals,
				 nDofs, Mesh_GetGlobalSize( self->feMesh, 0 ),
				 periodic[0] || self->periodic[0], periodic[1] || self->periodic[1], False,
				 nPeriodicInds[0], nPeriodicInds[1], 0,
				 periodicInds[0], periodicInds[1], NULL,
				 dstArray[0], &nEqNums );
   }
   else if( nDims == 3 ) {
      GenerateEquationNumbering( vGrid->sizes[0], vGrid->sizes[1], vGrid->sizes[2],
				 nLocals, locals,
				 nDofs, Mesh_GetGlobalSize( self->feMesh, 0 ),
				 periodic[0] || self->periodic[0], periodic[1] || self->periodic[1], periodic[2] || self->periodic[2],
				 nPeriodicInds[0], nPeriodicInds[1], nPeriodicInds[2],
				 periodicInds[0], periodicInds[1], periodicInds[2],
				 dstArray[0], &nEqNums );
   }
   else if( nDims == 1 ) {
      GenerateEquationNumbering( vGrid->sizes[0], 1, 1,
				 nLocals, locals,
				 nDofs, Mesh_GetGlobalSize( self->feMesh, 0 ),
				 periodic[0], False, False,
				 nPeriodicInds[0], 0, 0,
				 periodicInds[0], NULL, NULL,
				 dstArray[0], &nEqNums );
   }
   else
      abort();

   /* Free periodic arrays. */
   for( ii = 0; ii < nDims; ii++ )
   {
      if( periodicInds[ii] )
         FreeArray( periodicInds[ii] );
   }

   /* Setup owned mapping part 1. */
   STree_Clear( self->ownedMap );
   for( ii = 0; ii < nLocals; ii++ )
   {
      Grid_Lift( vGrid, locals[ii], inds );
      for( jj = 0; jj < nDims; jj++ )
      {
         if( (periodic[jj] || self->periodic[jj]) && inds[jj] == vGrid->sizes[jj] - 1 )
         {
            inds[jj] = 0;
            ind = Grid_Project( vGrid, inds );
            if( !FeMesh_NodeGlobalToDomain( self->feMesh, ind, &ind ) )
               break;
         }
      }
      if( jj < nDims )
         continue;
      for( jj = 0; jj < nDofs; jj++ )
      {
         if( dstArray[ii][jj] == -1 || STreeMap_HasKey( self->ownedMap, dstArray[ii] + jj ) )
            continue;
         STreeMap_Insert( self->ownedMap, dstArray[ii] + jj, &ii );
      }
   }

   /* Setup owned mapping. */
   tmpArray = AllocArray( int, nLocals * nDofs );
   memcpy( tmpArray, dstArray[0], nLocals * nDofs * sizeof(int) );
   qsort( tmpArray, nLocals * nDofs, sizeof(int), stgCmpInt );
   doneSet = STree_New();
   STree_SetItemSize( doneSet, sizeof(int) );
   STree_SetIntCallbacks( doneSet );
   for( nLocalEqNums = 0, ii = 0; ii < nLocals * nDofs; ii++ )
   {
      if( tmpArray[ii] != -1 && STreeMap_HasKey( self->ownedMap, tmpArray + ii ) && !STree_Has( doneSet, tmpArray + ii ) )
      {
         if( !nLocalEqNums )
            self->_lowestLocalEqNum = tmpArray[ii];
         *(int*)STreeMap_Map( self->ownedMap, tmpArray + ii ) = nLocalEqNums;
         STree_Insert( doneSet, tmpArray + ii );
         nLocalEqNums++;
      }
   }
   lastOwnedEqNum = -1; /* Don't need this anymore. */
   FreeArray( tmpArray );
   Stg_Class_Delete( doneSet );

   /* Transfer remote equation numbers. */
   sync = Mesh_GetSync( self->feMesh, 0 );
   Sync_SyncArray( sync, dstArray[0], nDofs * sizeof(int),
                   dstArray[0] + nLocals * nDofs, nDofs * sizeof(int),
                   nDofs * sizeof(int) );

   /* Allocate for location matrix. */
   /* first store nDomainEls for usage during destroy */
   self->nDomainEls = Mesh_GetDomainSize( self->feMesh, nDims );
   locMat = AllocArray( int**, self->nDomainEls );
   for( ii = 0; ii < self->nDomainEls; ii++ )
      locMat[ii] = AllocArray2D( int, FeMesh_GetElementNodeSize( self->feMesh, 0 ), nDofs );

   /* Fill in location matrix. */
   inc = IArray_New();
   for( ii = 0; ii < Mesh_GetDomainSize( self->feMesh, nDims ); ii++ )
   {
      FeMesh_GetElementNodes( self->feMesh, ii, inc );
      elNodes = IArray_GetPtr( inc );
      for( jj = 0; jj < FeMesh_GetElementNodeSize( self->feMesh, 0 ); jj++ )
      {
         for( kk = 0; kk < nDofs; kk++ )
         locMat[ii][jj][kk] = dstArray[elNodes[jj]][kk];
      }
   }
   Stg_Class_Delete( inc );

   /* Fill in our other weird values. */
   self->mapNodeDof2Eq = dstArray;
   self->locationMatrix = locMat;
   self->locationMatrixBuilt = True;
   self->remappingActivated = False;
   self->localEqNumsOwnedCount = nLocalEqNums;

   /* Bcast global sum from highest rank. */
   self->globalSumUnconstrainedDofs = nEqNums;

   /* Construct lowest global equation number list. */
   self->_lowestGlobalEqNums = AllocArray( int, nRanks );
   (void)MPI_Allgather( &self->_lowestLocalEqNum, 1, MPI_UNSIGNED,
                        self->_lowestGlobalEqNums, 1, MPI_UNSIGNED,
                        mpiComm );

   FreeArray( locals );

   /*
   printf( "%d: localEqNumsOwned = %d\n", rank, self->localEqNumsOwnedCount );
   printf( "%d: globalSumUnconstrainedDofs = %d\n", rank, self->globalSumUnconstrainedDofs );
   */
}




/*

Input:
  nlocal - number of locally ownded nodes
  g_node_id - global indices of nodes owned locally. Size nlocal
  dof - degrees of freedom per node
  nglobal - number of global nodes
  npx - number of consider to be periodic in x (local to this proc)
  npy - number of consider to be periodic in y (local to this proc)
  periodic_x_gnode_id - global indices of nodes (on this proc) which are on right hand side boundary
  periodic_y_gnode_id - global indices of nodes (on this proc) whipch are on top boundary
  eqnums - contains any dirichlet boundary conditions. Size nlocal*dof

Output;
  eqnums - contains full list of eqnums

Assumptions:
- Ordering eqnums[] = { (node_0,[dof_0,dof_1,..,dof_x]), (node_1,[dof_0,dof_1,..,dof_x]), ... }
- Any dirichlet set along a boundary deemed to be periodic will be clobbered.
- Processors may have duplicate nodes in the g_node_id[] list.
- A number in the corner is considered part of both boundaries (horiz and vert)
- If npx is not 0, then periodicity is assumed in x
- If npy is not 0, then periodicity is assumed in y
- Dofs constrained to be dirichlet must be marked with a negative number.
- Dofs are NOT split across processors.
- We can define a logical i,j,k ordering to uniquely identify nodes.

*/

PetscErrorCode _VecScatterBeginEnd( VecScatter vscat, Vec FROM, Vec TO, InsertMode addv,ScatterMode mode )
{
#if( (PETSC_VERSION_MAJOR==2) && (PETSC_VERSION_MINOR==3) && (PETSC_VERSION_SUBMINOR==2) )
	// 2.3.2 ordering of args
	VecScatterBegin( FROM, TO, addv, mode, vscat );
	VecScatterEnd( FROM, TO, addv, mode, vscat );
#else
	// 2.3.3 or 3.0.0
	VecScatterBegin( vscat, FROM, TO, addv, mode );
	VecScatterEnd( vscat, FROM, TO, addv, mode );
#endif

	PetscFunctionReturn(0);
}

int GenerateEquationNumbering(
		int NX, int NY, int NZ,
		int nlocal, int g_node_id[],
		int dof, int nglobal,
		PetscTruth periodic_x, PetscTruth periodic_y, PetscTruth periodic_z,
		int npx, int npy, int npz,
		int periodic_x_gnode_id[], int periodic_y_gnode_id[], int periodic_z_gnode_id[],
		int eqnums[], int *neqnums )
{
	PetscErrorCode ierr;
	PetscInt periodic_mask;
	Vec global_eqnum, g_ownership;
	PetscInt i;
	PetscMPIInt rank;        /* processor rank */
	PetscMPIInt size;        /* size of communicator */
	Vec local_ownership, local_eqnum;
	PetscInt *_g_node_id;
	IS is_gnode, is_eqnum;
	VecScatter vscat_ownership, vscat_eqnum;
	PetscScalar *_local_ownership, *_local_eqnum;
	PetscInt local_eqnum_count,global_eqnum_count;
	PetscScalar val[10];
	PetscInt d,idx[10];
	PetscInt *to_fetch,cnt,number_to_fetch;
	PetscInt eq_cnt;
	Vec offset_list;
	VecScatter vscat_offset;
	Vec seq_offset_list;
	PetscInt offset, inc;

	PetscInt spanx,spany,spanz,total;
	PetscInt loc;
	PetscReal max;
	PetscInt n_inserts;


	ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);
	ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);

	if( dof>=10 ) {
		Stg_SETERRQ(PETSC_ERR_SUP, "Max allowable degrees of freedom per node = 10. Change static size" );
	}


	/*
	Claim locally owned nodes. Duplicate nodes on the interior will be resolved by the processor
	which inserts last.
	*/
	VecCreate( PETSC_COMM_WORLD, &g_ownership );
	VecSetSizes( g_ownership, PETSC_DECIDE, nglobal );
	VecSetFromOptions( g_ownership );

	for( i=0; i<nlocal; i++ ) {
		VecSetValue( g_ownership, g_node_id[i], rank, INSERT_VALUES );
	}
	VecAssemblyBegin(g_ownership);
	VecAssemblyEnd(g_ownership);


	/* Mask out the periodic boundaries. */
	periodic_mask = -6699.0;
	if (periodic_x_gnode_id!=NULL) {
		for( i=0; i<npx; i++ ) {
			VecSetValue( g_ownership, periodic_x_gnode_id[i], periodic_mask, INSERT_VALUES );
		}
	}
	if (periodic_y_gnode_id!=NULL) {
		for( i=0; i<npy; i++ ) {
			VecSetValue( g_ownership, periodic_y_gnode_id[i], periodic_mask, INSERT_VALUES );
		}
	}
	if (periodic_z_gnode_id!=NULL) {
		for( i=0; i<npz; i++ ) {
			VecSetValue( g_ownership, periodic_z_gnode_id[i], periodic_mask, INSERT_VALUES );
		}
	}
	VecAssemblyBegin(g_ownership);
	VecAssemblyEnd(g_ownership);

	/*
	PetscPrintf(PETSC_COMM_WORLD, "g_ownership \n");
	VecView( g_ownership, PETSC_VIEWER_STDOUT_WORLD );
	*/

	/* Get all locally owned nodes */
	VecCreate( PETSC_COMM_SELF, &local_ownership );
	VecSetSizes( local_ownership, PETSC_DECIDE, nlocal );
	VecSetFromOptions( local_ownership );

	PetscMalloc( sizeof(PetscInt)*nlocal, &_g_node_id);
	for( i=0; i<nlocal; i++ ) {
		_g_node_id[i] = g_node_id[i];
	}
	ISCreateGeneralWithArray( PETSC_COMM_WORLD, nlocal, _g_node_id, &is_gnode );
	VecScatterCreate( g_ownership, is_gnode, local_ownership, PETSC_NULL, &vscat_ownership );


	/* assign unique equation numbers */
	VecSet( local_ownership, -6699 );
	_VecScatterBeginEnd( vscat_ownership, g_ownership, local_ownership, INSERT_VALUES, SCATTER_FORWARD );


	/* Count instances of rank in the local_ownership vector */
	VecGetArray( local_ownership, &_local_ownership );
	local_eqnum_count = 0;
	for( i=0; i<nlocal; i++ ) {
		if( ((PetscInt)_local_ownership[i]) == rank ) {
			local_eqnum_count++;
		}
	}

	(void)MPI_Allreduce( &local_eqnum_count, &global_eqnum_count, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD );
	/* PetscPrintf( PETSC_COMM_SELF,
	    "[%d] number of local,global equations (without dofs) %d,%d \n", rank, local_eqnum_count, global_eqnum_count ); */
	/* check */
	spanx = NX;
	spany = NY;
	spanz = NZ;
	if( periodic_x==PETSC_TRUE ) {
		spanx--;
	}
	if( periodic_y==PETSC_TRUE ) {
		spany--;
	}
	if( periodic_z==PETSC_TRUE ) {
		spanz--;
	}
	total = spanx*spany*spanz;
	if( total!=global_eqnum_count ) {
		Stg_SETERRQ(PETSC_ERR_SUP, "Something stinks. Computed global size for nodes does not match expected" );
	}



	VecCreate( PETSC_COMM_WORLD, &global_eqnum );
	VecSetSizes( global_eqnum, PETSC_DECIDE, nglobal*dof );
	VecSetFromOptions( global_eqnum );
	VecSet( global_eqnum, 0.0 );

	/* Load existing eqnums in */
	for( i=0; i<nlocal; i++ ) {
		n_inserts = 0;
		for( d=0; d<dof; d++ ) {
/*
			idx[d] = -(g_node_id[i]*dof + d);
			val[d] = 0.0;
*/
			if( eqnums[ i*dof + d ] < 0.0 ) {
				idx[n_inserts] = g_node_id[i]*dof + d;
				val[n_inserts] = eqnums[ i*dof + d ];
				n_inserts++;
			}
		}
		/* only insert dirichlet bc's */
		VecSetValues( global_eqnum, n_inserts, idx, val, INSERT_VALUES );
	}
	VecAssemblyBegin(global_eqnum);
	VecAssemblyEnd(global_eqnum);





	/* Generate list of eqnums to get */
	PetscMalloc( sizeof(PetscInt)*nlocal*dof, &to_fetch );
	cnt = 0;
	for( i=0; i<nlocal; i++ ) {
		if( _local_ownership[i]==rank ) {
			for( d=0; d<dof; d++ ) {
				to_fetch[cnt] = g_node_id[i]*dof + d;
				cnt++;
			}
		}
	}
	number_to_fetch = cnt;


	VecCreate( PETSC_COMM_SELF, &local_eqnum );
	VecSetSizes( local_eqnum, PETSC_DECIDE, number_to_fetch);
	VecSetFromOptions( local_eqnum );

	ISCreateGeneralWithArray( PETSC_COMM_SELF, number_to_fetch, to_fetch, &is_eqnum );
	VecScatterCreate( global_eqnum, is_eqnum, local_eqnum, PETSC_NULL, &vscat_eqnum );

	VecSet( local_eqnum, -6699 );
	_VecScatterBeginEnd( vscat_eqnum, global_eqnum, local_eqnum, INSERT_VALUES, SCATTER_FORWARD );


	/* compute offset */
	/* count how many entries there are */
	VecGetArray( local_eqnum, &_local_eqnum );
	eq_cnt = 0;
	for( i=0; i<number_to_fetch; i++ ) {
		if( (PetscInt)_local_eqnum[i]==0 ) {
			eq_cnt++;
		}
	}
	VecRestoreArray( local_eqnum, &_local_eqnum );

    VecCreate( PETSC_COMM_WORLD, &offset_list );
    VecSetSizes( offset_list, PETSC_DECIDE, size );
    VecSetFromOptions( offset_list );
    VecSetValue( offset_list, rank, eq_cnt, INSERT_VALUES );
    VecAssemblyBegin(offset_list);
    VecAssemblyEnd(offset_list);
    /*
    PetscPrintf(PETSC_COMM_WORLD, "offset_list \n");
    VecView( offset_list, PETSC_VIEWER_STDOUT_WORLD );
    */

    VecScatterCreateToAll(offset_list,&vscat_offset,&seq_offset_list);
    _VecScatterBeginEnd( vscat_offset, offset_list, seq_offset_list, INSERT_VALUES, SCATTER_FORWARD );

    {
        PetscScalar *_seq_offset_list;

        VecGetArray( seq_offset_list, &_seq_offset_list );
        offset = 0;
        for (i=0; i<rank; i++) {
            offset+=_seq_offset_list[ i ];
        }
        VecRestoreArray( seq_offset_list, &_seq_offset_list );
    }
    Stg_VecScatterDestroy(&vscat_offset);
    Stg_VecDestroy(&offset_list);
    Stg_VecDestroy(&seq_offset_list);

//    PetscPrintf( PETSC_COMM_SELF, "[%d]: offset = %d \n", rank, offset ); 

	VecGetArray( local_eqnum, &_local_eqnum );
	inc = 0;
	for( i=0; i<number_to_fetch; i++ ) {
		if( (PetscInt)_local_eqnum[i]==0 ) {
			_local_eqnum[i] = offset+inc;
			inc++;
		}
	}
	VecRestoreArray( local_eqnum, &_local_eqnum );

	_VecScatterBeginEnd( vscat_eqnum, local_eqnum, global_eqnum, INSERT_VALUES, SCATTER_REVERSE );
	/*
	PetscPrintf(PETSC_COMM_WORLD, "global_eqnum \n");
	VecView( global_eqnum, PETSC_VIEWER_STDOUT_WORLD );
	*/

	/* For each periodic boundary, get the mapped nodes */

	if( periodic_x==PETSC_TRUE ) {
		VecScatter vscat_p;
		IS is_from;
		PetscInt *from, *to;
		Vec mapped;
		PetscScalar *_mapped;
		PetscInt c;

		PetscMalloc( sizeof(PetscInt)*npx*dof, &from );
		PetscMalloc( sizeof(PetscInt)*npx*dof, &to );

		c = 0;
		for( i=0; i<npx; i++ ) {
			PetscInt I,J,K,gid,from_gid;

			gid = periodic_x_gnode_id[i];
			K = gid/(NX*NY);
			J = (gid - K*(NX*NY))/NX;
			I = gid - K*(NX*NY) - J*NX;
			from_gid = (I-(NX-1)) + J*NX + K*(NX*NY);

			for( d=0; d<dof; d++ ) {
				to[c] = gid * dof + d;
				from[c] = from_gid * dof + d;
				c++;
			}
		}


		VecCreate( PETSC_COMM_SELF, &mapped );
		VecSetSizes( mapped, PETSC_DECIDE, npx*dof );
		VecSetFromOptions( mapped );

		ISCreateGeneralWithArray( PETSC_COMM_SELF, npx*dof, from, &is_from );
		VecScatterCreate( global_eqnum, is_from, mapped, PETSC_NULL, &vscat_p );

		_VecScatterBeginEnd( vscat_p, global_eqnum, mapped, INSERT_VALUES, SCATTER_FORWARD );
		if( npx>0 ) {
			VecGetArray( mapped, &_mapped );
			VecSetValues( global_eqnum, npx*dof, to, _mapped,  INSERT_VALUES );
			VecRestoreArray( mapped, &_mapped );
		}

		VecAssemblyBegin(global_eqnum);
		VecAssemblyEnd(global_eqnum);

		Stg_VecScatterDestroy(&vscat_p );
		Stg_ISDestroy(&is_from );
		Stg_VecDestroy(&mapped );
		PetscFree( from );
		PetscFree( to );
	}


	if( periodic_y==PETSC_TRUE ) {
		VecScatter vscat_p;
		IS is_from;
		PetscInt *from, *to;
		Vec mapped;
		PetscScalar *_mapped;
		PetscInt c;

		PetscMalloc( sizeof(PetscInt)*npy*dof, &from );
		PetscMalloc( sizeof(PetscInt)*npy*dof, &to );

		c = 0;
		for( i=0; i<npy; i++ ) {
			PetscInt I,J,K,gid,from_gid;

			gid = periodic_y_gnode_id[i];
			K = gid/(NX*NY);
			J = (gid - K*(NX*NY))/NX;
			I = gid - K*(NX*NY) - J*NX;
			from_gid = I + (J - (NY - 1))*NX + K*(NX*NY);

			for( d=0; d<dof; d++ ) {
				to[c] = gid * dof + d;
				from[c] = from_gid * dof + d;
				c++;
			}
		}


		VecCreate( PETSC_COMM_SELF, &mapped );
		VecSetSizes( mapped, PETSC_DECIDE, npy*dof );
		VecSetFromOptions( mapped );

		ISCreateGeneralWithArray( PETSC_COMM_SELF, npy*dof, from, &is_from );
		VecScatterCreate( global_eqnum, is_from, mapped, PETSC_NULL, &vscat_p );

		_VecScatterBeginEnd( vscat_p, global_eqnum, mapped, INSERT_VALUES, SCATTER_FORWARD );
		if( npy>0 ) {
			VecGetArray( mapped, &_mapped );
			VecSetValues( global_eqnum, npy*dof, to, _mapped,  INSERT_VALUES );
			VecRestoreArray( mapped, &_mapped );
		}

		VecAssemblyBegin(global_eqnum);
		VecAssemblyEnd(global_eqnum);

		Stg_VecScatterDestroy(&vscat_p );
		Stg_ISDestroy(&is_from );
		Stg_VecDestroy(&mapped );
		PetscFree( from );
		PetscFree( to );
	}

	if( periodic_z==PETSC_TRUE ) {
		VecScatter vscat_p;
		IS is_from;
		PetscInt *from, *to;
		Vec mapped;
		PetscScalar *_mapped;
		PetscInt c;

		PetscMalloc( sizeof(PetscInt)*npz*dof, &from );
		PetscMalloc( sizeof(PetscInt)*npz*dof, &to );

		c = 0;
		for( i=0; i<npz; i++ ) {
			PetscInt I,J,K,gid,from_gid;

			gid = periodic_z_gnode_id[i];
			K = gid/(NX*NY);
			J = (gid - K*(NX*NY))/NX;
			I = gid - K*(NX*NY) - J*NX;
			from_gid = I + J*NX + (K - (NZ-1))*(NX*NY);

			for( d=0; d<dof; d++ ) {
				to[c] = gid * dof + d;
				from[c] = from_gid * dof + d;
				c++;
			}
		}


		VecCreate( PETSC_COMM_SELF, &mapped );
		VecSetSizes( mapped, PETSC_DECIDE, npz*dof );
		VecSetFromOptions( mapped );

		ISCreateGeneralWithArray( PETSC_COMM_SELF, npz*dof, from, &is_from );
		VecScatterCreate( global_eqnum, is_from, mapped, PETSC_NULL, &vscat_p );

		_VecScatterBeginEnd( vscat_p, global_eqnum, mapped, INSERT_VALUES, SCATTER_FORWARD );
		if( npz>0 ) {
			VecGetArray( mapped, &_mapped );
			VecSetValues( global_eqnum, npz*dof, to, _mapped,  INSERT_VALUES );
			VecRestoreArray( mapped, &_mapped );
		}

		VecAssemblyBegin(global_eqnum);
		VecAssemblyEnd(global_eqnum);

		Stg_VecScatterDestroy(&vscat_p );
		Stg_ISDestroy(&is_from );
		Stg_VecDestroy(&mapped );
		PetscFree( from );
		PetscFree( to );
	}


	/*
	PetscPrintf(PETSC_COMM_WORLD, "global_eqnum following periodic \n");
	VecView( global_eqnum, PETSC_VIEWER_STDOUT_WORLD );
	*/


	/* Finally, scatter stuff from global_eqnums into MY array */
	{
		IS is_all_my_eqnums;
		PetscInt *all_my_eqnum_index;
		PetscInt _I,_D,CNT;
		Vec all_my_eqnums;
		PetscScalar *_all_my_eqnums;
		VecScatter vscat_p;

		PetscMalloc( sizeof(PetscInt)*dof*nlocal, &all_my_eqnum_index );

		CNT = 0;
		for( _I=0; _I<nlocal; _I++ ) {
			for( _D=0; _D<dof; _D++ ) {
				all_my_eqnum_index[CNT] = g_node_id[_I]*dof + _D;
				CNT++;
			}
		}

		ISCreateGeneralWithArray( PETSC_COMM_SELF, nlocal*dof, all_my_eqnum_index, &is_all_my_eqnums );
		VecCreate( PETSC_COMM_SELF, &all_my_eqnums );
		VecSetSizes( all_my_eqnums, PETSC_DECIDE, nlocal*dof );
		VecSetFromOptions( all_my_eqnums );
		VecScatterCreate( global_eqnum, is_all_my_eqnums, all_my_eqnums, PETSC_NULL, &vscat_p );
		_VecScatterBeginEnd( vscat_p, global_eqnum, all_my_eqnums, INSERT_VALUES, SCATTER_FORWARD );
		VecGetArray( all_my_eqnums, &_all_my_eqnums );

		for( i=0; i<nlocal*dof; i++ ) {
			eqnums[i] = (int)_all_my_eqnums[i];
		}
		VecRestoreArray( all_my_eqnums, &_all_my_eqnums );

		Stg_VecScatterDestroy(&vscat_p );
		Stg_VecDestroy(&all_my_eqnums );
		Stg_ISDestroy(&is_all_my_eqnums );
		PetscFree( all_my_eqnum_index );
	}

	VecMax( global_eqnum, &loc, &max );
	*neqnums = (int)max;
	(*neqnums)++;

	/* tidy up */
	VecRestoreArray( local_ownership, &_local_ownership );


	Stg_VecDestroy(&g_ownership );
	Stg_VecDestroy(&local_ownership );
	PetscFree( _g_node_id );
	Stg_ISDestroy(&is_gnode );
	Stg_VecScatterDestroy(&vscat_ownership );

	Stg_VecDestroy(&global_eqnum );
	Stg_VecDestroy(&local_eqnum );
	PetscFree( to_fetch );
	Stg_ISDestroy(&is_eqnum );
	Stg_VecScatterDestroy(&vscat_eqnum );

	return 0;
}
