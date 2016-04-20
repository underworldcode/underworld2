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

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type WeightsCalculator_Type = "WeightsCalculator";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/


WeightsCalculator* _WeightsCalculator_New(  WEIGHTSCALCULATOR_DEFARGS  ) {
    WeightsCalculator* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(WeightsCalculator) );
    self = (WeightsCalculator*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

    /* General info */

    /* Virtual Info */
    self->_calculate = _calculate;

    return self;
}

void _WeightsCalculator_Init( void* weightsCalculator, int dim ) {
    WeightsCalculator* self = (WeightsCalculator*)weightsCalculator;

    self->dim = dim;
    self->cellLocalVolume = pow( 2.0, (double) dim );
}


/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _WeightsCalculator_Delete( void* weightsCalculator ) {
    WeightsCalculator* self = (WeightsCalculator*)weightsCalculator;

    /* Delete parent */
    _Stg_Component_Delete( self );
}


void _WeightsCalculator_Print( void* weightsCalculator, Stream* stream ) {
    WeightsCalculator* self = (WeightsCalculator*)weightsCalculator;

    /* Print parent */
    _Stg_Component_Print( self, stream );
}


void* _WeightsCalculator_Copy( void* weightsCalculator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    WeightsCalculator*  self = (WeightsCalculator*)weightsCalculator;
    WeightsCalculator*  newWeightsCalculator;

    newWeightsCalculator = (WeightsCalculator*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );

    return (void*)newWeightsCalculator;
}



void _WeightsCalculator_AssignFromXML( void* weightsCalculator, Stg_ComponentFactory* cf, void* data ) {
    WeightsCalculator*   self          = (WeightsCalculator*) weightsCalculator;
    Dimension_Index      dim;

    //self->context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", FiniteElementContext, False, data );
    //if( !self->context  )
    //    self->context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", FiniteElementContext, True, data  );

    dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );

    _WeightsCalculator_Init( self, dim );
}

void _WeightsCalculator_Build( void* weightsCalculator, void* data ) {
/*      WeightsCalculator*      self = (WeightsCalculator*)weightsCalculator; */

}

void _WeightsCalculator_Destroy( void* weightsCalculator, void* data ) {
/*      WeightsCalculator*      self = (WeightsCalculator*)weightsCalculator; */
}


void _WeightsCalculator_Initialise( void* weightsCalculator, void* data ) {
/*      WeightsCalculator*      self = (WeightsCalculator*)weightsCalculator; */

}
void _WeightsCalculator_Execute( void* weightsCalculator, void* data ) {
/*      WeightsCalculator*      self = (WeightsCalculator*)weightsCalculator; */

}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void WeightsCalculator_CalculateCell( void* weightsCalculator, void* swarm, Cell_LocalIndex lCell_I ) {
    WeightsCalculator*  self = (WeightsCalculator*)weightsCalculator;

    self->_calculate( self, swarm, lCell_I );
}



/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


