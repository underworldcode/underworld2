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

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "Discretisation.h"


/* Textual name of this class */
const Type C2Generator_Type = "C2Generator";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

C2Generator* C2Generator_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                                    _sizeOfSelf = sizeof(C2Generator);
	Type                                                            type = C2Generator_Type;
	Stg_Class_DeleteFunction*                                    _delete = _C2Generator_Delete;
	Stg_Class_PrintFunction*                                      _print = _C2Generator_Print;
	Stg_Class_CopyFunction*                                        _copy = NULL;
	Stg_Component_DefaultConstructorFunction*        _defaultConstructor = (void* (*)(Name))_C2Generator_New;
	Stg_Component_ConstructFunction*                          _construct = _C2Generator_AssignFromXML;
	Stg_Component_BuildFunction*                                  _build = _C2Generator_Build;
	Stg_Component_InitialiseFunction*                        _initialise = _C2Generator_Initialise;
	Stg_Component_ExecuteFunction*                              _execute = _C2Generator_Execute;
	Stg_Component_DestroyFunction*                              _destroy = NULL;
	AllocationType                                    nameAllocationType = NON_GLOBAL;
	MeshGenerator_SetDimSizeFunc*                         setDimSizeFunc = CartesianGenerator_SetDimSize;
	MeshGenerator_GenerateFunc*                             generateFunc = CartesianGenerator_Generate;
	CartesianGenerator_SetTopologyParamsFunc*      setTopologyParamsFunc = C2Generator_SetTopologyParams;
	CartesianGenerator_GenElementsFunc*                  genElementsFunc = _CartesianGenerator_GenElements;
	CartesianGenerator_GenFacesFunc*                        genFacesFunc = _CartesianGenerator_GenFaces;
	CartesianGenerator_GenEdgesFunc*                        genEdgesFunc = _CartesianGenerator_GenEdges;
	CartesianGenerator_GenVerticesFunc*                  genVerticesFunc = _CartesianGenerator_GenVertices;
	CartesianGenerator_GenElementVertexIncFunc*  genElementVertexIncFunc = C2Generator_GenElementVertexInc;
	CartesianGenerator_GenVolumeEdgeIncFunc*        genVolumeEdgeIncFunc = _CartesianGenerator_GenVolumeEdgeInc;
	CartesianGenerator_GenVolumeFaceIncFunc*        genVolumeFaceIncFunc = _CartesianGenerator_GenVolumeFaceInc;
	CartesianGenerator_GenFaceVertexIncFunc*        genFaceVertexIncFunc = C2Generator_GenFaceVertexInc;
	CartesianGenerator_GenFaceEdgeIncFunc*            genFaceEdgeIncFunc = _CartesianGenerator_GenFaceEdgeInc;
	CartesianGenerator_GenEdgeVertexIncFunc*        genEdgeVertexIncFunc = C2Generator_GenEdgeVertexInc;
	CartesianGenerator_GenElementTypesFunc*          genElementTypesFunc = C2Generator_GenElementTypes;
  CartesianGenerator_CalcGeomFunc*                calcGeomFunc = CartesianGenerator_CalcGeom;

	C2Generator* self = _C2Generator_New(  C2GENERATOR_PASSARGS  );

   _MeshGenerator_Init( (MeshGenerator*)self, context );
   _CartesianGenerator_Init( (CartesianGenerator*)self );
   _C2Generator_Init( self );
   return self;
}

C2Generator* _C2Generator_New(  C2GENERATOR_DEFARGS  ) {
	C2Generator*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(C2Generator) );
	self = (C2Generator*)_CartesianGenerator_New(  CARTESIANGENERATOR_PASSARGS  );



	return self;
}

