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
#include <assert.h>
#include <time.h>

#include <string>
#include <sstream>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include "types.h"
#include "ConstitutiveMatrix.h"
#include "ConstitutiveMatrixCartesian.h"
}


/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type ConstitutiveMatrixCartesian_Type = "ConstitutiveMatrixCartesian";

void _ConstitutiveMatrixCartesian_Set_Fn_Visc1( void* _self, Fn::Function* fn_visc1 ){
    ConstitutiveMatrixCartesian*  self = (ConstitutiveMatrixCartesian*)_self;
    
    // record fn to struct
    ConstitutiveMatrixCartesian_cppdata* cppdata = (ConstitutiveMatrixCartesian_cppdata*) self->cppdata;
    
    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);

    cppdata->func_visc1 = fn_visc1->getFunction(cppdata->input);
    // check output conforms
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->func_visc1(cppdata->input));
    if( !iodub )
        throw std::invalid_argument("Viscosity function is expected to return 'double' type values.");
    if( iodub->size() != 1 )
        throw std::invalid_argument("Viscosity function is expected to return scalar values.");

}

void _ConstitutiveMatrixCartesian_Set_Fn_Visc2( void* _self, Fn::Function* fn_visc2 ){
    ConstitutiveMatrixCartesian*  self = (ConstitutiveMatrixCartesian*)_self;
    
    // record fn to struct
    ConstitutiveMatrixCartesian_cppdata* cppdata = (ConstitutiveMatrixCartesian_cppdata*) self->cppdata;
    
    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);

    cppdata->func_visc2 = fn_visc2->getFunction(cppdata->input);
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->func_visc2(cppdata->input));
    if( !iodub )
        throw std::invalid_argument("Second viscosity function is expected to return 'double' type values.");
    if( iodub->size() != 1 )
        throw std::invalid_argument("Second viscosity function is expected to return scalar values.");
    
}

void _ConstitutiveMatrixCartesian_Set_Fn_Director( void* _self, Fn::Function* fn_director ){
    ConstitutiveMatrixCartesian*  self = (ConstitutiveMatrixCartesian*)_self;
    
    // record fn to struct
    ConstitutiveMatrixCartesian_cppdata* cppdata = (ConstitutiveMatrixCartesian_cppdata*) self->cppdata;
    
    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);
    
    // now setup director
    cppdata->func_director = fn_director->getFunction(cppdata->input);
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->func_director(cppdata->input));
    if( !iodub )
        throw std::invalid_argument("Director function is expected to return 'double' type values.");
    if( iodub->size() != self->dim )
    {
        std::stringstream ss;
        ss << "Director function is expected to return vector values of dimensionality " << self->dim << ".\n"  \
           << "Function provided returns values with size " << iodub->size() << ".";
        throw std::invalid_argument(ss.str());
    }
}


/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
ConstitutiveMatrixCartesian* _ConstitutiveMatrixCartesian_New(  CONSTITUTIVEMATRIXCARTESIAN_DEFARGS  )
{
   ConstitutiveMatrixCartesian* self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(ConstitutiveMatrixCartesian) );
   self = (ConstitutiveMatrixCartesian*) _ConstitutiveMatrix_New(  CONSTITUTIVEMATRIX_PASSARGS  );

   /* Function pointers for this class that are not on the parent class should be set here */

   self->beenHere = 0;
   self->cppdata = (void*) new ConstitutiveMatrixCartesian_cppdata;

   return self;
}

