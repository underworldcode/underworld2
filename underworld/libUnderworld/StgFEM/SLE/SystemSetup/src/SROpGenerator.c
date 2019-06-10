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
#include <StgFEM/Discretisation/Discretisation.h>

#include "SystemSetup.h"

#include "petsc.h"
#include "petscvec.h"
#include "petscmat.h"

/* Textual name of this class */
const Type SROpGenerator_Type = "SROpGenerator";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

SROpGenerator* SROpGenerator_New( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(SROpGenerator);
	Type                                                      type = SROpGenerator_Type;
	Stg_Class_DeleteFunction*                              _delete = _SROpGenerator_Delete;
	Stg_Class_PrintFunction*                                _print = _SROpGenerator_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))SROpGenerator_New;
	Stg_Component_ConstructFunction*                    _construct = _SROpGenerator_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _SROpGenerator_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _SROpGenerator_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _SROpGenerator_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _SROpGenerator_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	MGOpGenerator_SetNumLevelsFunc*               setNumLevelsFunc = _MGOpGenerator_SetNumLevels;
	MGOpGenerator_HasExpiredFunc*                   hasExpiredFunc = SROpGenerator_HasExpired;
	MGOpGenerator_GenerateFunc*                       generateFunc = SROpGenerator_Generate;

	return _SROpGenerator_New(  SROPGENERATOR_PASSARGS  );
}

SROpGenerator* _SROpGenerator_New(  SROPGENERATOR_DEFARGS  ) {
	SROpGenerator*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SROpGenerator) );
	self = (SROpGenerator*)_MGOpGenerator_New(  MGOPGENERATOR_PASSARGS  );

	/* Virtual info */

	/* SROpGenerator info */
	_SROpGenerator_Init( self );

	return self;
}

void _SROpGenerator_Init( SROpGenerator* self ) {
	assert( self && Stg_CheckType( self, SROpGenerator ) );

	self->fineVar = NULL;
	self->fineEqNum = NULL;
	self->meshes = NULL;
	self->topMaps = NULL;
	self->eqNums = NULL;
	self->nLocalEqNums = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _SROpGenerator_Delete( void* srOpGenerator ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	/* Delete the parent. */
	_MGOpGenerator_Delete( self );
}

void _SROpGenerator_Print( void* srOpGenerator, Stream* stream ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;

	/* Set the Journal for printing informations */
	Stream* srOpGeneratorStream;
	srOpGeneratorStream = Journal_Register( InfoStream_Type, (Name)"SROpGeneratorStream"  );

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	/* Print parent */
	Journal_Printf( stream, "SROpGenerator (ptr): (%p)\n", self );
	_MGOpGenerator_Print( self, stream );
}

void _SROpGenerator_AssignFromXML( void* srOpGenerator, Stg_ComponentFactory* cf, void* data ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;
	FeVariable*	var;

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	var = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"fineVariable", FeVariable, True, data  );
	SROpGenerator_SetFineVariable( self, var );
}

void _SROpGenerator_Build( void* srOpGenerator, void* data ) {
}

void _SROpGenerator_Initialise( void* srOpGenerator, void* data ) {
}

void _SROpGenerator_Execute( void* srOpGenerator, void* data ) {
}

void _SROpGenerator_Destroy( void* srOpGenerator, void* data ) {
}

Bool SROpGenerator_HasExpired( void* srOpGenerator ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	return False;
}

