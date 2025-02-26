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
#include <StGermain/libStGermain/src/StGermain.h>
#include "StgDomain/src/StgDomain.h"

typedef struct {
   Stg_ComponentFactory* cf;
} ShapeSuiteData;

void ShapeSuite_Setup( ShapeSuiteData* data ) { 
}

void ShapeSuite_Teardown( ShapeSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void ShapeSuite_GeneratePoints( ShapeSuiteData* data, Dimension_Index dim, char* inputFileName ) {
/** Test Definition: */
   Stg_ComponentFactory* cf;
   DomainContext*        context = NULL;
   Dictionary*           dictionary;
   Stg_Shape*            shape;
   unsigned              testCoordCount, index;
   Name                  outputPath;
   Coord                 coord;
   Stream*               stream = Journal_Register( Info_Type, (Name)inputFileName );
   char                  xml_input[PCU_PATH_MAX];

   Journal_Enable_AllTypedStream( False );

   /* read in the xml input file */
   pcu_filename_input( inputFileName, xml_input );
   data->cf = cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
   stgMainBuildAndInitialise( cf );
   context = (DomainContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context" ); 

   dictionary = context->dictionary;
   outputPath = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"outputPath" );

   Stream_RedirectFile_WithPrependedPath( stream, outputPath, "test.dat" );
   shape = (Stg_Shape*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"shape" );
   assert( shape );

   testCoordCount = Dictionary_GetUnsignedInt_WithDefault( dictionary, "testCoordCount", 10000 );

   Journal_Enable_TypedStream( InfoStream_Type, True );

   /* Test to see if random points are in shape */
   srand48(0);
   for (index = 0 ; index < testCoordCount ; index++ ) {
      coord[ I_AXIS ] = drand48() - 1.0;
      coord[ J_AXIS ] = drand48() - 1.0;
      if ( dim == 3 ) 
         coord[ K_AXIS ] = drand48() - 1.0;

      if ( Stg_Shape_IsCoordInside( shape, coord ) ) 
         Journal_Printf( stream, "%u\n", index );
   }
   Stream_CloseAndFreeFile( stream );
}

void ShapeSuite_TestConvexHull2D( ShapeSuiteData* data ) {
   Dimension_Index dim = 2;
   char            expected_file[PCU_PATH_MAX];

   ShapeSuite_GeneratePoints( data, dim, "testConvexHull2D.xml" );
   pcu_filename_expected( "testConvexHull2D.expected", expected_file );
   pcu_check_fileEq( "output/test.dat", expected_file );
   remove("output/test.dat");
}

void ShapeSuite_TestConvexHull3D( ShapeSuiteData* data ) {
   Dimension_Index dim = 3;
   char            expected_file[PCU_PATH_MAX];

   ShapeSuite_GeneratePoints( data, dim, "testConvexHull3D.xml" );
   pcu_filename_expected( "testConvexHull3D.expected", expected_file );
   pcu_check_fileEq( "output/test.dat", expected_file );
   remove("output/test.dat");
}


void ShapeSuite_TestPolygonShape2D( ShapeSuiteData* data ) {
   Dimension_Index dim = 2;
   char            expected_file[PCU_PATH_MAX];

   ShapeSuite_GeneratePoints( data, dim, "testPolygonShape.xml" );
   pcu_filename_expected( "testPolygonShape2D.expected", expected_file );
   pcu_check_fileEq( "output/test.dat", expected_file );
   remove("output/test.dat");
}


void ShapeSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ShapeSuiteData );
   pcu_suite_setFixtures( suite, ShapeSuite_Setup, ShapeSuite_Teardown );
   pcu_suite_addTest( suite, ShapeSuite_TestConvexHull2D );
   pcu_suite_addTest( suite, ShapeSuite_TestConvexHull3D );
   pcu_suite_addTest( suite, ShapeSuite_TestPolygonShape2D );
}


