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

#include "types.h"
#include "ConstitutiveMatrix.h"

#include <assert.h>
#include <string.h>

const Type ConstitutiveMatrix_Type = "ConstitutiveMatrix";

ConstitutiveMatrix* _ConstitutiveMatrix_New(  CONSTITUTIVEMATRIX_DEFARGS  )
{
   ConstitutiveMatrix*  self;

   assert( _sizeOfSelf >= sizeof(ConstitutiveMatrix) );

   /* General info */
   self = (ConstitutiveMatrix*)_StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

   /* Virtual functions */
   self->_setValue            = _setValue;
   self->_getViscosity        = _getViscosity;
   self->_isotropicCorrection = _isotropicCorrection;
   self->_setSecondViscosity  = _setSecondViscosity;
   self->_assemble_D_B        = _assemble_D_B;
   self->_calculateStress     = _calculateStress;

   return self;
}

void _ConstitutiveMatrix_Init(
      ConstitutiveMatrix*                     self,
      Dimension_Index                        dim,
      Bool                                   storeConstitutiveMatrix,
      Bool                                   restartUsingPreviousStokesSol )
{
   /* General and Function pointers for this class that are not on the parent class should be set here should already be set */

   /* ConstitutiveMatrix info */
   self->storeConstitutiveMatrix = storeConstitutiveMatrix;
   self->restartUsingPreviousStokesSol = restartUsingPreviousStokesSol;

   self->matrixData = NULL;
   self->dim        = dim;
   self->isSwarmTypeIntegrationPointsSwarm = Stg_Class_IsInstance( self->integrationSwarm, IntegrationPointsSwarm_Type );
   Journal_Firewall(
      self->isSwarmTypeIntegrationPointsSwarm,
      NULL,
      "Error In %s - ConstitutiveMatrix %s cannot use %s. An instance of IntegrationPointsSwarm is required.\n",
      __func__,
      self->name,
      self->integrationSwarm->name );
   
   self->isDiagonal          = False;
   self->columnSize          = 0;
   self->rowSize             = 0;

   /* If we are restarting, there will be an existing valid solution for the velocity, pressure
   etc fields - thus we record this so any yield rheologies will behave correctly */
   if ( self->context && True == self->context->loadFieldsFromCheckpoint && restartUsingPreviousStokesSol == True ) {
      self->previousSolutionExists = True;
   }
   else {
      /* Otherwise, we don't want to set this as true till we've done at least one iteration of the
      first solve */
      self->previousSolutionExists = False;
   }

   self->sle = NULL;
   self->sleNonLinearIteration_I = 0;

}

void _ConstitutiveMatrix_Delete( void* constitutiveMatrix ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*)constitutiveMatrix;

   Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );

   /* Stg_Class_Delete parent*/
   _StiffnessMatrixTerm_Delete( self );

}

void _ConstitutiveMatrix_Print( void* constitutiveMatrix, Stream* stream ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*)constitutiveMatrix;

   /* General info */
   Journal_PrintPointer( stream, constitutiveMatrix );
   Stream_Indent( stream );

   /* Print parent */
   _StiffnessMatrixTerm_Print( self, stream );

   /* Function pointers for this class that are not on the parent class should be set here */
   Journal_PrintPointer( stream, self->_setValue );
   Journal_PrintPointer( stream, self->_getViscosity );
   Journal_PrintPointer( stream, self->_isotropicCorrection );
   Journal_PrintPointer( stream, self->_setSecondViscosity );
   Journal_PrintPointer( stream, self->_assemble_D_B );
   Journal_PrintPointer( stream, self->_calculateStress );

   /* Regular Info */
   Journal_PrintPointer( stream, self->debug );
   ConstitutiveMatrix_PrintContents( self, stream );

   Journal_PrintBool( stream, self->isDiagonal );
   Journal_PrintValue( stream, self->dim );
   Journal_PrintValue( stream, self->columnSize );
   Journal_PrintValue( stream, self->rowSize );
   Journal_PrintBool( stream, self->previousSolutionExists );

   Stream_UnIndent( stream );
}


