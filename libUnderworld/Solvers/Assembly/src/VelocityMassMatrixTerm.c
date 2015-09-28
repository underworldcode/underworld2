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
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "types.h"
#include "VelocityMassMatrixTerm.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type VelocityMassMatrixTerm_Type = "VelocityMassMatrixTerm";

/* Creation implementation / Virtual constructor */
VelocityMassMatrixTerm* _VelocityMassMatrixTerm_New(  VELOCITYMASSMATRIXTERM_DEFARGS  )
{
   VelocityMassMatrixTerm* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(VelocityMassMatrixTerm) );
   self = (VelocityMassMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

   /* Virtual info */

   return self;
}

void _VelocityMassMatrixTerm_Init( void* matrixTerm ) {
   VelocityMassMatrixTerm* self = (VelocityMassMatrixTerm*)matrixTerm;

   self->errorStream   = Journal_Register( Error_Type, (Name)self->name  );

   Journal_Firewall( self->stiffnessMatrix->rowVariable == self->stiffnessMatrix->columnVariable,
      self->errorStream,
      "\n\nError - in %s: This Matrix term requires identical row and column variables for the stiffness matrix.\n",
      __func__ );

}

void _VelocityMassMatrixTerm_Delete( void* matrixTerm ) {
   VelocityMassMatrixTerm* self = (VelocityMassMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Delete( self );
}

void _VelocityMassMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
   VelocityMassMatrixTerm* self = (VelocityMassMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Print( self, stream );

   /* General info */
}

void* _VelocityMassMatrixTerm_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                 _sizeOfSelf = sizeof(VelocityMassMatrixTerm);
   Type                                                         type = VelocityMassMatrixTerm_Type;
   Stg_Class_DeleteFunction*                                 _delete = _VelocityMassMatrixTerm_Delete;
   Stg_Class_PrintFunction*                                   _print = _VelocityMassMatrixTerm_Print;
   Stg_Class_CopyFunction*                                     _copy = NULL;
   Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _VelocityMassMatrixTerm_DefaultNew;
   Stg_Component_ConstructFunction*                       _construct = _VelocityMassMatrixTerm_AssignFromXML;
   Stg_Component_BuildFunction*                               _build = _VelocityMassMatrixTerm_Build;
   Stg_Component_InitialiseFunction*                     _initialise = _VelocityMassMatrixTerm_Initialise;
   Stg_Component_ExecuteFunction*                           _execute = _VelocityMassMatrixTerm_Execute;
   Stg_Component_DestroyFunction*                           _destroy = _VelocityMassMatrixTerm_Destroy;
   StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _VelocityMassMatrixTerm_AssembleElement;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_VelocityMassMatrixTerm_New(  VELOCITYMASSMATRIXTERM_PASSARGS  );
}

void _VelocityMassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
   VelocityMassMatrixTerm*            self             = (VelocityMassMatrixTerm*)matrixTerm;

   /* Construct Parent */
   _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

   _VelocityMassMatrixTerm_Init( self );
}

void _VelocityMassMatrixTerm_Build( void* matrixTerm, void* data ) {
   VelocityMassMatrixTerm*             self             = (VelocityMassMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Build( self, data );
}

void _VelocityMassMatrixTerm_Initialise( void* matrixTerm, void* data ) {
   VelocityMassMatrixTerm* self = (VelocityMassMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Initialise( self, data );
}

void _VelocityMassMatrixTerm_Execute( void* matrixTerm, void* data ) {
   _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _VelocityMassMatrixTerm_Destroy( void* matrixTerm, void* data ) {
   _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _VelocityMassMatrixTerm_AssembleElement(
      void*                                              matrixTerm,
      StiffnessMatrix*                                   stiffnessMatrix,
      Element_LocalIndex                                 lElement_I,
      SystemLinearEquations*                             sle,
      FiniteElementContext*                              context,
      double**                                           elStiffMat )
{
   VelocityMassMatrixTerm*   self = (VelocityMassMatrixTerm*)matrixTerm;
   Swarm*                              swarm         = self->integrationSwarm;
   FeVariable*                         variable1     = stiffnessMatrix->rowVariable;
   Dimension_Index                     dim           = stiffnessMatrix->dim;
   int                                 dofsPerNode   = variable1->fieldComponentCount;
   IntegrationPoint*                   currIntegrationPoint;
   double*                             xi;
   double                              weight;
   Particle_InCellIndex                cParticle_I, cellParticleCount;
   Index                               nodesPerEl;
   Index                               A,B;
   Index                               i;
   double                              detJac;
   Cell_Index                          cell_I;
   ElementType*                        elementType;
   double                              N[27];

   /* Set the element type */
   elementType = FeMesh_GetElementType( variable1->feMesh, lElement_I );
   nodesPerEl = elementType->nodeCount;

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {

      currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

      xi = currIntegrationPoint->xi;
      weight = currIntegrationPoint->weight;

      /* Calculate Determinant of Jacobian and Shape Functions */
      detJac = ElementType_JacobianDeterminant( elementType, variable1->feMesh, lElement_I, xi, dim );
      ElementType_EvaluateShapeFunctionsAt( elementType, xi, N );

      for( A=0; A<nodesPerEl; A++ ) {
         for( B=0; B<nodesPerEl; B++ ) {
            for ( i = 0; i < dofsPerNode ; i++ ) {
		elStiffMat[dofsPerNode*A+i][dofsPerNode*B+i] += detJac * weight * N[A] * N[B];/* no "cross" terms in this integral Maxbx = Mayby = Mazbz = int(Na*Nb) */
            }
         }
      }
   }
}


