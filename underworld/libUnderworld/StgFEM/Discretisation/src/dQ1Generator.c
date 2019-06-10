/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "Discretisation.h"


/* Textual name of this class */
const Type dQ1Generator_Type = "dQ1Generator";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

dQ1Generator* dQ1Generator_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(dQ1Generator);
	Type                                                      type = dQ1Generator_Type;
	Stg_Class_DeleteFunction*                              _delete = _dQ1Generator_Delete;
	Stg_Class_PrintFunction*                                _print = _dQ1Generator_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))_dQ1Generator_New;
	Stg_Component_ConstructFunction*                    _construct = _dQ1Generator_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _dQ1Generator_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _dQ1Generator_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _dQ1Generator_Execute;
	Stg_Component_DestroyFunction*                        _destroy = NULL;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	MeshGenerator_SetDimSizeFunc*                   setDimSizeFunc = _MeshGenerator_SetDimSize;
	MeshGenerator_GenerateFunc*                       generateFunc = (MeshGenerator_GenerateFunc*)dQ1Generator_Generate;

	dQ1Generator* self = _dQ1Generator_New(  DQ1GENERATOR_PASSARGS  );

   _MeshGenerator_Init( (MeshGenerator*)self, context );
   _dQ1Generator_Init( self );

   return self;
}

dQ1Generator* _dQ1Generator_New(  DQ1GENERATOR_DEFARGS  ) {
	dQ1Generator*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(dQ1Generator) );
	self = (dQ1Generator*)_MeshGenerator_New(  MESHGENERATOR_PASSARGS  );

	return self;
}

void _dQ1Generator_Init( dQ1Generator* self ) {
	assert( self && Stg_CheckType( self, dQ1Generator ) );

	self->elMesh = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _dQ1Generator_Delete( void* generator ) {
	dQ1Generator*	self = (dQ1Generator*)generator;

	/* Delete the parent. */
	_MeshGenerator_Delete( self );
}

void _dQ1Generator_Print( void* generator, Stream* stream ) {
	dQ1Generator*	self = (dQ1Generator*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, (Name)"dQ1GeneratorStream"  );

	/* Print parent */
	Journal_Printf( stream, "dQ1Generator (ptr): (%p)\n", self );
	_MeshGenerator_Print( self, stream );
}

void _dQ1Generator_AssignFromXML( void* generator, Stg_ComponentFactory* cf, void* data ) {
	dQ1Generator*	self = (dQ1Generator*)generator;
	Mesh*		elMesh;

	assert( self );
	assert( cf );

	_MeshGenerator_AssignFromXML( self, cf, data );

	elMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"elementMesh", Mesh, True, data  );
	dQ1Generator_SetElementMesh( self, elMesh );
}

void _dQ1Generator_Build( void* generator, void* data ) {
	_MeshGenerator_Build( generator, data );
}

void _dQ1Generator_Initialise( void* generator, void* data ) {
	_MeshGenerator_Initialise( generator, data );
}

void _dQ1Generator_Execute( void* generator, void* data ) {
}

void _dQ1Generator_Destroy( void* generator, void* data ) {
	dQ1Generator*	self = (dQ1Generator*)generator;

   Stg_Component_Destroy( self->elMesh, data, False );
   _MeshGenerator_Destroy( self, data );
}