//void SROpGenerator_Generate( void* srOpGenerator, Matrix*** pOps, Matrix*** rOps ) {
void SROpGenerator_Generate( void* srOpGenerator, Mat** pOps, Mat** rOps ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( pOps && rOps );

	//*pOps = AllocArray( Matrix*, self->nLevels );
	//*rOps = AllocArray( Matrix*, self->nLevels );
	//memset( *pOps, 0, self->nLevels * sizeof(Matrix*) );
	//memset( *rOps, 0, self->nLevels * sizeof(Matrix*) );
	*pOps = AllocArray( Mat, self->nLevels );
	*rOps = AllocArray( Mat, self->nLevels );
	memset( *pOps, 0, self->nLevels * sizeof(Mat) );
	memset( *rOps, 0, self->nLevels * sizeof(Mat) );

/*
	self->fineEqNum = self->fineVar->eqNum;
	SROpGenerator_GenMeshes( self );
	SROpGenerator_GenOps( self, *pOps, *rOps );
	SROpGenerator_DestructMeshes( self );
*/
        SROpGenerator_Simple( self, *pOps, *rOps );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void SROpGenerator_SetFineVariable( void* srOpGenerator, void* _variable ) {
	SROpGenerator*	self = (SROpGenerator*)srOpGenerator;
	FeVariable*	variable = (FeVariable*)_variable;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( !variable || Stg_CheckType( variable, FeVariable ) );

	SROpGenerator_DestructMeshes( self );
	self->fineVar = variable;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void SROpGenerator_GenMeshes( SROpGenerator* self ) {
	unsigned	nLevels;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	nLevels = self->nLevels;
	self->meshes = AllocArray( Mesh*, nLevels );
	memset( self->meshes, 0, nLevels * sizeof(Mesh*) );
	self->topMaps = AllocArray( unsigned*, nLevels );
	memset( self->topMaps, 0, nLevels * sizeof(unsigned*) );
	self->eqNums = AllocArray( unsigned**, nLevels );
	memset( self->eqNums, 0, nLevels * sizeof(unsigned**) );
	self->nLocalEqNums = AllocArray( unsigned, nLevels );
	memset( self->nLocalEqNums, 0, nLevels * sizeof(unsigned) );
	self->eqNumBases = AllocArray( unsigned, nLevels );
	memset( self->eqNumBases, 0, nLevels * sizeof(unsigned) );

	self->meshes[nLevels - 1] = (Mesh*)self->fineEqNum->feMesh;
	self->eqNumBases[nLevels - 1] = self->fineEqNum->firstOwnedEqNum;
	for( l_i = nLevels - 2; l_i < nLevels; l_i-- ) {
		SROpGenerator_GenLevelMesh( self, l_i );
		SROpGenerator_GenLevelTopMap( self, l_i );
		SROpGenerator_GenLevelEqNums( self, l_i );
	}
}

void SROpGenerator_GenLevelMesh( SROpGenerator* self, unsigned level ) {
	Stream*			errorStream = Journal_Register( ErrorStream_Type, (Name)"SROpGenerator::GenLevelMesh"  );
	Mesh			*fMesh, *cMesh;
	CartesianGenerator	*fGen, *cGen;
	unsigned		nDims;
	unsigned*		cSize;
	double			crdMin[3], crdMax[3];
	unsigned		d_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( self->meshes );
	assert( level < self->nLevels );

	fMesh = self->meshes[level + 1];
	nDims = Mesh_GetDimSize( fMesh );
	fGen = (CartesianGenerator*)fMesh->generator;
	Journal_Firewall( fGen && !strcmp( fGen->type, CartesianGenerator_Type ),
			  errorStream,
			  "\n" \
			  "****************************************************************\n" \
			  "* Error: Simple regular multigrid operator generation requires *\n" \
			  "*        a fine mesh that has been generated with a            *\n" \
			  "*        cartesian generator.                                  *\n" \
			  "****************************************************************\n" \
			  "\n" );

	cGen = CartesianGenerator_New( "", NULL );
	CartesianGenerator_SetDimSize( cGen, nDims );
	cSize = AllocArray( unsigned, nDims );
	for( d_i = 0; d_i < nDims; d_i++ )
		cSize[d_i] = fGen->elGrid->sizes[d_i] / 2;
	CartesianGenerator_SetTopologyParams( cGen, cSize, fGen->maxDecompDims, fGen->minDecomp, fGen->maxDecomp );
	Mesh_GetGlobalCoordRange( fMesh, crdMin, crdMax );
	CartesianGenerator_SetGeometryParams( cGen, crdMin, crdMax );
	CartesianGenerator_SetShadowDepth( cGen, 0 );
	FreeArray( cSize );

	cMesh = (Mesh*)FeMesh_New( "" );
	Mesh_SetGenerator( cMesh, cGen );
	FeMesh_SetElementFamily( cMesh, ((FeMesh*)fMesh)->feElFamily );
	Stg_Component_Build( cMesh, NULL, False );
	Stg_Component_Initialise( cMesh, NULL, False );
	self->meshes[level] = cMesh;
}

void SROpGenerator_GenLevelTopMap( SROpGenerator* self, unsigned level ) {
	Stream*		errorStream = Journal_Register( ErrorStream_Type, (Name)"SROpGenerator::GenLevelTopMap"  );
	Mesh		*fMesh, *cMesh;
	unsigned	nDomainNodes;
	unsigned	nLevels;
	unsigned	nDims;
	unsigned	nearest;
	double		*cVert, *fVert;
	unsigned	d_i, n_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( self->meshes );
	assert( level < self->nLevels );

	fMesh = self->meshes[level + 1];
	cMesh = self->meshes[level];
	nDims = Mesh_GetDimSize( fMesh );
	nLevels = self->nLevels;
	nDomainNodes = Mesh_GetDomainSize( cMesh, MT_VERTEX );
	self->topMaps[level] = ReallocArray( self->topMaps[level], unsigned, nDomainNodes );
	for( n_i = 0; n_i < nDomainNodes; n_i++ ) {
		cVert = Mesh_GetVertex( cMesh, n_i );
		nearest = Mesh_NearestVertex( fMesh, cVert );
		fVert = Mesh_GetVertex( fMesh, nearest );
		for( d_i = 0; d_i < nDims; d_i++ ) {
			if( !Num_Approx( cVert[d_i], fVert[d_i] ) )
				break;
		}

		Journal_Firewall( d_i == nDims,
				  errorStream,
				  "\n" \
				  "*****************************************************************\n" \
				  "* Error: The finest grid could not be sub-sampled to all coarse *\n" \
				  "*        levels. This is due to the size of the finest grid.    *\n" \
				  "*****************************************************************\n" \
				  "\n" );

		if( level < nLevels - 2 )
			self->topMaps[level][n_i] = self->topMaps[level + 1][nearest];
		else
			self->topMaps[level][n_i] = nearest;
	}
}

void SROpGenerator_GenLevelEqNums( SROpGenerator* self, unsigned level ) {
	Mesh*			cMesh;
	unsigned*		nNodalDofs;
	unsigned		nDomainNodes, nLocalNodes;
	DofLayout*		dofLayout;
	unsigned**		dstArray;
	unsigned		curEqNum;
	unsigned		maxDofs;
	unsigned		topNode;
	unsigned		base, subTotal;
	MPI_Comm		comm;
	unsigned		nProcs, rank;
	MPI_Status		status;
	unsigned*		tuples;
	Sync*			sync;
	unsigned		n_i, dof_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( self->meshes && self->topMaps && self->eqNums );
	assert( level < self->nLevels );

	cMesh = self->meshes[level];
	nDomainNodes = Mesh_GetDomainSize( cMesh, MT_VERTEX );
	nLocalNodes = Mesh_GetLocalSize( cMesh, MT_VERTEX );
	dofLayout = self->fineEqNum->dofLayout;
	comm = Comm_GetMPIComm( Mesh_GetCommTopology( cMesh, MT_VERTEX ) );
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/* Allocate for destination array. */
	nNodalDofs = AllocArray( unsigned, nDomainNodes );
	for( n_i = 0; n_i < nDomainNodes; n_i++ )
		nNodalDofs[n_i] = dofLayout->dofCounts[self->topMaps[level][n_i]];
	dstArray = AllocComplex2D( unsigned, nDomainNodes, nNodalDofs );

	/* Build initial destination array and store max dofs. */
	curEqNum = 0;
	maxDofs = 0;
	for( n_i = 0; n_i < nLocalNodes; n_i++ ) {
		if( nNodalDofs[n_i] > maxDofs )
			maxDofs = nNodalDofs[n_i];

		topNode = self->topMaps[level][n_i];
		for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
			if( self->fineEqNum->mapNodeDof2Eq[topNode][dof_i] != (unsigned)-1 )
				dstArray[n_i][dof_i] = curEqNum++;
			else
				dstArray[n_i][dof_i] = (unsigned)-1;
		}
	}

	/* Order the equation numbers based on processor rank; cascade counts forward. */
	base = 0;
	subTotal = curEqNum;
	if( rank > 0 ) {
		insist( MPI_Recv( &base, 1, MPI_UNSIGNED, rank - 1, 6669, comm, &status ), == MPI_SUCCESS );
		subTotal = base + curEqNum;
	}
	if( rank < nProcs - 1 )
		insist( MPI_Send( &subTotal, 1, MPI_UNSIGNED, rank + 1, 6669, comm ), == MPI_SUCCESS );

	/* Modify existing destination array and dump to a tuple array. */
	tuples = AllocArray( unsigned, nDomainNodes * maxDofs );
	for( n_i = 0; n_i < nLocalNodes; n_i++ ) {
		for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
			if( dstArray[n_i][dof_i] != (unsigned)-1 )
				dstArray[n_i][dof_i] += base;
			tuples[n_i * maxDofs + dof_i] = dstArray[n_i][dof_i];
		}
	}

	/* Update all other procs. */
	sync = Mesh_GetSync( cMesh, MT_VERTEX );
	Sync_SyncArray( sync, tuples, maxDofs * sizeof(unsigned),
			tuples + nLocalNodes * maxDofs, maxDofs * sizeof(unsigned),
			maxDofs * sizeof(unsigned) );

	/* Update destination array's domain indices. */
	for( n_i = nLocalNodes; n_i < nDomainNodes; n_i++ ) {
		topNode = self->topMaps[level][n_i];
		for( dof_i = 0; dof_i < nNodalDofs[n_i]; dof_i++ ) {
			if( self->fineEqNum->mapNodeDof2Eq[topNode][dof_i] != (unsigned)-1 )
				dstArray[n_i][dof_i] = tuples[n_i * maxDofs + dof_i];
			else
				dstArray[n_i][dof_i] = -1;
		}
	}

	/* Destroy arrays. */
	FreeArray( nNodalDofs );
	FreeArray( tuples );

	self->eqNums[level] = dstArray;
	self->nLocalEqNums[level] = curEqNum;
	self->eqNumBases[level] = base;
}

