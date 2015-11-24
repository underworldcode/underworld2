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
#include <StgDomain/Geometry/Geometry.h>

#include "Mesh.h"


/* Textual name of this class */
const Type Mesh_Type = "Mesh";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh* Mesh_New( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Mesh);
	Type                                                      type = Mesh_Type;
	Stg_Class_DeleteFunction*                              _delete = _Mesh_Delete;
	Stg_Class_PrintFunction*                                _print = _Mesh_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))_Mesh_New;
	Stg_Component_ConstructFunction*                    _construct = _Mesh_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Mesh_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Mesh_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _Mesh_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _Mesh_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	Mesh* self = _Mesh_New(  MESH_PASSARGS  );

	_Mesh_Init( self );
   return self;
}

Mesh* _Mesh_New(  MESH_DEFARGS  ) {
	Mesh* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh) );
	self = (Mesh*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	self->topo = (MeshTopology*)IGraph_New( "" );
	self->vertices = NULL;
    self->verticesVariable = NULL;
    self->vGlobalIdsVar = NULL;
    self->verticesgid = NULL;
    self->e_n = NULL;
    self->enMapVar = NULL;
    self->elgid = NULL;
    self->eGlobalIdsVar = NULL;

	self->vars = List_New();
	List_SetItemSize( self->vars, sizeof(MeshVariable*) );

	self->minSep = 0.0;
	self->minAxialSep = NULL;
	self->minLocalCrd = NULL;
	self->maxLocalCrd = NULL;
	self->minDomainCrd = NULL;
	self->maxDomainCrd = NULL;
	self->minGlobalCrd = NULL;
	self->maxGlobalCrd = NULL;

	self->algorithms = Mesh_Algorithms_New( "", NULL );
	Mesh_Algorithms_SetMesh( self->algorithms, self );
	self->nElTypes = 0;
	self->elTypes = NULL;
	self->elTypeMap = NULL;

	self->topoDataSizes = UIntMap_New();
	self->topoDataInfos = NULL;
	self->topoDatas = NULL;
	self->info = ExtensionManager_New_OfExistingObject( "mesh_info", self );
	self->vertGridId = (unsigned)-1;
	self->elGridId = (unsigned)-1;
	self->periodicId = (unsigned)-1;
	self->localOriginId = (unsigned)-1;
	self->localRangeId = (unsigned)-1;

	self->generator = NULL;
	self->emReg = NULL;

	self->isCheckpointedAndReloaded = False;
	self->isDeforming     = False;

	self->isRegular = False;
    self->parentMesh = NULL;

	return self;
}

