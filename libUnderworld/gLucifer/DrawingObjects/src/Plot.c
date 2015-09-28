/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <float.h>
#include <string.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include <gLucifer/Base/Base.h>


#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
#include "Plot.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucPlot_Type = "lucPlot";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucPlot* _lucPlot_New(  LUCPLOT_DEFARGS  )
{
   lucPlot*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucPlot) );
   self = (lucPlot*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucPlot_Init(
   lucPlot*        self,
   char*           dataFileName,
   char*           colourMapFileName,
   unsigned int    columnX,
   unsigned int    columnY,
   unsigned int    columnZ,
   unsigned int    skipRows,
   double          coordZ,
   char*           labelX,
   char*           labelY,
   char*           labelZ,
   Bool            lines,
   Bool            points,
   Bool            bars,
   unsigned int    ticksX,
   unsigned int    ticksY,
   unsigned int    ticksZ,               
   Bool            flipY,
   Bool            setX,
   Bool            setY,
   Bool            setZ,            
   double         minX,
   double           minY,
   double           minZ,
   double         maxX,
   double           maxY,
   double           maxZ,            
   unsigned int    pointSize,
   Bool            colourMapFromFile)
{
   /* Default output path */
   if (self->context)
      self->outputPath = self->context->outputPath;
   else
      self->outputPath = StG_Strdup(".");

   /* Check input file for leading slash or ./ */
   if (dataFileName && ((dataFileName[0] != '/') || (dataFileName[0] != '.')))
      Stg_asprintf(&self->dataFileName, "%s/%s", self->outputPath, dataFileName);
   else
      self->dataFileName = StG_Strdup(dataFileName);
   /* Check colourMap file for leading slash or ./ */
   if (colourMapFileName && ((colourMapFileName[0] != '/') || (colourMapFileName[0] != '.')))
      Stg_asprintf(&self->colourMapFileName, "%s/%s", self->outputPath, colourMapFileName);
   else
      self->colourMapFileName = StG_Strdup(colourMapFileName);

   //printf("colourMapFileName is %s dataFileName is %s", self->colourMapFileName, self->dataFileName);
   self->columnX = columnX;
   self->columnY = columnY;
   self->columnZ = columnZ;   
   self->skipRows = skipRows;
   self->coordZ = coordZ;
   self->labelX = labelX;
   self->labelY = labelY;
   self->labelZ = labelZ;
   self->lines = lines;
   self->points = points;
   self->bars = bars;
   self->ticksX = ticksX;
   self->ticksY = ticksY;
   self->ticksZ = ticksZ;
   self->flipY = flipY;
   self->setX = setX;
   self->setY = setY;
   self->setZ = setZ;
   self->minX = minX;
   self->minY = minY;
   self->minZ = minZ;
   self->maxX = maxX;
   self->maxY = maxY;
   self->maxZ = maxZ;
   self->pointSize = pointSize;
   self->colourMapFromFile = colourMapFromFile;
   self->dataX = self->dataY = self->dataZ = NULL;
   self->rows = 0;

   /* Append to property string */
   lucDrawingObject_AppendProps(self, "pointSmooth=0\npointSize=%d\nshape=1\nlength=0\nwidth=1\nheight=1\n", pointSize); 

}

void _lucPlot_Delete( void* drawingObject )
{
   lucPlot*  self = (lucPlot*)drawingObject;

   _lucDrawingObject_Delete( self );
}

void _lucPlot_Print( void* drawingObject, Stream* stream )
{
   lucPlot*  self = (lucPlot*)drawingObject;
   _lucDrawingObject_Print( self, stream );
}

void* _lucPlot_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucPlot);
   Type                                                             type = lucPlot_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucPlot_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucPlot_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucPlot_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucPlot_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucPlot_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucPlot_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucPlot_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucPlot_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucPlot_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucPlot_New(  LUCPLOT_PASSARGS  );
}