void _ConstitutiveMatrixCartesian_Init(
      ConstitutiveMatrixCartesian*                 self )
{
   self->rowSize = self->columnSize = StGermain_nSymmetricTensorVectorComponents( self->dim );
   self->Dtilda_B = Memory_Alloc_2DArray( double, self->rowSize, self->dim, (Name)"D~ times B matrix" );

   if( self->dim == 2 ) {
      self->_setValue = _ConstitutiveMatrixCartesian2D_SetValueInAllEntries;
      self->_setSecondViscosity = _ConstitutiveMatrixCartesian2D_SetSecondViscosity;
      self->_getViscosity = _ConstitutiveMatrixCartesian2D_GetIsotropicViscosity;
      self->_isotropicCorrection = _ConstitutiveMatrixCartesian2D_IsotropicCorrection;
      self->_assemble_D_B = _ConstitutiveMatrixCartesian2D_Assemble_D_B;
      self->_calculateStress = _ConstitutiveMatrixCartesian2D_CalculateStress;
   } else {
      self->_setValue = _ConstitutiveMatrixCartesian3D_SetValueInAllEntries;
      self->_setSecondViscosity = _ConstitutiveMatrixCartesian3D_SetSecondViscosity;
      self->_getViscosity = _ConstitutiveMatrixCartesian3D_GetIsotropicViscosity;
      self->_isotropicCorrection = _ConstitutiveMatrixCartesian3D_IsotropicCorrection;
      self->_assemble_D_B = _ConstitutiveMatrixCartesian3D_Assemble_D_B;
      self->_calculateStress = _ConstitutiveMatrixCartesian3D_CalculateStress;
   }

  /* store each particle's constitutiveMatrix */
  if( self->storeConstitutiveMatrix )
      assert(0);  // this has been deprecated.  check previous history if function is required.
      //ConstitutiveMatrixCartesian_SetupParticleStorage( self );

  // set to true here, if non isotropic rheologies are use they should switch this to be false automatically
  self->isDiagonal=True;
}

void _ConstitutiveMatrixCartesian_Delete( void* constitutiveMatrix ) {
   ConstitutiveMatrixCartesian* self = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   delete (ConstitutiveMatrixCartesian_cppdata*)self->cppdata;

   _ConstitutiveMatrix_Delete( self  );
}

void _ConstitutiveMatrixCartesian_Print( void* constitutiveMatrix, Stream* stream ) {
   ConstitutiveMatrixCartesian* self = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   _ConstitutiveMatrix_Print( self, stream );

   /* General info */
}

void* _ConstitutiveMatrixCartesian_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                  _sizeOfSelf = sizeof(ConstitutiveMatrixCartesian);
	Type                                                          type = ConstitutiveMatrixCartesian_Type;
	Stg_Class_DeleteFunction*                                  _delete = _ConstitutiveMatrixCartesian_Delete;
	Stg_Class_PrintFunction*                                    _print = _ConstitutiveMatrixCartesian_Print;
	Stg_Class_CopyFunction*                                      _copy = NULL;
	Stg_Component_DefaultConstructorFunction*      _defaultConstructor = _ConstitutiveMatrixCartesian_DefaultNew;
	Stg_Component_ConstructFunction*                        _construct = _ConstitutiveMatrixCartesian_AssignFromXML;
	Stg_Component_BuildFunction*                                _build = _ConstitutiveMatrixCartesian_Build;
	Stg_Component_InitialiseFunction*                      _initialise = _ConstitutiveMatrixCartesian_Initialise;
	Stg_Component_ExecuteFunction*                            _execute = _ConstitutiveMatrixCartesian_Execute;
	Stg_Component_DestroyFunction*                            _destroy = _ConstitutiveMatrixCartesian_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*      _assembleElement = _ConstitutiveMatrixCartesian_AssembleElement;
	ConstitutiveMatrix_SetValueFunc*                         _setValue = NULL;
	ConstitutiveMatrix_GetValueFunc*                     _getViscosity = NULL;
	ConstitutiveMatrix_SetValueFunc*              _isotropicCorrection = NULL;
	ConstitutiveMatrix_SetSecondViscosityFunc*     _setSecondViscosity = NULL;
	ConstitutiveMatrix_Assemble_D_B_Func*                _assemble_D_B = NULL;
	ConstitutiveMatrix_CalculateStressFunc*           _calculateStress = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_ConstitutiveMatrixCartesian_New(  CONSTITUTIVEMATRIXCARTESIAN_PASSARGS  );
}

void _ConstitutiveMatrixCartesian_AssignFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) {
   ConstitutiveMatrixCartesian*            self             = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   /* Construct Parent */
   _ConstitutiveMatrix_AssignFromXML( self, cf, data );

   _ConstitutiveMatrixCartesian_Init( self );
}

void _ConstitutiveMatrixCartesian_Build( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrixCartesian*             self             = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   _ConstitutiveMatrix_Build( self, data );

   /* Commented out with the section ~line 695
   if( self->storedConstSwarmVar ) 
      Stg_Component_Build( self->storedConstSwarmVar, NULL, False );
     */
}

