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
#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"

#include "ShadowInfo.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "StandardParticle.h"

const Type ElementCellLayout_Type = "ElementCellLayout";

ElementCellLayout* ElementCellLayout_New( Name name, AbstractContext* context, void* mesh ) {
	ElementCellLayout* self = _ElementCellLayout_DefaultNew( name );

	self->isConstructed = True;
	_CellLayout_Init( (CellLayout*)self );
	_ElementCellLayout_Init( self, mesh );

	return self;
}

void* _ElementCellLayout_DefaultNew( Name name ){
	/* Variables set in this function */
	SizeT                                                _sizeOfSelf = sizeof(ElementCellLayout);
	Type                                                        type = ElementCellLayout_Type;
	Stg_Class_DeleteFunction*                                _delete = _ElementCellLayout_Delete;
	Stg_Class_PrintFunction*                                  _print = _ElementCellLayout_Print;
	Stg_Class_CopyFunction*                                    _copy = _ElementCellLayout_Copy;
	Stg_Component_DefaultConstructorFunction*    _defaultConstructor = _ElementCellLayout_DefaultNew;
	Stg_Component_ConstructFunction*                      _construct = _ElementCellLayout_AssignFromXML;
	Stg_Component_BuildFunction*                              _build = _ElementCellLayout_Build;
	Stg_Component_InitialiseFunction*                    _initialise = _ElementCellLayout_Initialise;
	Stg_Component_ExecuteFunction*                          _execute = _ElementCellLayout_Execute;
	Stg_Component_DestroyFunction*                          _destroy = _ElementCellLayout_Destroy;
	AllocationType                                nameAllocationType = NON_GLOBAL;
	CellLayout_CellCountFunction*                    _cellLocalCount = _ElementCellLayout_CellLocalCount;
	CellLayout_CellCountFunction*                   _cellShadowCount = _ElementCellLayout_CellShadowCount;
	CellLayout_PointCountFunction*                       _pointCount = _ElementCellLayout_PointCount;
	CellLayout_InitialisePointsFunction*           _initialisePoints = _ElementCellLayout_InitialisePoints;
	CellLayout_MapElementIdToCellIdFunction*   _mapElementIdToCellId = _ElementCellLayout_MapElementIdToCellId;
	CellLayout_IsInCellFunction*                           _isInCell = _ElementCellLayout_IsInCell;
	CellLayout_CellOfFunction*                               _cellOf = _ElementCellLayout_CellOf;
	CellLayout_GetShadowInfoFunction*                 _getShadowInfo = _ElementCellLayout_GetShadowInfo;

	return (void*) _ElementCellLayout_New(  ELEMENTCELLLAYOUT_PASSARGS  );
}

ElementCellLayout* _ElementCellLayout_New(  ELEMENTCELLLAYOUT_DEFARGS  ) {
	ElementCellLayout* self;

	/* Allocate memory */
	self = (ElementCellLayout*)_CellLayout_New(  CELLLAYOUT_PASSARGS  );

	/* General info */

	/* Virtual info */

	/* ElementCellLayout info */

	return self;
}


void _ElementCellLayout_Init( ElementCellLayout* self, void* mesh ) {
	/* General and Virtual info should already be set */

	/* ElementCellInfo info */
	self->mesh = (Mesh*)mesh;
	self->incArray = IArray_New();
}

void _ElementCellLayout_Delete( void* elementCellLayout ) {
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;

	/* Stg_Class_Delete parent class */
	_CellLayout_Delete( self );
}

void _ElementCellLayout_Print( void* elementCellLayout, Stream* stream ) {
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;

	/* Set the Journal for printing informations */
	Stream* elementCellLayoutStream = stream;

	/* General info */
	Journal_Printf( elementCellLayoutStream, "ElementCellLayout (ptr): %p\n", self );

	/* Parent class info */
	_CellLayout_Print( self, elementCellLayoutStream );

	/* Virtual info */

	/* ElementCellLayout info */
	Journal_Printf( elementCellLayoutStream, "\tmesh (ptr): %p\n", self->mesh );
}