//void SROpGenerator_GenOps( SROpGenerator* self, Matrix** pOps, Matrix** rOps ) {
void SROpGenerator_GenOps( SROpGenerator* self, Mat* pOps, Mat* rOps ) {
	unsigned	nLevels;
	//Matrix		*fineMat, *P;
	Mat		fineMat, P;
	unsigned	nRows, nCols;
	unsigned	l_i;
	/* unsigned	nProcs; */

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( pOps && rOps );

	//fineMat = MatrixSolver_GetMatrix( self->solver );
	fineMat = self->solver->matrix;
	nLevels = self->nLevels;

	for( l_i = nLevels - 1; l_i > 0; l_i-- ) {
		nRows = self->eqNums[l_i] ? self->nLocalEqNums[l_i] :
			self->fineEqNum->localEqNumsOwnedCount;
		nCols = self->nLocalEqNums[l_i - 1];

		//Matrix_Duplicate( fineMat, (void**)&P );
		//Matrix_SetComm( P, fineMat->comm );
		//Matrix_SetLocalSize( P, nRows, nCols );
		MatCreate( self->solver->comm, &P );
		MatSetSizes( P, nRows, nCols, PETSC_DETERMINE, PETSC_DETERMINE );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
                MatSetUp(P);
#endif
		MatSetFromOptions( P );
		//if( !strcmp( P->type, PETScMatrix_Type ) ) {
		//	unsigned	*nDiagNonZeros, *nOffDiagNonZeros;
		//
		//	SROpGenerator_CalcOpNonZeros( self, l_i, &nDiagNonZeros, &nOffDiagNonZeros );
			//PETScMatrix_SetNonZeroStructure( (PETScMatrix*)P, 0, nDiagNonZeros, nOffDiagNonZeros );
		//	MPI_Comm_size( self->solver->comm, &nProcs );
		//	if( nProcs > 1 )
		//		MatMPIAIJSetPreallocation( P, PETSC_NULL, nDiagNonZeros, PETSC_NULL, nOffDiagNonZeros );
		//	else
		//		MatSeqAIJSetPreallocation( P, PETSC_NULL, nDiagNonZeros );
		//	FreeArray( nDiagNonZeros );
		//	FreeArray( nOffDiagNonZeros );
		//}

		SROpGenerator_GenLevelOp( self, l_i, P );
		pOps[l_i] = P;
		//Stg_Class_AddRef( P );
		rOps[l_i] = P;
		//Stg_Class_AddRef( P );
	}
}

void SROpGenerator_GenLevelOp( SROpGenerator* self, unsigned level, Mat P ) {
	Mesh		*fMesh, *cMesh;
	unsigned	nDims;
	unsigned	nLocalFineNodes;
	DofLayout*	dofLayout;
	unsigned	ind;
	unsigned	nInc, *inc;
	unsigned	maxInc;
	unsigned	fTopNode, cTopNode;
	unsigned	fEqNum, cEqNum;
	double		*localCoord, *basis;
        double          *fineNodeCoord;
	IArray		*incArray;
	unsigned	n_i, dof_i, inc_i, e_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( self->meshes );
	assert( level < self->nLevels );
	assert( P );

	fMesh = self->meshes[level];
	cMesh = self->meshes[level - 1];
	nDims = Mesh_GetDimSize( fMesh );
	nLocalFineNodes = Mesh_GetLocalSize( fMesh, MT_VERTEX );
	dofLayout = self->fineEqNum->dofLayout;
	localCoord = AllocArray( double, nDims );

	maxInc = 0;
	for( e_i = 0; e_i < Mesh_GetDomainSize( cMesh, nDims ); e_i++ ) {
		nInc = Mesh_GetIncidenceSize( cMesh, nDims, e_i, MT_VERTEX );
		if( nInc > maxInc )
			maxInc = nInc;
	}
	basis = AllocArray( double, maxInc );

	incArray = IArray_New();
	for( n_i = 0; n_i < nLocalFineNodes; n_i++ ) {
		if( self->topMaps[level] )
			fTopNode = self->topMaps[level][n_i];
		else
			fTopNode = n_i;

		for( dof_i = 0; dof_i < dofLayout->dofCounts[fTopNode]; dof_i++ ) {
			if( self->eqNums[level] )
				fEqNum = self->eqNums[level][n_i][dof_i];
			else
				fEqNum = self->fineEqNum->mapNodeDof2Eq[fTopNode][dof_i];

			if( fEqNum == (unsigned)-1 )
				continue;

                        fineNodeCoord = Mesh_GetVertex( fMesh, n_i );
                        // find the element index on the coarse mesh, that contains the coord of the fine node
                        // note: for node boardering on elements (most of them) it doesn't matter which element is returned
                        // as both should give the same shape functions
			insist( Mesh_SearchElements( cMesh, fineNodeCoord, &ind ), == True );
			FeMesh_CoordGlobalToLocal( cMesh, ind, fineNodeCoord, localCoord );

                        // evaluate the shape functions at the local coord
			FeMesh_EvalBasis( cMesh, ind, localCoord, basis );
			Mesh_GetIncidence( cMesh, nDims, ind, MT_VERTEX, incArray );
			nInc = IArray_GetSize( incArray );
			inc = IArray_GetPtr( incArray );
                        /* */
			for( inc_i = 0; inc_i < nInc; inc_i++ ) {
				cTopNode = self->topMaps[level - 1][inc[inc_i]];
				cEqNum = self->eqNums[level - 1][inc[inc_i]][dof_i];
				if( cEqNum != (unsigned)-1 && !Num_Approx( basis[inc_i], 0.0 ) )
					MatSetValues( P, 1, &fEqNum, 1, &cEqNum, &(basis[inc_i]), INSERT_VALUES );
			}
		}
	}
	Stg_Class_Delete( incArray );

	FreeArray( localCoord );
	FreeArray( basis );

	MatAssemblyBegin( P, MAT_FINAL_ASSEMBLY );
	MatAssemblyEnd( P, MAT_FINAL_ASSEMBLY );
}

