/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "pcu/pcu.h"

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <gLucifer/Base/Base.h>
#include <gLucifer/Base/types.h>
#include <gLucifer/Base/ColourMap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>

typedef struct
{
   Stg_ComponentFactory* cf;
} gLuciferBaseSuiteData;

void gLuciferBaseSuite_Setup( gLuciferBaseSuiteData* data )
{
}

void gLuciferBaseSuite_Teardown( gLuciferBaseSuiteData* data )
{
   stgMainDestroy( data->cf );
}

void gLuciferBaseSuite_ColourMapTestValue(lucColourMap* colourMap, Stream* stream, double val)
{
   lucColour colour;
   lucColourMap_GetColourFromValue(colourMap, val, &colour, -1);
   if (isnan(colour.red) || isnan(colour.green) || isnan(colour.blue) || isnan(colour.opacity) ||
         isinf(colour.red) || isinf(colour.green) || isinf(colour.blue) || isinf(colour.opacity))
      Journal_Printf( stream, "%f\tinvalid\n", val);
   else
      Journal_Printf( stream, "%9.6f %9.6f  %02x %02x %02x %02x\n", val, lucColourMap_ScaleValue(colourMap, val),
                      (int)(255 * colour.red), (int)(255 * colour.green),
                      (int)(255 * colour.blue), (int)(255 * colour.opacity));
}

void gLuciferBaseSuite_TestColourMaps( gLuciferBaseSuiteData* data )
{
   /** Test Definition: */
   Stg_ComponentFactory*	cf;
   DomainContext*				context = NULL;
   Dictionary*					dictionary;
   Name							outputPath;
   Stream*						stream = Journal_Register( Info_Type, (Name)"testColourMaps.xml");
   char							xml_input[PCU_PATH_MAX];
   char expected_file[PCU_PATH_MAX];

   lucColourMap*    colourMap;
   unsigned int testValueCountInRange, testValueCountBelowRange, testValueCountAboveRange;

   Journal_Enable_AllTypedStream( False  );

   /* read in the xml input file */
   pcu_filename_input( "testColourMaps.xml", xml_input );
   data->cf = cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
   stgMainBuildAndInitialise( cf );
   context = (DomainContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context"  );

   dictionary = context->dictionary;
   outputPath = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"outputPath"  );

   Stream_RedirectFile_WithPrependedPath( stream, outputPath, "testColourMaps.dat" );

   testValueCountInRange = Dictionary_GetUnsignedInt_WithDefault( dictionary, "testValueCountInRange", 100 );
   testValueCountBelowRange = Dictionary_GetUnsignedInt_WithDefault( dictionary, "testValueCountBelowRange", 10 );
   testValueCountAboveRange = Dictionary_GetUnsignedInt_WithDefault( dictionary, "testValueCountAboveRange", 10 );

   Journal_Enable_TypedStream( Info_Type, True );

   /* Run tests on all colour maps */
   Index            component_I;
   Stg_ObjectList*  componentList = context->CF->LCRegister->componentList;
   Stg_Component*   component;
   for ( component_I = 0 ; component_I < componentList->count ; component_I++ )
   {
      component = (Stg_Component*) Stg_ObjectList_At( componentList, component_I );

      if ( Stg_Class_IsInstance( component, lucColourMap_Type ) )
      {
         colourMap = (lucColourMap*)component;

         /* Set a sample range for dynamic range rather than using default */
         if (colourMap->dynamicRange && colourMap->minimum == colourMap->maximum)
         {
            if (colourMap->logScale)
               lucColourMap_SetMinMax(colourMap, 1, 10);
            else
               lucColourMap_SetMinMax(colourMap, 0, 1);
         }

         _lucColourMap_Print( colourMap, stream );

         /* Map test values to colours and output */
         Journal_Printf( stream, " Value     Scaled    R  G  B  A\n");
         int i;
         double val, inc;
         inc = (colourMap->maximum - colourMap->minimum) / (double)testValueCountInRange;

         /* Test below range */
         for (i=0, val = colourMap->minimum - inc; i<testValueCountBelowRange; i++, val -= inc)
            gLuciferBaseSuite_ColourMapTestValue(colourMap, stream, val);

         /* Test in range */
         for (val = colourMap->minimum; val <= colourMap->maximum; val += inc)
            gLuciferBaseSuite_ColourMapTestValue(colourMap, stream, val);

         /* Test above range */
         for (i=0, val = colourMap->maximum + inc; i<testValueCountAboveRange; i++, val += inc)
            gLuciferBaseSuite_ColourMapTestValue(colourMap, stream, val);

         /* Testing centreValue */
         if (colourMap->centreOnFixedValue)
            gLuciferBaseSuite_ColourMapTestValue(colourMap, stream, colourMap->centreValue);
      }
   }

   Stream_CloseAndFreeFile( stream );

   pcu_filename_expected( "testColourMaps.expected", expected_file );
   pcu_check_fileEq( "output/testColourMaps.dat", expected_file );

   /* remove generated dat file */
   if ( remove("output/testColourMaps.dat") != 0 ) pcu_assert(0);
   if ( remove("output/input.xml") != 0 ) pcu_assert(0);
}

