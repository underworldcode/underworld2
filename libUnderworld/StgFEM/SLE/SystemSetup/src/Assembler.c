/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "Discretisation/Discretisation.h"
#include "SystemSetup.h"


/* Textual name of this class */
const Type Assembler_Type = "Assembler";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Assembler* Assembler_New() {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Assembler);
	Type                              type = Assembler_Type;
	Stg_Class_DeleteFunction*      _delete = _Assembler_Delete;
	Stg_Class_PrintFunction*        _print = _Assembler_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

        return _Assembler_New(  ASSEMBLER_PASSARGS  );
}

Assembler* _Assembler_New(  ASSEMBLER_DEFARGS  ) {
	Assembler*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Assembler) );
	self = (Assembler*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */

	/* Assembler info */
	_Assembler_Init( self );

	return self;
}

void _Assembler_Init( Assembler* self ) {
	assert( self && Stg_CheckType( self, Assembler ) );

	self->rowVar = NULL;
	self->colVar = NULL;
	self->swarm = NULL;
	self->rowRCB = NULL;
	self->rowUCB = NULL;
	self->colRCB = NULL;
	self->colUCB = NULL;
	self->obj = NULL;

	self->elInd = 0;
	self->particle = NULL;
	self->shapeFuncs = NULL;
	self->detJac = 0.0;
	self->globalDerivs = NULL;
	self->rowInd = 0;
	self->rowNodeInd = 0;
	self->rowDofInd = 0;
	self->rowEq = 0;
	self->colInd = 0;
	self->colNodeInd = 0;
	self->colDofInd = 0;
	self->colEq = 0;

	self->rowInc = IArray_New();
	self->colInc = IArray_New();
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Assembler_Delete( void* assembler ) {
	Assembler*	self = (Assembler*)assembler;

	assert( self && Stg_CheckType( self, Assembler ) );

	FreeArray( self->shapeFuncs );
	FreeArray( self->globalDerivs );
	Stg_Class_Delete( self->rowInc );
	Stg_Class_Delete( self->colInc );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _Assembler_Print( void* assembler, Stream* stream ) {
	Assembler*	self = (Assembler*)assembler;
	Stream* 	assemblerStream;

	assert( self && Stg_CheckType( self, Assembler ) );
	
	/* Set the Journal for printing informations */
	assemblerStream = Journal_Register( InfoStream_Type, (Name)"AssemblerStream"  );

	/* Print parent */
	Journal_Printf( stream, "Assembler (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Assembler_SetVariables( void* assembler, FeVariable* rowVar, FeVariable* columnVar ) {
	Assembler*	self = (Assembler*)assembler;

	assert( self && Stg_CheckType( self, Assembler ) );
	assert( rowVar || columnVar );

	self->rowVar = rowVar ? rowVar : columnVar;
	self->colVar = columnVar ? columnVar : rowVar;
	Assembler_Update( self );
}

void Assembler_SetIntegrationSwarm( void* assembler, Swarm* swarm ) {
	Assembler*	self = (Assembler*)assembler;

	assert( self && Stg_CheckType( self, Assembler ) );

	self->swarm = swarm;
}

void Assembler_SetCallbacks( void* assembler, 
			     Assembler_CallbackType* particle, 
                             Assembler_CallbackType* rowRestricted, 
                             Assembler_CallbackType* rowUnrestricted, 
                             Assembler_CallbackType* colRestricted, 
                             Assembler_CallbackType* colUnrestricted, 
                             void* object )
{
        Assembler*      self = (Assembler*)assembler;

        assert( self && Stg_CheckType( self, Assembler ) );

	self->partCB = particle;
        self->rowRCB = rowRestricted;
        self->rowUCB = rowUnrestricted;
        self->colRCB = colRestricted;
        self->colUCB = colUnrestricted;
        self->obj = object;
}

void Assembler_Update( void* assembler ) {
	Assembler*	self = (Assembler*)assembler;
	unsigned	nDerivs, nDims;
	unsigned	e_i;

	assert( self && Stg_CheckType( self, Assembler ) );

	nDerivs = 0;
	nDims = 0;
	if( self->rowVar ) {
		FeMesh*	mesh = self->rowVar->feMesh;

		nDims = Mesh_GetDimSize( mesh );
		for( e_i = 0; e_i < FeMesh_GetElementLocalSize( mesh ); e_i++ ) {
			if( FeMesh_GetElementNodeSize( mesh, e_i ) > nDerivs )
				nDerivs = FeMesh_GetElementNodeSize( mesh, e_i );
		}
	}
	self->shapeFuncs = ReallocArray( self->shapeFuncs, double, nDerivs );
	self->globalDerivs = ReallocArray2D( self->globalDerivs, double, nDims, nDerivs );
}

void Assembler_IntegrateMatrixElement( void* assembler, unsigned element ) {
	Assembler*		self = (Assembler*)assembler;
	FeMesh*			mesh;
	Swarm*			swarm;
	unsigned		nDims;
	unsigned		cellInd, nParticles;
	ElementType*		elType;
	unsigned		p_i;

	assert( self && Stg_CheckType( self, Assembler ) );
	assert( self->rowVar && self->colVar );

	mesh = self->rowVar->feMesh;
	nDims = Mesh_GetDimSize( mesh );
	swarm = self->swarm;	assert( swarm );
	assert( swarm->cellLayout );
	assert( swarm->cellParticleCountTbl );
	cellInd = CellLayout_MapElementIdToCellId( swarm->cellLayout, element );
	nParticles = swarm->cellParticleCountTbl[cellInd];
	elType = FeMesh_GetElementType( mesh, element );

	for( p_i = 0; p_i < nParticles; p_i++ ) {
		self->particle = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cellInd, p_i );
		assert( self->particle );
		ElementType_EvaluateShapeFunctionsAt( elType, self->particle->xi, self->shapeFuncs );
		ElementType_ShapeFunctionsGlobalDerivs( elType, mesh, element, self->particle->xi, nDims, 
							&self->detJac, self->globalDerivs );
		if( self->partCB && !self->partCB( self->obj, self ) )
			continue;

		Assembler_LoopMatrixElement( self, element );
	}
}

void Assembler_LoopMatrixElement( void* assembler, unsigned element ) {
	Assembler*		self = (Assembler*)assembler;
	unsigned		nDims;
	FeVariable		*rowVar, *colVar;
	FeMesh			*rowMesh, *colMesh;
	FeEquationNumber	*rowEqNum, *colEqNum;
	DofLayout		*rowDofs, *colDofs;
	unsigned		nRowDofs, nColDofs;
	unsigned		nRowElNodes, *rowElNodes;
	unsigned		nColElNodes, *colElNodes;
	int			rowEq, colEq;
	unsigned		rowInd, colInd;
	unsigned		n_i, n_j, dof_i, dof_j;

	assert( self && Stg_CheckType( self, Assembler ) );
	assert( self->rowVar && self->colVar );

	self->elInd = element;
	rowVar = self->rowVar;
	colVar = self->colVar;
	rowEqNum = rowVar->eqNum;			assert( rowEqNum );
	colEqNum = colVar->eqNum;			assert( colEqNum );
	rowMesh = rowVar->feMesh;
	colMesh = colVar->feMesh;
	rowDofs = rowVar->dofLayout;			assert( rowDofs );
	colDofs = colVar->dofLayout;			assert( colDofs );
	nDims = Mesh_GetDimSize( rowMesh );		assert( nDims );
	Mesh_GetIncidence( rowMesh, nDims, element, MT_VERTEX, self->rowInc );
	nRowElNodes = IArray_GetSize( self->rowInc );
	rowElNodes = IArray_GetPtr( self->rowInc );
	Mesh_GetIncidence( colMesh, Mesh_GetDimSize( colMesh ), element, MT_VERTEX, 
			   self->colInc );
	nColElNodes = IArray_GetSize( self->colInc );
	colElNodes = IArray_GetPtr( self->colInc );
	assert( FeMesh_GetElementLocalSize( rowMesh ) == FeMesh_GetElementLocalSize( colMesh ) );
	assert( nDims == Mesh_GetDimSize( colMesh ) );
	assert( rowEqNum->locationMatrix );
	assert( colEqNum->locationMatrix );
	assert( rowEqNum->locationMatrix[element] );
	assert( colEqNum->locationMatrix[element] );
	assert( rowDofs->dofCounts );
	assert( colDofs->dofCounts );

	rowInd = 0;
	for( n_i = 0; n_i < nRowElNodes; n_i++ ) {
		assert( rowEqNum->locationMatrix[element][n_i] );
		nRowDofs = rowDofs->dofCounts[rowElNodes[n_i]];
		for( dof_i = 0; dof_i < nRowDofs; dof_i++ ) {
			rowEq = rowEqNum->locationMatrix[element][n_i][dof_i];

			self->rowInd = rowInd++;
			self->rowElNodeInd = n_i;
			self->rowNodeInd = rowElNodes[n_i];
			self->rowDofInd = dof_i;
			self->rowEq = rowEq;
			if( rowVar->bcs && FeVariable_IsBC( rowVar, rowElNodes[n_i], dof_i ) ) {
				if( !self->rowRCB || !self->rowRCB( self->obj, self ) )
					continue;
			}
			else if( self->rowUCB && !self->rowUCB( self->obj, self ) )
				continue;

			colInd = 0;
			for( n_j = 0; n_j < nColElNodes; n_j++ ) {
				assert( colEqNum->locationMatrix[element][n_j] );
				nColDofs = colDofs->dofCounts[colElNodes[n_j]];
				for( dof_j = 0; dof_j < nColDofs; dof_j++ ) {
					colEq = colEqNum->locationMatrix[element][n_j][dof_j];

					self->colInd = colInd++;
					self->colElNodeInd = n_j;
					self->colNodeInd = colElNodes[n_j];
					self->colDofInd = dof_j;
					self->colEq = colEq;
					if( colVar->bcs && FeVariable_IsBC( colVar, colElNodes[n_j], dof_j ) ) {
						if( self->colRCB )
							self->colRCB( self->obj, self );
					}
					else if( self->colUCB )
						self->colUCB( self->obj, self );
				}
			}
		}
	}
}

void Assembler_LoopMatrixDiagonal( void* assembler ) {
	Assembler*		self = (Assembler*)assembler;
	unsigned		nDims;
	FeVariable		*rowVar;
	FeMesh			*rowMesh;
	FeEquationNumber	*rowEqNum;
	DofLayout		*rowDofs;
	unsigned		nRowDofs;
	unsigned		nRowNodes;
	int			rowEq;
	unsigned		n_i, dof_i;

	assert( self && Stg_CheckType( self, Assembler ) );
	assert( self->rowVar && self->colVar );
	assert( self->rowVar == self->colVar );

	rowVar = self->rowVar;
	rowEqNum = rowVar->eqNum;			assert( rowEqNum );
	rowMesh = rowVar->feMesh;
	rowDofs = rowVar->dofLayout;			assert( rowDofs );
	nDims = Mesh_GetDimSize( rowMesh );		assert( nDims );
	nRowNodes = FeMesh_GetNodeLocalSize( rowMesh );
	assert( rowEqNum->destinationArray );
	assert( rowDofs->dofCounts );

	for( n_i = 0; n_i < nRowNodes; n_i++ ) {
		assert( rowEqNum->destinationArray[n_i] );
		nRowDofs = rowDofs->dofCounts[n_i];
		for( dof_i = 0; dof_i < nRowDofs; dof_i++ ) {
			rowEq = rowEqNum->destinationArray[n_i][dof_i];

			self->rowNodeInd = n_i;
			self->rowDofInd = dof_i;
			self->rowEq = rowEq;
			self->colNodeInd = n_i;
			self->colDofInd = dof_i;
			self->colEq = rowEq;
			if( rowVar->bcs && FeVariable_IsBC( rowVar, n_i, dof_i ) ) {
				if( self->rowRCB )
					self->rowRCB( self->obj, self );
			}
			else if( self->rowUCB )
				self->rowUCB( self->obj, self );
		}
	}
}

#if 0
void Assembler_LoopVectorElement( void* assembler, unsigned element ) {
	Assembler*		self = (Assembler*)assembler;
	unsigned		nDims;
	FeVariable		*rowVar;
	FeMesh			*rowMesh;
	FeEquationNumber	*rowEqNum;
	DofLayout		*rowDofs;
	unsigned		nRowDofs;
	unsigned		nRowElNodes, *rowElNodes;
	int			rowEq;
	unsigned		n_i, dof_i;

	assert( self && Stg_CheckType( self, Assembler ) );
	assert( self->rowVar && self->colVar );
	assert( self->colCB );

	self->elInd = element;
	rowVar = self->rowVar;
	rowEqNum = rowVar->eqNum;			assert( rowEqNum );
	rowMesh = rowVar->feMesh;
	rowDofs = rowVar->dofLayout;			assert( rowDofs );
	nDims = Mesh_GetDimSize( rowMesh );		assert( nDims );
	Mesh_GetIncidence( rowMesh, nDims, element, MT_VERTEX, self->rowInc );
	nRowElNodes = IArray_GetSize( self->rowInc );
	rowElNodes = IArray_GetPtr( self->rowInc );
	assert( rowEqNum->locationMatrix );
	assert( rowEqNum->locationMatrix[element] );
	assert( rowDofs->dofCounts );

	for( n_i = 0; n_i < nRowElNodes; n_i++ ) {
		assert( rowEqNum->locationMatrix[element][n_i] );
		nRowDofs = rowDofs->dofCounts[rowElNodes[n_i]];
		for( dof_i = 0; dof_i < nRowDofs; dof_i++ ) {
			rowEq = rowEqNum->locationMatrix[element][n_i][dof_i];

			self->rowNodeInd = n_i;
			self->rowDofInd = dof_i;
			self->rowEq = rowEq;
			self->colCB( self );
		}
	}
}
#endif


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


