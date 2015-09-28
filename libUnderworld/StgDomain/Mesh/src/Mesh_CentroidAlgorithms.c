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
const Type Mesh_CentroidAlgorithms_Type = "Mesh_CentroidAlgorithms";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh_CentroidAlgorithms* Mesh_CentroidAlgorithms_New( Name name, AbstractContext* context ) {
	/* Variables set in this function */
	SizeT                                                   _sizeOfSelf = sizeof(Mesh_CentroidAlgorithms);
	Type                                                           type = Mesh_CentroidAlgorithms_Type;
	Stg_Class_DeleteFunction*                                   _delete = _Mesh_CentroidAlgorithms_Delete;
	Stg_Class_PrintFunction*                                     _print = _Mesh_CentroidAlgorithms_Print;
	Stg_Class_CopyFunction*                                       _copy = NULL;
	Stg_Component_DefaultConstructorFunction*       _defaultConstructor = (void* (*)(Name))_Mesh_CentroidAlgorithms_New;
	Stg_Component_ConstructFunction*                         _construct = _Mesh_CentroidAlgorithms_AssignFromXML;
	Stg_Component_BuildFunction*                                 _build = _Mesh_CentroidAlgorithms_Build;
	Stg_Component_InitialiseFunction*                       _initialise = _Mesh_CentroidAlgorithms_Initialise;
	Stg_Component_ExecuteFunction*                             _execute = _Mesh_CentroidAlgorithms_Execute;
	Stg_Component_DestroyFunction*                             _destroy = _Mesh_CentroidAlgorithms_Destroy;
	AllocationType                                   nameAllocationType = NON_GLOBAL;
	Mesh_Algorithms_SetMeshFunc*                            setMeshFunc = _Mesh_Algorithms_SetMesh;
	Mesh_Algorithms_UpdateFunc*                              updateFunc = Mesh_CentroidAlgorithms_Update;
	Mesh_Algorithms_NearestVertexFunc*                nearestVertexFunc = Mesh_CentroidAlgorithms_NearestVertex;
	Mesh_Algorithms_SearchFunc*                              searchFunc = Mesh_CentroidAlgorithms_Search;
	Mesh_Algorithms_SearchElementsFunc*              searchElementsFunc = Mesh_CentroidAlgorithms_SearchElements;
	Mesh_Algorithms_GetMinimumSeparationFunc*  getMinimumSeparationFunc = _Mesh_Algorithms_GetMinimumSeparation;
	Mesh_Algorithms_GetLocalCoordRangeFunc*      getLocalCoordRangeFunc = Mesh_CentroidAlgorithms_GetLocalCoordRange;
	Mesh_Algorithms_GetDomainCoordRangeFunc*    getDomainCoordRangeFunc = Mesh_CentroidAlgorithms_GetDomainCoordRange;
	Mesh_Algorithms_GetGlobalCoordRangeFunc*    getGlobalCoordRangeFunc = Mesh_CentroidAlgorithms_GetGlobalCoordRange;

   Mesh_CentroidAlgorithms* self = _Mesh_CentroidAlgorithms_New(  MESH_CENTROIDALGORITHMS_PASSARGS  );

	_Mesh_Algorithms_Init( (Mesh_Algorithms*)self, context );
	_Mesh_CentroidAlgorithms_Init( self );
   return self;

}

Mesh_CentroidAlgorithms* _Mesh_CentroidAlgorithms_New(  MESH_CENTROIDALGORITHMS_DEFARGS  ) {
	Mesh_CentroidAlgorithms* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh_CentroidAlgorithms) );
	self = (Mesh_CentroidAlgorithms*)_Mesh_Algorithms_New(  MESH_ALGORITHMS_PASSARGS  );

	return self;
}