void* _ElementCellLayout_Copy( void* elementCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ElementCellLayout*	self = (ElementCellLayout*)elementCellLayout;
	ElementCellLayout*	newElementCellLayout;
	PtrMap*			map = ptrMap;
	Bool			ownMap = False;

	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}

	newElementCellLayout = _CellLayout_Copy( self, dest, deep, nameExt, ptrMap );

	if( deep ) {
		newElementCellLayout->mesh = (Mesh*)Stg_Class_Copy( self->mesh, NULL, deep, nameExt, map );
	}
	else {
		newElementCellLayout->mesh = self->mesh;
	}

	if( ownMap ) {
		Stg_Class_Delete( map );
	}

	return (void*)newElementCellLayout;
}

void _ElementCellLayout_AssignFromXML( void* elementCellLayout, Stg_ComponentFactory *cf, void* data ){
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;
	Mesh*              mesh;

	_CellLayout_AssignFromXML( self, cf, data );

	mesh =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Mesh", Mesh, True, data  ) ;

	_ElementCellLayout_Init( self, mesh );
}

void _ElementCellLayout_Build( void *elementCellLayout, void *data ){
	ElementCellLayout*	self = (ElementCellLayout*)elementCellLayout;

	Stg_Component_Build( self->mesh, NULL, False );

	if( !Mesh_HasIncidence( self->mesh, Mesh_GetDimSize( self->mesh ), MT_VERTEX ) ) {
		Stream* elementCellLayoutStream = Journal_Register( ErrorStream_Type, (Name)self->type  );
		Journal_Printf( elementCellLayoutStream, "Warning: Mesh not configured to build element node table. "
			"Activating it now.\n" );
		abort();
	}

	ElementCellLayout_BuildShadowInfo( self );
}

void _ElementCellLayout_Initialise( void *elementCellLayout, void *data ){
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;
	Stg_Component_Initialise( self->mesh, data, False );
}

void _ElementCellLayout_Execute( void *elementCellLayout, void *data ){
}

void _ElementCellLayout_Destroy( void *elementCellLayout, void *data ){
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;

	Stg_Class_Delete( self->incArray );
	if( self->cellShadowInfo.procNbrInfo )
		ElementCellLayout_DestroyShadowInfo( self );

	_CellLayout_Destroy( self, data );
}

Cell_Index _ElementCellLayout_CellLocalCount( void* elementCellLayout ) {
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;
	return Mesh_GetLocalSize( self->mesh, Mesh_GetDimSize( self->mesh ) );
}

Cell_Index _ElementCellLayout_CellShadowCount( void* elementCellLayout ) {
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;
	return Mesh_GetRemoteSize( self->mesh, Mesh_GetDimSize( self->mesh ) );
}

Cell_PointIndex _ElementCellLayout_PointCount( void* elementCellLayout, Cell_Index cellIndex ) {
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;

	Mesh_GetIncidence( self->mesh, Mesh_GetDimSize( self->mesh ), cellIndex, MT_VERTEX,
			   self->incArray );
	return IArray_GetSize( self->incArray );
}

void _ElementCellLayout_InitialisePoints( void* elementCellLayout, Cell_Index cellIndex, Cell_PointIndex pointCount,
					  Cell_Points points )
{
	ElementCellLayout* self = (ElementCellLayout*)elementCellLayout;
	Cell_PointIndex point_I;
	unsigned*	inc;

	Mesh_GetIncidence( self->mesh, Mesh_GetDimSize( self->mesh ), cellIndex, MT_VERTEX,
			   self->incArray );
	inc = (unsigned*)IArray_GetPtr( self->incArray );

	/* point to the mesh's node's coordinates */
	for( point_I = 0; point_I < pointCount; point_I++ ) {
		points[point_I] = Mesh_GetVertex( self->mesh, inc[point_I] );
	}
}


Cell_Index _ElementCellLayout_MapElementIdToCellId( void* elementCellLayout, unsigned element_dI ) {

	#ifdef CAUTIOUS
	{
		ElementCellLayout*      self = (ElementCellLayout*)elementCellLayout;
		Stream* errorStr = Journal_Register( Error_Type, (Name)self->type  );
		Journal_Firewall( element_dI < Mesh_GetDomainSize( self->mesh, Mesh_GetDimSize( self->mesh ) ), errorStr, "Error - in %s(): User asked "
			"for cell corresponding to element %d, but the mesh that this cell layout is based on only "
			"has %d elements.\n", __func__, element_dI, Mesh_GetDomainSize( self->mesh, Mesh_GetDimSize( self->mesh ) ) );
	}
	#endif

	return element_dI;
}


