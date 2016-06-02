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

#include "types.h"

#include "ShadowInfo.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"
#include "CLLCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "StandardParticle.h"

const Type CLLCellLayout_Type = "CLLCellLayout";

void* _CLLCellLayout_DefaultNew( Name name ){
	/* Variables set in this function */
	SizeT                                                _sizeOfSelf = sizeof(CLLCellLayout);
	Type                                                        type = CLLCellLayout_Type;
   Stg_Class_DeleteFunction*                                _delete = _ElementCellLayout_Delete;
   Stg_Class_PrintFunction*                                  _print = _ElementCellLayout_Print;
   Stg_Class_CopyFunction*                                    _copy = NULL;
	Stg_Component_DefaultConstructorFunction*    _defaultConstructor = _CLLCellLayout_DefaultNew;
	Stg_Component_ConstructFunction*                      _construct = _CLLCellLayout_AssignFromXML;
	Stg_Component_BuildFunction*                              _build = _CLLCellLayout_Build;
	Stg_Component_InitialiseFunction*                    _initialise = _CLLCellLayout_Initialise;
   Stg_Component_ExecuteFunction*                          _execute = _ElementCellLayout_Execute;
	Stg_Component_DestroyFunction*                          _destroy = _CLLCellLayout_Destroy;
	AllocationType                                nameAllocationType = NON_GLOBAL;
   CellLayout_CellCountFunction*                    _cellLocalCount = _ElementCellLayout_CellLocalCount;
   CellLayout_CellCountFunction*                   _cellShadowCount = _ElementCellLayout_CellShadowCount;
   CellLayout_PointCountFunction*                       _pointCount = _ElementCellLayout_PointCount;
   CellLayout_InitialisePointsFunction*           _initialisePoints = _ElementCellLayout_InitialisePoints;
   CellLayout_MapElementIdToCellIdFunction*   _mapElementIdToCellId = _ElementCellLayout_MapElementIdToCellId;
   CellLayout_IsInCellFunction*                           _isInCell = _ElementCellLayout_IsInCell;
   CellLayout_CellOfFunction*                               _cellOf = _ElementCellLayout_CellOf;
   CellLayout_GetShadowInfoFunction*                 _getShadowInfo = _ElementCellLayout_GetShadowInfo;

	return (void*) _CLLCellLayout_New(  CLLCELLLAYOUT_PASSARGS  );
}

CLLCellLayout* _CLLCellLayout_New(  CLLCELLLAYOUT_DEFARGS  ) {
	CLLCellLayout* self;
	
	/* Allocate memory */
	self = (CLLCellLayout*)_CellLayout_New(  CELLLAYOUT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	
	/* CLLCellLayout info */
   self->geometryMesh = NULL;
   self->mesh = NULL;
   self->cellSize = 0;
   self->meshDim = 0;
   self->parallelDecompose = True;
   self->expandFactor = 0;
	
	return self;
}


void _CLLCellLayout_Init( CLLCellLayout* self, void* geometryMesh, double cellSize, unsigned meshDim, Bool parallelDecompose, unsigned expandFactor ) {
	/* General and Virtual info should already be set */
	
	/* ElementCellInfo info */
   self->geometryMesh = (Mesh*)geometryMesh;
   self->cellSize = cellSize;
   self->meshDim  = meshDim;
   self->parallelDecompose = parallelDecompose;
   self->expandFactor = expandFactor;
	self->incArray = IArray_New();
}

void _CLLCellLayout_AssignFromXML( void* cLLCellLayout, Stg_ComponentFactory *cf, void* data ){
	CLLCellLayout* self = (CLLCellLayout*)cLLCellLayout;
   Mesh*          geometryMesh;
   double         cellSize;
   unsigned       meshDim;
   Bool           parallelDecompose;
   unsigned       expandFactor;

	_CellLayout_AssignFromXML( self, cf, data );

   /** mesh used to determine problem domain, including local processor support etc */
   geometryMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"GeometryMesh", Mesh, True, data  ) ;
   /** size of the cells */
   cellSize     = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"CellSize", 0.  );
   /** dimensionality of CellLayout.  we sometimes want to embed 2D mesh in 3D problem. */
   meshDim      = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"MeshDim", 0  );
   /** should elementLayout only overlay processor local domain support ? */
   parallelDecompose = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"ParallelDecompose", True  );
   /** factor (multiplied by cellSize) by which support should be expanded.  in particular useful for RBF usage */
   expandFactor = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"ExpandFactor", 1  );

   _CLLCellLayout_Init( self, geometryMesh, cellSize, meshDim, parallelDecompose, expandFactor );
}
	