void* _ConstitutiveMatrix_Copy( void* constitutiveMatrix, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   ConstitutiveMatrix*  self = (ConstitutiveMatrix*)constitutiveMatrix;
   ConstitutiveMatrix*  newConstitutiveMatrix;

   /* TODO */ abort();
   if (deep) {
      newConstitutiveMatrix->matrixData = Memory_Alloc_2DArray( double, self->columnSize, self->rowSize, (Name)self->name );
   }
   return (void* )newConstitutiveMatrix;
}

void _ConstitutiveMatrix_AssignFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) {
   ConstitutiveMatrix*	self = (ConstitutiveMatrix*)constitutiveMatrix;
   Dimension_Index		dim;
   Bool						storeConstitutiveMatrix, restartUsingPreviousStokesSol;

   _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

   dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
   dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim  );

   restartUsingPreviousStokesSol = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"restartUsingPreviousStokesSol", True  );

   storeConstitutiveMatrix = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"storeConstitutiveMatrix", False  );

   _ConstitutiveMatrix_Init( self, dim, storeConstitutiveMatrix, restartUsingPreviousStokesSol );
}

void _ConstitutiveMatrix_Build( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*)constitutiveMatrix;
   Stream*             errorStream = Journal_Register( Error_Type, (Name)self->type  );

   _StiffnessMatrixTerm_Build( self, data );

   Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );

   self->matrixData = Memory_Alloc_2DArray( double, self->columnSize, self->rowSize, (Name)self->name  );

}


void _ConstitutiveMatrix_Initialise( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrix* self          = (ConstitutiveMatrix*)constitutiveMatrix;

   Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );


   _StiffnessMatrixTerm_Initialise( self, data );

   ConstitutiveMatrix_ZeroMatrix( self ) ;
}


void _ConstitutiveMatrix_Execute( void* constitutiveMatrix, void* data ) {
   _StiffnessMatrixTerm_Execute( constitutiveMatrix, data );
}

void _ConstitutiveMatrix_Destroy( void* constitutiveMatrix, void* data ) {
   ConstitutiveMatrix* self          = (ConstitutiveMatrix*)constitutiveMatrix;

   Memory_Free( self->matrixData );

   _StiffnessMatrixTerm_Destroy( self, data );
}

/* +++ Private Functions +++ */

/* +++ Public Functions +++ */

void ConstitutiveMatrix_Assemble(
      void*                                              constitutiveMatrix,
      Element_LocalIndex                                 lElement_I,
      int                                                particleIndex,
      IntegrationPoint*                                  particle )
{
    assert(0);
}

void ConstitutiveMatrix_ZeroMatrix( void* constitutiveMatrix ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*)constitutiveMatrix;

   memset( self->matrixData[0], 0, (self->columnSize * self->rowSize)*sizeof(double) );
        memset( self->derivs, 0, 3 * 3 * sizeof(double) );
}

void ConstitutiveMatrix_SetIsotropicViscosity( void* constitutiveMatrix, double viscosity ) {
   ConstitutiveMatrix* self = (ConstitutiveMatrix*)constitutiveMatrix;

   ConstitutiveMatrix_ZeroMatrix( self );
   ConstitutiveMatrix_IsotropicCorrection( self, viscosity );
}

void ConstitutiveMatrix_MultiplyByValue( void* constitutiveMatrix, double factor ) {
   ConstitutiveMatrix* self       = (ConstitutiveMatrix*)constitutiveMatrix;
   Index               row_I;
   Index               col_I;
   Index               columnSize = self->columnSize;
   Index               rowSize    = self->rowSize;
   double*             columnValue;

   for ( col_I = 0 ; col_I < columnSize ; col_I++ ) {
      columnValue = self->matrixData[ col_I ];

      for ( row_I = 0 ; row_I < rowSize ; row_I++ ) {
         columnValue[ row_I ] *= factor;
      }

   }
}

void ConstitutiveMatrix_PrintContents( void* constitutiveMatrix, Stream* stream ) {
   ConstitutiveMatrix* self   = (ConstitutiveMatrix*)constitutiveMatrix;
   Index               row_I;
   Index               col_I;

   for ( col_I = 0 ; col_I < self->columnSize ; col_I++ ) {
      for ( row_I = 0 ; row_I < self->rowSize ; row_I++ ) {
         Journal_Printf( stream, "matrixData[ %u ][ %u ] = %.4g; \t", col_I, row_I, self->matrixData[ col_I ][ row_I ] );
      }
      Journal_Printf( stream, "\n" );
   }
}
