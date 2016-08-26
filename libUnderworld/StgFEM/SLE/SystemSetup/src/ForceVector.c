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
#include "StgFEM/Discretisation/Discretisation.h"

#include "types.h"

#include "SolutionVector.h"
#include "ForceVector.h"
#include "ForceTerm.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "EntryPoint.h"

/* Textual name of this class */
const Type ForceVector_Type = "ForceVector";

/** Name of this class' entry points */
static const char	ForceVector_assembleForceVectorStr[] = "assembleForceVector";

ForceVector* ForceVector_New(
	Name							name,
	FeVariable*					feVariable,
	Dimension_Index			dim,
	void*							entryPoint_Register,
	MPI_Comm						comm )
{
	ForceVector* self = _ForceVector_DefaultNew( name );

	self->isConstructed = True;
	_SolutionVector_Init( (SolutionVector*)self, comm, feVariable );
	_ForceVector_Init( self, dim, entryPoint_Register );

	return self;
}

void* _ForceVector_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(ForceVector);
	Type                                                      type = ForceVector_Type;
	Stg_Class_DeleteFunction*                              _delete = _ForceVector_Delete;
	Stg_Class_PrintFunction*                                _print = _ForceVector_Print;
	Stg_Class_CopyFunction*                                  _copy = _ForceVector_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _ForceVector_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _ForceVector_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _ForceVector_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _ForceVector_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _ForceVector_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _ForceVector_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return _ForceVector_New(  FORCEVECTOR_PASSARGS  );
}

ForceVector* _ForceVector_New(  FORCEVECTOR_DEFARGS  ) {
	ForceVector* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ForceVector) );
	self = (ForceVector*)_SolutionVector_New(  SOLUTIONVECTOR_PASSARGS  );

	return self;
}


void _ForceVector_Init( void* forceVector, Dimension_Index dim, EntryPoint_Register* entryPoint_Register ) {
	ForceVector* self = (ForceVector*)  forceVector;

	/* ForceVector info */
	self->dim = dim;
	self->entryPoint_Register = entryPoint_Register;

	/* Create Stream */
	self->debug = Stream_RegisterChild( StgFEM_SLE_SystemSetup_Debug, self->type );

	/* Create Entry Point for assembleForceVector */
	Stg_asprintf( &self->_assembleForceVectorEPName, "%s-%s", self->name, ForceVector_assembleForceVectorStr );
	self->assembleForceVector = FeEntryPoint_New( self->_assembleForceVectorEPName, FeEntryPoint_AssembleForceVector_CastType );
	if(entryPoint_Register)
        EntryPoint_Register_Add( self->entryPoint_Register, self->assembleForceVector );

	/* Add default hook to assembleForceVector entry point */
	EP_ReplaceAll( self->assembleForceVector, ForceVector_GlobalAssembly_General );

	self->forceTermList = Stg_ObjectList_New();

	self->inc = IArray_New();

}

void _ForceVector_Delete( void* forceVector ) {
	ForceVector* self = (ForceVector*)forceVector;

	Journal_DPrintf( self->debug, "In %s - for %s\n", __func__, self->name );

	/* Stg_Class_Delete parent*/
	_SolutionVector_Delete( self );
}

void _ForceVector_Print( void* forceVector, Stream* stream ) {}


void* _ForceVector_Copy( void* forceVector, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {}

void _ForceVector_AssignFromXML( void* forceVector, Stg_ComponentFactory* cf, void* data ) {
	ForceVector*    self = (ForceVector*)forceVector;
	Dimension_Index dim = 0;
	void*           entryPointRegister = NULL;

	_SolutionVector_AssignFromXML( self, cf, data );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0 );
	dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim );

	_ForceVector_Init( self, dim, entryPointRegister );
}

void _ForceVector_Build( void* forceVector, void* data ) {
   ForceVector* self = (ForceVector*)forceVector;

   _SolutionVector_Build( self, data );

   /* update the size depending on our now built feVariable */
   self->localSize = self->eqNum->localEqNumsOwnedCount;

   Stream_IndentBranch( StgFEM_Debug );
   Journal_DPrintfL( self->debug, 2, "Allocating the L.A. Force Vector with %d local entries.\n", self->localSize );
   Stream_UnIndentBranch( StgFEM_Debug );

}


void _ForceVector_Initialise( void* forceVector, void* data ) {
	ForceVector* self = (ForceVector*)forceVector;

	_SolutionVector_Initialise( self, data );
}

void _ForceVector_Execute( void* forceVector, void* data ) {
}

void _ForceVector_Destroy( void* forceVector, void* data ) {
	ForceVector* self = (ForceVector*)forceVector;

	Memory_Free( self->_assembleForceVectorEPName );

	/* Don't delete entry point: E.P register will delete it automatically */
	Stg_Class_Delete( self->forceTermList );

	Stg_Class_Delete( self->inc );

	_SolutionVector_Destroy( self, data );
}

