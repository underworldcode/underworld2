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

#include <gLucifer/Base/Base.h>

#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
#include "ContourCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucContourCrossSection_Type = "lucContourCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucContourCrossSection* _lucContourCrossSection_New(  LUCCONTOURCROSSSECTION_DEFARGS  )
{
   lucContourCrossSection*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucContourCrossSection) );
   self = (lucContourCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucContourCrossSection_Init(
   lucContourCrossSection*      self,
   Bool                         showValues,
   Bool                         printUnits,
   double                       minIsovalue,
   double                       maxIsovalue,
   double                       interval )
{
   self->showValues = showValues;
   self->printUnits = printUnits;
   self->interval = interval;
   self->minIsovalue = minIsovalue;
   self->maxIsovalue = maxIsovalue;
}

void _lucContourCrossSection_Delete( void* drawingObject )
{
   lucContourCrossSection*  self = (lucContourCrossSection*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucContourCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucContourCrossSection*  self = (lucContourCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucContourCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucContourCrossSection);
   Type                                                             type = lucContourCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucContourCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucContourCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucContourCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucContourCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucContourCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucContourCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucContourCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucContourCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucContourCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucContourCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucContourCrossSection_New(  LUCCONTOURCROSSSECTION_PASSARGS  );
}

void _lucContourCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucContourCrossSection*      self = (lucContourCrossSection*)drawingObject;

   /* Construct Parent */
   self->defaultResolution = 8;   /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );

   _lucContourCrossSection_Init(
      self,
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"showValues", True  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"printUnits", False  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minIsovalue", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxIsovalue", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"interval", 0.33 )  ) ;

   /* Drawing settings for this component */
   //TODO: Set via python properties
   //self->lit = False;
}

void _lucContourCrossSection_Build( void* drawingObject, void* data ) 
{
   lucScalarFieldCrossSection*     self        = (lucScalarFieldCrossSection*)drawingObject;

   /* Build field variable in parent */
   _lucCrossSection_Build(self, data);
}

void _lucContourCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucContourCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucContourCrossSection_Destroy( void* drawingObject, void* data ) {} 

void _lucContourCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   _lucCrossSection_Setup(drawingObject, database, _context);
}

void _lucContourCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucContourCrossSection* self = (lucContourCrossSection*)drawingObject;

   self->printedIndex = -1;
   self->coordIndex = 0;
   lucContourCrossSection_DrawCrossSection(self, database);
}

void lucContourCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database )
{
   lucContourCrossSection* self = (lucContourCrossSection*)drawingObject;
   double                  minIsovalue = self->minIsovalue;
   double                  maxIsovalue = self->maxIsovalue;
   double                  isovalue;

   /* Custom max, min, use global min/max if equal (defaults to both zero) */
   if (self->minIsovalue == self->maxIsovalue)
   {
      //minIsovalue = FieldVariable_GetMinGlobalFieldMagnitude(self->fieldVariable);
      //maxIsovalue = FieldVariable_GetMaxGlobalFieldMagnitude(self->fieldVariable);
   }

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, False);

   if (database->rank == 0)
   {
      /* Draw isovalues at interval */
      if ( self->interval <= 0.0 ) return;

      for ( isovalue = minIsovalue ; isovalue <= maxIsovalue ; isovalue += self->interval )
      {
         if ( self->colourMap )
            lucColourMap_SetColourFromValue( self->colourMap, isovalue, self->opacity);

         lucContourCrossSection_DrawContour( self, database, isovalue);
         self->coordIndex++;
      }
   }

   /* Free memory */
   lucCrossSection_FreeSampleData(self);
}

#define LEFT   0
#define RIGHT  1
#define BOTTOM 2
#define TOP    3

