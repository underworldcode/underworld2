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
const Type Mesh_RegularAlgorithms_Type = "Mesh_RegularAlgorithms";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh_RegularAlgorithms* Mesh_RegularAlgorithms_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                                   _sizeOfSelf = sizeof(Mesh_RegularAlgorithms);
	Type                                                           type = Mesh_RegularAlgorithms_Type;
	Stg_Class_DeleteFunction*                                   _delete = _Mesh_RegularAlgorithms_Delete;
	Stg_Class_PrintFunction*                                     _print = _Mesh_RegularAlgorithms_Print;
	Stg_Class_CopyFunction*                                       _copy = NULL;
	Stg_Component_DefaultConstructorFunction*       _defaultConstructor = (void* (*)(Name))_Mesh_RegularAlgorithms_New;
	Stg_Component_ConstructFunction*                         _construct = _Mesh_RegularAlgorithms_AssignFromXML;
	Stg_Component_BuildFunction*                                 _build = _Mesh_RegularAlgorithms_Build;
	Stg_Component_InitialiseFunction*                       _initialise = _Mesh_RegularAlgorithms_Initialise;
	Stg_Component_ExecuteFunction*                             _execute = _Mesh_RegularAlgorithms_Execute;
	Stg_Component_DestroyFunction*                             _destroy = _Mesh_RegularAlgorithms_Destroy;
	AllocationType                                   nameAllocationType = NON_GLOBAL;
	Mesh_Algorithms_SetMeshFunc*                            setMeshFunc = Mesh_RegularAlgorithms_SetMesh;
	Mesh_Algorithms_UpdateFunc*                              updateFunc = Mesh_RegularAlgorithms_Update;
	Mesh_Algorithms_NearestVertexFunc*                nearestVertexFunc = _Mesh_Algorithms_NearestVertex;
	Mesh_Algorithms_SearchFunc*                              searchFunc = _Mesh_Algorithms_Search;
	Mesh_Algorithms_SearchElementsFunc*              searchElementsFunc = Mesh_RegularAlgorithms_SearchElements;
	Mesh_Algorithms_GetMinimumSeparationFunc*  getMinimumSeparationFunc = _Mesh_Algorithms_GetMinimumSeparation;
	Mesh_Algorithms_GetLocalCoordRangeFunc*      getLocalCoordRangeFunc = _Mesh_Algorithms_GetLocalCoordRange;
	Mesh_Algorithms_GetDomainCoordRangeFunc*    getDomainCoordRangeFunc = _Mesh_Algorithms_GetDomainCoordRange;
	Mesh_Algorithms_GetGlobalCoordRangeFunc*    getGlobalCoordRangeFunc = _Mesh_Algorithms_GetGlobalCoordRange;

	Mesh_RegularAlgorithms* self = _Mesh_RegularAlgorithms_New(  MESH_REGULARALGORITHMS_PASSARGS  );

	/* Mesh_RegularAlgorithms info */
	_Mesh_Algorithms_Init( (Mesh_Algorithms*)self, context );
	_Mesh_RegularAlgorithms_Init( self );

   return self;
}

Mesh_RegularAlgorithms* _Mesh_RegularAlgorithms_New(  MESH_REGULARALGORITHMS_DEFARGS  ) {
	Mesh_RegularAlgorithms* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh_RegularAlgorithms) );
	self = (Mesh_RegularAlgorithms*)_Mesh_Algorithms_New(  MESH_ALGORITHMS_PASSARGS  );

	return self;
}

