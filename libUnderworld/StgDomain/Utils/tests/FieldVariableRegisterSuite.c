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

#include "pcu/pcu.h"
#include <StGermain/StGermain.h> 
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h" 
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "FieldVariableRegisterSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} FieldVariableRegisterSuiteData;

InterpolationResult FieldVariableRegisterSuite_dummyInterpolateValueAt( void* sdVariable, Coord coord, double* value ) { return OUTSIDE_GLOBAL; }
void FieldVariableRegisterSuite_dummyWithinElement( void* sdVariable, Element_DomainIndex dEl_I, Coord coord, double* value ) { return ; }
double FieldVariableRegisterSuite_dummyGetMinGlobalValue( void* sdVariable ) { return 0; }
double FieldVariableRegisterSuite_dummyGetMaxGlobalValue( void* sdVariable ) { return 1; }
void FieldVariableRegisterSuite_dummyGetMinAndMaxLocalCoords( void* sdVariable, Coord min, Coord max ) { return ; }
void FieldVariableRegisterSuite_dummyGetMinAndMaxGlobalCoords( void* sdVariable, Coord min, Coord max ) { return ; }

void FieldVariableRegisterSuite_Setup( FieldVariableRegisterSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void FieldVariableRegisterSuite_Teardown( FieldVariableRegisterSuiteData* data ) {
} 

void FieldVariableRegisterSuite_TestGetByIndex( FieldVariableRegisterSuiteData* data ) {
   int                     procToWatch;
   FieldVariable_Register* fV_Register;
   FieldVariable*          testFVs[] = { NULL, NULL, NULL };
   Name                    fvNames[] = { "testFV1", "testFV2", "testFV3" };
   Index                   ii;
   Index                   fV_Index;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {
      fV_Register = FieldVariable_Register_New();

      for( ii=0; ii < 3; ii++ ) {
         testFVs[ii] = FieldVariable_New(
            fvNames[ii],
            NULL,
            0,
            3,
            False,
            data->comm,
            fV_Register );
         testFVs[ii]->_interpolateValueAt = FieldVariableRegisterSuite_dummyInterpolateValueAt;
         testFVs[ii]->_getMinGlobalFieldMagnitude = FieldVariableRegisterSuite_dummyGetMinGlobalValue;
         testFVs[ii]->_getMaxGlobalFieldMagnitude = FieldVariableRegisterSuite_dummyGetMaxGlobalValue;
         testFVs[ii]->_getMinAndMaxLocalCoords = FieldVariableRegisterSuite_dummyGetMinAndMaxLocalCoords;
         testFVs[ii]->_getMinAndMaxGlobalCoords = FieldVariableRegisterSuite_dummyGetMinAndMaxGlobalCoords;

      }
      for( ii=0; ii < 3; ii++ ) {
         fV_Index = FieldVariable_Register_GetIndex( fV_Register, fvNames[ii] );
         pcu_check_true( fV_Index == ii );
         pcu_check_streq( (FieldVariable_Register_GetByIndex( fV_Register, fV_Index ) )->name, fvNames[ii] );
      }
      Stg_Class_Delete(fV_Register);
   }
}

void FieldVariableRegisterSuite_TestGetByName( FieldVariableRegisterSuiteData* data ) {
   int                     procToWatch;
   FieldVariable_Register* fV_Register;
   FieldVariable*          testFVs[] = { NULL, NULL, NULL };
   Name                    fvNames[] = { "testFV1", "testFV2", "testFV3" };
   Index                   ii;
   Index                   fV_Index;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {
      fV_Register = FieldVariable_Register_New();

      for( ii=0; ii < 3; ii++ ) {
         testFVs[ii] = FieldVariable_New(
            fvNames[ii],
            NULL,
            0,
            3,
            False,
            data->comm,
            fV_Register );
         testFVs[ii]->_interpolateValueAt = FieldVariableRegisterSuite_dummyInterpolateValueAt;
         testFVs[ii]->_getMinGlobalFieldMagnitude = FieldVariableRegisterSuite_dummyGetMinGlobalValue;
         testFVs[ii]->_getMaxGlobalFieldMagnitude = FieldVariableRegisterSuite_dummyGetMaxGlobalValue;
         testFVs[ii]->_getMinAndMaxLocalCoords = FieldVariableRegisterSuite_dummyGetMinAndMaxLocalCoords;
         testFVs[ii]->_getMinAndMaxGlobalCoords = FieldVariableRegisterSuite_dummyGetMinAndMaxGlobalCoords;
      }
      for( ii=0; ii < 3; ii++ ) {
         fV_Index = FieldVariable_Register_GetIndex( fV_Register, fvNames[ii] );
         pcu_check_true( fV_Index == ii );
         pcu_check_streq( (FieldVariable_Register_GetByName( fV_Register, fvNames[ii] ) )->name, fvNames[ii] );
      }
      Stg_Class_Delete(fV_Register);
   }
}

void FieldVariableRegisterSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, FieldVariableRegisterSuiteData );
   pcu_suite_setFixtures( suite, FieldVariableRegisterSuite_Setup, FieldVariableRegisterSuite_Teardown );
   pcu_suite_addTest( suite, FieldVariableRegisterSuite_TestGetByIndex );
   pcu_suite_addTest( suite, FieldVariableRegisterSuite_TestGetByName );
}


