/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>

#include <gLucifer/Base/src/Base.h>


#include "types.h"
#include "CrossSection.h"
#include "Isosurface.h"
#include "IsosurfaceCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucIsosurfaceCrossSection_Type = "lucIsosurfaceCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucIsosurfaceCrossSection* _lucIsosurfaceCrossSection_New(  LUCISOSURFACECROSSSECTION_DEFARGS  )
{
   lucIsosurfaceCrossSection*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucIsosurfaceCrossSection) );
   self = (lucIsosurfaceCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucIsosurfaceCrossSection_Init(
   lucIsosurfaceCrossSection*       self,
   IJK                              resolution,
   double                           minIsovalue,
   double                           maxIsovalue,
   double                           interval,
   lucIsosurface*                   isosurface)
{
   self->isosurface = isosurface;
   memcpy( self->resolution, resolution, sizeof(IJK) );
   self->interval = interval;
   self->minIsovalue = minIsovalue;
   self->maxIsovalue = maxIsovalue;
}

void _lucIsosurfaceCrossSection_Delete( void* drawingObject )
{
   lucIsosurfaceCrossSection*  self = (lucIsosurfaceCrossSection*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucIsosurfaceCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucIsosurfaceCrossSection*  self = (lucIsosurfaceCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucIsosurfaceCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucIsosurfaceCrossSection);
   Type                                                             type = lucIsosurfaceCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucIsosurfaceCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucIsosurfaceCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucIsosurfaceCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucIsosurfaceCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucIsosurfaceCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucIsosurfaceCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucIsosurfaceCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucIsosurfaceCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucIsosurfaceCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucIsosurfaceCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucIsosurfaceCrossSection_New(  LUCISOSURFACECROSSSECTION_PASSARGS  );
}

void _lucIsosurfaceCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucIsosurfaceCrossSection*     self = (lucIsosurfaceCrossSection*)drawingObject;
   IJK              resolution = {100,100,100};
   lucIsosurface*          isosurface;

   /* Construct Parent */
   self->defaultResolution = 100;   /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );

   /* Create and initialise isosurface object */
   isosurface = (lucIsosurface*)_lucIsosurface_DefaultNew(self->name);
   _lucIsosurface_AssignFromXML(isosurface, cf, data );
   /* Force global sampling on */
   isosurface->sampleGlobal = True;

   _lucIsosurfaceCrossSection_Init(
      self,
      resolution,
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minIsovalue", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxIsovalue", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"interval", 0.33 ),
      isosurface);
}

void _lucIsosurfaceCrossSection_Build( void* drawingObject, void* data )
{
   lucIsosurfaceCrossSection*     self        = (lucIsosurfaceCrossSection*)drawingObject;

   /* Build field variable in parent */
   _lucCrossSection_Build(self, data);

   _lucIsosurface_Build(self->isosurface, data);

}

void _lucIsosurfaceCrossSection_Initialise( void* drawingObject, void* data ) 
{
   lucIsosurfaceCrossSection* self = (lucIsosurfaceCrossSection*)drawingObject;
   _lucIsosurface_Initialise(self->isosurface, data);
}
void _lucIsosurfaceCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucIsosurfaceCrossSection_Destroy( void* drawingObject, void* data ) {}

#define LEFT_BOTTOM     0
#define RIGHT_BOTTOM    1
#define LEFT_TOP        2
#define RIGHT_TOP       3
#define LEFT            4
#define RIGHT           5
#define TOP             6
#define BOTTOM          7

void _lucIsosurfaceCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucIsosurfaceCrossSection* self = (lucIsosurfaceCrossSection*)drawingObject;

   /* Parent setup */
   _lucCrossSection_Setup(drawingObject, database, _context);

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, False);
}

void _lucIsosurfaceCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucIsosurfaceCrossSection* self = (lucIsosurfaceCrossSection*)drawingObject;
   lucIsosurface*             isosurf = self->isosurface;
   double                     minIsovalue     = self->minIsovalue;
   double                     maxIsovalue     = self->maxIsovalue;
   lucColourMap*              colourMap       = self->colourMap;
   int                        i, j, k;
   double                     isovalue;
   Index                      triangle_I;

   /* Custom max, min, use global min/max if equal (defaults to both zero) */
   if (minIsovalue == maxIsovalue)
   {
      //minIsovalue = FieldVariable_GetMinGlobalFieldMagnitude(self->fieldVariable);
      //maxIsovalue = FieldVariable_GetMaxGlobalFieldMagnitude(self->fieldVariable);
   }

   if (colourMap)
      lucColourMap_SetMinMax(colourMap, minIsovalue, maxIsovalue);

   /* Copy object id */
   isosurf->id = self->id; 

   if (self->rank == 0)
   {
      /* Calculate a value we can use to offset each surface section slightly
       *  so they appear in the same position but don't actually overlap */
      Coord min, max;
      float shift = 0;
      float range = 0;
      int d;
      Mesh_GetGlobalCoordRange(self->mesh, min, max );
      for (d=0; d<3; d++)
         range += (max[d] - min[d]) / 5000.0;
      range /= 3.0;

      /* Allocate Memory */
      Vertex** points = Memory_Alloc_2DArray( Vertex , 8, 1, "array for marching squares");

      /* Draw isovalues at each interval from min to max */
      for ( isovalue = minIsovalue ; isovalue <= maxIsovalue ; isovalue += self->interval )
      {
         float nshift[3] = {shift * self->normal[0], shift * self->normal[1], shift * self->normal[2]};
         shift += range;

         isosurf->triangleCount = 0;   /* Reset */
         isosurf->colourMap = NULL;

         //if ( colourMap )
         //   lucColourMap_GetColourFromValue(colourMap, isovalue, &isosurf->colour, self->opacity);

         /* Run marching rectangles for this isovalue */
         isosurf->isovalue = isovalue;
         for ( i = 0 ; i < self->resolutionA-1 ; i++ )
         {
            for ( j = 0 ; j < self->resolutionB-1 ; j++ )
            {
               /* Copy vertex */
               for (k = 0; k<3; k++)
               {
                  points[LEFT_BOTTOM]->pos[k] = self->vertices[i][j][k] + nshift[k];
                  points[RIGHT_BOTTOM]->pos[k] = self->vertices[i+1][j][k] + nshift[k];
                  points[LEFT_TOP]->pos[k] = self->vertices[i][j+1][k] + nshift[k];
                  points[RIGHT_TOP]->pos[k] = self->vertices[i+1][j+1][k] + nshift[k];
               }
               /* Copy value */
               points[LEFT_BOTTOM]->value = self->values[i][j][0];
               points[RIGHT_BOTTOM]->value = self->values[i+1][j][0];
               points[LEFT_TOP]->value = self->values[i][j+1][0];
               points[RIGHT_TOP]->value = self->values[i+1][j+1][0];

               /* Interpolate mid-points and create triangles */
               lucIsosurface_WallElement( isosurf, points );
            }
         }

         /* Draw the surface section */
         isosurf->colourMap = self->colourMap;
         _lucIsosurface_Draw(self->isosurface, database, _context );

         /* Export colour values */
         if (self->colourMap)
         {
            float iso = isovalue;
            for ( triangle_I = 0 ; triangle_I < isosurf->triangleCount ; triangle_I++)
               for (i=0; i<3; i++)
                  lucDatabase_AddValues(database, 1, lucTriangleType, lucColourValueData, self->colourMap, &iso);
         }

      }

      Memory_Free( points );
   }

   /* Free memory */
   lucCrossSection_FreeSampleData(self);
}

