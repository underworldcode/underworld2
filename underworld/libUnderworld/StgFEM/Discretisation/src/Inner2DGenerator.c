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
#include "Discretisation.h"


/* Textual name of this class */
const Type Inner2DGenerator_Type = "Inner2DGenerator";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Inner2DGenerator* Inner2DGenerator_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Inner2DGenerator);
	Type                                                      type = Inner2DGenerator_Type;
	Stg_Class_DeleteFunction*                              _delete = _Inner2DGenerator_Delete;
	Stg_Class_PrintFunction*                                _print = _Inner2DGenerator_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))_Inner2DGenerator_New;
	Stg_Component_ConstructFunction*                    _construct = _Inner2DGenerator_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Inner2DGenerator_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Inner2DGenerator_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _Inner2DGenerator_Execute;
	Stg_Component_DestroyFunction*                        _destroy = NULL;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	MeshGenerator_SetDimSizeFunc*                   setDimSizeFunc = _MeshGenerator_SetDimSize;
	MeshGenerator_GenerateFunc*                       generateFunc = (MeshGenerator_GenerateFunc*)Inner2DGenerator_Generate;

	Inner2DGenerator* self = _Inner2DGenerator_New(  INNER2DGENERATOR_PASSARGS  );

   _MeshGenerator_Init( (MeshGenerator*)self, context );
   _Inner2DGenerator_Init( self );

   return self;
}

Inner2DGenerator* _Inner2DGenerator_New(  INNER2DGENERATOR_DEFARGS  ) {
	Inner2DGenerator*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Inner2DGenerator) );
	self = (Inner2DGenerator*)_MeshGenerator_New(  MESHGENERATOR_PASSARGS  );

	return self;
}

void _Inner2DGenerator_Init( Inner2DGenerator* self ) {
	assert( self && Stg_CheckType( self, Inner2DGenerator ) );

	self->elMesh = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Inner2DGenerator_Delete( void* generator ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;

	/* Delete the parent. */
	_MeshGenerator_Delete( self );
}

void _Inner2DGenerator_Print( void* generator, Stream* stream ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, (Name)"Inner2DGeneratorStream"  );

	/* Print parent */
	Journal_Printf( stream, "Inner2DGenerator (ptr): (%p)\n", self );
	_MeshGenerator_Print( self, stream );
}

void _Inner2DGenerator_AssignFromXML( void* generator, Stg_ComponentFactory* cf, void* data ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;
	Mesh*		elMesh;

	assert( self );
	assert( cf );

	_MeshGenerator_AssignFromXML( self, cf, data );

	elMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"elementMesh", Mesh, True, data  );
	Inner2DGenerator_SetElementMesh( self, elMesh );
}

void _Inner2DGenerator_Build( void* generator, void* data ) {
	_MeshGenerator_Build( generator, data );
}

void _Inner2DGenerator_Initialise( void* generator, void* data ) {
	_MeshGenerator_Initialise( generator, data );
}

void _Inner2DGenerator_Execute( void* generator, void* data ) {
}

void _Inner2DGenerator_Destroy( void* generator, void* data ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;

   Stg_Component_Destroy( self->elMesh, data, False );
   _MeshGenerator_Destroy( self, data );
}