Bool _ElementCellLayout_IsInCell( void* elementCellLayout, Cell_Index cellIndex, void* _particle ) {
	ElementCellLayout*      self     = (ElementCellLayout*)elementCellLayout;
	GlobalParticle*	        particle = (GlobalParticle*)_particle;
	unsigned		elDim, elInd;

	return Mesh_ElementHasPoint( self->mesh, cellIndex, particle->coord, &elDim, &elInd );
}

Cell_Index _ElementCellLayout_CellOf_Irregular( void* elementCellLayout, void* _particle ) {
   /* algorithm for irregular searches, this method uses the existing particle information to
      optimise the search
   */
  ElementCellLayout* self     = (ElementCellLayout*)elementCellLayout;
  Mesh*              mesh     = self->mesh;
  GlobalParticle*    particle = (GlobalParticle*)_particle;

  unsigned		elInd, elDim, cell_id, elDomainSize;

  cell_id      = particle->owningCell;
  elDomainSize = Mesh_GetDomainSize( mesh, Mesh_GetDimSize( mesh ) );

  /* check if particle already has an owning cell, for search optimisation */
  if( cell_id < elDomainSize ) {
    IArray *inc = self->incArray;
    int el_i;
    unsigned nEls;
    int *neighbourEls;

    /* search current cell */
    if( Mesh_ElementHasPoint( mesh, cell_id, particle->coord, &elDim, &elInd ) ) {
      return elInd;
    }

    /* get neighbour cells */
    Mesh_GetIncidence( mesh, Mesh_GetDimSize( mesh ), cell_id, Mesh_GetDimSize(mesh), inc );
    nEls = IArray_GetSize( inc );
    neighbourEls = IArray_GetPtr( inc );

    /* search neighbouring cells */
    for( el_i = 0; el_i<nEls; el_i++ ) {
      if( Mesh_ElementHasPoint( mesh, neighbourEls[el_i], particle->coord, &elDim, &elInd ) ) {
        return elInd;
      }
    }
  }

  // brute force search - if not found indicate problem
  if( !Mesh_SearchElements( self->mesh, particle->coord, &elInd ) )
    elInd = elDomainSize;

  return elInd;
}

Cell_Index _ElementCellLayout_CellOf( void* elementCellLayout, void* _particle ) {
	ElementCellLayout*      self     = (ElementCellLayout*)elementCellLayout;
	GlobalParticle*	        particle = (GlobalParticle*)_particle;
	unsigned		elInd;

  /* this objects highjacks the conditional statement of isRegular made by the mesh
   * in order to optimise the search algorithm using the particles existing owningCell */
  if( self->mesh->isRegular ) {
    // for regularly spaced grid search
      if( !Mesh_SearchElements( self->mesh, particle->coord, &elInd ) )
      elInd = Mesh_GetDomainSize( self->mesh, Mesh_GetDimSize( self->mesh ) );
  } else {
    // for irregularly spaced grid search
    elInd = _ElementCellLayout_CellOf_Irregular( elementCellLayout, _particle );
  }

	return elInd;
}


ShadowInfo* _ElementCellLayout_GetShadowInfo( void* elementCellLayout ) {
	ElementCellLayout*      self = (ElementCellLayout*)elementCellLayout;

	return &self->cellShadowInfo;
}

void ElementCellLayout_DestroyShadowInfo( ElementCellLayout* self ) {
	unsigned	nIncProcs = self->cellShadowInfo.procNbrInfo->procNbrCnt;

	/* Extract neighbouring proc information. */
	Memory_Free( self->cellShadowInfo.procNbrInfo->procNbrTbl );
   if( nIncProcs ) {
      Memory_Free( self->cellShadowInfo.procShadowedCnt );
      Memory_Free( self->cellShadowInfo.procShadowCnt );
      Memory_Free( self->cellShadowInfo.procShadowedTbl );
      Memory_Free( self->cellShadowInfo.procShadowTbl );
   }
	Memory_Free( self->cellShadowInfo.procNbrInfo );
}

