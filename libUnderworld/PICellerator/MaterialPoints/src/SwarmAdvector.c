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
#include <StgFEM/StgFEM.h>

#include <PICellerator/PopulationControl/PopulationControl.h>
#include <PICellerator/Weights/Weights.h>

#include "types.h"
#include "SwarmAdvector.h"

#include "GeneralSwarm.h"
#include "PeriodicBoundariesManager.h"
#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type SwarmAdvector_Type = "SwarmAdvector";

/*-------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
SwarmAdvector* SwarmAdvector_New(
		Name                                       name,
		DomainContext*                             context,
		TimeIntegrator*                            timeIntegrator,
		FeVariable*                                velocityField,
		Bool                                       allowFallbackToFirstOrder,
		GeneralSwarm*                       swarm,
		PeriodicBoundariesManager*                 periodicBCsManager )
{
	SwarmAdvector* self = (SwarmAdvector*) _SwarmAdvector_DefaultNew( name );

	/* 	SwarmAdvector_InitAll */
	_TimeIntegrand_Init( self, context, timeIntegrator, swarm->particleCoordVariable->variable, 0, NULL,
		allowFallbackToFirstOrder );
	_SwarmAdvector_Init( self, velocityField, swarm, periodicBCsManager );

	return self;
}

SwarmAdvector* _SwarmAdvector_New(  SWARMADVECTOR_DEFARGS  )
{
	SwarmAdvector* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SwarmAdvector) );
	self = (SwarmAdvector*)_TimeIntegrand_New(  TIMEINTEGRAND_PASSARGS  );

	/* General info */

	/* Virtual Info */

	return self;
}

void _SwarmAdvector_Init(
		SwarmAdvector*                             self,
		FeVariable*                                velocityField,
		GeneralSwarm*                       swarm,
		PeriodicBoundariesManager*                 periodicBCsManager )
{
	self->velocityField = velocityField;
	self->swarm = swarm;

  // check we don't have a GaussSwarm as swarm input (only way to check is by the cellLayout Type)
   Journal_Firewall( !Stg_Class_IsInstance(swarm->cellLayout, SingleCellLayout_Type ),
         NULL,
         "Error in '%s'. Can attach a SwarmAdvector to swarm '%s' because the swarm's cell layout '%s'"
         "is of type '%s' which is incompatible with advection.\nAre you sure you want to advect this swarm?"
         " If you change it's cell layout",
         __func__, swarm->name, swarm->cellLayout->name, swarm->cellLayout->type );

//    if( swarm->swarmAdvector == NULL )
//	   swarm->swarmAdvector = self;	/* Attach ourselves to the swarm */
//    else
//	   Journal_Firewall( 0 ,
//			NULL,
//			"In func - %s.\nSwarm '%s' appears to already have an advector (%s) attached,\n"
//			"but now advector (%s) is trying to attach itself.\n"
//			"Please check your input file, and ensure there is only one advector per swarm.",
//			__func__,
//			swarm->name,
//			swarm->swarmAdvector->name,
//			self->name );

	swarm->isAdvecting = True;
	self->variable = swarm->particleCoordVariable->variable;

   Journal_Firewall(
                    Stg_Class_IsInstance( swarm->cellLayout, ElementCellLayout_Type ),
                    NULL,
                    "Error In func %s: %s expects a materialSwarm with cellLayout of type ElementCellLayout.",
                    __func__, self->type );

	 /* if not regular use quicker algorithm for SwarmAdvection */
	 if( ((ElementCellLayout*)swarm->cellLayout)->mesh->isRegular == False && self->type == SwarmAdvector_Type ) {
	    self->_calculateTimeDeriv = _SwarmAdvector_TimeDeriv_Quicker4IrregularMesh;
	 }
	self->periodicBCsManager = periodicBCsManager;
}


/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _SwarmAdvector_Delete( void* swarmAdvector ) {
	SwarmAdvector* self = (SwarmAdvector*)swarmAdvector;

	/* Delete parent */
	_TimeIntegrand_Delete( self );
}


void _SwarmAdvector_Print( void* swarmAdvector, Stream* stream ) {
	SwarmAdvector* self = (SwarmAdvector*)swarmAdvector;

	/* Print parent */
	_TimeIntegrand_Print( self, stream );
}


void* _SwarmAdvector_Copy( void* swarmAdvector, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	SwarmAdvector*	self = (SwarmAdvector*)swarmAdvector;
	SwarmAdvector*	newSwarmAdvector;

	newSwarmAdvector = (SwarmAdvector*)_TimeIntegrand_Copy( self, dest, deep, nameExt, ptrMap );

	newSwarmAdvector->velocityField = self->velocityField;
	newSwarmAdvector->swarm         = self->swarm;
	newSwarmAdvector->periodicBCsManager = self->periodicBCsManager;

	return (void*)newSwarmAdvector;
}

