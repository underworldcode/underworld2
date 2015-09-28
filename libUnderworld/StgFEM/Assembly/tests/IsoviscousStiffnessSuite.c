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
#include <string.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

/* silly stgermain, I must define this */
#define CURR_MODULE_NAME "StgFEM.c"

typedef struct {
   FiniteElementContext* context;
} IsoviscousStiffnessData;

void IsoviscousStiffness_Setup( IsoviscousStiffnessData* data ) { 
   Journal_Enable_AllTypedStream( False );
   data->context=NULL;
}

void IsoviscousStiffness_Teardown( IsoviscousStiffnessData* data ) {
}

void IsoviscousStiffness2D( IsoviscousStiffnessData* data ) {
   StiffnessMatrix*      stiffnessMatrix;
   Dictionary*           dictionary;
   FiniteElementContext* context;
   Stg_ComponentFactory* cf;
   PetscViewer           expViewer;
   PetscReal             matrixNorm, errorNorm, test;
   Mat                   expected;
   char                  expected_file[PCU_PATH_MAX];
   char                  *filename, *matrixName;
   double                tolerance;
   char                  xml_input[PCU_PATH_MAX];
   Stream*               infoStream = Journal_Register( Info_Type, (Name)CURR_MODULE_NAME );
   char                  rFile[PCU_PATH_MAX];
   int                   err;

   pcu_docstring( "This test compares a Stiffness matrix against a previously generated stiffness matrix"
      "The stiffness matrix is generated from a 2D FEM model for an isoviscous fluid flow." 
      "See testIsoviscous.xml for the actual xml used" );

   /* read in the xml input file */
   pcu_filename_input( "IsoviscousStiffnessMatrix.xml", xml_input );
   cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
   context = (FiniteElementContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context" );
   data->context = context;
   dictionary = context->dictionary;

   stgMainBuildAndInitialise( cf );

   /* Test is to check the relative error between an
       1 ) expected stiffness matrix, (made years ago)
       2) the current stiffness matrix.

       both matricies are built using only an Arrhenius rheology 
    */

   /* get the tolerance */
   tolerance = Dictionary_GetDouble( dictionary, "StiffnessMatrixCompareTolerance" );

   /* Get Matrix */
   matrixName = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"CompareStiffnessMatrix"  );
   Journal_Printf( infoStream, "Comparing stiffness matrix '%s'\n", matrixName );
   stiffnessMatrix = (StiffnessMatrix*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)matrixName );
   assert( stiffnessMatrix  );

   StiffnessMatrix_Assemble( stiffnessMatrix, False, NULL, context );

   /* Get Stored Matrix from file */
   filename = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"StiffnessMatrixCompareFilename"  );
   Journal_Printf( infoStream, "Checking with file '%s'\n", filename );

   pcu_filename_expected( filename, expected_file );
   PetscViewerBinaryOpen( context->communicator, expected_file, FILE_MODE_READ, &expViewer );

   Stg_MatLoad( expViewer, MATAIJ, &expected );

   MatNorm( expected, NORM_FROBENIUS, &matrixNorm );
   assert( matrixNorm != 0 );

   MatAXPY( expected, -1, (stiffnessMatrix->matrix) , DIFFERENT_NONZERO_PATTERN );
   MatNorm( expected, NORM_FROBENIUS, &errorNorm );
   test = errorNorm / matrixNorm;

   pcu_check_lt( test, tolerance );

   /* Check tolerance */
   /*
   stream = Journal_Register( Info_Type, (Name)"StiffnessMatrixComparison"  );
   Stream_RedirectFile_WithPrependedPath( stream, context->outputPath, "StiffnessMatrixCompare.dat" );
   Journal_PrintValue( infoStream, tolerance );
   Journal_Printf( stream, "Comparison between stiffness matrix '%s' %s with tolerance %4g.\n", 
         matrixName, 
         ( errorNorm/matrixNorm < tolerance ? "passed" : "failed" ),
         tolerance );
         */

   /* 
   Stream_CloseFile( stream );
       To view the expected and computed matricies uncomment this
   PetscViewerASCIIOpen(context->communicator, "numerical.dat",&currViewer);
   PetscViewerASCIIOpen(context->communicator, "expected.dat",&parallelViewer);
   MatView( stiffnessMatrix->matrix, currViewer ); //PETSC_VIEWER_STDOUT_WORLD );
   MatView( expected, parallelViewer ); //PETSC_VIEWER_STDOUT_WORLD );
   Stg_PetscViewerDestroy(&currViewer);
   Stg_PetscViewerDestroy(&parallelViewer);
   */
   if( data->context->rank == 0 ) {
      /* Now clean output path */
      sprintf(rFile, "%s/input.xml", data->context->outputPath );
      err = remove( rFile );
      if( err == -1 ) printf("Error in %s, can't delete the input.xml\n", __func__);
   }
   stgMainDestroy( cf );
}

void IsoviscousStiffnessSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, IsoviscousStiffnessData );
   pcu_suite_setFixtures( suite, IsoviscousStiffness_Setup, IsoviscousStiffness_Teardown );
   pcu_suite_addTest( suite, IsoviscousStiffness2D );
}