void _lucPlot_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucPlot*     self = (lucPlot*)drawingObject;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   _lucPlot_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"dataFileName", NULL),
     Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colourMapFileName", NULL),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"columnX", 1 ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"columnY", 2 ),
     Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"columnZ", 0 ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"skipRows", 0),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"coordZ", 0.01 ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelX", ""),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelY", ""),
     Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelZ", ""),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"lines", True),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"points", False),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"bars", False),
     Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"ticksX", 1 ),
     Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"ticksY", 1 ),
     Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"ticksZ", 1 ),
     Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"flipY", False),
     Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"setX", False),
     Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"setY", False),
     Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"setZ", False),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minX", 0.0),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minY", 0.0),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minZ", 0.0),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxX", 1.0),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxY", 1.0),
     Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxZ", 1.0),
     Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"pointSize", 5 ),
     Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"colourMapFromFile", False)          
   );

   /* Drawing settings for this component */
   self->lit = False;
   
}

void _lucPlot_Build( void* drawingObject, void* data ) {}

void _lucPlot_Initialise( void* drawingObject, void* data ) {}
void _lucPlot_Execute( void* drawingObject, void* data ) {}

void _lucPlot_Destroy( void* drawingObject, void* data )
{
   lucPlot* self    = (lucPlot*)drawingObject;
   if (self->dataFileName) Memory_Free(self->dataFileName);
   if (self->colourMapFileName) Memory_Free(self->colourMapFileName);
   if (self->dataX) Memory_Free(self->dataX);
   if (self->dataY) Memory_Free(self->dataY);
   if (self->dataZ) Memory_Free(self->dataZ);
}

void lucPlot_ReadDataFile( void* drawingObject )
{
   lucPlot* self = (lucPlot*)drawingObject;
   FILE* fp;
   double data[100];
   int i;
   char temp[1024];
   if (self->rank > 0) return;

   /* Read input file data */
   fp = fopen( self->dataFileName, "r" );
   if (!fp)
   {
      Journal_Printf( lucError, "Error in func '%s' - file not found %s.\n", __func__, self->dataFileName);
      return;
   }

   for ( i=0; i < self->skipRows; i++)
      fgets(temp, 1024, fp);  

   self->rows = 0;
   while (fgets(temp, 1024, fp))
   {
      char * tok;
      int count = 0;
      tok = strtok(temp," ,\t");
      while (tok != NULL)
      {
         data[count] = atof(tok);
         tok = strtok(NULL, " ,\t");
         count++;
         if (count==100) break;
      }

      /*if (self->columnX) printf(" [X] %f", data[self->columnX-1]);
      if (self->columnY) printf(" [Y] %f", data[self->columnY-1]);
      if (self->columnZ) printf(" [Z] %f", data[self->columnZ-1]); 
      printf("\n");*/

      self->rows++;
      if (self->columnX)
      {
         self->dataX = Memory_Realloc_Array( self->dataX, double, self->rows );
         self->dataX[self->rows-1] = data[self->columnX-1];
      }
      if (self->columnY)
      {
         self->dataY = Memory_Realloc_Array( self->dataY, double, self->rows );
         self->dataY[self->rows-1] = data[self->columnY-1];
      }
      if (self->columnZ)
      {
         self->dataZ = Memory_Realloc_Array( self->dataZ, double, self->rows );
         self->dataZ[self->rows-1] = data[self->columnZ-1];
      }
   }
   fclose( fp );
}

#define swap_double(A,B) \
   { \
      double temp = A;  \
      A = B;            \
      B = temp;         \
   }

#define lineVertex(x,y,z) \
         {  \
            float _vert[3] = {x,y,z};  \
            lucDatabase_AddVertices(database, 1, lucLineType, _vert); \
         }

#define labelVertex(x,y,z,label) \
         {  \
            float _vert[3] = {x,y,z};  \
            lucDatabase_AddVertices(database, 1, lucLabelType, _vert); \
            lucDatabase_AddLabel(database, lucLabelType, label); \
         }