void SROpGenerator_CalcOpNonZeros( SROpGenerator* self, unsigned level,
				   unsigned** nDiagNonZeros, unsigned** nOffDiagNonZeros )
{
	Mesh		*fMesh, *cMesh;
	unsigned	nLocalFineNodes;
	DofLayout*	dofLayout;
	unsigned	dim, ind;
	unsigned	nInc, *inc;
	unsigned	fTopNode, cTopNode;
	unsigned	fEqNum, cEqNum;
	unsigned	nLocalEqNums;
	IArray*		incArray;
	unsigned	n_i, dof_i, dof_j, inc_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );
	assert( self->meshes );
	assert( level < self->nLevels );

	fMesh = self->meshes[level];
	cMesh = self->meshes[level - 1];
	nLocalFineNodes = Mesh_GetLocalSize( fMesh, MT_VERTEX );
	dofLayout = self->fineEqNum->dofLayout;
	if( self->eqNums[level] )
		nLocalEqNums = self->nLocalEqNums[level];
	else
		nLocalEqNums = self->fineEqNum->localEqNumsOwnedCount;

	*nDiagNonZeros = AllocArray( unsigned, nLocalEqNums );
	memset( *nDiagNonZeros, 0, nLocalEqNums * sizeof(unsigned) );
	*nOffDiagNonZeros = AllocArray( unsigned, nLocalEqNums );
	memset( *nOffDiagNonZeros, 0, nLocalEqNums * sizeof(unsigned) );

	incArray = IArray_New();

	for( n_i = 0; n_i < nLocalFineNodes; n_i++ ) {
		if( self->topMaps[level] )
			fTopNode = self->topMaps[level][n_i];
		else
			fTopNode = n_i;

		for( dof_i = 0; dof_i < dofLayout->dofCounts[fTopNode]; dof_i++ ) {
			if( self->eqNums[level] )
				fEqNum = self->eqNums[level][n_i][dof_i];
			else
				fEqNum = self->fineEqNum->mapNodeDof2Eq[fTopNode][dof_i];

			if( fEqNum == (unsigned)-1 )
				continue;

			insist( Mesh_SearchElements( cMesh, Mesh_GetVertex( fMesh, n_i ), &ind ), == True );
			dim = Mesh_GetDimSize( fMesh );
			if( dim == MT_VERTEX ) {
				cTopNode = self->topMaps[level - 1][ind];
				for( dof_j = 0; dof_j < dofLayout->dofCounts[cTopNode]; dof_j++ ) {
					cEqNum = self->eqNums[level - 1][ind][dof_j];
					if( cEqNum == (unsigned)-1 )
						continue;

					if( cEqNum - self->eqNumBases[level - 1] < nLocalEqNums )
						(*nDiagNonZeros)[fEqNum - self->eqNumBases[level]]++;
					else
						(*nOffDiagNonZeros)[fEqNum - self->eqNumBases[level]]++;
				}
			}
			else {
				Mesh_GetIncidence( cMesh, dim, ind, MT_VERTEX, incArray );
				nInc = IArray_GetSize( incArray );
				inc = IArray_GetPtr( incArray );
				for( inc_i = 0; inc_i < nInc; inc_i++ ) {
					cTopNode = self->topMaps[level - 1][inc[inc_i]];
					for( dof_j = 0; dof_j < dofLayout->dofCounts[cTopNode]; dof_j++ ) {
						cEqNum = self->eqNums[level - 1][inc[inc_i]][dof_j];
						if( cEqNum == (unsigned)-1 )
							continue;

						if( cEqNum - self->eqNumBases[level - 1] < nLocalEqNums )
							(*nDiagNonZeros)[fEqNum - self->eqNumBases[level]]++;
						else
							(*nOffDiagNonZeros)[fEqNum - self->eqNumBases[level]]++;
					}
				}
			}
		}
	}

	Stg_Class_Delete( incArray );
}

void SROpGenerator_DestructMeshes( SROpGenerator* self ) {
	unsigned	nLevels;
	unsigned	l_i;

	assert( self && Stg_CheckType( self, SROpGenerator ) );

	if( self->meshes ) {
		nLevels = self->nLevels;
		for( l_i = 0; l_i < nLevels - 1; l_i++ ) {
			FreeObject( self->meshes[l_i] );
			FreeArray( self->topMaps[l_i] );
			FreeArray( self->eqNums[l_i] );
		}
		KillArray( self->meshes );
		KillArray( self->topMaps );
		KillArray( self->eqNums );
		KillArray( self->nLocalEqNums );
		KillArray( self->eqNumBases );
	}
}


//void SROpGenerator_Simple( SROpGenerator *self, Matrix **pOps, Matrix **rOps ) {
void SROpGenerator_Simple( SROpGenerator *self, Mat* pOps, Mat* rOps ) {
   double t0;
   if( self->nLevels > 1 ) {
      //Matrix *P;
      Mat P;
      int ii;

      t0 = MPI_Wtime();
      P = SROpGenerator_SimpleFinestLevel( self );
      PetscPrintf( PETSC_COMM_WORLD, "SROpGenerator_SimpleFinestLevel: time = %5.5e \n", MPI_Wtime()-t0 );
      pOps[self->nLevels - 1] = rOps[self->nLevels - 1] = P;

      for( ii = self->nLevels - 2; ii > 0; ii-- ) {
	     t0 = MPI_Wtime();
         P = SROpGenerator_SimpleCoarserLevel( self, ii );
	     PetscPrintf( PETSC_COMM_WORLD, "  [%d] SROpGenerator_SimpleCoarserLevel: time = %5.5e \n",ii,  MPI_Wtime()-t0 );
         pOps[ii] = rOps[ii] = P;
      }
   }
}