void _ConstitutiveMatrixCartesian_Initialise( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrixCartesian*             self             = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   _ConstitutiveMatrix_Initialise( self, data );
}

void _ConstitutiveMatrixCartesian_Execute( void* constitutiveMatrix, void* data ) {
   _ConstitutiveMatrix_Execute( constitutiveMatrix, data );
}

void _ConstitutiveMatrixCartesian_Destroy( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrixCartesian* self = (ConstitutiveMatrixCartesian*)constitutiveMatrix;

   Memory_Free( self->Dtilda_B );
   Memory_Free( self->Ni );

   _ConstitutiveMatrix_Destroy( constitutiveMatrix, data );
}

void _ConstitutiveMatrixCartesian_AssembleElement(
      void*                                              constitutiveMatrix,
      StiffnessMatrix*                                   stiffnessMatrix,
      Element_LocalIndex                                 lElement_I,
      SystemLinearEquations*                             sle,
      FiniteElementContext*                              context,
      double**                                           elStiffMat )
{
   ConstitutiveMatrixCartesian*     self       = (ConstitutiveMatrixCartesian*) constitutiveMatrix;
   IntegrationPointsSwarm* swarm               = (IntegrationPointsSwarm*)self->integrationSwarm;
   FeVariable*             variable1           = stiffnessMatrix->rowVariable;
   Dimension_Index         dim                 = stiffnessMatrix->dim;
   IntegrationPoint*       particle;
   Particle_InCellIndex cParticle_I;
   Particle_InCellIndex cellParticleCount;
   Element_NodeIndex       elementNodeCount;
   Node_ElementLocalIndex  rowNode_I;
   Node_ElementLocalIndex  colNode_I;
   double**                GNx;
   double                  detJac;
   Cell_Index              cell_I;
   ElementType*            elementType;
   double                  Bj_x, Bj_y;
   double                  Bi_x;
   double                  Bi_y;
   double                  Bi_z;
   Dof_Index               rowNodeDof_I;
   Dof_Index               colNodeDof_I;
   Dof_Index               nodeDofCount;
   double**                Dtilda_B;
   double                  vel[3], velDerivs[9], *Ni, eta;
   Bool                    oneToMany;

   self->sle = sle;

   /* Set the element type */
   elementType       = FeMesh_GetElementType( variable1->feMesh, lElement_I );
   elementNodeCount  = elementType->nodeCount;
   nodeDofCount      = dim;

   /* allocate */
   if( elementNodeCount > self->max_nElNodes ) {
       self->max_nElNodes = elementNodeCount;
       self->GNx = Memory_Realloc_2DArray( self->GNx, double, dim, elementNodeCount );
       self->Ni =  Memory_Realloc_Array( self->Ni, double, elementNodeCount );
   }
   GNx = self->GNx;
   Ni = self->Ni;
   Dtilda_B = self->Dtilda_B;

   /* Get number of particles per element */
   cell_I            = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   /* Determine whether this is the first solve for not */
   Journal_Firewall( sle != NULL, Journal_Register( Error_Type, (Name)ConstitutiveMatrix_Type  ),
         "In func %s: SLE is NULL.\n", __func__ );

   /* Note: we may have deliberately set the previousSolutionExists flag to true in the
      parent ConstitutiveMatrix constructor if in restart mode, even if the SLE hasn't executed yet
      in this run - so only update to the sle's value when SLE is confirming it has
      executed */
   if ( True == sle->hasExecuted ) {
      self->previousSolutionExists = sle->hasExecuted;
   }

   self->sleNonLinearIteration_I = sle->nonLinearIteration_I;

   /*
	* Keep a flag indicating whether we are usinga one-to-one swarm mapper or not.
	*/

   ConstitutiveMatrixCartesian_cppdata* cppdata = (ConstitutiveMatrixCartesian_cppdata*)self->cppdata;

   /* check that things are setup correctly */
   
    if( cppdata->func_visc2 && !cppdata->func_director )
        throw std::invalid_argument("You do not appear to have a director set. If you have specified a second viscosity, you must also set a director.");

   debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
   cppdata->input->index() = lElement_I;  // set the elementId for the fem coordinate
   

   /* Loop over points to build Stiffness Matrix */
   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

      /* Calculate Determinant of Jacobian and Shape Function Global Derivatives */
      ElementType_ShapeFunctionsGlobalDerivs(
         elementType,
         variable1->feMesh, lElement_I,
         particle->xi, dim, &detJac, GNx );

        /* Evalulate velocity and velocity derivatives at this particle. */
        FeVariable_InterpolateWithinElement(
           variable1, lElement_I, particle->xi, vel );
        FeVariable_InterpolateDerivatives_WithGNx(
           variable1, lElement_I, GNx, velDerivs );

        debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId

        /* evaluate function */
        std::shared_ptr<const IO_double> visc1 = debug_dynamic_cast<const IO_double>(cppdata->func_visc1(cppdata->input));

        ConstitutiveMatrix_SetIsotropicViscosity( self, visc1->at() );
       
        if ( cppdata->func_visc2 ){
            std::shared_ptr<const IO_double> visc2    = debug_dynamic_cast<const IO_double>(cppdata->func_visc2(cppdata->input));
            std::shared_ptr<const IO_double> director = debug_dynamic_cast<const IO_double>(cppdata->func_director(cppdata->input));
            ConstitutiveMatrix_SetSecondViscosity( self, visc2->at(), director->data() );
        }

		eta = self->matrixData[2][2];

      /* Turn D Matrix into D~ Matrix by multiplying in the weight and the detJac (this is a shortcut for speed) */
      ConstitutiveMatrix_MultiplyByValue( constitutiveMatrix, detJac * particle->weight );

      for( rowNode_I = 0 ; rowNode_I < elementNodeCount ; rowNode_I++ ) {
         rowNodeDof_I = rowNode_I*nodeDofCount;
                        Bj_x = GNx[0][rowNode_I];
                        Bj_y = GNx[1][rowNode_I];

         /* Build D~ * B */
         ConstitutiveMatrix_Assemble_D_B( constitutiveMatrix, GNx, rowNode_I, Dtilda_B );

         for( colNode_I = 0 ; colNode_I < elementNodeCount ; colNode_I++ ) {
            colNodeDof_I = colNode_I*nodeDofCount;
            Bi_x = GNx[ I_AXIS ][colNode_I];
            Bi_y = GNx[ J_AXIS ][colNode_I];

            /* Build BTrans * ( D~ * B ) */
            if ( dim == 2 ) {
          if( !sle->nlFormJacobian ) {

             elStiffMat[ colNodeDof_I     ][ rowNodeDof_I     ] += Bi_x * Dtilda_B[0][0] + Bi_y * Dtilda_B[2][0];
             elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 1 ] += Bi_x * Dtilda_B[0][1] + Bi_y * Dtilda_B[2][1];
             elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I     ] += Bi_y * Dtilda_B[1][0] + Bi_x * Dtilda_B[2][0];
             elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 1 ] += Bi_y * Dtilda_B[1][1] + Bi_x * Dtilda_B[2][1];

          }
          else {
             double DuDx, DuDy, DvDx, DvDy;
             double DetaDu, DetaDv;
             double intFac, fac;

             DuDx = velDerivs[0]; DuDy = velDerivs[1];
             DvDx = velDerivs[2]; DvDy = velDerivs[3];
             DetaDu = self->derivs[0] * Bj_x + self->derivs[1] * Bj_y + self->derivs[2] * Ni[rowNode_I];
             DetaDv = self->derivs[3] * Bj_x + self->derivs[4] * Bj_y + self->derivs[5] * Ni[rowNode_I];
             intFac = particle->weight * detJac;

             fac = eta * Bj_y + DuDy * DetaDu + DvDx * DetaDu;
             elStiffMat[colNodeDof_I][rowNodeDof_I] +=
                intFac * (2.0 * Bi_x * (eta * Bj_x + DuDx * DetaDu) + Bi_y * fac);
             elStiffMat[colNodeDof_I + 1][rowNodeDof_I] +=
                intFac * (2.0 * Bi_y * DvDy * DetaDu + Bi_x * fac);

             fac = eta * Bj_x + DvDx * DetaDv + DuDy * DetaDv;
             elStiffMat[colNodeDof_I][rowNodeDof_I + 1] +=
                intFac * (2.0 * Bi_x * DuDx * DetaDv + Bi_y * fac);
             elStiffMat[colNodeDof_I + 1][rowNodeDof_I + 1] +=
                intFac * (2.0 * Bi_y * (eta * Bj_y + DvDy * DetaDv) + Bi_x * fac);

          }
            }
            else {
               Bi_z = GNx[ K_AXIS ][colNode_I];

               elStiffMat[ colNodeDof_I     ][ rowNodeDof_I     ] +=
                  Bi_x * Dtilda_B[0][0] + Bi_y * Dtilda_B[3][0] + Bi_z * Dtilda_B[4][0];
               elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 1 ] +=
                  Bi_x * Dtilda_B[0][1] + Bi_y * Dtilda_B[3][1] + Bi_z * Dtilda_B[4][1];
               elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 2 ] +=
                  Bi_x * Dtilda_B[0][2] + Bi_y * Dtilda_B[3][2] + Bi_z * Dtilda_B[4][2];

               elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I     ] +=
                  Bi_y * Dtilda_B[1][0] + Bi_x * Dtilda_B[3][0] + Bi_z * Dtilda_B[5][0];
               elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 1 ] +=
                  Bi_y * Dtilda_B[1][1] + Bi_x * Dtilda_B[3][1] + Bi_z * Dtilda_B[5][1];
               elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 2 ] +=
                  Bi_y * Dtilda_B[1][2] + Bi_x * Dtilda_B[3][2] + Bi_z * Dtilda_B[5][2];

               elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I     ] +=
                  Bi_z * Dtilda_B[2][0] + Bi_x * Dtilda_B[4][0] + Bi_y * Dtilda_B[5][0];
               elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I + 1 ] +=
                  Bi_z * Dtilda_B[2][1] + Bi_x * Dtilda_B[4][1] + Bi_y * Dtilda_B[5][1];
               elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I + 2 ] +=
                  Bi_z * Dtilda_B[2][2] + Bi_x * Dtilda_B[4][2] + Bi_y * Dtilda_B[5][2];
            }
         }
      }
   }
}