void ElementCellLayout_BuildShadowInfo( ElementCellLayout* self ) {
  Comm*       comm;
  int         nIncProcs;
  const int*  incProcs;
  unsigned    nDims, n_i;

	nDims = Mesh_GetDimSize( self->mesh );
	comm = Mesh_GetCommTopology( self->mesh, nDims );
	Comm_GetNeighbours( comm, &nIncProcs, &incProcs );

	/* Extract neighbouring proc information. */
	self->cellShadowInfo.procNbrInfo = Memory_Alloc_Unnamed( ProcNbrInfo );
	self->cellShadowInfo.procNbrInfo->procNbrCnt = nIncProcs;
	self->cellShadowInfo.procNbrInfo->procNbrTbl = AllocArray( unsigned, nIncProcs );
	memcpy( self->cellShadowInfo.procNbrInfo->procNbrTbl, incProcs, nIncProcs * sizeof(unsigned) );

	/* Count shadow info. */
	if( nIncProcs ) {
		self->cellShadowInfo.procShadowedCnt = AllocArray( unsigned, nIncProcs );
		memset( self->cellShadowInfo.procShadowedCnt, 0, nIncProcs * sizeof(unsigned) );
		self->cellShadowInfo.procShadowCnt = AllocArray( unsigned, nIncProcs );
		memset( self->cellShadowInfo.procShadowCnt, 0, nIncProcs * sizeof(unsigned) );
	}
	for( n_i = 0; n_i < Mesh_GetSharedSize( self->mesh, nDims ); n_i++ ) {
		unsigned	nSharers;
		unsigned*	sharers;
		unsigned	s_i;

		Mesh_GetSharers( self->mesh, nDims, n_i,
				 &nSharers, &sharers );
		for( s_i = 0; s_i < nSharers; s_i++ )
			self->cellShadowInfo.procShadowedCnt[sharers[s_i]]++;
	}
	for( n_i = 0; n_i < Mesh_GetRemoteSize( self->mesh, nDims ); n_i++ ) {
		unsigned	owner;

		owner = Mesh_GetOwner( self->mesh, nDims, n_i );
		self->cellShadowInfo.procShadowCnt[owner]++;
	}

	/* Build shadow info indices. */
	if( nIncProcs ) {
		self->cellShadowInfo.procShadowedTbl = Memory_Alloc_2DComplex_Unnamed( unsigned, nIncProcs,
										       self->cellShadowInfo.procShadowedCnt );
		self->cellShadowInfo.procShadowTbl = Memory_Alloc_2DComplex_Unnamed( unsigned, nIncProcs,
										     self->cellShadowInfo.procShadowCnt );
		memset( self->cellShadowInfo.procShadowedCnt, 0, nIncProcs * sizeof(unsigned) );
		memset( self->cellShadowInfo.procShadowCnt, 0, nIncProcs * sizeof(unsigned) );
	}
	for( n_i = 0; n_i < Mesh_GetSharedSize( self->mesh, nDims ); n_i++ ) {
		unsigned	local;
		unsigned	curInd;
		unsigned	nSharers;
		unsigned*	sharers;
		unsigned	s_i;

		local = Mesh_SharedToLocal( self->mesh, nDims, n_i );

		Mesh_GetSharers( self->mesh, nDims, n_i,
				 &nSharers, &sharers );
		for( s_i = 0; s_i < nSharers; s_i++ ) {
			curInd = self->cellShadowInfo.procShadowedCnt[sharers[s_i]]++;
			self->cellShadowInfo.procShadowedTbl[sharers[s_i]][curInd] = local;
		}
	}
	for( n_i = 0; n_i < Mesh_GetRemoteSize( self->mesh, nDims ); n_i++ ) {
		unsigned	domain;
		unsigned	curInd;
		unsigned	owner;

		domain = Mesh_GetLocalSize( self->mesh, nDims ) + n_i;
		owner = Mesh_GetOwner( self->mesh, nDims, n_i );
		curInd = self->cellShadowInfo.procShadowCnt[owner]++;
		self->cellShadowInfo.procShadowTbl[owner][curInd] = domain;
	}
}