PetscErrorCode _VecGetOwnershipRanges( Vec X, PetscInt **_ranges )
{
  PetscInt start,end;
  PetscMPIInt nproc,rank;
  MPI_Comm comm;
  Vec all, out;
  VecScatter scat;
  PetscInt *ranges,i;
  PetscScalar *v;
  double t0;

  t0 = MPI_Wtime();

  PetscObjectGetComm( (PetscObject)X, &comm );
  MPI_Comm_size( comm, &nproc );
  MPI_Comm_rank( comm, &rank );
  VecGetOwnershipRange( X, &start, &end );

  VecCreate( comm, &all );
  VecSetSizes( all, PETSC_DECIDE, nproc+1 );
  VecSetFromOptions( all );

  VecSetValue( all, rank, (PetscScalar)start, INSERT_VALUES );
  VecSetValue( all, rank+1, (PetscScalar)end, INSERT_VALUES );
  VecAssemblyBegin( all );
  VecAssemblyEnd( all );

  VecScatterCreateToAll( all, &scat, &out );

#if(PETSC_VERSION_MAJOR==2)
#if(PETSC_VERSION_SUBMINOR==2)
  VecScatterBegin(all,out,INSERT_VALUES,SCATTER_FORWARD,scat);
  VecScatterEnd(all,out,INSERT_VALUES,SCATTER_FORWARD,scat);
#elif(PETSC_VERSION_SUBMINOR==3)
  VecScatterBegin(scat, all,out,INSERT_VALUES,SCATTER_FORWARD);
  VecScatterEnd(scat, all,out,INSERT_VALUES,SCATTER_FORWARD);
#endif
#endif

#if(PETSC_VERSION_MAJOR==3)
  VecScatterBegin(scat, all,out,INSERT_VALUES,SCATTER_FORWARD);
  VecScatterEnd(scat, all,out,INSERT_VALUES,SCATTER_FORWARD);
#endif


  PetscMalloc( sizeof(PetscInt)*(nproc+1), &ranges );
  VecGetArray( out, &v );
  for( i=0; i<nproc+1; i++ ) {
    ranges[i] = (PetscInt)v[i];
  }
  VecRestoreArray( out, &v );

  *_ranges = ranges;

  Stg_VecScatterDestroy(&scat );
  Stg_VecDestroy(&out );
  Stg_VecDestroy(&all );


  PetscFunctionReturn(0);
}


/*
Efficiency alert!!
  This comment relates to both SROpGenerator_SimpleFinestLevel() and
  SROpGenerator_SimpleCoarserLevel().

  For O(n) setup times, we MUST provide some information concerning the
  preallocation of the nonzero structure for the restriction operators.

  As a first hack we check the spatial dimension and choose a value accordingly.
  In doing so we have assumed that the fine grid is obtained by bisecting
  the coarse grid. Coarse grid nodes are hiearchical. We have also assumed
  that the elements are quads (2d) and hexs (3d).

  The hack above made little effort to miminise memory usage and as such we allocate
  the same amount of memory for the on and off diagonal parts of the restriction
  operator. This kinda kills use in parallel.

  An more accurate estimate has thus been implemented.
*/