void* _SwarmAdvector_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                               _sizeOfSelf = sizeof(SwarmAdvector);
	Type                                                       type = SwarmAdvector_Type;
	Stg_Class_DeleteFunction*                               _delete = _SwarmAdvector_Delete;
	Stg_Class_PrintFunction*                                 _print = _SwarmAdvector_Print;
	Stg_Class_CopyFunction*                                   _copy = _SwarmAdvector_Copy;
	Stg_Component_DefaultConstructorFunction*   _defaultConstructor = _SwarmAdvector_DefaultNew;
	Stg_Component_ConstructFunction*                     _construct = _SwarmAdvector_AssignFromXML;
	Stg_Component_BuildFunction*                             _build = _SwarmAdvector_Build;
	Stg_Component_InitialiseFunction*                   _initialise = _SwarmAdvector_Initialise;
	Stg_Component_ExecuteFunction*                         _execute = _SwarmAdvector_Execute;
	Stg_Component_DestroyFunction*                         _destroy = _SwarmAdvector_Destroy;
	TimeIntegrand_CalculateTimeDerivFunction*  _calculateTimeDeriv = _SwarmAdvector_TimeDeriv;
	TimeIntegrand_IntermediateFunction*              _intermediate = _SwarmAdvector_Intermediate;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _SwarmAdvector_New(  SWARMADVECTOR_PASSARGS  );
}


void _SwarmAdvector_AssignFromXML( void* swarmAdvector, Stg_ComponentFactory* cf, void* data ) {
	SwarmAdvector*	            self          = (SwarmAdvector*) swarmAdvector;
	FeVariable*                 velocityField;
	GeneralSwarm*        swarm;
	PeriodicBoundariesManager*  periodicBCsManager;

	_TimeIntegrand_AssignFromXML( self, cf, data );

	velocityField      = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityField", FeVariable, True, data  );
	swarm              = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", GeneralSwarm, True, data  );
	periodicBCsManager = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"PeriodicBCsManager", PeriodicBoundariesManager, False, data  );

	_SwarmAdvector_Init( self, velocityField, swarm, periodicBCsManager );
}

void _SwarmAdvector_Build( void* swarmAdvector, void* data ) {
   SwarmAdvector*	self = (SwarmAdvector*) swarmAdvector;
   GeneralSwarm *swarm=NULL;

   Stg_Component_Build( self->velocityField, data, False );
   Stg_Component_Build( self->swarm, data, False );

   swarm=self->swarm;

   /* Test if mesh is periodic and a periodic boundaries manager hasn't been given */
   if ( !self->periodicBCsManager && Stg_Class_IsInstance( ((ElementCellLayout*)swarm->cellLayout)->mesh->generator, CartesianGenerator_Type ) ) {
      CartesianGenerator* cartesianGenerator = (CartesianGenerator*) ((ElementCellLayout*)swarm->cellLayout)->mesh->generator;
      if ( cartesianGenerator->periodic[ I_AXIS ] ||
           cartesianGenerator->periodic[ J_AXIS ] ||
           cartesianGenerator->periodic[ K_AXIS ] ) {
         /* Create a periodicBCsManager if there isn't one already */
         self->periodicBCsManager = PeriodicBoundariesManager_New( "periodicBCsManager", (PICelleratorContext*)self->context, (Mesh*)((ElementCellLayout*)swarm->cellLayout)->mesh, (Swarm*)swarm, NULL );
   }


}
	if ( self->periodicBCsManager )
		Stg_Component_Build( self->periodicBCsManager, data, False );
   _TimeIntegrand_Build( self, data );

}

void _SwarmAdvector_Initialise( void* swarmAdvector, void* data ) {
	SwarmAdvector*	self = (SwarmAdvector*) swarmAdvector;

   Stg_Component_Initialise( self->velocityField, data, False );
   Stg_Component_Initialise( self->swarm, data, False );
	if ( self->periodicBCsManager )
		Stg_Component_Initialise( self->periodicBCsManager, data, False );
	_TimeIntegrand_Initialise( self, data );
}

void _SwarmAdvector_Execute( void* swarmAdvector, void* data ) {
	SwarmAdvector*	self = (SwarmAdvector*)swarmAdvector;

	_TimeIntegrand_Execute( self, data );
}

void _SwarmAdvector_Destroy( void* swarmAdvector, void* data ) {
	SwarmAdvector*	self = (SwarmAdvector*)swarmAdvector;

	_TimeIntegrand_Destroy( self, data );
   Stg_Component_Destroy( self->velocityField, data, False );
   Stg_Component_Destroy( self->swarm, data, False );
	if ( self->periodicBCsManager )
		Stg_Component_Destroy( self->periodicBCsManager, data, False );
}