void WeightsCalculator_CalculateAll( void* weightsCalculator, void* _swarm ) {
    WeightsCalculator*   self           = (WeightsCalculator*)weightsCalculator;
    Swarm*               swarm          = (Swarm*) _swarm;
    Cell_LocalIndex          cellLocalCount = swarm->cellLocalCount;
    Cell_LocalIndex          lCell_I;
    unsigned int         numberOfCompletionPrintIncrements=3;
    double               completionRatioIncrement= 1 / (double)numberOfCompletionPrintIncrements;
    double               nextCompletionRatioToPrint=0;
    Cell_Index           nextCompletedCellCountToPrint=0;
    Cell_Index           nextPlusOneCompletedCellCountToPrint=0;
    Stream*              stream = Journal_Register( Info_Type, (Name)self->type  );

    Journal_RPrintf( stream, "In func %s(): for swarm \"%s\"\n", __func__, swarm->name );
    Stream_Indent( stream );
    Stream_SetPrintingRank( stream, 0 );

    nextCompletionRatioToPrint = completionRatioIncrement;
    nextCompletedCellCountToPrint = ceil(cellLocalCount * nextCompletionRatioToPrint - 0.001 );

    /* Loop over all local cells */
    for ( lCell_I = 0 ; lCell_I < cellLocalCount ; lCell_I++ ) {
/*              WeightsCalculator_CheckEmptyCell( self, swarm, lCell_I );*/
        WeightsCalculator_CalculateCell( self, swarm, lCell_I );
/*               TODO: parallelise */

        if ( (lCell_I+1) >= nextCompletedCellCountToPrint ) {
            nextPlusOneCompletedCellCountToPrint = ceil(( cellLocalCount
                                                          * (nextCompletionRatioToPrint + completionRatioIncrement )) - 0.001 );

            while ( (lCell_I+1) >= nextPlusOneCompletedCellCountToPrint )
            {
                nextCompletionRatioToPrint += completionRatioIncrement;
                nextPlusOneCompletedCellCountToPrint = ceil(( cellLocalCount
                                                              * (nextCompletionRatioToPrint + completionRatioIncrement )) - 0.001 );
                if ( nextCompletionRatioToPrint >= 1.0 ) {
                    nextCompletionRatioToPrint = 1.0;
                    break;
                }
            }
            Journal_Printf( stream, "done %.0f%% (%u cells)...\n",
                            (nextCompletionRatioToPrint * 100),
                            lCell_I+1 );
            nextCompletionRatioToPrint += completionRatioIncrement;
            nextCompletedCellCountToPrint = ceil(cellLocalCount * nextCompletionRatioToPrint - 0.001);
        }
    }
    Stream_UnIndent( stream );
    Journal_RPrintf( stream, "%s(): finished update of weights for swarm \"%s\"\n"
                     /*"\ttook %g secs\n"*/, __func__, swarm->name );
}


void WeightsCalculator_SetWeightsValueAll( void* weightsCalculator, void* _swarm, double weight ) {
    WeightsCalculator*           self              = (WeightsCalculator*) weightsCalculator;
    Swarm*                       swarm             = (Swarm*) _swarm;
    Cell_LocalIndex              lCell_I;

    for ( lCell_I = 0 ; lCell_I < swarm->cellLocalCount ; lCell_I++ )
        WeightsCalculator_SetWeightsValueAllInCell( self, swarm, lCell_I, weight );
}

void WeightsCalculator_SetWeightsValueAllInCell( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I, double weight ) {
    Swarm*                       swarm             = (Swarm*) _swarm;
    Particle_InCellIndex         cParticleCount    = swarm->cellParticleCountTbl[lCell_I];
    Particle_InCellIndex         cParticle_I;
    IntegrationPoint*            particle;

    /* Loop over particles in this cell */
    for ( cParticle_I = 0 ; cParticle_I < cParticleCount ; cParticle_I++ ) {
        particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );

        particle->weight = weight;
    }
}

#define TOLERENCE 0.0001
#define MAX_ORDER 100

Constraint_Index WeightsCalculator_FindConstraintOrder( void* weightsCalculator, void* _swarm, Dimension_Index dim, Stream* stream ) {
    WeightsCalculator*           self              = (WeightsCalculator*) weightsCalculator;
    Swarm*                       swarm             = (Swarm*) _swarm;
    Constraint_Index             order             =  0;
    double                       error;

    while ( (error = WeightsCalculator_TestConstraint( self, swarm, dim, order )) < TOLERENCE ) {
        order++;
        if (order > MAX_ORDER) {
            Journal_Printf( stream, "Reached maximum number of constraints to calculate.\n");
            break;
        }
    }

    /* If test fails then decrement to last order that the test passed */
    order--;

    if (order == (Constraint_Index) -1)
        Journal_Printf( stream, "Weights '%s' for swarm '%s' satisfy no weight constraints.\n",
                        self->name, swarm->name );
    else
        Journal_Printf( stream, "Weights '%s' for swarm '%s' satisfy weight constraints to order %u.\n",
                        self->name, swarm->name, order );

    return order;
}