float clamp(float x, float min, float max) 
{
  return ( x > max ) ? max : ( x < min ) ? min : x;
}

void _lucPlot_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucPlot* self = (lucPlot*)drawingObject;
   lucColourMap* colourMap = self->colourMap;
   int i;
   double *coloursR = NULL;
   double *coloursG = NULL;
   double *coloursB = NULL;
   double xtick, ytick, ztick;
   
   double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX}, max[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};

   if (self->rank > 0) return;

   /* Read the data */
   lucPlot_ReadDataFile(self);

   if (!self->rows) return;

   /* Calibrate Colour Map */
   if ( colourMap ) lucColourMap_SetMinMax( colourMap, 1, self->rows-1);

   /* Get colour map if from file */
   if (self->colourMapFromFile && self->colourMapFileName) {
      
      /* Read input file data */
      FILE* fp = fopen( self->colourMapFileName, "r" );
      if (!fp)
      {
         /* Attempt to open in output path */
         char* filename;
         Stg_asprintf(&filename, "%s/%s", self->outputPath, self->colourMapFileName);
         fp = fopen( filename, "r" );
         Memory_Free( filename );
         if (!fp)
         {
            Journal_Printf( lucError, "Error in func '%s' - file not found %s.\n", __func__, self->colourMapFileName);
            return;
         }
      }
      coloursR = Memory_Alloc_Array(double, self->rows, "colours R");
      coloursG = Memory_Alloc_Array(double, self->rows, "colours G");
      coloursB = Memory_Alloc_Array(double, self->rows, "colours B");
      for(i = 0; i < self->rows; i++)
      {
         fscanf(fp, "%lf%lf%lf\n", &coloursR[i], &coloursG[i], &coloursB[i]);
         //Clamp values to range [0,1]
         coloursR[i] = clamp(coloursR[i], 0.0, 1.0);
         coloursG[i] = clamp(coloursG[i], 0.0, 1.0);
         coloursB[i] = clamp(coloursB[i], 0.0, 1.0);
      }
      fclose( fp );
   }
   
   /* Remember to get Z going into the page need to minus instead of + */
   for (i=0; i<self->rows; i++)
   {
      if (self->dataX)
      {
         if (self->dataX[i] < min[0]) min[0] = self->dataX[i];
         if (self->dataX[i] > max[0]) max[0] = self->dataX[i];
      }
      
      if (self->dataY)
      {
         if (self->dataY[i] < min[1]) min[1] = self->dataY[i];
         if (self->dataY[i] > max[1]) max[1] = self->dataY[i];
      }
      
      if (self->dataZ)
      {
         if (self->dataZ[i] < min[2]) min[2] = self->dataZ[i];
         if (self->dataZ[i] > max[2]) max[2] = self->dataZ[i];
      }
      else
         min[2] = max[2] = 0;
   }
   
   /* Set to zero if less than tolerance */
   if (fabs(min[0]) < DBL_EPSILON) min[0] = 0;
   if (fabs(min[1]) < DBL_EPSILON) min[1] = 0;
   if (fabs(min[2]) < DBL_EPSILON) min[2] = 0;

   /* Draw range */
   lineVertex(-0.02,   self->flipY ? 1 : 0,    self->coordZ);   /* X axis */
   lineVertex( 1.05,   self->flipY ? 1 : 0,    self->coordZ);
   lineVertex( 0,     -0.02,                   self->coordZ);   /* Y axis */
   lineVertex( 0,      1.05,                   self->coordZ);
   if (self->dataZ)
   {
      lineVertex(0,  self->flipY ? 1 : 0,   self->coordZ+0.02);  /* Z axis */
      lineVertex(0,  self->flipY ? 1 : 0,   self->coordZ-1.05);
   }
   

     if (self->setX) {
      min[0] = self->minX;
      max[0] = self->maxX;
   }
   if (self->setY) {
      min[1] = self->minY;
      max[1] = self->maxY;
   }
   if (self->setZ) {
      min[2] = self->minZ;
      max[2] = self->maxZ;
   }
   
   /* Draw tick marks at max values and then divide up according to specified number of tick marks */
   if (self->dataX) {
      for (i = 0; i <= self->ticksX; i++) {
         xtick = i*(1/(double)self->ticksX);
         lineVertex( xtick,    self->flipY ? 1.0 : 0, self->coordZ);   /* X tick */
         lineVertex( xtick,    self->flipY ? 1.02 : -0.02, self->coordZ);
         //printf("xtick is now %g\n", xtick);
      }
   }
   
   if (self->dataY) {
      for (i = 0; i <= self->ticksY; i++) {
         ytick = i*(1/(double)self->ticksY);
         lineVertex( 0,    self->flipY ? (1 - ytick) : ytick, self->coordZ);   /* Y tick */      
         lineVertex(-0.02, self->flipY ? (1 - ytick) : ytick, self->coordZ);
         
      }
   }
   
   if (self->dataZ) {
      for (i = 0; i <= self->ticksZ; i++) {
         ztick = -i*(1/(double)self->ticksZ);
         lineVertex( 0,    self->flipY ? 1.0 : 0, self->coordZ+ztick);   /* Z tick */      
         lineVertex(-0.02, self->flipY ? 1.0 : 0, self->coordZ+ztick);
         
      }   
   }
   
   /* Labels */
   char string[20];
   /* X */
   sprintf(string, "%.4g", min[0]);
   labelVertex( 0.02, self->flipY ? 1.05 : -0.02, self->coordZ, string);
   sprintf(string, "%.4g", max[0]);
   labelVertex( 1.02, self->flipY ? 1.05 : -0.02, self->coordZ, string);

   /* Y */
   sprintf(string, "%7.4g", min[1]);
   labelVertex( -0.1, self->flipY ? 0.98 : 0.02, self->coordZ, string);
   sprintf(string, "%7.4g", max[1]);
   labelVertex( -0.1, self->flipY ? 0.02 : 1.02, self->coordZ, string);

   /* Z */
   if (self->dataZ) {
      sprintf(string, "%7.4g", min[2]);
      labelVertex( -0.05, self->flipY ? 1.02 : 0.02, self->coordZ-0.02, string);
      sprintf(string, "%7.4g", max[2]);
      labelVertex( -0.08, self->flipY ? 1.02 : 0.02, self->coordZ-1.02, string);
            
   }
   /* Axis labels */
   if (self->flipY)
   {
      labelVertex( 0.1,  1.10, self->coordZ,   self->labelX);
      labelVertex( -0.1, -0.05, self->coordZ,   self->labelY);
      if (self->dataZ) {
         labelVertex( -0.1, 1.10, self->coordZ-0.1,   self->labelZ);

      }
   }
   else
   {
      labelVertex( 0.1, -0.05, self->coordZ,   self->labelX);
      labelVertex( -0.1,  1.05, self->coordZ,   self->labelY);
      if (self->dataZ) {
         labelVertex( -0.05,  0.05, self->coordZ-0.1,   self->labelZ);
      }
      
   }
   //printf("Range X %f-%f Y %f-%f\n", min[0], max[0], min[1], max[1]);

   /* Plot lines */
   if (self->lines)
   {
      float old_coord[3];
      for (i=0; i<self->rows; i++)
      {
         float coord[3] = {0, 0, self->coordZ};
         if (self->dataX) coord[0] = (self->dataX[i] - min[0]) / (max[0] - min[0]);
         if (self->dataY) coord[1] = (self->dataY[i] - min[1]) / (max[1] - min[1]);
         if (self->dataZ) coord[2] = self->coordZ -(self->dataZ[i] - min[2]) / (max[2] - min[2]);
         if (i>0) 
         {
            lucDatabase_AddVertices(database, 1, lucLineType, old_coord);
            lucDatabase_AddVertices(database, 1, lucLineType, coord);
         }
         memcpy(old_coord, coord, 3 * sizeof(float));
      }
   }

   /* Plot dots */
   if (self->points)
   {
      for (i=0; i<self->rows; i++)
      {
        float coord[3] = {0, 0, self->coordZ};
        if (self->dataX) {
           if (min[0] != max[0])
              coord[0] = (self->dataX[i] - min[0]) / (max[0] - min[0]);
           else
              coord[0] = self->dataX[i];
        }
        if (self->dataY) {
           if (min[1] != max[1])
              coord[1] = (self->dataY[i] - min[1]) / (max[1] - min[1]);
           else
              coord[1] = self->dataY[i];
        }
        if (self->dataZ) {
           if (min[2] != max[2])
              coord[2] = self->coordZ - (self->dataZ[i] - min[2]) / (max[2] - min[2]);
           else
              coord[2] = self->coordZ - self->dataZ[i];
        }
        if (self->colourMapFromFile) {
           //printf("coloursR[%d] is %g,coloursG[%d] is %g, coloursB[%d] is %g\n", i, coloursR[i], i, coloursG[i], i, coloursB[i]);
           self->colour.red = coloursR[i];
           self->colour.green = coloursG[i];
           self->colour.blue = coloursB[i];
           self->opacity = 1.0;
           lucDatabase_AddRGBA(database, lucPointType, self->opacity, &self->colour);
        }
        lucDatabase_AddVertices(database, 1, lucPointType, coord);
        //printf("coord_x %g coord_y %g coord_z %g\n", coord[0], coord[1], coord[2]);
      }
   }

   /* Plot bars */
   if (self->bars)
   {
      double lastcoord[3] = {0, 0, self->coordZ};
      for (i=0; i<self->rows; i++)
      {

        if (self->colourMapFromFile) {
           //printf("PLOT coloursR[%d] is %g,coloursG[%d] is %g, coloursB[%d] is %g\n", i, coloursR[i], i, coloursG[i], i, coloursB[i]);
           self->colour.red = coloursR[i];
           self->colour.green = coloursG[i];
           self->colour.blue = coloursB[i];
           self->opacity = 1.0;
           lucDatabase_AddRGBA(database, lucShapeType, self->opacity, &self->colour);
        } 

        float coord[3] = {0, 0, self->coordZ};
        if (self->dataX) {
           if (min[0] != max[0])
              coord[0] = (self->dataX[i] - min[0]) / (max[0] - min[0]);
           else
              coord[0] = self->dataX[i];
        }
        if (self->dataY) {
           if (min[1] != max[1])
              coord[1] = (self->dataY[i] - min[1]) / (max[1] - min[1]);
           else
              coord[1] = self->dataY[i];
        }
        if (self->dataZ) {
           if (min[2] != max[2])
              coord[2] = self->coordZ - (self->dataZ[i] - min[2]) / (max[2] - min[2]);
           else
              coord[2] = self->coordZ -self->dataZ[i];
        }
        
         if (i > 0)
         {
            float coordf[3] = {lastcoord[0] + (0.5 * (coord[0] - lastcoord[0])),
                               coord[1] * 0.5, coord[2]};
            float width = coord[0] - lastcoord[0];
            lucDatabase_AddVertices(database, 1, lucShapeType, coordf);
            lucDatabase_AddValues(database, 1, lucShapeType, lucXWidthData, NULL, &width);
            lucDatabase_AddValues(database, 1, lucShapeType, lucYHeightData, NULL, &coord[1]);
            /* Set the colour from the colour map - if provided */
            if ( colourMap )
            {
               float value = i;
               lucDatabase_AddValues(database, 1, lucShapeType, lucColourValueData, colourMap, &value);
            }
         }

         lastcoord[0] = coord[0];
      }
   }

   if (coloursR) Memory_Free(coloursR);
   if (coloursG) Memory_Free(coloursG);
   if (coloursB) Memory_Free(coloursB);
}

