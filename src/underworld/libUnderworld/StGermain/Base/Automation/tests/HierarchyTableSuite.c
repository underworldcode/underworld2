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

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "HierarchyTableSuite.h"

typedef struct {
   HierarchyTable*   hTable;
   HierarchyTable*   savedHierarchyTable;
   int					rank;
} HierarchyTableSuiteData;


const Type A_Type = "A";
const Type B_Type = "B";
const Type C_Type = "C";
const Type D_Type = "D";
const Type AA_Type = "AA";
const Type BB_Type = "BB";


void HierarchyTableSuite_Setup( HierarchyTableSuiteData* data ) {
   /* We need to operate on the stgHierarchyTable ptr, since HierarchyTable_New() even uses this */
   data->savedHierarchyTable = stgHierarchyTable;
   stgHierarchyTable = NULL;
   data->hTable = HierarchyTable_New();
   stgHierarchyTable = data->savedHierarchyTable;
   HierarchyTable_RegisterParent( data->hTable, B_Type, A_Type );
   HierarchyTable_RegisterParent( data->hTable, C_Type, B_Type );
   HierarchyTable_RegisterParent( data->hTable, D_Type, C_Type );
   HierarchyTable_RegisterParent( data->hTable, BB_Type, AA_Type );
   MPI_Comm_rank( MPI_COMM_WORLD, &data->rank );
}

void HierarchyTableSuite_Teardown( HierarchyTableSuiteData* data ) {
   Stg_Class_Delete( data->hTable );
}
   

void HierarchyTableSuite_TestIsChild( HierarchyTableSuiteData* data ) {
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, A_Type, A_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, A_Type, B_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, A_Type, C_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, A_Type, D_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, A_Type, AA_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, A_Type, BB_Type ) );

   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, B_Type, A_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, B_Type, B_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, B_Type, C_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, B_Type, D_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, B_Type, AA_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, B_Type, BB_Type ) );
   
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, C_Type, A_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, C_Type, B_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, C_Type, C_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, C_Type, D_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, C_Type, AA_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, C_Type, BB_Type ) ); 

   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, D_Type, A_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, D_Type, B_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, D_Type, C_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, D_Type, D_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, D_Type, AA_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, D_Type, BB_Type ) );

   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, AA_Type, A_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, AA_Type, B_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, AA_Type, C_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, AA_Type, D_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, AA_Type, AA_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, AA_Type, BB_Type ) ); 

   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, BB_Type, A_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, BB_Type, B_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, BB_Type, C_Type ) );
   pcu_check_true( False == HierarchyTable_IsChild( data->hTable, BB_Type, D_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, BB_Type, AA_Type ) );
   pcu_check_true( True == HierarchyTable_IsChild( data->hTable, BB_Type, BB_Type ) ); 
}


void HierarchyTableSuite_TestPrintParents( HierarchyTableSuiteData* data ) {
   Stream*           stream = Journal_Register( Info_Type, (Name)"testStream" );
   const char* const testFilename = "testHTable-PrintParents.txt";

   if (data->rank==0 ) {
      Stream_RedirectFile( stream, testFilename );
   }

   HierarchyTable_PrintParents( data->hTable, A_Type, stream );
   HierarchyTable_PrintParents( data->hTable, B_Type, stream );
   HierarchyTable_PrintParents( data->hTable, C_Type, stream );
   HierarchyTable_PrintParents( data->hTable, D_Type, stream );
   HierarchyTable_PrintParents( data->hTable, AA_Type, stream );
   HierarchyTable_PrintParents( data->hTable, BB_Type, stream );

   /* Just do these print tests with rank 0, to avoid I/O problems */
   if (data->rank==0) {
      char  expectedFilename[PCU_PATH_MAX];

      pcu_filename_expected( testFilename, expectedFilename );
      pcu_check_fileEq( testFilename, expectedFilename );
      remove( testFilename );
   }
}
   

void HierarchyTableSuite_TestPrintChildren( HierarchyTableSuiteData* data ) {
   Stream*     stream = Journal_Register( Info_Type, (Name)"testStream" );
   const char* const testFilename = "testHTable-PrintChildren.txt";

   if (data->rank==0 ) {
      Stream_RedirectFile( stream, testFilename );
   }

   HierarchyTable_PrintChildren( data->hTable, A_Type, stream );
   HierarchyTable_PrintChildren( data->hTable, B_Type, stream );
   HierarchyTable_PrintChildren( data->hTable, C_Type, stream );
   HierarchyTable_PrintChildren( data->hTable, D_Type, stream );
   HierarchyTable_PrintChildren( data->hTable, AA_Type, stream );
   HierarchyTable_PrintChildren( data->hTable, BB_Type, stream );

   /* Just do these print tests with rank 0, to avoid I/O problems */
   if (data->rank==0) {
      char  expectedFilename[PCU_PATH_MAX];

      pcu_filename_expected( testFilename, expectedFilename );
      pcu_check_fileEq( testFilename, expectedFilename );
      remove( testFilename );
   }
}


void HierarchyTableSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, HierarchyTableSuiteData );
   pcu_suite_setFixtures( suite, HierarchyTableSuite_Setup, HierarchyTableSuite_Teardown );
   pcu_suite_addTest( suite, HierarchyTableSuite_TestIsChild );
   pcu_suite_addTest( suite, HierarchyTableSuite_TestPrintParents );
   pcu_suite_addTest( suite, HierarchyTableSuite_TestPrintChildren );
}