void _Mesh_Init( Mesh* self ) {}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_Delete( void* mesh ) {
	Mesh*	self = (Mesh*)mesh;

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Mesh_Print( void* mesh, Stream* stream ) {
	Mesh*	self = (Mesh*)mesh;
	
	/* Print parent */
	Journal_Printf( stream, "Mesh (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void _Mesh_AssignFromXML( void* mesh, Stg_ComponentFactory* cf, void* data ) {
	Mesh*			self = (Mesh*)mesh;

	_Mesh_Init( self );
}

void _Mesh_Build( void* mesh, void* data ) {
	Mesh*			self = (Mesh*)mesh;
	unsigned		nDims;
	unsigned		d_i;

	assert( self );

    if( self->generator ) {
        Stg_Component_Build( self->generator, data, False );
        MeshGenerator_Generate( self->generator, self, data );
    }

	if( self->algorithms ) Stg_Component_Build( self->algorithms, data, False );

	nDims = Mesh_GetDimSize( self );
	if( !nDims ){
        self->isBuilt = False;
        return;
    }

	self->topoDataInfos = Memory_Alloc_Array( ExtensionManager*, nDims, "mesh::topoDataInfos" );
	self->topoDatas = Memory_Alloc_Array( void*, nDims, "mesh::topoDatas" );

	for( d_i = 0; d_i < nDims; d_i++ ) {
		char		name[20];
		unsigned	size;

		if( !UIntMap_Map( self->topoDataSizes, d_i, &size ) || !size ||
		    !Mesh_GetDomainSize( self, d_i ) )
		{
			self->topoDataInfos[d_i] = NULL;
			self->topoDatas[d_i] = NULL;
			continue;
		}

		sprintf( name, "topoData(%d)", d_i );
		self->topoDataInfos[d_i] = ExtensionManager_New_OfStruct( name, size );
		self->topoDatas[d_i] = (void*)ExtensionManager_Malloc( self->topoDataInfos[d_i], Mesh_GetDomainSize( self, d_i ) );
	}

	/*
	** Set up the geometric information.
	*/

	self->minAxialSep = Memory_Alloc_Array( double, nDims, "Mesh::minAxialSep" );
	self->minLocalCrd = Memory_Alloc_Array( double, nDims, "Mesh::minLocalCrd" );
	self->maxLocalCrd = Memory_Alloc_Array( double, nDims, "Mesh::maxLocalCrd" );
	self->minDomainCrd = Memory_Alloc_Array( double, nDims, "Mesh::minLocalCrd" );
	self->maxDomainCrd = Memory_Alloc_Array( double, nDims, "Mesh::maxLocalCrd" );
	self->minGlobalCrd = Memory_Alloc_Array( double, nDims, "Mesh::minGlobalCrd" );
	self->maxGlobalCrd = Memory_Alloc_Array( double, nDims, "Mesh::maxGlobalCrd" );

	Mesh_DeformationUpdate( self );

}

void _Mesh_Initialise( void* mesh, void* data ) {
	Mesh*	self = (Mesh*)mesh;

	if( self->algorithms ) Stg_Component_Initialise( self->algorithms, data, False );
	if( self->generator ) Stg_Component_Initialise( self->generator, data, False );


}

void _Mesh_Execute( void* mesh, void* data ) {
}

void _Mesh_Destroy( void* mesh, void* data ) {
   Mesh*		self = (Mesh*)mesh;
	unsigned	d_i;

   Mesh_Destruct( self );
   Stg_Component_Destroy( self->algorithms, NULL, False );
   Stg_Class_Delete( self->info );
   Stg_Class_Delete( self->vars );
   Stg_Class_Delete( self->topoDataSizes );

	for( d_i = 0; d_i < Mesh_GetDimSize( self ); d_i++ ) {
		if( self->topoDataInfos[d_i] )	
			Stg_Class_Delete( self->topoDataInfos[d_i] );
		if( self->topoDatas[d_i] )
			Memory_Free( self->topoDatas[d_i] );
	}
    if (self->topoDataInfos) Memory_Free( self->topoDataInfos );
    if (self->topoDatas)     Memory_Free( self->topoDatas );

    Stg_Class_Delete( self->topo );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Mesh_SetExtensionManagerRegister( void* mesh, void* extMgrReg ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	self->emReg = extMgrReg;
	if( extMgrReg )
		ExtensionManager_Register_Add( extMgrReg, self->info );
}

void Mesh_SetTopologyDataSize( void* mesh, MeshTopology_Dim topodim, unsigned size ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	UIntMap_Insert( self->topoDataSizes, topodim, size );
}

void Mesh_SetGenerator( void* mesh, void* generator ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	Mesh_Destruct( self );
	self->generator = generator;
}

void Mesh_SetAlgorithms( void* mesh, void* algorithms ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self && Stg_CheckType( self, Mesh ) );

	FreeObject( self->algorithms );
	if( algorithms ) {
		assert( Stg_CheckType( algorithms, Mesh_Algorithms ) );
		self->algorithms = algorithms;
	}
	else
		self->algorithms = Mesh_Algorithms_New( "", NULL );

	Mesh_Algorithms_SetMesh( self->algorithms, self );
}

unsigned Mesh_GetDimSize( void* mesh ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return self->topo->nDims;
}

unsigned Mesh_GetGlobalSize( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Decomp_GetNumGlobals( Sync_GetDecomp( IGraph_GetDomain( self->topo, topodim ) ) );
}
/*@
  Mesh_GetLocalSize
  -returns number of entities on local processor
  Example:
  to get the number of nodes on local processor in Mesh
  numLocalNodes = Mesh_GetLocalSize( Mesh,  MT_VERTEX);
  
  MT_VERTEX is a MeshTopology_Dim and denotes a node as opposed to an edge say.
  (see StgDomain/Mesh/src/types.h)

  Note:
  "Local" refers to entities on local processor that are in the domain of that the current processor.
  "Domain" refers to "Local" entities as well as entities from the shadowed region.
  So Domain is a superset of Local.

  @*/
unsigned Mesh_GetLocalSize( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

        return Decomp_GetNumLocals( Sync_GetDecomp( IGraph_GetDomain( self->topo, topodim ) ) );
}

unsigned Mesh_GetRemoteSize( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_GetNumRemotes( IGraph_GetDomain( self->topo, topodim ) );
}

unsigned Mesh_GetDomainSize( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

        return Sync_GetNumDomains( IGraph_GetDomain( self->topo, topodim ) );
}

unsigned Mesh_GetSharedSize( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_GetNumShared( IGraph_GetDomain( self->topo, topodim ) );
}

MeshTopology* Mesh_GetTopology( void* mesh ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return self->topo;
}

Sync* Mesh_GetSync( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return (Sync*)IGraph_GetDomain( self->topo, topodim );
}

Bool Mesh_GlobalToDomain( void* mesh, MeshTopology_Dim topodim, unsigned global, unsigned* domain ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_TryGlobalToDomain( IGraph_GetDomain( self->topo, topodim ), global, domain );
}
/*@
  Mesh_DomainToGlobal
  -maps a locally numbered entity to that entity's global number.
   (e.g. entity is a node if topodim=MT_VERTEX (see StgDomain/Mesh/src/types.h))
  Example:

  globalNodeNumber=Mesh_DomainToGlobal( feMesh, MT_VERTEX, localNodeNumber);
  
  Note:
  "Local" refers to entities on local processor that are in the domain of that the current processor.
  "Domain" refers to "Local" entities as well as entities from the shadowed region.
  So Domain is a superset of Local.

  This function can be used with "Local" indices as well as "Domain" indices
  because it is generally assumed that any Shadow entity is always numbered after a Local
  entity. So the Local and Domain indices are equal for all "Local" indices.

  @*/
unsigned Mesh_DomainToGlobal( void* mesh, MeshTopology_Dim topodim, unsigned domain ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_DomainToGlobal( IGraph_GetDomain( self->topo, topodim ), domain );
}

Bool Mesh_LocalToShared( void* mesh, MeshTopology_Dim topodim, unsigned domain, unsigned* shared ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_TryLocalToShared( IGraph_GetDomain( self->topo, topodim ), domain, shared );
}

unsigned Mesh_SharedToLocal( void* mesh, MeshTopology_Dim topodim, unsigned shared ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return Sync_SharedToLocal( IGraph_GetDomain( self->topo, topodim ), shared );
}

unsigned Mesh_GetOwner( void* mesh, MeshTopology_Dim topodim, unsigned remote ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return Sync_GetOwner( IGraph_GetDomain( self->topo, topodim ), remote );
}

void Mesh_GetSharers(
	void*					mesh,
	MeshTopology_Dim	topodim,
	unsigned				shared, 
	unsigned*			nSharers,
	unsigned**			sharers )
{
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	Sync_GetSharers( IGraph_GetDomain( self->topo, topodim ), shared, nSharers, sharers );
}

Bool Mesh_HasIncidence( void* mesh, MeshTopology_Dim fromDim, MeshTopology_Dim toDim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	return IGraph_HasIncidence( self->topo, fromDim, toDim );
}

unsigned Mesh_GetIncidenceSize( void* mesh, MeshTopology_Dim fromDim, unsigned fromInd, MeshTopology_Dim toDim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return IGraph_GetIncidenceSize( self->topo, fromDim, fromInd, toDim );
}

void Mesh_GetIncidence( void* mesh, MeshTopology_Dim fromDim, unsigned fromInd, MeshTopology_Dim toDim, IArray* inc ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topo );

	MeshTopology_GetIncidence( self->topo, fromDim, fromInd, toDim, inc );
}

unsigned Mesh_NearestVertex( void* mesh, double* point ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return Mesh_Algorithms_NearestVertex( self->algorithms, point );
}

Bool Mesh_Search( void* mesh, double* point, MeshTopology_Dim* topodim, unsigned* ind ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self && Stg_CheckType( self, Mesh ) );

	return Mesh_Algorithms_Search( self->algorithms, point, topodim, ind );
}