void _Mesh_RegularAlgorithms_Init( void* algorithms ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;

	assert( self && Stg_CheckType( self, Mesh_RegularAlgorithms ) );

	self->sep = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_RegularAlgorithms_Delete( void* algorithms ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;

	/* Delete the parent. */
	_Mesh_Algorithms_Delete( self );
}

void _Mesh_RegularAlgorithms_Print( void* algorithms, Stream* stream ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	
	/* Print parent */
	Journal_Printf( stream, "Mesh_RegularAlgorithms (ptr): (%p)\n", self );
	_Mesh_Algorithms_Print( self, stream );
}

void _Mesh_RegularAlgorithms_AssignFromXML( void* algorithms, Stg_ComponentFactory* cf, void* data ) {
	_Mesh_Algorithms_AssignFromXML( algorithms, cf, data );
   _Mesh_RegularAlgorithms_Init( algorithms );
}

void _Mesh_RegularAlgorithms_Build( void* algorithms, void* data ) {
    _Mesh_Algorithms_Build( algorithms, data );
}

void _Mesh_RegularAlgorithms_Initialise( void* algorithms, void* data ) {
    _Mesh_Algorithms_Initialise( algorithms, data );
}

void _Mesh_RegularAlgorithms_Execute( void* algorithms, void* data ) {
    _Mesh_Algorithms_Execute( algorithms, data );
}

void _Mesh_RegularAlgorithms_Destroy( void* algorithms, void* data ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;

	Mesh_RegularAlgorithms_Destruct( self );

   _Mesh_Algorithms_Destroy( algorithms, data );
}

void Mesh_RegularAlgorithms_SetMesh( void* algorithms, void* mesh ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;

	assert( self && Stg_CheckType( self, Mesh_RegularAlgorithms ) );

	Mesh_RegularAlgorithms_Destruct( self );
	_Mesh_Algorithms_SetMesh( self, mesh );
}

void Mesh_RegularAlgorithms_Update( void* algorithms ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	unsigned		nDims;
	Grid*			eGrid;
	int			ii;

	assert( self && Stg_CheckType( self, Mesh_RegularAlgorithms ) );
	assert( self->mesh );

	Mesh_RegularAlgorithms_Destruct( self );
	_Mesh_Algorithms_Update( self );

	nDims = Mesh_GetDimSize( self->mesh );
	self->minCrd = AllocArray( double, nDims );
	self->maxCrd = AllocArray( double, nDims );
	Mesh_GetGlobalCoordRange( self->mesh, self->minCrd, self->maxCrd );

	self->sep = AllocArray( double, nDims );
	eGrid = *Mesh_GetExtension( self->mesh, Grid**,  self->mesh->elGridId );
	for( ii = 0; ii < nDims; ii++ )
		self->sep[ii] = (self->maxCrd[ii] - self->minCrd[ii]) / eGrid->sizes[ii];
}

Bool _Mesh_RegularAlgorithms_Search( void* algorithms, void* _mesh, double* point, 
				     MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	Mesh*			mesh = (Mesh*)_mesh;

	assert( self );
	assert( mesh );
	assert( dim );
	assert( ind );

	/* TODO */
	abort();

	return False;
}

Bool Mesh_RegularAlgorithms_SearchElements( void* algorithms, double* point, unsigned* elInd ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	Mesh*			mesh;
	unsigned		nDims;
	unsigned		inds[3];
	Grid			*elGrid;
	double			out, frac, integer;
	unsigned		d_i;

	assert( self );
	assert( Mesh_GetDimSize( self->mesh ) <= 3 );
	assert( elInd );

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	elGrid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, mesh->elGridId );
	for( d_i = 0; d_i < nDims; d_i++  ) {
		if( Num_Approx( point[d_i] - self->maxCrd[d_i], 0.0 ) )
			inds[d_i] = elGrid->sizes[d_i] - 1;
		else if( point[d_i] < self->minCrd[d_i] || point[d_i] > self->maxCrd[d_i] )
			return False;
		else {
			out = (point[d_i] - self->minCrd[d_i]) / self->sep[d_i];
			frac = modf( out, &integer );
			inds[d_i] = (unsigned)integer;
			if( inds[d_i] > 0 && Num_Approx( frac, 0.0 ) )
				inds[d_i]--;
		}
	}

	*elInd = Grid_Project( elGrid, inds );
	return Mesh_GlobalToDomain( mesh, nDims, *elInd, elInd );
}

double _Mesh_RegularAlgorithms_GetMinimumSeparation( void* algorithms, void* _mesh, double* perDim ) {
	/* TODO */
	abort();

	return 0.0;
}

void _Mesh_RegularAlgorithms_GetLocalCoordRange( void* algorithms, void* _mesh, double* min, double* max ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	Mesh*			mesh = (Mesh*)_mesh;

	assert( self );
	assert( mesh );
	assert( min );
	assert( max );

	/* TODO */
	abort();
}

void _Mesh_RegularAlgorithms_GetDomainCoordRange( void* algorithms, void* _mesh, double* min, double* max ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	Mesh*			mesh = (Mesh*)_mesh;

	assert( self );
	assert( mesh );
	assert( min );
	assert( max );

	/* TODO */
	abort();
}

void _Mesh_RegularAlgorithms_GetGlobalCoordRange( void* algorithms, void* _mesh, double* min, double* max ) {
	Mesh_RegularAlgorithms*	self = (Mesh_RegularAlgorithms*)algorithms;
	Mesh*			mesh = (Mesh*)_mesh;

	assert( self );
	assert( mesh );
	assert( min );
	assert( max );

	/* TODO */
	abort();
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Mesh_RegularAlgorithms_Destruct( Mesh_RegularAlgorithms* self ) {
	assert( self && Stg_CheckType( self, Mesh_RegularAlgorithms ) );

	KillArray( self->sep );
	KillArray( self->minCrd );
	KillArray( self->maxCrd );
}