Mat SROpGenerator_SimpleFinestLevel( SROpGenerator *self ) {
   FeMesh *mesh;
   int nDims, nDofsPerNode;
   int sideSizes[3];
   int inds[2][3], offsInds[3], nOffs[3];
   int nGlobalNodes;
   int nGlobalEqs;
   Grid *vertGrid, *elGrid, *grid[2], *offsGrid;
   int nodes[8];
   int nodeInd;
   double dfrac;
   int nEntries, indices[8];
   double values[8];
   Mat P;
   Mat mat;
   int ii, jj, kk;
   PetscInt o_nz, d_nz;
   PetscInt *o_nnz, *d_nnz;
   PetscInt sr,er, sc,ec, row_idx;
   Vec vec_o_nnz, vec_d_nnz;
   PetscScalar *v;
   PetscTruth is_seq, is_q1/* test to see if NOT a Q1 mesh, if not then assume it is Q2 */;
   PetscInt p, proc_owner, *rranges,*cranges;
   MPI_Comm comm;
   PetscMPIInt nproc,rank;

   /* Calculate level side lengths. */
   mesh = self->fineVar->feMesh;
   nDims = Mesh_GetDimSize( mesh );

   /* crude estimate of number of nonzeros in the on-off diagonal portitions of the matrix */
   o_nz = d_nz = 0;
   if( nDims == 2 ) {
	o_nz = d_nz = 4;
   }
   if( nDims==3 ) {
	o_nz = d_nz = 8;
   }

   nDofsPerNode = self->fineVar->dofLayout->dofCounts[0]; /* ASSUME */
   elGrid   = *Mesh_GetExtension( mesh, Grid**,  mesh->elGridId );
   vertGrid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   is_q1 = PETSC_FALSE;
   /* test to see if NOT a Q1 mesh, if not then assume it is Q2 */
   if( elGrid->sizes[0] + 1 == vertGrid->sizes[0] ){
     is_q1 = PETSC_TRUE;
     /* test some more here? */
   }
   nGlobalNodes = 1;
   for( ii = 0; ii < nDims; ii++ ) {
      sideSizes[ii] = elGrid->sizes[ii] / 2;
      if( sideSizes[ii] * 2 != elGrid->sizes[ii] ) {
         printf( "(MG) Error: Too many levels specified for geometric multigrid.\n" );
         printf( "            Please modify mesh size or reduce number of levels.\n" );
         exit( 1 );
      }
      /* Need to check for Q2 vs Q1 Mesh here */
      if( is_q1 ){
        sideSizes[ii]++;
      }else{/* assume Q2 elements */
        sideSizes[ii] *= 2;
        sideSizes[ii]++;
      }
      nGlobalNodes *= sideSizes[ii];
   }

   /* Have PETSc create the operator matrix and then extract local sizes
      and offsets into the global vector. */
   nGlobalEqs = nGlobalNodes * nDofsPerNode;

   MatCreate( MPI_COMM_WORLD, &P );
   // a bit stupid because we know the global eqNum on the fine Mesh
   MatSetSizes( P, self->fineEqNum->localEqNumsOwnedCount, PETSC_DECIDE, PETSC_DECIDE, nGlobalEqs );
   MatSetType( P, MATAIJ );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
   MatSetUp(P);
#endif

   PetscObjectGetComm( (PetscObject)P, &comm );
   MPI_Comm_size( comm, &nproc );
   MPI_Comm_rank( comm, &rank );

   {  Vec L,R;
      PetscInt M,N, m,n;

      MatGetSize( P, &M,&N );
      MatGetLocalSize( P, &m,&n );
//      PetscPrintf( PETSC_COMM_WORLD, "size = %D x %D \n", M, N );
      MatGetVecs( P, &R, &L );
      VecGetOwnershipRange( L, &sr,&er );
      VecGetOwnershipRange( R, &sc,&ec );
//      printf( "  [%d] mxn = %d x %d ; sr,er = %d -> %d ; sc,ec = %d -> %d \n", rank, m,n,  sr,er, sc,ec );

      _VecGetOwnershipRanges( L, &rranges );
      _VecGetOwnershipRanges( R, &cranges );

      Stg_VecDestroy(&L );
      Stg_VecDestroy(&R );
    }

   PetscMalloc( sizeof(PetscInt)*(er-sr), &o_nnz );
   PetscMalloc( sizeof(PetscInt)*(er-sr), &d_nnz );

   /* Need a grid representing the coarse level. */
   grid[0] = Grid_New();
   offsGrid = Grid_New();
   Grid_SetNumDims( grid[0], nDims );
   Grid_SetNumDims( offsGrid, nDims );
   Grid_SetSizes( grid[0], sideSizes );

   /* Determine preallocation information */
   MatGetVecs( P, PETSC_NULL, &vec_d_nnz );
   VecDuplicate( vec_d_nnz, &vec_o_nnz );

   /* Loop over fine nodes. */
   for( ii = 0; ii < Mesh_GetLocalSize( mesh, 0 ); ii++ ) {
      nodeInd = Mesh_DomainToGlobal( mesh, 0, ii );
      Grid_Lift( vertGrid, nodeInd, inds[1] );

      /* An odd grid index means we need to interpolate from surrounding
         coarse nodes in the current dimension. */
      for( jj = 0; jj < nDims; jj++ ) {
         /* Store the offsets we need to consider. */
         nOffs[jj] = (inds[1][jj] & 1) ? 2 : 1;
      }

      /* 'Multiply' the offsets to build the set of nodes we need to
         interpolate from. */
      Grid_SetSizes( offsGrid, nOffs );
      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         Grid_Lift( offsGrid, jj, offsInds );
         for( kk = 0; kk < nDims; kk++ )
            inds[0][kk] = (inds[1][kk] >> 1) + offsInds[kk];

         /* Store the coarse global node number. */
         nodes[jj] = Grid_Project( grid[0], inds[0] );
      }

      /* Insert this row into the operator matrix. */
      nEntries = offsGrid->nPoints;
      for( kk = 0; kk < nDofsPerNode; kk++ ) {
         /* Skip the entire thing if it's a BC. */
         if( self->fineEqNum->mapNodeDof2Eq[ii][kk] == -1 )
            continue;

         row_idx = (PetscInt)(self->fineEqNum->mapNodeDof2Eq[ii][kk]);


         /* find owning proc */
         proc_owner = -1;
         for( p=0; p<nproc; p++ ) {
            if( (row_idx>=rranges[p]) && (row_idx<rranges[p+1]) ) {
               proc_owner = p;
               break;
            }
         }


         for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
            indices[jj] = nodes[jj] * nDofsPerNode + kk;
          //  printf( "[%d]: (owner=%d): row=%d : c_index=%d \n", rank, proc_owner, row_idx, indices[jj] );
	    if( (row_idx>=rranges[proc_owner]) && (row_idx<rranges[proc_owner+1]) ) {
               if( (indices[jj]>=cranges[proc_owner]) && (indices[jj]<cranges[proc_owner+1]) ) {
                  VecSetValue( vec_d_nnz, row_idx, 1.0, ADD_VALUES );
               }
               else {
                  VecSetValue( vec_o_nnz, row_idx, 1.0, ADD_VALUES );
               }
            }
         }


/*
        for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
           indices[jj] = nodes[jj] * nDofsPerNode + kk;
	   printf( "[%d]: row=%d : c_index=%d \n", rank, row_idx, indices[jj] );
           if( (row_idx>=sr) && (row_idx<er) ) {
              if( (indices[jj]>=sc) && (indices[jj]<ec) ) {
                VecSetValue( vec_d_nnz, row_idx, 1.0, ADD_VALUES );
              }
            }
           else {
              VecSetValue( vec_o_nnz, row_idx, 1.0, ADD_VALUES );
           }
        }
*/


      }
   }
   VecAssemblyBegin( vec_d_nnz );
   VecAssemblyEnd( vec_d_nnz );

   VecAssemblyBegin( vec_o_nnz );
   VecAssemblyEnd( vec_o_nnz );

   VecGetArray( vec_d_nnz, &v );
   for( kk=0; kk<(er-sr); kk++ ) {
      d_nnz[kk] = (PetscInt)v[kk];
   }
   VecRestoreArray( vec_d_nnz, &v );

   VecGetArray( vec_o_nnz, &v );
   for( kk=0; kk<(er-sr); kk++ ) {
      o_nnz[kk] = (PetscInt)v[kk];
   }
   VecRestoreArray( vec_o_nnz, &v );

   Stg_PetscObjectTypeCompare( (PetscObject)vec_o_nnz, VECSEQ, &is_seq );
   if(nproc==1) {
      MatSeqAIJSetPreallocation( P, PETSC_NULL, d_nnz );
   }
   else {
//     for( kk=0; kk<(er-sr); kk++ ) {
//       printf( "[%d]: row=%d : d_nnz=%d : o_nnz=%d \n", rank, kk+sr, d_nnz[kk], o_nnz[kk] );
//     }
      MatMPIAIJSetPreallocation( P, PETSC_NULL, d_nnz, PETSC_NULL, o_nnz );

//    MatMPIAIJSetPreallocation( P, d_nz,PETSC_NULL, o_nz,PETSC_NULL );
   }

   /* Loop over fine nodes. */
   for( ii = 0; ii < Mesh_GetLocalSize( mesh, 0 ); ii++ ) {
//      if( ii%5000 == 0 ) {
//        PetscPrintf( PETSC_COMM_WORLD, "Done %d of %d \n", ii, Mesh_GetLocalSize( mesh, 0 ) );
//      }
      nodeInd = Mesh_DomainToGlobal( mesh, 0, ii );
      Grid_Lift( vertGrid, nodeInd, inds[1] );

      /* An odd grid index means we need to interpolate from surrounding
         coarse nodes in the current dimension. */
      for( jj = 0; jj < nDims; jj++ ) {
         /* Store the offsets we need to consider. */
         nOffs[jj] = (inds[1][jj] & 1) ? 2 : 1;
      }

      /* 'Multiply' the offsets to build the set of nodes we need to
         interpolate from. */
      Grid_SetSizes( offsGrid, nOffs );
      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         Grid_Lift( offsGrid, jj, offsInds );
         for( kk = 0; kk < nDims; kk++ )
            inds[0][kk] = (inds[1][kk] >> 1) + offsInds[kk];

         /* Store the coarse global node number. */
         nodes[jj] = Grid_Project( grid[0], inds[0] );
      }

      /* Insert this row into the operator matrix. */
      dfrac = 1.0 / (double)offsGrid->nPoints;
      nEntries = offsGrid->nPoints;
      for( kk = 0; kk < nDofsPerNode; kk++ ) {
         /* Skip the entire thing if it's a BC. */
         if( self->fineEqNum->mapNodeDof2Eq[ii][kk] == -1 )
            continue;

         for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
            indices[jj] = nodes[jj] * nDofsPerNode + kk;
            values[jj] = dfrac;
         }
         MatSetValues( P, 1, self->fineEqNum->mapNodeDof2Eq[ii] + kk,
                       nEntries, indices, values, INSERT_VALUES );
      }
   }