/*@
 * Mesh_SearchElements (
 * mesh -- is a mesh
 * point -- is a global coordinate
 * elInd -- will be filled in by a local elementID
 * )
 * returns:
 * False if the point is not in the DOMAIN space of the proc 
 * True if the point is in the DOMAIN space
 *
@*/
Bool Mesh_SearchElements( void* mesh, double* point, unsigned* elInd ) {
	Mesh*	self = (Mesh*)mesh;

/* 	assert( self && Stg_CheckType( self, Mesh ) ); */

	return Mesh_Algorithms_SearchElements( self->algorithms, point, elInd );
}

Bool Mesh_ElementHasPoint( void* mesh, unsigned element, double* point, 
			   MeshTopology_Dim* topodim, unsigned* ind )
{
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( element < Mesh_GetDomainSize( self, Mesh_GetDimSize( self ) ) );
	assert( self->elTypeMap );
	assert( self->elTypeMap[element] < self->nElTypes );
	assert( self->elTypes[self->elTypeMap[element]] );

	return Mesh_ElementType_ElementHasPoint( self->elTypes[self->elTypeMap[element]], element, point, 
						 topodim, ind );
}

Mesh_ElementType* Mesh_GetElementType( void* mesh, unsigned element ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( element < Mesh_GetDomainSize( self, Mesh_GetDimSize( self ) ) );
	assert( self->elTypeMap );
	assert( self->elTypeMap[element] < self->nElTypes );
	assert( self->elTypes );

	return self->elTypes[self->elTypeMap[element]];
}