void lucContourCrossSection_DrawContour(void* drawingObject, lucDatabase* database, double isovalue)
{
   lucContourCrossSection* self = (lucContourCrossSection*)drawingObject;
   unsigned int elementType;
   unsigned int i, j;

   /* Start marching rectangles */
   //glBegin(GL_LINES);

   for ( i = 0 ; i < self->resolutionA-1; i++ )
   {
      for ( j = 0 ; j < self->resolutionB-1; j++ )
      {
         /* Assign a unique number to the square type from 0 to 15 */
         elementType = 0;
         if (self->values[i][j][0]     > isovalue) 	elementType += 1;
         if (self->values[i+1][j][0]   > isovalue) 	elementType += 2;
         if (self->values[i][j+1][0]   > isovalue) 	elementType += 4;
         if (self->values[i+1][j+1][0] > isovalue) 	elementType += 8;

         switch ( elementType )
         {
         case 0:
            /*  @@  */
            /*  @@  */
            break;
         case 1:
            /*  @@  */
            /*  #@  */
            lucContourCrossSection_PlotPoint(self, database, LEFT,   isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 2:
            /*  @@  */
            /*  @#  */
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 3:
            /*  @@  */
            /*  ##  */
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            break;
         case 4:
            /*  #@  */
            /*  @@  */
            lucContourCrossSection_PlotPoint(self, database, LEFT  , isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, TOP   , isovalue, i, j);
            break;
         case 5:
            /*  #@  */
            /*  #@  */
            lucContourCrossSection_PlotPoint(self, database, TOP   , isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 6:
            /*  #@  */
            /*  @#  */
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, TOP , isovalue, i, j);

            lucContourCrossSection_PlotPoint(self, database, RIGHT , isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 7:
            /*  #@  */
            /*  ##  */
            lucContourCrossSection_PlotPoint(self, database, TOP, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            break;
         case 8:
            /*  @#  */
            /*  @@  */
            lucContourCrossSection_PlotPoint(self, database, TOP, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            break;
         case 9:
            /*  @#  */
            /*  #@  */
            lucContourCrossSection_PlotPoint(self, database, TOP, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);

            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            break;
         case 10:
            /*  @#  */
            /*  @#  */
            lucContourCrossSection_PlotPoint(self, database, TOP, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 11:
            /*  @#  */
            /*  ##  */
            lucContourCrossSection_PlotPoint(self, database, TOP, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            break;
         case 12:
            /*  ##  */
            /*  @@  */
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            break;
         case 13:
            /*  ##  */
            /*  #@  */
            lucContourCrossSection_PlotPoint(self, database, RIGHT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 14:
            /*  ##  */
            /*  @#  */
            lucContourCrossSection_PlotPoint(self, database, LEFT, isovalue, i, j);
            lucContourCrossSection_PlotPoint(self, database, BOTTOM, isovalue, i, j);
            break;
         case 15:
            /*  ##  */
            /*  ##  */
            break;
         default:
            Journal_Printf( lucError, "In func %s: Cannot find element %d.\n", __func__, elementType );
            abort();
         }

      }
   }
   //glEnd();
}

void lucContourCrossSection_PlotPoint(lucContourCrossSection* self, lucDatabase* database, char edge, double isovalue, int aIndex, int bIndex)
{
   Coord vertex;
   double aPos = (double)aIndex;
   double bPos = (double)bIndex;
   double leftBtm = self->values[aIndex][bIndex][0];
   double rightBtm = self->values[aIndex+1][bIndex][0];
   double leftTop = self->values[aIndex][bIndex+1][0];
   double rightTop = self->values[aIndex+1][bIndex+1][0];

   switch (edge)
   {
   case BOTTOM:
      aPos += (isovalue - leftBtm)/(rightBtm - leftBtm);
      break;
   case TOP:
      aPos += (isovalue - leftTop)/(rightTop - leftTop);
      bPos += 1.0;
      break;
   case LEFT:
      bPos += (isovalue - leftBtm)/(leftTop - leftBtm);
      break;
   case RIGHT:
      aPos += 1.0;
      bPos += (isovalue - rightBtm)/(rightTop - rightBtm);
      break;
   }

   lucCrossSection_Interpolate2d(self, aPos / (double)(self->resolutionA-1), bPos / (double)(self->resolutionB-1), vertex);

   /* Dump vertex pos */
   float pos[3] = {vertex[0], vertex[1], vertex[2]};
   float value = isovalue;
   lucDatabase_AddVertices(database, 1, lucLineType, pos);
   if (self->colourMap)
      lucDatabase_AddValues(database, 1, lucLineType, lucColourValueData, self->colourMap, &value);

   if (self->showValues && self->coordIndex % 4 == edge) 
   { 
      if (self->printedIndex < self->coordIndex && (0 == bIndex || 0 == aIndex || bIndex == (self->resolutionB-1) || aIndex == (self->resolutionA-1)))
      {
         char label[32];
         double dimCoeff = 1.0; /* coefficient for dimensionalising field */
         //TODO: Fix scaling/units
//         /* very hacky to get the scaling component */
//         Scaling* theScaling = NULL;
//         if (self->fieldVariable->context) theScaling = self->fieldVariable->context->scaling;
//         if (self->fieldVariable->o_units && theScaling)
//            dimCoeff = Scaling_ParseDimCoeff( theScaling, self->fieldVariable->o_units );
//
         /* Add the vertex for the label as a point */
         lucDatabase_AddVertices(database, 1, lucPointType, pos);
         /* Add to the label data */
         sprintf(label, " %g", isovalue * dimCoeff);
         //sprintf(label, " %g%s", isovalue * dimCoeff,
         //         self->printUnits && self->fieldVariable->o_units ? self->fieldVariable->o_units : "");
         lucDatabase_AddLabel(database, lucPointType, label);

         self->printedIndex = self->coordIndex;
      }
   }
}