//   PetscPrintf( PETSC_COMM_WORLD, "Done all MatSetValues \n");

   /* Assemble the matrix. */
   MatAssemblyBegin( P, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd( P, MAT_FINAL_ASSEMBLY );

/*
   {
      PetscInt M,N, m,n;
      for( p=0; p<nproc; p++ ) {
        PetscPrintf( PETSC_COMM_WORLD, "[%d]: r_range (%d->%d) ; c_range (%d->%d)\n", p, rranges[p], rranges[p+1], cranges[p], cranges[p+1] );
      }

      MatGetSize( P, &M,&N );
      MatGetLocalSize( P, &m,&n );
      PetscPrintf( PETSC_COMM_WORLD, "P_size = %D x %D \n", M, N );
      MatGetOwnershipRange( P, &sr,&er );
      printf( "  [%d] mxn = %d x %d ; sr,er = %d -> %d (local rows=%d) \n", rank, m,n,  sr,er, er-sr );
    }
    PetscViewerSetFormat( PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_DENSE );
    MatView( P, PETSC_VIEWER_STDOUT_WORLD );
*/

   /* Piss off those grids. */
   Stg_Class_Delete( grid[0] );

   Stg_Class_Delete(offsGrid);

   Stg_VecDestroy(&vec_o_nnz );
   Stg_VecDestroy(&vec_d_nnz );
   PetscFree( o_nnz );
   PetscFree( d_nnz );
   PetscFree( rranges );
   PetscFree( cranges );


   /* Create a new matrix. */
   mat = P;
   return mat;
}