Bool _SwarmAdvector_TimeDeriv( void* swarmAdvector, Index array_I, double* timeDeriv ) {
  SwarmAdvector*      self          = (SwarmAdvector*) swarmAdvector;
  GeneralSwarm*       swarm         = self->swarm;
  CellLayout*         layout        = swarm->cellLayout;
  FeVariable*         velocityField = (FeVariable*) self->velocityField;
  FeMesh*             mesh          = velocityField->feMesh;
  double*             coord;
  InterpolationResult result;

  /* Get Coordinate of Object using Variable */
  coord = StgVariable_GetPtrDouble( self->variable, array_I );

  // if a non regular mesh and ElementCellLayout use the particle and mesh information to optimise search
  if( !mesh->isRegular &&
      Stg_Class_IsInstance(layout, ElementCellLayout_Type) )
  {
    GlobalParticle*     particle = (GlobalParticle*)Swarm_ParticleAt( swarm, array_I );
    FeMesh_ElementType* eType;
    unsigned            cellID;
    double              xi[3];

    // find the cell/element the particle is in
    cellID = CellLayout_CellOf( layout, particle );

    if( cellID >= CellLayout_CellLocalCount( layout ) ) return False; // if not on local proc report False
    /*
       CellLayout_CellOf() will actually evaulate the local coordinate and store it
       on a temporary valiable (on 'FeMesh_ElementType->local[]'). Below we exploit that
    */

    // get particle's local coordinate - HACKY but efficient
    eType = (FeMesh_ElementType*)mesh->elTypes[mesh->elTypeMap[cellID]];
    FeMesh_ElementTypeGetLocal( eType, xi );

    // do interpoplation
    FeVariable_InterpolateWithinElement( velocityField, cellID, xi, timeDeriv );
    return True;
  }
  else
  {
    // if mesh is regular
    result = FieldVariable_InterpolateValueAt( velocityField, coord, timeDeriv );
  }

  if ( result == OTHER_PROC || result == OUTSIDE_GLOBAL || isinf(timeDeriv[0]) || isinf(timeDeriv[1]) ||
     ( swarm->dim == 3 && isinf(timeDeriv[2]) ) )
  {
    return False;
  }

  return True;
}

Bool _SwarmAdvector_TimeDeriv_Quicker4IrregularMesh( void* swarmAdvector, Index array_I, double* timeDeriv ) {
   SwarmAdvector*      self          = (SwarmAdvector*) swarmAdvector;
   FeVariable*      velocityField = (FeVariable*) self->velocityField;
   GlobalParticle* particle=NULL;
   FeMesh_ElementType* eType=NULL;
   double xi[3];
   unsigned cellID;

   particle = (GlobalParticle*)Swarm_ParticleAt( self->swarm, array_I );
   assert( particle );

   // get particle's cell/element id - ASSUMES 1-to-1 cellID to elementID
   // if not local return False
   cellID = CellLayout_CellOf( self->swarm->cellLayout, particle );
   if( cellID >= CellLayout_CellLocalCount( self->swarm->cellLayout ) )
      return False;

   /* below is some funky code to optimise time in retrieving :
      Why? for an irregular mesh the function CellLayout_CellOf() will actually
      evaulate the local coordinate and store it on a temporary valiable (on 'FeMesh_ElementType->local[]')
      we will access below, instead of re-evaluating the GlobaToLocal loop (i.e. Newton-Raphson iteration)
      with a function like FeMesh_CoordGlobalToLocal().

      This ASSUMES that the temporary storage isn't modified between CellLayout_CellOf() and here.
   */

   // get particle's local coordinate
   eType = (FeMesh_ElementType*)velocityField->feMesh->elTypes[velocityField->feMesh->elTypeMap[cellID]];
   FeMesh_ElementTypeGetLocal( eType, xi );

   // do interpolation
   FeVariable_InterpolateWithinElement( velocityField, cellID, xi, timeDeriv );

   return True;
}


void _SwarmAdvector_Intermediate( void* swarmAdvector, Index lParticle_I ) {
	SwarmAdvector*      self          = (SwarmAdvector*) swarmAdvector;

	if ( self->periodicBCsManager ) {
      Journal_Firewall( (Swarm*)self->swarm == (Swarm*)self->periodicBCsManager->swarm, NULL,
            "Error in %s. The SwarmAdvector %s and periodBCsManager %s don't use the same swarm, erroroneos condition\n",
            self->swarm->name, self->periodicBCsManager->swarm->name );

		// use the polymorphic function not a static one!
		self->periodicBCsManager->_updateParticle( self->periodicBCsManager, lParticle_I );
	}
}
/*-------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/
/*---------------------------------------------------------------------------------------------------------------------
** Entry Point Hooks
*/

double SwarmAdvector_MaxDt( void* swarmAdvector ) {
	SwarmAdvector*	self = (SwarmAdvector*) swarmAdvector;
	double                  velMax        = 0;
	double                  minSeparation = 0;
	double                  minSeparationEachDim[3] = { 0, 0, 0 };
	FeVariable*             velFeVar      = self->velocityField;
	double			localDt;
	double			globalDt;

	velMax = FieldVariable_GetMaxGlobalFieldMagnitude( velFeVar );

	FeVariable_GetMinimumSeparation( velFeVar, &minSeparation, minSeparationEachDim );

	localDt = 0.5 * minSeparation / velMax;
	(void)MPI_Allreduce( &localDt, &globalDt, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );

	return globalDt;
}


/*-------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