void _ConstitutiveMatrixCartesian2D_SetValueInAllEntries( void* constitutiveMatrix, double value ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;

   if ( fabs( value ) < 1.0e-20 )
      ConstitutiveMatrix_ZeroMatrix( self );
   else {
      double**            D      = self->matrixData;

      D[0][0] = D[0][1] = D[0][2] = value;
      D[1][0] = D[1][1] = D[1][2] = value;
      D[2][0] = D[2][1] = D[2][2] = value;

      self->isDiagonal = False;
   }
}

void _ConstitutiveMatrixCartesian3D_SetValueInAllEntries( void* _constitutiveMatrix, double value ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*)_constitutiveMatrix;

   if ( fabs( value ) < 1.0e-20 )
      ConstitutiveMatrix_ZeroMatrix( self );
   else {
      double**            D      = self->matrixData;

      D[0][0] = D[0][1] = D[0][2] = D[0][3] = D[0][4] = D[0][5] = value;
      D[1][0] = D[1][1] = D[1][2] = D[1][3] = D[1][4] = D[1][5] = value;
      D[2][0] = D[2][1] = D[2][2] = D[2][3] = D[2][4] = D[2][5] = value;
      D[3][0] = D[3][1] = D[3][2] = D[3][3] = D[3][4] = D[3][5] = value;
      D[4][0] = D[4][1] = D[4][2] = D[4][3] = D[4][4] = D[4][5] = value;
      D[5][0] = D[5][1] = D[5][2] = D[5][3] = D[5][4] = D[5][5] = value;

      self->isDiagonal = False;
   }
}