void gLuciferBaseSuite_TestWindow( gLuciferBaseSuiteData* data )
{
   /** Test Definition: */
   Stg_ComponentFactory*	cf;
   DomainContext*				context = NULL;
   Dictionary*					dictionary;
   Name							outputPath;
   Stream*						stream = Journal_Register( Info_Type, (Name)"testWindow.xml");
   char							xml_input[PCU_PATH_MAX];
   char expected_file[PCU_PATH_MAX];

   Journal_Enable_AllTypedStream( False  );

   /* read in the xml input file */
   pcu_filename_input( "testWindow.xml", xml_input );
   data->cf = cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
   stgMainBuildAndInitialise( cf );
   context = (DomainContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context"  );

   dictionary = context->dictionary;
   outputPath = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"outputPath"  );

   Stream_RedirectFile_WithPrependedPath( stream, outputPath, "testWindow.dat" );
   //shape = (Stg_gLuciferBase*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"shape" );
   //assert( shape  );

   Journal_Enable_TypedStream( Info_Type, True );

   /* Simply print all components - basic xml parsing test only */
   Index            component_I;
   Stg_ObjectList*  componentList = context->CF->LCRegister->componentList;
   Stg_Component*   component;
   for ( component_I = 0 ; component_I < componentList->count ; component_I++ )
   {
      component = (Stg_Component*) Stg_ObjectList_At( componentList, component_I );
      Journal_Printf( stream, "Component %d: %s\n", component_I, component->name );

      if ( Stg_Class_IsInstance( component, lucWindow_Type ) )
      {
         lucWindow* window = (lucWindow*)component;
         Viewport_Index  viewport_I;
         lucViewport*    viewport;

         Journal_Printf(stream, "lucWindow: %s, %d viewports, %d rows, width x height %dx%d\n", 
                        window->name, window->viewportCount, window->viewportLayout[0], window->width, window->height);
         for ( viewport_I = 0 ; viewport_I < window->viewportCount ; viewport_I++ )
         {
            viewport = window->viewportList[ viewport_I ];
            Journal_Printf( stream, "\tViewport %s\n", viewport->name);

            DrawingObject_Index object_I;
            DrawingObject_Index objectCount = lucDrawingObject_Register_GetCount( viewport->drawingObject_Register );
            for ( object_I = 0 ; object_I < objectCount ; object_I++ )
            {
               lucDrawingObject* object = lucDrawingObject_Register_GetByIndex( viewport->drawingObject_Register, object_I );
               Journal_Printf( stream, "\t\tDrawingObject %s\n", object->name);
            }
         }
      }
   }
   Stream_CloseAndFreeFile( stream );

   pcu_filename_expected( "testWindow.expected", expected_file );
   pcu_check_fileEq( "output/testWindow.dat", expected_file );

   /* remove generated dat file */
	if( remove("output/testWindow.dat") != 0 ) { pcu_assert(0); }
   if( remove("output/input.xml") != 0 ) { pcu_assert(0); }

}


void gLuciferBaseSuite( pcu_suite_t* suite )
{
   pcu_suite_setData( suite, gLuciferBaseSuiteData );
   pcu_suite_setFixtures( suite, gLuciferBaseSuite_Setup, gLuciferBaseSuite_Teardown );
   pcu_suite_addTest( suite, gLuciferBaseSuite_TestColourMaps );
   pcu_suite_addTest( suite, gLuciferBaseSuite_TestWindow );
}