Mat SROpGenerator_SimpleCoarserLevel( SROpGenerator *self, int level ) {
   FeMesh *mesh;
   int nDims, nDofsPerNode, rowDof;
   int sideSizes[2][3];
   int inds[2][3], offsInds[3], nOffs[3];
   int nGlobalNodes[2];
   int nGlobalEqs[2];
   int eqRangeBegin, eqRangeEnd;
   Grid *vertGrid, *elGrid, *grid[2], *offsGrid;
   int nodes[8];
   int ifrac, nodeInd;
   double dfrac;
   int nEntries, indices[8];
   double values[8];
   Mat P;
   //PETScMatrix *mat;
   Mat mat;
   int ii, jj, kk;
   PetscInt o_nz, d_nz;
   PetscInt *o_nnz, *d_nnz;
   PetscInt sr,er, sc,ec, row_idx;
   Vec vec_o_nnz, vec_d_nnz;
   PetscScalar *v;
   PetscInt p, proc_owner, *row_ranges, *col_ranges;
   MPI_Comm comm;
   PetscMPIInt nproc;
   PetscTruth is_q1/* test to see if NOT a Q1 mesh, if not then assume it is Q2 */;

   /* Calculate depth fraction. */
   ifrac = 1;
   for( ii = 0; ii < self->nLevels - level - 1; ii++ )
      ifrac *= 2;

   /* Calculate level side lengths. */
   mesh = self->fineVar->feMesh;
   nDims = Mesh_GetDimSize( mesh );

   /* crude estimate of number of nonzeros in the on-off diagonal portitions of the matrix */
   o_nz = d_nz = 0;
   if( nDims==2 ) {
      o_nz = d_nz = 4;
   }
   if( nDims==3 ) {
        o_nz = d_nz = 8;
   }

   nDofsPerNode = self->fineVar->dofLayout->dofCounts[0]; /* ASSUME */
   elGrid = *Mesh_GetExtension( mesh, Grid**,  mesh->elGridId );
   vertGrid = *Mesh_GetExtension( mesh, Grid**,  mesh->vertGridId );
   is_q1 = PETSC_FALSE;
   /* test to see if NOT a Q1 mesh, if not then assume it is Q2 */
   if( elGrid->sizes[0] + 1 == vertGrid->sizes[0] ){
     is_q1 = PETSC_TRUE;
     /* test some more here? */
   }
   nGlobalNodes[0] = nGlobalNodes[1] = 1;
   for( ii = 0; ii < nDims; ii++ ) {
      sideSizes[1][ii] = elGrid->sizes[ii] / ifrac;
      sideSizes[0][ii] = elGrid->sizes[ii] / (ifrac * 2);
      if( sideSizes[1][ii] * ifrac != elGrid->sizes[ii] ||
          sideSizes[0][ii] * (ifrac * 2) != elGrid->sizes[ii] )
      {
         printf( "(MG) Error: Too many levels specified for geometric multigrid.\n" );
         printf( "            Please modify mesh size or reduce number of levels.\n" );
         exit( 1 );
      }
      /* Need to check for Q2 vs Q1 Mesh here */
      if( is_q1 ){
      sideSizes[1][ii]++; sideSizes[0][ii]++;
      }else{/* assume Q2 elements */
        sideSizes[1][ii] *= 2;
        sideSizes[0][ii] *= 2;
        sideSizes[1][ii]++; sideSizes[0][ii]++;
      }
      nGlobalNodes[1] *= sideSizes[1][ii];
      nGlobalNodes[0] *= sideSizes[0][ii];
   }

   /* Have PETSc create the operator matrix and then extract local sizes
      and offsets into the global vector. */
   nGlobalEqs[1] = nGlobalNodes[1] * nDofsPerNode;
   nGlobalEqs[0] = nGlobalNodes[0] * nDofsPerNode;

/*
   Stg_MatCreateAIJ( MPI_COMM_WORLD,
                    PETSC_DECIDE, PETSC_DECIDE, nGlobalEqs[1], nGlobalEqs[0],
                    o_nz, PETSC_NULL, d_nz, PETSC_NULL, &P );
   MatGetOwnershipRange( P, &eqRangeBegin, &eqRangeEnd );
*/
   MatCreate( MPI_COMM_WORLD, &P );
   MatSetSizes( P, PETSC_DECIDE, PETSC_DECIDE, nGlobalEqs[1], nGlobalEqs[0] );
   MatSetType( P, MATAIJ );
#if (((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=3)) || (PETSC_VERSION_MAJOR>3) )
   MatSetUp(P);
#endif
   MatGetOwnershipRange( P, &eqRangeBegin, &eqRangeEnd );

   {
      Vec L,R;
      PetscInt M,N;
      MatGetSize( P, &M,&N );
//      PetscPrintf( PETSC_COMM_WORLD, "size = %D x %D \n", M, N );
      MatGetVecs( P, &R, &L );
      VecGetOwnershipRange( L, &sr,&er );
      VecGetOwnershipRange( R, &sc,&ec );
//      printf( "  2) sr,er = %d -> %d \n", sr, er );
//      printf( "  2) sc,ec = %d -> %d \n", sc, ec );

      _VecGetOwnershipRanges( L, &row_ranges );
      _VecGetOwnershipRanges( R, &col_ranges );

      Stg_VecDestroy(&L );
      Stg_VecDestroy(&R );
    }


   PetscMalloc( sizeof(PetscInt)*(er-sr), &o_nnz );
   PetscMalloc( sizeof(PetscInt)*(er-sr), &d_nnz );



   /* Need a grid representing the coarse level. */
   grid[1] = Grid_New();
   grid[0] = Grid_New();
   offsGrid = Grid_New();
   Grid_SetNumDims( grid[1], nDims );
   Grid_SetNumDims( grid[0], nDims );
   Grid_SetNumDims( offsGrid, nDims );
   Grid_SetSizes( grid[1], sideSizes[1] );
   Grid_SetSizes( grid[0], sideSizes[0] );

   /* Determine preallocation */
   MatGetVecs( P, PETSC_NULL, &vec_d_nnz );
   VecDuplicate( vec_d_nnz, &vec_o_nnz );
   PetscObjectGetComm( (PetscObject)P, &comm );
   MPI_Comm_size( comm, &nproc );



   /* Loop over the finer of the two levels. */
   for( ii = eqRangeBegin; ii < eqRangeEnd; ii++ ) {
      /* Convert the global equation number to the global node index. */
      nodeInd = ii / nDofsPerNode;
      rowDof = ii - nodeInd * nDofsPerNode;
      Grid_Lift( grid[1], nodeInd, inds[1] );

      /* An odd grid index means we need to interpolate from surrounding
         coarse nodes in the current dimension. */
      for( jj = 0; jj < nDims; jj++ ) {
         /* Store the offsets we need to consider. */
         nOffs[jj] = (inds[1][jj] & 1) ? 2 : 1;
      }

      /* 'Multiply' the offsets to build the set of nodes we need to
         interpolate from. */
      Grid_SetSizes( offsGrid, nOffs );
      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         Grid_Lift( offsGrid, jj, offsInds );
         for( kk = 0; kk < nDims; kk++ )
            inds[0][kk] = (inds[1][kk] >> 1) + offsInds[kk];

         /* Store the coarse global node number. */
         nodes[jj] = Grid_Project( grid[0], inds[0] );
      }

      /* Insert this row into the operator matrix. */
      row_idx = (PetscInt)ii;
      /* find owning proc */
      proc_owner = -1;
      for( p=0; p<nproc; p++ ) {
         if( (row_idx>=row_ranges[p]) && (row_idx<row_ranges[p+1]) ) {
            proc_owner = p;
            break;
         }
      }
/*
      if( (row_idx>=sr) && (row_idx<er) ) {
        for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
          indices[jj] = nodes[jj] * nDofsPerNode + rowDof;
          if( (indices[jj]>=sc) && (indices[jj]<ec) ) {
            VecSetValue( vec_d_nnz, row_idx, 1.0, ADD_VALUES );
          }
        }
      }
      else {
        VecSetValue( vec_o_nnz, row_idx, 1.0, ADD_VALUES );
      }
*/

      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         indices[jj] = nodes[jj] * nDofsPerNode + rowDof;
//         printf("ridx=%d,cidx=%d : owner:%d rrange [%d-%d] : crange [%d-%d] \n", row_idx, indices[jj], proc_owner, row_ranges[proc_owner],row_ranges[proc_owner+1], col_ranges[proc_owner], col_ranges[proc_owner+1] );
         if( (row_idx>=row_ranges[proc_owner]) && (row_idx<row_ranges[proc_owner+1]) ) {
            if( (indices[jj]>=col_ranges[proc_owner]) && (indices[jj]<col_ranges[proc_owner+1]) ) {
               VecSetValue( vec_d_nnz, row_idx, 1.0, ADD_VALUES );
            }
            else {
               VecSetValue( vec_o_nnz, row_idx, 1.0, ADD_VALUES );
            }
         }
      }

   }

   VecAssemblyBegin( vec_d_nnz );
   VecAssemblyEnd( vec_d_nnz );

   VecAssemblyBegin( vec_o_nnz );
   VecAssemblyEnd( vec_o_nnz );

   VecGetArray( vec_d_nnz, &v );
   for( kk=0; kk<(er-sr); kk++ ) {
      d_nnz[kk] = (PetscInt)v[kk];
   }
   VecRestoreArray( vec_d_nnz, &v );

   VecGetArray( vec_o_nnz, &v );
   for( kk=0; kk<(er-sr); kk++ ) {
      o_nnz[kk] = (PetscInt)v[kk];
   }
   VecRestoreArray( vec_o_nnz, &v );


   if(nproc==1) {
      MatSeqAIJSetPreallocation( P, PETSC_NULL, d_nnz );
   }
   else {
      MatMPIAIJSetPreallocation( P, PETSC_NULL, d_nnz, PETSC_NULL, o_nnz );
   }


   /* Loop over the finer of the two levels. */
   for( ii = eqRangeBegin; ii < eqRangeEnd; ii++ ) {
      /* Convert the global equation number to the global node index. */
      nodeInd = ii / nDofsPerNode;
      rowDof = ii - nodeInd * nDofsPerNode;
      Grid_Lift( grid[1], nodeInd, inds[1] );

      /* An odd grid index means we need to interpolate from surrounding
         coarse nodes in the current dimension. */
      for( jj = 0; jj < nDims; jj++ ) {
         /* Store the offsets we need to consider. */
         nOffs[jj] = (inds[1][jj] & 1) ? 2 : 1;
      }

      /* 'Multiply' the offsets to build the set of nodes we need to
         interpolate from. */
      Grid_SetSizes( offsGrid, nOffs );
      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         Grid_Lift( offsGrid, jj, offsInds );
         for( kk = 0; kk < nDims; kk++ )
            inds[0][kk] = (inds[1][kk] >> 1) + offsInds[kk];

         /* Store the coarse global node number. */
         nodes[jj] = Grid_Project( grid[0], inds[0] );
      }

      /* Insert this row into the operator matrix. */
      dfrac = 1.0 / (double)offsGrid->nPoints;
      nEntries = offsGrid->nPoints;
      for( jj = 0; jj < offsGrid->nPoints; jj++ ) {
         indices[jj] = nodes[jj] * nDofsPerNode + rowDof;
         values[jj] = dfrac;
      }
      MatSetValues( P, 1, &ii, nEntries, indices, values, INSERT_VALUES );
   }

   /* Assemble the matrix. */
   MatAssemblyBegin( P, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd( P, MAT_FINAL_ASSEMBLY );

   /* Piss off those grids. */
   Stg_Class_Delete( grid[0] );
   Stg_Class_Delete( grid[1] );
   Stg_Class_Delete( offsGrid );

   Stg_VecDestroy(&vec_o_nnz );
   Stg_VecDestroy(&vec_d_nnz );
   PetscFree( o_nnz );
   PetscFree( d_nnz );
   PetscFree( row_ranges );
   PetscFree( col_ranges );


   /* Create a new matrix. */
   //mat = PETScMatrix_New( "" );
   //mat->petscMat = P;
   mat = P;
   return mat;
}