double _ConstitutiveMatrixCartesian2D_GetIsotropicViscosity( void* constitutiveMatrix ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;

   return self->matrixData[2][2];
}

double _ConstitutiveMatrixCartesian3D_GetIsotropicViscosity( void* constitutiveMatrix ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;

   return self->matrixData[3][3];
}

void _ConstitutiveMatrixCartesian2D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D      = self->matrixData;

   D[0][0] += 2.0 * isotropicCorrection;
   D[1][1] += 2.0 * isotropicCorrection;
   D[2][2] += isotropicCorrection;
}

void _ConstitutiveMatrixCartesian3D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D      = self->matrixData;

   D[0][0] += 2.0 * isotropicCorrection;
   D[1][1] += 2.0 * isotropicCorrection;
   D[2][2] += 2.0 * isotropicCorrection;

   D[3][3] += isotropicCorrection;
   D[4][4] += isotropicCorrection;
   D[5][5] += isotropicCorrection;
}

void _ConstitutiveMatrixCartesian2D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, const XYZ director ) {
   ConstitutiveMatrix* self      = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D         = self->matrixData;
   double              n1        = director[ I_AXIS ];
   double              n2        = director[ J_AXIS ];
   double              a0;
   double              a1;

   a0 = 4.0 * deltaViscosity * n1 * n1 * n2 * n2;
   a1 = 2.0 * deltaViscosity * n1 * n2 * (n2*n2 - n1*n1);

   D[0][0] += -a0 ;  D[0][1] +=  a0 ;  D[0][2] += -a1 ;
   D[1][0] +=  a0 ;  D[1][1] += -a0 ;  D[1][2] +=  a1 ;
   D[2][0] += -a1 ;  D[2][1] +=  a1 ;  D[2][2] +=  a0 - deltaViscosity ;

   self->isDiagonal = False;
}

