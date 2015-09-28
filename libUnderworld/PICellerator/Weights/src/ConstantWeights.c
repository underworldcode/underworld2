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

#include "types.h"
#include "WeightsCalculator.h"
#include "ConstantWeights.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type ConstantWeights_Type = "ConstantWeights";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ConstantWeights* ConstantWeights_New( Name name, int dim ) {
    ConstantWeights *self = _ConstantWeights_DefaultNew( name );

    self->isConstructed = True;
    _WeightsCalculator_Init( self, dim );
    _ConstantWeights_Init( self );

	return self;
}

ConstantWeights* _ConstantWeights_New(  CONSTANTWEIGHTS_DEFARGS  ) {
    ConstantWeights* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(ConstantWeights) );
    self = (ConstantWeights*)_WeightsCalculator_New(  WEIGHTSCALCULATOR_PASSARGS  );

    /* General info */

    /* Virtual Info */

    return self;
}

void _ConstantWeights_Init( void* constantWeights  ) {
	ConstantWeights* self;
		
	self = (ConstantWeights*)constantWeights;
}


/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ConstantWeights_Delete( void* constantWeights ) {
    ConstantWeights* self = (ConstantWeights*)constantWeights;
        
    /* Delete parent */
    _WeightsCalculator_Delete( self );
}


void _ConstantWeights_Print( void* constantWeights, Stream* stream ) {
    ConstantWeights* self = (ConstantWeights*)constantWeights;
        
    /* Print parent */
    _WeightsCalculator_Print( self, stream );
}



void* _ConstantWeights_Copy( void* constantWeights, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    ConstantWeights*    self = (ConstantWeights*)constantWeights;
    ConstantWeights*    newConstantWeights;
        
    newConstantWeights = (ConstantWeights*)_WeightsCalculator_Copy( self, dest, deep, nameExt, ptrMap );
        
    return (void*)newConstantWeights;
}

void* _ConstantWeights_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(ConstantWeights);
	Type                                                      type = ConstantWeights_Type;
	Stg_Class_DeleteFunction*                              _delete = _ConstantWeights_Delete;
	Stg_Class_PrintFunction*                                _print = _ConstantWeights_Print;
	Stg_Class_CopyFunction*                                  _copy = _ConstantWeights_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _ConstantWeights_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _ConstantWeights_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _ConstantWeights_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _ConstantWeights_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _ConstantWeights_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _ConstantWeights_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	WeightsCalculator_CalculateFunction*                _calculate = _ConstantWeights_Calculate;

    return (void*) _ConstantWeights_New(  CONSTANTWEIGHTS_PASSARGS  );
}


void _ConstantWeights_AssignFromXML( void* constantWeights, Stg_ComponentFactory* cf, void* data ) {
    ConstantWeights*         self          = (ConstantWeights*) constantWeights;

    _WeightsCalculator_AssignFromXML( self, cf, data );
        
    _ConstantWeights_Init( self );
}

void _ConstantWeights_Build( void* constantWeights, void* data ) {
    ConstantWeights*    self = (ConstantWeights*)constantWeights;

    _WeightsCalculator_Build( self, data );
}

void _ConstantWeights_Destroy( void* constantWeights, void* data ) {
    ConstantWeights*    self = (ConstantWeights*)constantWeights;

    _WeightsCalculator_Destroy( self, data );
}

void _ConstantWeights_Initialise( void* constantWeights, void* data ) {
    ConstantWeights*    self = (ConstantWeights*)constantWeights;
        
    _WeightsCalculator_Initialise( self, data );
}
void _ConstantWeights_Execute( void* constantWeights, void* data ) {
    ConstantWeights*    self = (ConstantWeights*)constantWeights;
        
    _WeightsCalculator_Execute( self, data );
}

/*-------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/
void _ConstantWeights_Calculate( void* constantWeights, void* _swarm, Cell_LocalIndex lCell_I ) {
    ConstantWeights*             self            = (ConstantWeights*)  constantWeights;
    Swarm*                       swarm           = (Swarm*) _swarm;
    double                       weight;
    Particle_InCellIndex         cParticleCount;
                
    cParticleCount = swarm->cellParticleCountTbl[lCell_I];
    weight = self->cellLocalVolume / (double) cParticleCount;
    WeightsCalculator_SetWeightsValueAllInCell( self, swarm, lCell_I, weight );
}

/*-------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/