void dQ1Generator_Generate( void* generator, void* _mesh ) {
	dQ1Generator*	self = (dQ1Generator*)generator;
	FeMesh*		mesh = (FeMesh*)_mesh;
	Grid**		grid;
	Grid*		elGrid;

    /* this generator relies on a parent.. set mesh accordingly  */
    mesh->parentMesh = self->elMesh;

	assert( self && Stg_CheckType( self, dQ1Generator ) );
	assert( mesh && Stg_CheckType( mesh, FeMesh ) );

	dQ1Generator_BuildTopology( self, mesh );
	dQ1Generator_BuildGeometry( self, mesh );
	dQ1Generator_BuildElementTypes( self, mesh );

	elGrid = *(Grid**)ExtensionManager_Get( self->elMesh->info, self->elMesh, self->elMesh->elGridId  );
	mesh->elGridId = ExtensionManager_Add( mesh->info, (Name)"elementGrid", sizeof(Grid*) );
	grid = (Grid** )ExtensionManager_Get( mesh->info, mesh, mesh->elGridId );
	*grid = Grid_New( );
	Grid_SetNumDims( *grid, elGrid->nDims );
	Grid_SetSizes( *grid, elGrid->sizes );

	/* what is this? */
    mesh->elementMesh = True;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void dQ1Generator_SetElementMesh( void* generator, void* mesh ) {
	dQ1Generator*	self = (dQ1Generator*)generator;

	assert( self && Stg_CheckType( self, dQ1Generator ) );

	self->elMesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void dQ1Generator_BuildTopology( dQ1Generator* self, FeMesh* mesh ) {
	Mesh*		   elMesh;
	MeshTopology   *topo, *elTopo;
	unsigned	   nDims;
	unsigned	   nIncEls, *incEls;
	unsigned	   nDomainEls;
	Decomp		   *elDecomp, *nodeDecomp;
	Sync		   *elSync, *nodeSync;
	int		       nLocals, *locals;
	int		       nRemotes, *remotes;
	unsigned	   global;
	unsigned	   e_i, l_i, r_i;
    unsigned       numNodes;

	assert( self );
	assert( mesh );

	elMesh = self->elMesh;
	nDims  = Mesh_GetDimSize ( elMesh );
	elTopo = Mesh_GetTopology( elMesh );
	elSync = Mesh_GetSync    ( elMesh, nDims );

	topo = Mesh_GetTopology( mesh );
	MeshTopology_SetComm   ( topo, MeshTopology_GetComm( elTopo ) );
	MeshTopology_SetNumDims( topo, nDims );
	IGraph_SetDomain       ( topo, nDims, elSync );

	/* Need to redefine the nodes */
	elDecomp   = (Decomp*)Sync_GetDecomp( elSync );
	nodeDecomp = Decomp_New();
    
    numNodes=1<<nDims;/* 4 in 2D, 8 in 3D */
    
    nLocals = Decomp_GetNumLocals( elDecomp ) * numNodes;
    locals  = MemArray( int, nLocals, dQ1Generator_Type );
    for( l_i = 0; l_i < Decomp_GetNumLocals( elDecomp ); l_i++ ) {
      global = Decomp_LocalToGlobal( elDecomp, l_i );
      int node;
      for(node=0; node<numNodes; node++){
        locals[l_i * numNodes + node] = global * numNodes + node;
      }
    }

	Decomp_SetLocals( nodeDecomp, nLocals, locals );
	MemFree( locals );

	nodeSync = Sync_New();
	Sync_SetComm  ( nodeSync, Sync_GetComm( elSync ) );
	Sync_SetDecomp( nodeSync, nodeDecomp );

    nRemotes = Sync_GetNumRemotes( elSync ) * numNodes;
    remotes  = MemArray( int, nRemotes, dQ1Generator_Type );
    for( r_i = 0; r_i < Sync_GetNumRemotes( elSync ); r_i++ ) {
      global = Sync_RemoteToGlobal( elSync, r_i );
      int node;
      for(node=0; node<numNodes; node++){
        remotes[r_i * numNodes + node] = global * numNodes + node;
      }
    }
	Sync_SetRemotes( nodeSync, nRemotes, remotes );
	MemFree( remotes );

	IGraph_SetDomain( topo, 0, nodeSync );

	/* Same shadow depth. */
	topo->shadDepth = elTopo->shadDepth;

	/* Build the incidence. */
	nDomainEls = Mesh_GetDomainSize( elMesh, nDims );

    nIncEls = numNodes;
    incEls  = MemArray( unsigned, numNodes, dQ1Generator_Type );
    for( e_i = 0; e_i < nDomainEls; e_i++ ) {
      int node;
      for(node=0; node<numNodes; node++){
        incEls[node] = e_i * numNodes + node;
      }
      IGraph_SetIncidence( topo, nDims, e_i, 0, nIncEls, (int*)incEls );
    }
	FreeArray( incEls );

	IGraph_InvertIncidence( topo, MT_VERTEX, nDims );
}

void dQ1Generator_BuildGeometry( dQ1Generator* self, FeMesh* mesh ) {
	Mesh*		elMesh;
	double		localCrds[4][2] = {{-0.5, -0.5}, 
                                   { 0.5, -0.5}, 
                                   { 0.5,  0.5},
                                   {-0.5,  0.5} };
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
      Mesh_GenerateVertices( mesh, nDomainEls * 4, nDims );
      for( e_i = 0; e_i < nDomainEls; e_i++ ) {
        unsigned elInd = e_i * 4;
        int node;
        for(node=0; node<4; node++){
          FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds[node], globalCrd );
          vert = Mesh_GetVertex( mesh, elInd + node );
          memcpy( vert, globalCrd, nDims * sizeof(double) );
        }
      }//for
	}//if nDims == 2
	else if( nDims == 3 ) {
      double localCrds3D[8][3] = {{-0.5, -0.5, -0.5}, 
                                  { 0.5, -0.5, -0.5}, 
                                  { 0.5,  0.5, -0.5},
                                  {-0.5,  0.5, -0.5},  
                                  {-0.5, -0.5,  0.5},
                                  { 0.5, -0.5,  0.5},
                                  { 0.5,  0.5,  0.5},
                                  {-0.5,  0.5,  0.5} };
      double globalCrd3D[3];
      Mesh_GenerateVertices( mesh, nDomainEls * 8, nDims );     
      for( e_i = 0; e_i < nDomainEls; e_i++ ) {
        unsigned elInd = e_i * 8;
        int node;
        node;
        for(node=0; node<8; node++){
          FeMesh_CoordLocalToGlobal( elMesh, e_i, localCrds3D[node], globalCrd3D );
          vert = Mesh_GetVertex( mesh, elInd + node );
          memcpy( vert, globalCrd3D, nDims * sizeof(double) );
        }
      }//for
	}//else nDims == 3
}

void dQ1Generator_BuildElementTypes( dQ1Generator* self, FeMesh* mesh ) {
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