void _Mesh_CentroidAlgorithms_Init( void* centroidAlgorithms ) {
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;

	assert( self && Stg_CheckType( self, Mesh_CentroidAlgorithms ) );

	self->elMesh = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_CentroidAlgorithms_Delete( void* centroidAlgorithms ) {
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;

	/* Delete the parent. */
	_Mesh_Algorithms_Delete( self );
}

void _Mesh_CentroidAlgorithms_Print( void* _centroidAlgorithms, Stream* stream ) {
	Mesh_CentroidAlgorithms* self= (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
	
	/* Set the Journal for printing informations */
	Stream* centroidAlgorithmsStream;
	centroidAlgorithmsStream = Journal_Register( InfoStream_Type, (Name)"Mesh_CentroidAlgorithmsStream"  );

	/* Print parent */
	Journal_Printf( stream, "Mesh_CentroidAlgorithms (ptr): (%p)\n", self );
	_Mesh_Algorithms_Print( self, stream );
}

void _Mesh_CentroidAlgorithms_AssignFromXML( void* _centroidAlgorithms, Stg_ComponentFactory* cf, void* data ) {
    
    Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
    
    _Mesh_Algorithms_AssignFromXML( self, cf, data );

    _Mesh_CentroidAlgorithms_Init( self );
}

void _Mesh_CentroidAlgorithms_Build( void* _centroidAlgorithms, void* data ) {
    
    Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
    
    Stg_Component_Build( self->elMesh, data, False );  
    _Mesh_Algorithms_Build( self, data );
}

void _Mesh_CentroidAlgorithms_Initialise( void* _centroidAlgorithms, void* data ) {
    Mesh_CentroidAlgorithms*      self = (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
    
    Stg_Component_Initialise( self->elMesh, data, False );  
    _Mesh_Algorithms_Initialise( self, data );
}

void _Mesh_CentroidAlgorithms_Execute( void* _centroidAlgorithms, void* data ) {
    
    Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
    
    Stg_Component_Initialise( self->elMesh, data, False );  
    _Mesh_Algorithms_Initialise( self, data );
}

void _Mesh_CentroidAlgorithms_Destroy( void* _centroidAlgorithms, void* data ) {
    
    Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)_centroidAlgorithms;
    
    Stg_Component_Destroy( self->elMesh, data, False );  
    _Mesh_Algorithms_Destroy( self, data );
}

void Mesh_CentroidAlgorithms_Update( void* centroidAlgorithms ) {
}

#define Vec_Sep( nDims, v0, v1 )									\
	(((v0)[0] - (v1)[0]) * ((v0)[0] - (v1)[0]) +							\
	 (((nDims) >= 2) ? (((v0)[1] - (v1)[1]) * ((v0)[1] - (v1)[1]) + 				\
			    (((nDims) == 3) ? (((v0)[2] - (v1)[2]) * ((v0)[2] - (v1)[2])) : 0)) : 0))

unsigned Mesh_CentroidAlgorithms_NearestVertex( void* centroidAlgorithms, double* point ) {
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;
	unsigned			elInd;
	double dist, nearDist;
	unsigned near;
	unsigned nDims;
	double* vert;
	unsigned inc_i;

	assert( self );

	if( Mesh_SearchElements( self->elMesh, point, &elInd ) ) {
		unsigned	nInc, *inc;

		nDims = Mesh_GetDimSize( self->mesh );
		Mesh_GetIncidence( self->elMesh, Mesh_GetDimSize( self->mesh ), elInd, MT_VERTEX, 
				   self->incArray );
		nInc = IArray_GetSize( self->incArray );
		inc = (unsigned*)IArray_GetPtr( self->incArray );
		near = inc[0];
		vert = Mesh_GetVertex( self->mesh, inc[0] );
		nearDist = Vec_Sep( nDims, vert, point );
		for( inc_i = 1; inc_i < nInc; inc_i++ ) {
			vert = Mesh_GetVertex( self->mesh, inc[inc_i] );
			dist = Vec_Sep( nDims, vert, point );
			if( dist < nearDist ) {
				near = inc[inc_i];
				nearDist = dist;
			}
		}
		return near;
	}
	else
		return _Mesh_Algorithms_NearestVertex( self, point );
}

Bool Mesh_CentroidAlgorithms_Search( void* centroidAlgorithms, double* point, 
				     MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;

	assert( self );

	return Mesh_Search( self->elMesh, point, dim, ind );
}

Bool Mesh_CentroidAlgorithms_SearchElements( void* centroidAlgorithms, double* point, 
					     unsigned* elInd )
{
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;

	assert( self );

	return Mesh_SearchElements( self->elMesh, point, elInd );
}

void Mesh_CentroidAlgorithms_GetLocalCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)algorithms;

	assert( self );
	Mesh_GetLocalCoordRange( self->elMesh, min, max );
}

void Mesh_CentroidAlgorithms_GetDomainCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)algorithms;

	assert( self );
	Mesh_GetDomainCoordRange( self->elMesh, min, max );
}

void Mesh_CentroidAlgorithms_GetGlobalCoordRange( void* algorithms, double* min, double* max ) {
	Mesh_CentroidAlgorithms* self = (Mesh_CentroidAlgorithms*)algorithms;

	assert( self );
	Mesh_GetGlobalCoordRange( self->elMesh, min, max );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Mesh_CentroidAlgorithms_SetElementMesh( void* centroidAlgorithms, void* mesh ) {
	Mesh_CentroidAlgorithms*	self = (Mesh_CentroidAlgorithms*)centroidAlgorithms;

	assert( self && Stg_CheckType( self, Mesh_CentroidAlgorithms ) );

	self->elMesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