void Inner2DGenerator_Generate( void* generator, void* _mesh ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;
	FeMesh*		mesh = (FeMesh*)_mesh;
	Grid**		grid;
	Grid*		elGrid;

    /* this generator relies on a parent.. set mesh accordingly  */
    mesh->parentMesh = self->elMesh;

	assert( self && Stg_CheckType( self, Inner2DGenerator ) );
	assert( mesh && Stg_CheckType( mesh, FeMesh ) );

	Inner2DGenerator_BuildTopology( self, mesh );
	Inner2DGenerator_BuildGeometry( self, mesh );
	Inner2DGenerator_BuildElementTypes( self, mesh );

	elGrid = *(Grid**)ExtensionManager_Get( self->elMesh->info, self->elMesh, self->elMesh->elGridId  );
	mesh->elGridId = ExtensionManager_Add( mesh->info, (Name)"elementGrid", sizeof(Grid*) );
	grid = (Grid** )ExtensionManager_Get( mesh->info, mesh, mesh->elGridId );
	*grid = Grid_New( );
	Grid_SetNumDims( *grid, elGrid->nDims );
	Grid_SetSizes( *grid, elGrid->sizes );

	/* this looks like a hack */
        /* linear inner is not an element mesh? */
        mesh->elementMesh = True;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Inner2DGenerator_SetElementMesh( void* generator, void* mesh ) {
	Inner2DGenerator*	self = (Inner2DGenerator*)generator;

	assert( self && Stg_CheckType( self, Inner2DGenerator ) );

	self->elMesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Inner2DGenerator_BuildTopology( Inner2DGenerator* self, FeMesh* mesh ) {
	Mesh*		elMesh;
	MeshTopology	*topo, *elTopo;
	unsigned	nDims;
	unsigned	nIncEls, *incEls;
	unsigned	nDomainEls;
	Decomp		*elDecomp, *nodeDecomp;
	Sync		*elSync, *nodeSync;
	int		nLocals, *locals;
	int		nRemotes, *remotes;
	unsigned	global;
	unsigned	e_i, l_i, r_i;

	assert( self );
	assert( mesh );

	elMesh = self->elMesh;
	nDims = Mesh_GetDimSize( elMesh );
	elTopo = Mesh_GetTopology( elMesh );
	elSync = Mesh_GetSync( elMesh, nDims );

	topo = Mesh_GetTopology( mesh );
	MeshTopology_SetComm( topo, MeshTopology_GetComm( elTopo ) );
	MeshTopology_SetNumDims( topo, nDims );
	IGraph_SetDomain( topo, nDims, elSync );

	/* Need to redefine the nodes, nDims + 1 per parent element. */
	elDecomp = (Decomp*)Sync_GetDecomp( elSync );
	nodeDecomp = Decomp_New();

	if( nDims == 2 ) {
		nLocals = Decomp_GetNumLocals( elDecomp ) * 3;
		locals = MemArray( int, nLocals, Inner2DGenerator_Type );
		for( l_i = 0; l_i < Decomp_GetNumLocals( elDecomp ); l_i++ ) {
			global = Decomp_LocalToGlobal( elDecomp, l_i );
			locals[l_i * 3 + 0] = global * 3;
			locals[l_i * 3 + 1] = global * 3 + 1;
			locals[l_i * 3 + 2] = global * 3 + 2;
		}
	}
	else if( nDims == 3 ) {
		nLocals = Decomp_GetNumLocals( elDecomp ) * 4;
		locals = MemArray( int, nLocals, Inner2DGenerator_Type );
		for( l_i = 0; l_i < Decomp_GetNumLocals( elDecomp ); l_i++ ) {
			global = Decomp_LocalToGlobal( elDecomp, l_i );
			locals[l_i * 4 + 0] = global * 4;
			locals[l_i * 4 + 1] = global * 4 + 1;
			locals[l_i * 4 + 2] = global * 4 + 2;
			locals[l_i * 4 + 3] = global * 4 + 3;
		}
	}
	Decomp_SetLocals( nodeDecomp, nLocals, locals );
	MemFree( locals );

	nodeSync = Sync_New();
	Sync_SetComm( nodeSync, Sync_GetComm( elSync ) );
	Sync_SetDecomp( nodeSync, nodeDecomp );

	if( nDims == 2 ) {
		nRemotes = Sync_GetNumRemotes( elSync ) * 3;
		remotes = MemArray( int, nRemotes, Inner2DGenerator_Type );
		for( r_i = 0; r_i < Sync_GetNumRemotes( elSync ); r_i++ ) {
			global = Sync_RemoteToGlobal( elSync, r_i );
			remotes[r_i * 3 + 0] = global * 3;
			remotes[r_i * 3 + 1] = global * 3 + 1;
			remotes[r_i * 3 + 2] = global * 3 + 2;
		}
	}
	else if( nDims == 3 ) {
		nRemotes = Sync_GetNumRemotes( elSync ) * 4;
		remotes = MemArray( int, nRemotes, Inner2DGenerator_Type );
		for( r_i = 0; r_i < Sync_GetNumRemotes( elSync ); r_i++ ) {
			global = Sync_RemoteToGlobal( elSync, r_i );
			remotes[r_i * 4 + 0] = global * 4;
			remotes[r_i * 4 + 1] = global * 4 + 1;
			remotes[r_i * 4 + 2] = global * 4 + 2;
			remotes[r_i * 4 + 3] = global * 4 + 3;
		}
	}
	Sync_SetRemotes( nodeSync, nRemotes, remotes );
	MemFree( remotes );

	IGraph_SetDomain( topo, 0, nodeSync );

	/* Same shadow depth. */
	topo->shadDepth = elTopo->shadDepth;

	/* Build the incidence. */
	nDomainEls = Mesh_GetDomainSize( elMesh, nDims );
	if( nDims == 2 ) {
		nIncEls = 3;
		incEls = MemArray( unsigned, 3, Inner2DGenerator_Type );
		for( e_i = 0; e_i < nDomainEls; e_i++ ) {
			incEls[0] = e_i * 3;
			incEls[1] = e_i * 3 + 1;
			incEls[2] = e_i * 3 + 2;
			IGraph_SetIncidence( topo, nDims, e_i, 0, nIncEls, (int*)incEls );
		}
	}
	else if( nDims == 3 ) {
		nIncEls = 4;
		incEls = MemArray( unsigned, 4, Inner2DGenerator_Type );
		for( e_i = 0; e_i < nDomainEls; e_i++ ) {
			incEls[0] = e_i * 4;
			incEls[1] = e_i * 4 + 1;
			incEls[2] = e_i * 4 + 2;
			incEls[3] = e_i * 4 + 3;
			IGraph_SetIncidence( topo, nDims, e_i, 0, nIncEls, (int*)incEls );
		}
	}
	FreeArray( incEls );

	IGraph_InvertIncidence( topo, MT_VERTEX, nDims );
}

void Inner2DGenerator_BuildGeometry( Inner2DGenerator* self, FeMesh* mesh ) {
	Mesh*		elMesh;
	double		localCrds[3][2] = {{-0.5, -0.5}, 
                                   {0.5, -0.5}, 
                                   {0, 0.5}};
	double		globalCrd[2];
	double		*vert;
	unsigned	nDims;
	unsigned	nDomainEls;
	unsigned	e_i;

	assert( self );
	assert( mesh );

	elMesh = self->elMesh;
	nDims = Mesh_GetDimSize( elMesh );
	nDomainEls = Mesh_GetDomainSize( elMesh, nDims );

	if( nDims == 2 ) {
        Mesh_GenerateVertices( mesh, nDomainEls * 3, nDims );

		for( e_i = 0; e_i < nDomainEls; e_i++ ) {
			unsigned elInd = e_i * 3;

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds[0], globalCrd );
			vert = Mesh_GetVertex( mesh, elInd );
			memcpy( vert, globalCrd, nDims * sizeof(double) );

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds[1], globalCrd );
			vert = Mesh_GetVertex( mesh, elInd + 1 );
			memcpy( vert, globalCrd, nDims * sizeof(double) );

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds[2], globalCrd );
			vert = Mesh_GetVertex( mesh, elInd + 2 );
			memcpy( vert, globalCrd, nDims * sizeof(double) );
		}
	}

	else if( nDims == 3 ) {
		double localCrds3D[4][3] = { {-0.5, -0.5, -0.5},
		                             {0.25, 0.25, 0.25},
		                             {0.5, -0.25, -0.5},
		                             {-0.25, 0.5, 0.5} };
		double globalCrd3D[3];

        Mesh_GenerateVertices( mesh, nDomainEls * 4, nDims );

		for( e_i = 0; e_i < nDomainEls; e_i++ ) {
			unsigned elInd = e_i * 4;

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds3D[0], globalCrd3D );
			vert = Mesh_GetVertex( mesh, elInd );
			memcpy( vert, globalCrd3D, nDims * sizeof(double) );

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds3D[1], globalCrd3D );
			vert = Mesh_GetVertex( mesh, elInd + 1 );
			memcpy( vert, globalCrd3D, nDims * sizeof(double) );

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds3D[2], globalCrd3D );
			vert = Mesh_GetVertex( mesh, elInd + 2 );
			memcpy( vert, globalCrd3D, nDims * sizeof(double) );

			FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds3D[3], globalCrd3D );
			vert = Mesh_GetVertex( mesh, elInd + 3 );
			memcpy( vert, globalCrd3D, nDims * sizeof(double) );
		}
	}
}