void ForceVector_Assemble( void* forceVector ) {
	ForceVector* self = (ForceVector*)forceVector;
    int ii;

	((FeEntryPoint_AssembleForceVector_CallFunction*)EntryPoint_GetRun( self->assembleForceVector ))(
		self->assembleForceVector,
		self );

}

void ForceVector_PrintElementForceVector(
	ForceVector* self,
	Element_LocalIndex element_lI,
	Dof_EquationNumber** elementLM,
	double* elForceVecToAdd )
{
	DofLayout*		dofLayout = self->feVariable->dofLayout;
	FeMesh*			feMesh = self->feVariable->feMesh;
	unsigned		nInc, *inc;
	Dof_Index		dofsPerNode;
	Node_LocalIndex		nodesThisEl;
	Node_LocalIndex		node_I;
	Dof_Index		dof_I;
	Index			vec_I;
	IArray*			incArray;

	incArray = IArray_New();
	FeMesh_GetElementNodes( feMesh, element_lI, incArray );
	nInc = IArray_GetSize( incArray );
	inc = IArray_GetPtr( incArray );
	dofsPerNode = dofLayout->dofCounts[inc[0]];
	nodesThisEl = nInc;

	for ( node_I=0; node_I < nodesThisEl; node_I++ ) {
		for ( dof_I = 0; dof_I < dofsPerNode; dof_I++ ) {
			vec_I = node_I * dofsPerNode + dof_I;

			Journal_DPrintf( self->debug, "Entry[%d][%d] (LM (%4d)) = %.3f\n",
					 node_I, dof_I,
					 elementLM[node_I][dof_I],
					 elForceVecToAdd[vec_I] );
		}
	}

	Stg_Class_Delete( incArray );
}

/* from the depreciated Vector class */
void _ForceVector_VectorView( Vec v, Stream* stream ) {
	unsigned	entry_i;
	PetscInt	size;
	PetscScalar*	array;

	VecGetSize( v, &size );
	VecGetArray( v, &array );

	Journal_Printf( stream, "%p = [", v );
	for( entry_i = 0; entry_i < size; entry_i++ )
		Journal_Printf( stream, "\t%u: \t %.12g\n", entry_i, array[entry_i] );
	Journal_Printf( stream, "];\n" );

	VecRestoreArray( v, &array );
}