void _ConstitutiveMatrixCartesian3D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, const XYZ director ) {
   ConstitutiveMatrix* self      = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D         = self->matrixData;
   double              n1        = director[ I_AXIS ];
   double              n2        = director[ J_AXIS ];
   double              n3        = director[ K_AXIS ];
   double              a00,a01,a02,a03,a04,a05;
   double                  a11,a12,a13,a14,a15;
   double                      a22,a23,a24,a25;
   double                          a33,a34,a35;
   double                              a44,a45;
   double                                  a55;

   a00 = -4 * n1*n1 * ( 1 - n1*n1 ) * deltaViscosity;
   a01 =  4 * n1*n1 * n2*n2 * deltaViscosity;
   a02 =  4 * n1*n1 * n3*n3 * deltaViscosity;
   a03 =  2 * n1*n2 * (2*n1*n1-1) * deltaViscosity;
   a04 =  2 * n1*n3 * (2*n1*n1-1) * deltaViscosity;
   a05 =  4 * n1*n1 * n2*n3 * deltaViscosity;

   a11= 4 * n2*n2 * (n2*n2-1) * deltaViscosity;
   a12= 4 * n2*n2 * n3*n3 * deltaViscosity;
   a13= 2 * n1*n2 * (2*n2*n2-1) * deltaViscosity;
   a14= 4 * n1*n2 * n2*n3 * deltaViscosity;
   a15= 2 * n2*n3 * (2*n2*n2-1) * deltaViscosity;

   a22 = 4 * n3*n3 * (n3*n3-1) * deltaViscosity;
   a23 = 4 * n1*n2 * n3*n3 * deltaViscosity;
   a24 = 2 * n1*n3 * (2*n3*n3-1) * deltaViscosity;
   a25 = 2 * n2*n3 * (2*n3*n3-1) * deltaViscosity;

   a33 = (4 * n1*n1 * n2*n2 - n1*n1 - n2*n2) * deltaViscosity;
   a34 = (4 * n1*n1 * n2*n3 - n2*n3) * deltaViscosity;
   a35 = (4 * n1*n2 * n2*n3 - n1*n3) * deltaViscosity;

   a44 = (4 * n1*n1 * n3*n3 - n1*n1 -n3*n3) * deltaViscosity;
   a45 = (4 * n1*n2 * n3*n3 - n1*n2) * deltaViscosity;

   a55 = (4 * n3*n3 * n2*n2 - n3*n3 - n2*n2) * deltaViscosity;

   /* D_{anisotropic} to D */
   D[0][0] += a00 ; D[0][1] += a01 ; D[0][2] += a02 ; D[0][3] += a03 ; D[0][4] += a04 ; D[0][5] += a05 ;
   D[1][0] += a01 ; D[1][1] += a11 ; D[1][2] += a12 ; D[1][3] += a13 ; D[1][4] += a14 ; D[1][5] += a15 ;
   D[2][0] += a02 ; D[2][1] += a12 ; D[2][2] += a22 ; D[2][3] += a23 ; D[2][4] += a24 ; D[2][5] += a25 ;
   D[3][0] += a03 ; D[3][1] += a13 ; D[3][2] += a23 ; D[3][3] += a33 ; D[3][4] += a34 ; D[3][5] += a35 ;
   D[4][0] += a04 ; D[4][1] += a14 ; D[4][2] += a24 ; D[4][3] += a34 ; D[4][4] += a44 ; D[4][5] += a45 ;
   D[5][0] += a05 ; D[5][1] += a15 ; D[5][2] += a25 ; D[5][3] += a35 ; D[5][4] += a45 ; D[5][5] += a55 ;

   self->isDiagonal = False;
}