Comm* Mesh_GetCommTopology( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	return (Comm*)MeshTopology_GetComm( self->topo );
}

double* Mesh_GetVertex( void* mesh, unsigned domain ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( domain < Mesh_GetDomainSize( self, MT_VERTEX ) );
	assert( self->vertices );

	return self->vertices + domain*Mesh_GetDimSize(self);
}

Bool Mesh_HasExtension( void* mesh, const char* name ) {
	Mesh* self = (Mesh*)mesh;

	assert( self );

	return (ExtensionManager_GetHandle( self->info, (Name)name ) != -1 ) ?  True : False;
}

void* _Mesh_GetExtension( void* mesh, const unsigned int id ) {
	Mesh* self = (Mesh*)mesh;

	assert( self );
	assert( id != -1  );

	return ExtensionManager_Get( self->info, self, id  );
}

void* Mesh_GetTopologyData( void* mesh, MeshTopology_Dim topodim ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( self->topoDatas );
	assert( topodim < Mesh_GetDimSize( self ) );

	return self->topoDatas[topodim];
}

void Mesh_GetMinimumSeparation( void* mesh, double* minSep, double* axial ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( minSep );

	*minSep = self->minSep;
	if( axial )
		memcpy( axial, self->minAxialSep, Mesh_GetDimSize( self ) * sizeof(double) );
}

void Mesh_GetLocalCoordRange( void* mesh, double* min, double* max ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( min );
	assert( max );

	memcpy( min, self->minLocalCrd, Mesh_GetDimSize( self ) * sizeof(double) );
	memcpy( max, self->maxLocalCrd, Mesh_GetDimSize( self ) * sizeof(double) );
}

void Mesh_GetDomainCoordRange( void* mesh, double* min, double* max ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );
	assert( min );
	assert( max );

	memcpy( min, self->minDomainCrd, Mesh_GetDimSize( self ) * sizeof(double) );
	memcpy( max, self->maxDomainCrd, Mesh_GetDimSize( self ) * sizeof(double) );
}

void Mesh_GetGlobalCoordRange( void* mesh, double* min, double* max ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	memcpy( min, self->minGlobalCrd, Mesh_GetDimSize( self ) * sizeof(double) );
	memcpy( max, self->maxGlobalCrd, Mesh_GetDimSize( self ) * sizeof(double) );
}

void Mesh_DeformationUpdate( void* mesh ) {
	Mesh*	self = (Mesh*)mesh;

	assert( self );

	if( Mesh_GetDomainSize( self, 0 ) ) {
		self->minSep = Mesh_Algorithms_GetMinimumSeparation( self->algorithms, self->minAxialSep );
		Mesh_Algorithms_GetLocalCoordRange( self->algorithms, self->minLocalCrd, self->maxLocalCrd );
		Mesh_Algorithms_GetDomainCoordRange( self->algorithms, self->minDomainCrd, self->maxDomainCrd );
		Mesh_Algorithms_GetGlobalCoordRange( self->algorithms, self->minGlobalCrd, self->maxGlobalCrd );

		Mesh_Algorithms_Update( self->algorithms );
        Mesh_ElementType_Update( self->elTypes[0] );
	}
}