void ForceVector_GlobalAssembly_General( void* forceVector ) {
	ForceVector*            self                 = (ForceVector*) forceVector;
	FeVariable*             feVar                = self->feVariable;
  FeEquationNumber*       eqNum                = self->eqNum;
	Element_LocalIndex      element_lI;
	Element_LocalIndex      elementLocalCount;
	Node_ElementLocalIndex  nodeCountCurrElement = 0;
	Element_Nodes           nodeIdsInCurrElement = 0;
	Dof_Index               totalDofsThisElement = 0;
	Dof_Index               totalDofsPrevElement = 0;
	Dof_Index               dofCountLastNode     = 0;
	Dof_EquationNumber**    elementLM            = NULL;
	double*                 elForceVecToAdd      = NULL;
	/* For output printing */
	double                  outputPercentage=10;	/* Controls how often to give a status update of assembly progress */
	int                     outputInterval;

	Journal_DPrintf( self->debug, "In %s - for vector \"%s\"\n", __func__, self->name );

	Stream_IndentBranch( StgFEM_Debug );

	if ( Stg_ObjectList_Count( self->forceTermList ) > 0 ) {
		elementLocalCount = FeMesh_GetElementLocalSize( feVar->feMesh );

		/* Initialise Vector */
		outputInterval = (int)( (outputPercentage/100.0)*(double)(elementLocalCount) );
		if( outputInterval == 0 ) { outputInterval = elementLocalCount; }

		for( element_lI = 0; element_lI < elementLocalCount; element_lI++ ) {
			unsigned	nInc, *inc;

			FeMesh_GetElementNodes( feVar->feMesh, element_lI, self->inc );
			nInc = IArray_GetSize( self->inc );
			inc = IArray_GetPtr( self->inc );
			nodeCountCurrElement = nInc;
			/* Get the local node ids */
			nodeIdsInCurrElement = inc;

			/* Set value of elementLM: will automatically just index into global LM table if built */
			elementLM = eqNum->locationMatrix[element_lI];

			/* work out number of dofs at the node, using LM */
			/* Since: Number of entries in LM table for this element = (by defn.) Number of dofs this element */
			dofCountLastNode = feVar->dofLayout->dofCounts[nodeIdsInCurrElement[nodeCountCurrElement-1]];
			totalDofsThisElement = &elementLM[nodeCountCurrElement-1][dofCountLastNode-1] - &elementLM[0][0] + 1;

			if ( totalDofsThisElement > totalDofsPrevElement ) {
				if (elForceVecToAdd) Memory_Free( elForceVecToAdd );
				Journal_DPrintfL( self->debug, 2, "Reallocating elForceVecToAdd to size %d\n", totalDofsThisElement );
				elForceVecToAdd = Memory_Alloc_Array( double, totalDofsThisElement, "elForceVecToAdd" );
			}

			/* Initialise Values to Zero */
			memset( elForceVecToAdd, 0, totalDofsThisElement * sizeof(double) );

			/* Assemble this element's element force vector: going through each force term in list */
			ForceVector_AssembleElement( self, element_lI, elForceVecToAdd );


	        /* When keeping BCs in we come across a bit of a problem in parallel. We're not
	           allowed to add entries to the force vector here and then clobber it later with
	           an insert in order to set the BC. So, what we'll do is just add zero here, that
	           way later we can add the BC and it will be the same as inserting it.
	           --- Luke, 20 May 2008 */
	        if( !eqNum->removeBCs ) {
	           DofLayout* dofs;
	           int nDofs, curInd;
	           int ii, jj;

	           dofs = feVar->dofLayout; /* shortcut to the dof layout */
	           curInd = 0; /* need a counter to track where we are in the element force vector */
	           for( ii = 0; ii < nodeCountCurrElement; ii++ ) {
	              nDofs = dofs->dofCounts[inc[ii]]; /* number of dofs on this node */
	              for( jj = 0; jj < nDofs; jj++ ) {
	                 if( !FeVariable_IsBC( feVar, inc[ii], jj ) ) {
	                    curInd++;
	                    continue; /* only need to clear it if it's a bc */
	                 }
	                 elForceVecToAdd[curInd] = 0.0;
	                 curInd++;
	              }
	           }
	        }

			/* Ok, assemble into global matrix */
			//Vector_AddEntries( self->vector, totalDofsThisElement, (Index*)(elementLM[0]), elForceVecToAdd );
			VecSetValues( self->vector, totalDofsThisElement, (PetscInt*)elementLM[0], elForceVecToAdd, ADD_VALUES );

			/* Cleanup: If we haven't built the big LM for all elements, free the temporary one */
			if ( False == eqNum->locationMatrixBuilt ) {
				Memory_Free( elementLM );
			}
			totalDofsPrevElement = totalDofsThisElement;
		}

		Memory_Free( elForceVecToAdd );
	}
	else {
		Journal_DPrintf( self->debug, "No ForceTerms registered - returning.\n" );
	}

	/* If we're keeping BCs, insert them into the force vector. */
	if( !eqNum->removeBCs ) {
    FeMesh *mesh = feVar->feMesh;
    DofLayout* dofs = feVar->dofLayout;
    unsigned n_i, dof_i, nDofs, nNodes;
    int rowEq;
    double	bc;

    dofs = feVar->dofLayout;
    nNodes = FeMesh_GetNodeLocalSize( mesh );
    assert( eqNum->destinationArray );
    assert( dofs->dofCounts );

    // loop over dofs
    for( n_i = 0; n_i < nNodes; n_i++ ) {
      assert( eqNum->destinationArray[n_i] );
      nDofs = dofs->dofCounts[n_i];

      for( dof_i = 0; dof_i < nDofs; dof_i++ ) {
        rowEq = eqNum->destinationArray[n_i][dof_i];

        if( feVar->bcs && FeVariable_IsBC( feVar, n_i, dof_i ) ) {
          bc = DofLayout_GetValueDouble( dofs, n_i, dof_i );
          VecSetValues( self->vector, 1, &rowEq, &bc, ADD_VALUES );
        }
      }
    }
  }

  VecAssemblyBegin( self->vector );
  VecAssemblyEnd( self->vector );
}

void ForceVector_AssembleElement( void* forceVector, Element_LocalIndex element_lI, double* elForceVecToAdd ) {
	ForceVector*            self                 = (ForceVector*) forceVector;
	Index                   forceTermCount       = Stg_ObjectList_Count( self->forceTermList );
	Index                   forceTerm_I;
	ForceTerm*              forceTerm;

	for ( forceTerm_I = 0 ; forceTerm_I < forceTermCount ; forceTerm_I++ ) {
		forceTerm = (ForceTerm*) Stg_ObjectList_At( self->forceTermList, forceTerm_I );

		ForceTerm_AssembleElement( forceTerm, self, element_lI, elForceVecToAdd );
	}
}

void ForceVector_AddForceTerm( void* forceVector, void* forceTerm ) {
	ForceVector*            self                 = (ForceVector*) forceVector;

	Stg_ObjectList_Append( self->forceTermList, Stg_CheckType( forceTerm, ForceTerm ) );
}

void ForceVector_Zero( void* forceVector ) {
	ForceVector* self = (ForceVector*) forceVector;
    VecSet( self->vector, 0.0 );

}