/*
[B] = [ d/dx,     0  ]
      [    0,  d/dy  ]
      [ d/dy,  d/dx  ]  */
void _ConstitutiveMatrixCartesian2D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B ){
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D    = self->matrixData;
   double              d_dx = GNx[ I_AXIS ][ node_I ];
   double              d_dy = GNx[ J_AXIS ][ node_I ];

   if (self->isDiagonal) {
      D_B[0][0] = D[0][0] * d_dx;
      D_B[0][1] = 0.0;

      D_B[1][0] = 0.0;
      D_B[1][1] = D[1][1] * d_dy;

      D_B[2][0] = D[2][2] * d_dy;
      D_B[2][1] = D[2][2] * d_dx;
   }
   else {
      D_B[0][0] = D[0][0] * d_dx + D[0][2] * d_dy;
      D_B[0][1] = D[0][1] * d_dy + D[0][2] * d_dx;

      D_B[1][0] = D[1][0] * d_dx + D[1][2] * d_dy;
      D_B[1][1] = D[1][1] * d_dy + D[1][2] * d_dx;

      D_B[2][0] = D[2][0] * d_dx + D[2][2] * d_dy;
      D_B[2][1] = D[2][1] * d_dy + D[2][2] * d_dx;
   }
}


/*
[B] = [ d/dx,     0,      0  ]
      [    0,  d/dy,      0  ]
      [    0,     0,   d/dx  ]
      [ d/dy,  d/dx,      0  ]
      [ d/dz,     0,   d/dx  ]
      [    0,  d/dz,   d/dy  ] */
void _ConstitutiveMatrixCartesian3D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B ){
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D    = self->matrixData;
   double              d_dx = GNx[ I_AXIS ][ node_I ];
   double              d_dy = GNx[ J_AXIS ][ node_I ];
   double              d_dz = GNx[ K_AXIS ][ node_I ];

   if (self->isDiagonal) {
      D_B[0][0] = D[0][0] * d_dx;
      D_B[0][1] = 0.0;
      D_B[0][2] = 0.0;

      D_B[1][0] = 0.0;
      D_B[1][1] = D[1][1] * d_dy;
      D_B[1][2] = 0.0;

      D_B[2][0] = 0.0;
      D_B[2][1] = 0.0;
      D_B[2][2] = D[2][2] * d_dz;

      D_B[3][0] = D[3][3] * d_dy;
      D_B[3][1] = D[3][3] * d_dx;
      D_B[3][2] = 0.0;

      D_B[4][0] = D[4][4] * d_dz;
      D_B[4][1] = 0.0;
      D_B[4][2] = D[4][4] * d_dx;

      D_B[5][0] = 0.0;
      D_B[5][1] = D[5][5] * d_dz;
      D_B[5][2] = D[5][5] * d_dy;
   }
   else {
      D_B[0][0] = D[0][0] * d_dx + D[0][3] * d_dy + D[0][4] * d_dz;
      D_B[0][1] = D[0][1] * d_dy + D[0][3] * d_dx + D[0][5] * d_dz;
      D_B[0][2] = D[0][2] * d_dz + D[0][4] * d_dx + D[0][5] * d_dy;

      D_B[1][0] = D[1][0] * d_dx + D[1][3] * d_dy + D[1][4] * d_dz;
      D_B[1][1] = D[1][1] * d_dy + D[1][3] * d_dx + D[1][5] * d_dz;
      D_B[1][2] = D[1][2] * d_dz + D[1][4] * d_dx + D[1][5] * d_dy;

      D_B[2][0] = D[2][0] * d_dx + D[2][3] * d_dy + D[2][4] * d_dz;
      D_B[2][1] = D[2][1] * d_dy + D[2][3] * d_dx + D[2][5] * d_dz;
      D_B[2][2] = D[2][2] * d_dz + D[2][4] * d_dx + D[2][5] * d_dy;

      D_B[3][0] = D[3][0] * d_dx + D[3][3] * d_dy + D[3][4] * d_dz;
      D_B[3][1] = D[3][1] * d_dy + D[3][3] * d_dx + D[3][5] * d_dz;
      D_B[3][2] = D[3][2] * d_dz + D[3][4] * d_dx + D[3][5] * d_dy;

      D_B[4][0] = D[4][0] * d_dx + D[4][3] * d_dy + D[4][4] * d_dz;
      D_B[4][1] = D[4][1] * d_dy + D[4][3] * d_dx + D[4][5] * d_dz;
      D_B[4][2] = D[4][2] * d_dz + D[4][4] * d_dx + D[4][5] * d_dy;

      D_B[5][0] = D[5][0] * d_dx + D[5][3] * d_dy + D[5][4] * d_dz;
      D_B[5][1] = D[5][1] * d_dy + D[5][3] * d_dx + D[5][5] * d_dz;
      D_B[5][2] = D[5][2] * d_dz + D[5][4] * d_dx + D[5][5] * d_dy;
   }
}