Variable* Mesh_GenerateNodeGlobalIdVar( void* mesh ) {
    /* Returns a Variable that stores the global node indices.
     * Assumes the number of mesh nodes never changes.
     */

    Mesh* self = (Mesh*)mesh;
    char* name;
    int lVerts;

    // if variable already exists return it
    if( self->vGlobalIdsVar ) return self->vGlobalIdsVar;

    // Create the Variable data structure, int[local node count] 
    lVerts = Mesh_GetLocalSize( self, 0 );
    self->verticesgid = Memory_Alloc_Array( int, lVerts, "Mesh::vertsgid" );
    Stg_asprintf( &name, "%s-%s", self->name, "verticesGlobalIds" );
    self->vGlobalIdsVar = Variable_NewScalar( name, NULL, Variable_DataType_Int, (unsigned*)&lVerts, NULL, (void**)&self->verticesgid, NULL );
    Stg_Component_Build(self->vGlobalIdsVar, NULL, False);
    Stg_Component_Initialise(self->vGlobalIdsVar, NULL, False);
    free(name);

    // Evaluate the global indices for the local nodes
    int ii, gid;
    for( ii=0; ii<lVerts; ii++ ) {
        gid = Mesh_DomainToGlobal( self, MT_VERTEX, ii );
        Variable_SetValue( self->vGlobalIdsVar, ii, (void*)&gid );
    }

    // return new variable
    return self->vGlobalIdsVar;
}

Variable* Mesh_GenerateElGlobalIdVar( void* mesh ) {
    /* Returns a Variable that stores the global element indices.
     * Assumes the number of mesh elements never changes.
     */

    Mesh* self = (Mesh*)mesh;
    char* name;
    int lEls;
    unsigned dim;

    // if variable already exists return it
    if( self->eGlobalIdsVar ) return self->eGlobalIdsVar;

    dim = Mesh_GetDimSize(mesh);
    // Create the Variable data structure, int[local node count] 
    lEls = Mesh_GetLocalSize( self, dim );
    self->elgid = Memory_Alloc_Array( int, lEls, "Mesh::vertsgid" );
    Stg_asprintf( &name, "%s-%s", self->name, "verticesGlobalIds" );
    self->eGlobalIdsVar = Variable_NewScalar( name, NULL, Variable_DataType_Int, (unsigned*)&lEls, NULL, (void**)&self->elgid, NULL );
    Stg_Component_Build(self->eGlobalIdsVar, NULL, False);
    Stg_Component_Initialise(self->eGlobalIdsVar, NULL, False);
    free(name);

    // Evaluate the global indices for the local nodes
    int ii, gid;
    for( ii=0; ii<lEls; ii++ ) {
        gid = Mesh_DomainToGlobal( self, dim, ii );
        Variable_SetValue( self->eGlobalIdsVar, ii, (void*)&gid );
    }

    // return new variable
    return self->eGlobalIdsVar;
}


Variable* Mesh_GenerateENMapVar( void* mesh ) {
    /* Returns a Variable that stores the mapping of 
     * [local element] [global node indices]
     * Assumes the mapping never changes.
     */

    Mesh* self = (Mesh*)mesh;
    char* name;
    int n_i, e_i, nNbr, localElements, localtotal;
    unsigned buffy_tvs;     // buffer for global node indices
    unsigned dim, *nbr, temp;
    int *numberNodesPerEl = NULL;
    IArray* inc = NULL;
    Stream* error = Journal_Register( Error_Type, (Name)self->type );

    // if variable already exists return it
    if( self->enMapVar ) return self->enMapVar;

    /* go over local elementNode map to get size of data */
    inc = IArray_New( );
    localtotal=0;
    dim = Mesh_GetDimSize( self );
    localElements = Mesh_GetLocalSize( self, dim );
    numberNodesPerEl = Memory_Alloc_Array( int, localElements, "Mesh::numberNodesPerEl" );

    for( e_i=0 ; e_i < localElements; e_i++ ) {
        Mesh_GetIncidence( self, dim, (unsigned)e_i, MT_VERTEX, inc );
        nNbr = IArray_GetSize( inc );
        nbr = IArray_GetPtr( inc );

        numberNodesPerEl[e_i] = nNbr;
        localtotal += nNbr;
    }
    
    /* Create the Variable data structure, int[nbrNodesPerEl*local element count]
     * Note: this is stored in a 1D array - so whatever read or writes to this variable 
     * needs to know how to parse it. */ 
    self->e_n = Memory_Alloc_Array( int, localtotal, "Mesh::nodeConn" );
    Stg_asprintf( &name, "%s-%s", self->name, "nodeConn" );
    self->enMapVar = Variable_NewScalar( name, NULL, Variable_DataType_Int, 
            (unsigned*)&localtotal, NULL, (void**)&self->e_n, NULL );
    Stg_Component_Build(self->enMapVar, NULL, False);
    Stg_Component_Initialise(self->enMapVar, NULL, False);
    free(numberNodesPerEl);
    free(name);

    // Evaluate the global indices for the local nodes
    localtotal=0;
    for( e_i=0; e_i<localElements; e_i++ ) {
        Mesh_GetIncidence( self, dim, (unsigned)e_i, MT_VERTEX, inc );
        nNbr = IArray_GetSize( inc );
        nbr = IArray_GetPtr( inc );
        for( n_i=0; n_i< nNbr; n_i++ ) {
            buffy_tvs = Mesh_DomainToGlobal( self, MT_VERTEX, nbr[n_i] );
            Variable_SetValue( self->enMapVar, localtotal, (void*)&buffy_tvs );
            localtotal++;
        }
    }

    Stg_Class_Delete( inc );

    // return new variable
    return self->enMapVar;
}