double WeightsCalculator_TestConstraint( void* weightsCalculator, void* _swarm, Dimension_Index dim, Constraint_Index order ) {
    WeightsCalculator*           self              = (WeightsCalculator*) weightsCalculator;
    Swarm*                       swarm             = (Swarm*) _swarm;
    Cell_LocalIndex                  lCell_I;
    double                       error            = 0.0;

    /* Loop over all local cells */
    for ( lCell_I = 0 ; lCell_I < swarm->cellLocalCount ; lCell_I++ )
        error += WeightsCalculator_TestConstraintOverCell( self, swarm, lCell_I, dim, order );

    Journal_DPrintfL( swarm->debug, 2, "In func %s: Error for weights for constraints of order %u is %0.3g\n", __func__, order, error );
    return error;
}

#define IS_ODD(A)  ((A) % 2 == 1)

double WeightsCalculator_TestConstraintOverCell( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I, Dimension_Index dim, Constraint_Index order ) {
    Swarm*                       swarm             = (Swarm*) _swarm;
    Particle_InCellIndex         cParticle_I;
    IntegrationPoint*            particle;
    double                       volume            = pow(2.0, (double) dim);
    Index                        power_i, power_j;
    double                       rightHandSide;
    double                       leftHandSide;
    double                       error             = 0.0;
    double*                      xi;
    Constraint_Index             constraintCount   = 0;

    Journal_DPrintfL( swarm->debug, 2,
                      "In func %s: For Cell %u and Dimension %u and order %u\n", __func__, lCell_I, dim, order );
    Stream_Indent( swarm->debug );

    for ( power_j = 0 ; power_j <= order ; power_j++ ) {
        for ( power_i = power_j ; power_i <= order ; power_i++ ) {
            /* Sum number of constrainst calculated */
            constraintCount++;

            /************ Calculate Right Hand Side of Constraint **************************/
            if ( IS_ODD( power_i ) || IS_ODD( power_j ) || IS_ODD( order - power_i - power_j ) )
                rightHandSide = 0.0;
            else
                rightHandSide = volume/
                    (double)(( power_i + 1 )*( power_j + 1 )*( order - power_i - power_j + 1));

            Journal_DPrintfL( swarm->debug, 3,
                              "Constraint %u: \\Sigma w_p \\xi^%u \\eta^%u \\Zeta^%u = %0.3g\n",
                              constraintCount, power_i, order - power_i - power_j, power_j , rightHandSide );
            /************ Calculate Left Hand Side of Constraint **************************/
            leftHandSide = 0.0;

            /* Loop over particles in this cell */
            Stream_Indent( swarm->debug );
            for ( cParticle_I = 0 ; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
                particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );

                /* Get local Coordinates of particle */
                xi = particle->xi;

                leftHandSide += particle->weight *
                    pow( xi[ I_AXIS ], (double) power_i ) *
                    pow( xi[ J_AXIS ], (double) (order - power_i - power_j) ) *
                    pow( xi[ K_AXIS ], (double) power_j ) ;
                Journal_DPrintfL( swarm->debug, 3,
                                  "ParticleInCell %u, weight = %0.3g, local element coords = (%0.3g, %0.3g, %0.3g)\n",
                                  cParticle_I, particle->weight,
                                  xi[ I_AXIS ], xi[ J_AXIS ], xi[ K_AXIS ],
                                  power_i, (order - power_i - power_j), power_j );
            }
            Stream_UnIndent( swarm->debug );

            /* Calculate Error */
            error += fabs(leftHandSide - rightHandSide);

            Journal_DPrintfL( swarm->debug, 3, "RHS = %2.5f LHS = %2.5f: Total Error = %f\n", rightHandSide, leftHandSide, error );

        }
        /* 3D Constrain Equations collapse to 2D constraint equation only if j = 0 */
        if (dim == 2)
            break;
    }
    Stream_UnIndent( swarm->debug );

    Journal_Firewall( dim == 2 ? constraintCount == order + 1 : constraintCount*2 == (order + 1)*(order + 2) ,
                      Journal_Register( Error_Type, (Name)swarm->type  ),
                      "In func %s: Number of constraints %u incorrect for dimension %u which should be %u\n",
                      __func__ , dim == 2 ? order + 1 : (order + 1)*(order + 2)/2 );

    return error;
}