void _ConstitutiveMatrixCartesian2D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D    = self->matrixData;

   if (self->isDiagonal) {
      stress[0] = D[0][0] * strainRate[0];
      stress[1] = D[1][1] * strainRate[1];
      stress[2] = D[2][2] * 2.0 * strainRate[2];
   }
   else {
      stress[0] = D[0][0] * strainRate[0] + D[0][1] * strainRate[1] + D[0][2] * 2.0 * strainRate[2];
      stress[1] = D[1][0] * strainRate[0] + D[1][1] * strainRate[1] + D[1][2] * 2.0 * strainRate[2];
      stress[2] = D[2][0] * strainRate[0] + D[2][1] * strainRate[1] + D[2][2] * 2.0 * strainRate[2];
   }
}



void _ConstitutiveMatrixCartesian3D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
   double**            D    = self->matrixData;

   if (self->isDiagonal) {
      stress[0] = D[0][0] * strainRate[0];
      stress[1] = D[1][1] * strainRate[1];
      stress[2] = D[2][2] * strainRate[2];
      stress[3] = D[3][3] * 2.0 * strainRate[3];
      stress[4] = D[4][4] * 2.0 * strainRate[4];
      stress[5] = D[5][5] * 2.0 * strainRate[5];
   }
   else {
      stress[0] = D[0][0] * strainRate[0] + D[0][1] * strainRate[1] + D[0][2] * strainRate[2]
         + 2.0 * (D[0][3] * strainRate[3] + D[0][4] * strainRate[4] + D[0][5] * strainRate[5]);

      stress[1] = D[1][0] * strainRate[0] + D[1][1] * strainRate[1] + D[1][2] * strainRate[2]
         + 2.0 * (D[1][3] * strainRate[3] + D[1][4] * strainRate[4] + D[1][5] * strainRate[5]);

      stress[2] = D[2][0] * strainRate[0] + D[2][1] * strainRate[1] + D[2][2] * strainRate[2]
         + 2.0 * (D[2][3] * strainRate[3] + D[2][4] * strainRate[4] + D[2][5] * strainRate[5]);

      stress[3] = D[3][0] * strainRate[0] + D[3][1] * strainRate[1] + D[3][2] * strainRate[2]
         + 2.0 * (D[3][3] * strainRate[3] + D[3][4] * strainRate[4] + D[3][5] * strainRate[5]);

      stress[4] = D[4][0] * strainRate[0] + D[4][1] * strainRate[1] + D[4][2] * strainRate[2]
         + 2.0 * (D[4][3] * strainRate[3] + D[4][4] * strainRate[4] + D[4][5] * strainRate[5]);

      stress[5] = D[5][0] * strainRate[0] + D[5][1] * strainRate[1] + D[5][2] * strainRate[2]
         + 2.0 * (D[5][3] * strainRate[3] + D[5][4] * strainRate[4] + D[5][5] * strainRate[5]);
   }
}