void Inner2DGenerator_BuildElementTypes( Inner2DGenerator* self, FeMesh* mesh ) {
	unsigned		nDomainEls;
	Mesh_Algorithms*	algs;
	unsigned		e_i;

	assert( self );
	assert( mesh );

	mesh->nElTypes = 1;
	mesh->elTypes = AllocNamedArray( Mesh_ElementType*, mesh->nElTypes, "Mesh::elTypes" );
	mesh->elTypes[0] = (Mesh_ElementType*)Mesh_CentroidType_New();
	Mesh_ElementType_SetMesh( mesh->elTypes[0], mesh );
	Mesh_CentroidType_SetElementMesh( mesh->elTypes[0], self->elMesh );
	nDomainEls = Mesh_GetDomainSize( mesh, Mesh_GetDimSize( mesh ) );
	mesh->elTypeMap = AllocNamedArray( unsigned, nDomainEls, "Mesh::elTypeMap" );
	for( e_i = 0; e_i < nDomainEls; e_i++ )
		mesh->elTypeMap[e_i] = 0;

	algs = Mesh_CentroidAlgorithms_New( "", NULL );
	Mesh_CentroidAlgorithms_SetElementMesh( algs, self->elMesh );
	Mesh_SetAlgorithms( mesh, algs );
}