double WeightsCalculator_GetConstraintLHS( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I, Index power_i, Index power_j, Index power_k ) {
    Swarm*                       swarm           = (Swarm*)             _swarm;
    Particle_InCellIndex         cParticleCount  = swarm->cellParticleCountTbl[lCell_I];
    Particle_InCellIndex         cParticle_I;
    IntegrationPoint*            particle;
    double                       leftHandSide    = 0.0;
    double*                      xi;

    /* Loop over particles in this cell */
    for ( cParticle_I = 0 ; cParticle_I < cParticleCount ; cParticle_I++ ) {
        particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );

        /* Get local Coordinates of particle */
        xi = particle->xi;

        leftHandSide += particle->weight *
            pow( xi[ I_AXIS ], (double) power_i ) *
            pow( xi[ J_AXIS ], (double) power_j ) ;

        if ( swarm->dim == 3 )
            leftHandSide += particle->weight * pow( xi[ K_AXIS ], (double) power_k );

    }
    return leftHandSide;
}

double WeightsCalculator_GetLocalCoordSum( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I, Index power_i, Index power_j, Index power_k )
{
    Swarm*                       swarm           = (Swarm*) _swarm;
    Particle_InCellIndex         cParticleCount  = swarm->cellParticleCountTbl[lCell_I];
    Particle_InCellIndex         cParticle_I;
    IntegrationPoint*            particle;
    double                       localCoordSum   = 0.0;
    double*                      xi;

    /* Loop over particles in this cell */
    for ( cParticle_I = 0 ; cParticle_I < cParticleCount ; cParticle_I++ ) {
        particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );

        /* Get local Coordinates of particle */
        xi = particle->xi;

        localCoordSum +=
            pow( xi[ I_AXIS ], (double) power_i ) *
            pow( xi[ J_AXIS ], (double) power_j ) ;

        if ( swarm->dim == 3 )
            localCoordSum += pow( xi[ K_AXIS ], (double) power_k );

    }
    return localCoordSum;
}


double WeightsCalculator_SumCellWeights( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I ) {
    Swarm*                       swarm           = (Swarm*) _swarm;
    double                       weightsTotal    = 0.0;
    Particle_InCellIndex         cParticleCount  = swarm->cellParticleCountTbl[lCell_I];
    Particle_InCellIndex         cParticle_I;
    IntegrationPoint*            particle;

    /* Find Sum of the Weights */
    for ( cParticle_I = 0 ; cParticle_I < cParticleCount ; cParticle_I++ ) {
        particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );

        /* Put weight onto particle */
        weightsTotal += particle->weight;
    }

    return weightsTotal;
}

void WeightsCalculator_CheckEmptyCell( void* weightsCalculator, void* _swarm, Cell_LocalIndex lCell_I ) {
    WeightsCalculator*           self              = (WeightsCalculator*) weightsCalculator;
    Swarm*                       swarm           = (Swarm*) _swarm;
    Particle_InCellIndex         cParticleCount  = swarm->cellParticleCountTbl[lCell_I];

    if ( cParticleCount == 0 ) {
        Journal_Firewall( cParticleCount, Journal_Register( Error_Type, (Name)self->type  ),
                          "Error in func '%s' for %s '%s' and %s '%s' - Cell %u has no particles.\n"
                          "You must either add more initial particles or add population control.\n",
                          __func__, self->type, self->name, swarm->type, swarm->name, lCell_I );
    }
}