void _C2Generator_Init( C2Generator* self ) {
	assert( self );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _C2Generator_Delete( void* meshGenerator ) {
	C2Generator*	self = (C2Generator*)meshGenerator;

	/* Delete the parent. */
	_CartesianGenerator_Delete( self );
}

void _C2Generator_Print( void* meshGenerator, Stream* stream ) {
	C2Generator*	self = (C2Generator*)meshGenerator;
	
	/* Set the Journal for printing informations */
	Stream* meshGeneratorStream;
	meshGeneratorStream = Journal_Register( InfoStream_Type, (Name)"C2GeneratorStream" );

	assert( self  );

	/* Print parent */
	Journal_Printf( stream, "C2Generator (ptr): (%p)\n", self );
	_CartesianGenerator_Print( self, stream );
}

void _C2Generator_AssignFromXML( void* meshGenerator, Stg_ComponentFactory* cf, void* data ) {
	_CartesianGenerator_AssignFromXML( meshGenerator, cf, data );
}

void _C2Generator_Build( void* meshGenerator, void* data ) {
	_CartesianGenerator_Build( meshGenerator, data );
}

void _C2Generator_Initialise( void* meshGenerator, void* data ) {
}

void _C2Generator_Execute( void* meshGenerator, void* data ) {
}

void _C2Generator_Destroy( void* meshGenerator, void* data ) {
}

void C2Generator_SetTopologyParams( void* meshGenerator, unsigned* sizes, 
				    unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp )
{
	C2Generator*	self = (C2Generator*)meshGenerator;
	unsigned*	vertSizes;
	unsigned	d_i;

	assert( self );

	_CartesianGenerator_SetTopologyParams( self, sizes, 
					       maxDecompDims, minDecomp, maxDecomp );
	vertSizes = AllocArray( unsigned, self->vertGrid->nDims );
	for( d_i = 0; d_i < self->vertGrid->nDims; d_i++ ) {
		vertSizes[d_i] = self->vertGrid->sizes[d_i] * 2 - 1;
		self->vertOrigin[d_i] *= 2;
		self->vertRange[d_i] = self->vertRange[d_i] * 2 - 1;
	}
	Grid_SetSizes( self->vertGrid, vertSizes );
	FreeArray( vertSizes );
}

void C2Generator_GenElementVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	C2Generator*	self = (C2Generator*)meshGenerator;
	Stream*		stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned*	incEls;
	unsigned*	dimInds;
	unsigned	vertsPerEl;
	unsigned	nDims;
	unsigned	e_i, d_i;
	int nDomainEls;

	assert( self );
	assert( topo );
	assert( grids );

//	Journal_Printf( stream, "Generating element-vertex incidence...\n" );
	Stream_Indent( stream );

	vertsPerEl = (topo->nDims == 1) ? 3 : (topo->nDims == 2) ? 9 : 27;

	nDims = topo->nDims;
	nDomainEls = Sync_GetNumDomains( IGraph_GetDomain( topo, nDims ) );
	incEls = Memory_Alloc_Array_Unnamed( unsigned, vertsPerEl );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < nDomainEls; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( IGraph_GetDomain( topo, nDims ), e_i );
		unsigned	curNode = 0;

		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );
		for( d_i = 0; d_i < nDims; d_i++ )
			dimInds[d_i] *= 2;

		incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
		dimInds[0]++;
		incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
		dimInds[0]++;
		incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
		dimInds[0] -= 2;

		if( topo->nDims >= 2 ) {
			dimInds[1]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2;
			dimInds[1]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2;
			dimInds[1] -= 2;

			if( topo->nDims >= 3 ) {
				dimInds[2]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1] -= 2;
				dimInds[2]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0] -= 2;
				dimInds[1] -= 2;
				dimInds[2] -= 2;
			}
		}
		CartesianGenerator_MapToDomain( (CartesianGenerator*)self, IGraph_GetDomain( topo, 0), vertsPerEl, incEls );
		IGraph_SetIncidence( topo, topo->nDims, e_i, MT_VERTEX, vertsPerEl, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
//	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void C2Generator_GenFaceVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	C2Generator*	self 		= (C2Generator*)meshGenerator;
	Stream*		stream;
	unsigned	face_i;
	unsigned	gFace;
	unsigned	verts[9];
	unsigned*	dimInds		= Memory_Alloc_Array( unsigned, topo->nDims, "edgeDimensionIndices" );

	stream = Journal_Register( Info_Type, (Name)self->type  );
//	Journal_Printf( stream, "Generating face-vertex types...\n" );
	Stream_Indent( stream );

	for( face_i = 0; face_i < topo->remotes[MT_FACE]->nDomains; face_i++ ) {
		gFace = Sync_DomainToGlobal( topo->remotes[MT_FACE], face_i );

		if( gFace < grids[2][0]->nPoints ) {
			Grid_Lift( grids[2][0], gFace, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[1]++;
			verts[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[4] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[5] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[1]++;
			verts[6] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[7] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[8] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[1] -= 2;
		}
		else if( topo->nDims == 3 && gFace < grids[2][0]->nPoints + grids[2][1]->nPoints ) {
			Grid_Lift( grids[2][1], gFace - grids[2][0]->nPoints, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[2]++;
			verts[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[4] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[5] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[2]++;
			verts[6] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[7] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[8] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2; dimInds[2] -= 2;
		}
		else if( topo->nDims == 3 && gFace < grids[2][0]->nPoints + grids[2][1]->nPoints + grids[2][2]->nPoints ) {
			Grid_Lift( grids[2][2], gFace - grids[2][0]->nPoints - grids[2][1]->nPoints, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[1] -= 2; dimInds[2]++;
			verts[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[4] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[5] = Grid_Project( grids[0][0], dimInds );
			dimInds[1] -= 2; dimInds[2]++;
			verts[6] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[7] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[8] = Grid_Project( grids[0][0], dimInds );
			dimInds[1] -= 2; dimInds[2] -= 2;
		}

		CartesianGenerator_MapToDomain( (CartesianGenerator*)self, (Sync*)IGraph_GetDomain( topo, MT_VERTEX ), 9, verts );
		IGraph_SetIncidence( topo, MT_FACE, face_i, MT_VERTEX, 9, verts );
	}

	Memory_Free( dimInds );	

	MPI_Barrier( self->mpiComm );
	
//	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void C2Generator_GenEdgeVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	C2Generator*	self 		= (C2Generator*)meshGenerator;
	Sync*		sync		= IGraph_GetDomain( topo, MT_EDGE );
	Stream*		stream;
	unsigned	edge_i;
	unsigned	gEdge;
	unsigned	verts[3];
	unsigned*	dimInds		= Memory_Alloc_Array( unsigned, topo->nDims, "edgeDimensionIndices" );

	stream = Journal_Register( Info_Type, (Name)self->type  );
//	Journal_Printf( stream, "Generating edge-vertex incidence...\n" );
	Stream_Indent( stream );

	for( edge_i = 0; edge_i < Sync_GetNumDomains( sync ); edge_i++ ) {
		gEdge = Sync_DomainToGlobal( sync, edge_i );

		if( gEdge < grids[1][0]->nPoints ) {
			Grid_Lift( grids[1][0], gEdge, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[0] -= 2;
		}
		else if( gEdge < grids[1][0]->nPoints + grids[1][1]->nPoints ) {
			Grid_Lift( grids[1][1], gEdge - grids[1][0]->nPoints, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[1] -= 2;
		}
		else if( topo->nDims == 3 && gEdge < grids[1][0]->nPoints + grids[1][1]->nPoints + grids[1][2]->nPoints ) {
			Grid_Lift( grids[1][2], gEdge - grids[1][0]->nPoints - grids[1][1]->nPoints, dimInds );

			dimInds[0] *= 2;
			dimInds[1] *= 2;
			if( topo->nDims == 3 )
				dimInds[2] *= 2;

			verts[0] = Grid_Project( grids[0][0], dimInds );
			dimInds[2]++;
			verts[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[2]++;
			verts[2] = Grid_Project( grids[0][0], dimInds );
			dimInds[2] -= 2;
		}

		CartesianGenerator_MapToDomain( (CartesianGenerator*)self, (Sync*)IGraph_GetDomain( topo, MT_VERTEX ), 3, verts );
		IGraph_SetIncidence( topo, MT_EDGE, edge_i, MT_VERTEX, 3, verts );
	}

	Memory_Free( dimInds );

	MPI_Barrier( self->mpiComm );

//	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void C2Generator_GenElementTypes( void* meshGenerator, Mesh* mesh ) {
	C2Generator*	self = (C2Generator*)meshGenerator;
	Stream*		stream;
	unsigned	nDomainEls;
	unsigned	vertMap[8] = {0, 2, 6, 8, 18, 20, 24, 26};
	unsigned	e_i;

	assert( self );

	stream = Journal_Register( Info_Type, (Name)self->type  );
//	Journal_Printf( stream, "Generating element types...\n" );
	Stream_Indent( stream );

	mesh->nElTypes = 1;
	mesh->elTypes = AllocArray( Mesh_ElementType*, mesh->nElTypes );
	mesh->elTypes[0] = (Mesh_ElementType*)Mesh_HexType_New();
	Mesh_ElementType_SetMesh( mesh->elTypes[0], mesh );
	Mesh_HexType_SetVertexMap( mesh->elTypes[0], vertMap );
	nDomainEls = Mesh_GetDomainSize( mesh, Mesh_GetDimSize( mesh ) );
	mesh->elTypeMap = AllocArray( unsigned, nDomainEls );
	for( e_i = 0; e_i < nDomainEls; e_i++ )
		mesh->elTypeMap[e_i] = 0;

	if( self->regular )
		Mesh_SetAlgorithms( mesh, Mesh_RegularAlgorithms_New( "", NULL ) );

	MPI_Barrier( self->mpiComm );
//	Journal_Printf( stream, "... element types are '%s',\n", mesh->elTypes[0]->type );
//	Journal_Printf( stream, "... mesh algorithm type is '%s',\n", mesh->algorithms->type );
//	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