void Mesh_GenerateVertices( void* mesh, unsigned nVerts, unsigned nDims ){
	Mesh* self = (Mesh*)mesh;

    if (self->vertices)
        return;

    self->vertices = Memory_Alloc_Array( double, nDims*nVerts, "Mesh::verts" );

    char* name;
    /* Create name for normal variable */
    Stg_asprintf( &name, "%s-%s", self->name, "vertices" );
    self->verticesVariable = Variable_NewVector( name, NULL, Variable_DataType_Double, nDims, &nVerts, NULL, (void**)&self->vertices, NULL, "vert_i", "vert_j", "vert_j" );
    Stg_Component_Build(self->verticesVariable, NULL, False);
    Stg_Component_Initialise(self->verticesVariable, NULL, False);
    free(name);
}



void Mesh_Sync( void* mesh ) {
	Mesh*	self = (Mesh*)mesh;
	const Sync* sync;
	int nDims, nLocals, nDomains;

	assert( self );

	sync = Mesh_GetSync( self, 0 );
	nDims = Mesh_GetDimSize( self );
	nLocals = Mesh_GetLocalSize( self, 0 );
	nDomains = Mesh_GetDomainSize( self, 0 );
    if (nDomains > nLocals) // only perform this where where the domain size is greater than the local size... ie running in parallel
        Sync_SyncArray( sync, Mesh_GetVertex( self, 0 ), nDims * sizeof(double), Mesh_GetVertex( self, nLocals ), nDims * sizeof(double), nDims * sizeof(double) );

	/* TODO
	if( self->dataSyncArrays ) {
		unsigned	d_i;

		for( d_i = 0; d_i < Mesh_GetDimSize( self ); d_i++ ) {
			if( self->dataSyncArrays[d_i] )
				Decomp_Sync_Array_Sync( self->dataSyncArrays[d_i] );
		}
	}
	*/
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Mesh_Destruct( Mesh* self ) {
	unsigned	et_i/*, v_i*/;

	for( et_i = 0; et_i < self->nElTypes; et_i++ )
		Stg_Class_Delete( self->elTypes[et_i] );
	KillArray( self->elTypes );
	KillArray( self->elTypeMap );
	self->nElTypes = 0;

	KillArray( self->vertices );
    Stg_Component_Destroy(self->verticesVariable, NULL, False);
    self->verticesVariable = NULL;
    KillArray( self->verticesgid );
    Stg_Component_Destroy(self->vGlobalIdsVar, NULL, False);
    self->vGlobalIdsVar = NULL;
    KillArray( self->e_n );
    Stg_Component_Destroy(self->enMapVar, NULL, False);
    self->enMapVar = NULL;
    KillArray( self->elgid );
    Stg_Component_Destroy(self->eGlobalIdsVar, NULL, False);
    self->eGlobalIdsVar = NULL;

	self->generator = NULL;
	self->emReg = NULL;

	/*
	for( v_i = 0; v_i < List_GetSize( self->vars ); v_i++ ) {
		MeshVariable*	var;

		var = *(MeshVariable**)List_GetItem( self->vars, v_i );
		MeshVariable_SetMesh( var, NULL );
	}
	List_Clear( self->vars );
	*/
}