void _CLLCellLayout_Build( void *cLLCellLayout, void *data ){
	CLLCellLayout*	self = (CLLCellLayout*)cLLCellLayout;
   CartesianGenerator*           gen;
   unsigned*                     res;
   double                        *crdMin, *crdMax;
   char*                         name;

   Stg_Component_Build( self->geometryMesh, NULL, False );

   crdMin = Memory_Alloc_Array_Unnamed( double  , 3 );
   crdMax = Memory_Alloc_Array_Unnamed( double  , 3 );
   res    = Memory_Alloc_Array_Unnamed( unsigned, 3 );

   /** first we need to create a mesh to be used for the linked listing scheme */
   gen = CartesianGenerator_New( "", NULL );
   CartesianGenerator_SetDimSize( gen, self->meshDim );
   if(self->meshDim < ((DomainContext*)self->context)->dim){
      /** create a processor local mesh to prevent mesh being partitioned across all processors */
      /** to achieve this, see mpiComm to MPI_COMM_SELF */
      gen->mpiComm = MPI_COMM_SELF;

      /** determine size of local mesh ownership */
      if( self->parallelDecompose )
         Mesh_GetLocalCoordRange(self->geometryMesh, crdMin, crdMax);
      else
         Mesh_GetGlobalCoordRange(self->geometryMesh, crdMin, crdMax);
      /** assume that the particles are located in the XY plane TODO generalise */
      /** expand coord range to include an extra self->expandFactor*self->cellSize either side */
      crdMin[0] = crdMin[0] - self->expandFactor*self->cellSize;
      crdMin[1] = crdMin[2] - self->expandFactor*self->cellSize;
      crdMax[0] = crdMax[0] + self->expandFactor*self->cellSize;
      crdMax[1] = crdMax[2] + self->expandFactor*self->cellSize;
      /** determine required size for mesh dimensions to ensure mesh spacing =>self->cellSize.  make sure minimum is 1  */
      res[0] = ( crdMax[0] - crdMin[0] ) / self->cellSize >=1 ? ( crdMax[0] - crdMin[0] ) / self->cellSize : 1;
      res[1] = ( crdMax[1] - crdMin[1] ) / self->cellSize >=1 ? ( crdMax[1] - crdMin[1] ) / self->cellSize : 1;

      /** set the toplogocial and geometric parameters for the generator */
      CartesianGenerator_SetTopologyParams( gen, res, 0, NULL, NULL );
      CartesianGenerator_SetGeometryParams( gen, crdMin, crdMax );
      /** enable required incidence relations */
      MeshGenerator_SetIncidenceState( (void*)gen, 2, 2, True );
      /** ensure mesh is not read from file */
      gen->readFromFile = False;
      /** create a Mesh */
      Stg_asprintf( &name, "%s_CLL_Mesh", self->name );
      self->mesh = Mesh_New( name );
      Memory_Free( name );
      /** add to register to enable possible checkpointing */
      LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->mesh );
      self->mesh->isCheckpointedAndReloaded = False;
      self->mesh->isDeforming     = False;

      /** set mesh to use generator we just created  */
      Mesh_SetGenerator( self->mesh, gen );
      /** build the mesh */
      Stg_Component_Build( self->mesh, NULL, False );
   } else {
      /** to ensure mesh is only created across current processor, need to set the following */
      gen->mpiComm = MPI_COMM_SELF;
      /** determine size of local mesh ownership */
      Mesh_GetLocalCoordRange(self->geometryMesh, crdMin, crdMax);
      /** expand coord range to include an extra self->expandFactor*self->cellSize either side */
      crdMin[0] = crdMin[0] - self->expandFactor*self->cellSize;
      crdMin[1] = crdMin[1] - self->expandFactor*self->cellSize;
      crdMin[2] = crdMin[2] - self->expandFactor*self->cellSize;
      crdMax[0] = crdMax[0] + self->expandFactor*self->cellSize;
      crdMax[1] = crdMax[1] + self->expandFactor*self->cellSize;
      crdMax[2] = crdMax[2] + self->expandFactor*self->cellSize;

      /** determine required size for mesh dimensions to ensure mesh spacing =>2h, and a minimum of 1 */
      res[0] = ( crdMax[0] - crdMin[0] ) / self->cellSize >=1 ? ( crdMax[0] - crdMin[0] ) / self->cellSize : 1;
      res[1] = ( crdMax[1] - crdMin[1] ) / self->cellSize >=1 ? ( crdMax[1] - crdMin[1] ) / self->cellSize : 1;
      res[2] = ( crdMax[2] - crdMin[2] ) / self->cellSize >=1 ? ( crdMax[2] - crdMin[2] ) / self->cellSize : 1;

      /** set the toplogocial and geometric parameters for the generator */
      CartesianGenerator_SetTopologyParams( gen, res, 0, NULL, NULL );
      CartesianGenerator_SetGeometryParams( gen, crdMin, crdMax );
      /** enable required incidence relations */
      MeshGenerator_SetIncidenceState( (void*)gen, self->meshDim, self->meshDim, True );
      /** ensure mesh is not read from file */
      gen->readFromFile = False;
      /** create a mesh. */
      Stg_asprintf( &name, "%s_CLL_Mesh", self->name );
      self->mesh = Mesh_New( name );
      Memory_Free( name );
      /** add to register to enable possible checkpointing */
      LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->mesh );
      self->mesh->isCheckpointedAndReloaded = False;
      self->mesh->isDeforming     = False;

      /** set mesh to use generator we just created  */
      Mesh_SetGenerator( self->mesh, gen );
      /** build the mesh */
      Stg_Component_Build( self->mesh, NULL, False );
   }
   _ElementCellLayout_Build( cLLCellLayout, data );
}
	
void _CLLCellLayout_Initialise( void *cLLCellLayout, void *data ){
	CLLCellLayout* self = (CLLCellLayout*)cLLCellLayout;
   Stg_Component_Initialise( self->geometryMesh, data, False );
   Stg_Component_Initialise( self->mesh, data, False );
   _ElementCellLayout_Initialise( cLLCellLayout, data );

}
	
void _CLLCellLayout_Execute( void *cLLCellLayout, void *data ){
}

void _CLLCellLayout_Destroy( void *cLLCellLayout, void *data ){
   CLLCellLayout* self = (CLLCellLayout*)cLLCellLayout;

   Stg_Component_Destroy( self->mesh, data, False );

   ElementCellLayout_DestroyShadowInfo( (ElementCellLayout*)self );
   Stg_Class_Delete( self->incArray );

   _CellLayout_Destroy( self, data );
}
