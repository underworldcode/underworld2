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
#include <gLucifer/Base/src/DrawingObject.h>
#include "Isosurface.h"

void lucIsosurface_SampleLocal( void* drawingObject);
void lucIsosurface_SampleGlobal( void* drawingObject);
void VertexInterp(lucIsosurface* self, Vertex* point, Vertex* vertex1, Vertex* vertex2 );
void CreateTriangle(lucIsosurface* self, Vertex* point1, Vertex* point2, Vertex* point3, Bool wall);

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucIsosurface_Type = "lucIsosurface";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucIsosurface* _lucIsosurface_New(  LUCISOSURFACE_DEFARGS  )
{
   lucIsosurface*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucIsosurface) );
   self = (lucIsosurface*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucIsosurface_Init(
   lucIsosurface*                                     self,
   double                                             isovalue,
   IJK                                                resolution,
   Bool                                               drawWalls,
   Bool                                               sampleGlobal,
   lucDrawingObjectMask*                              mask )
{
   self->isovalue        = isovalue;
   memcpy( self->resolution, resolution, sizeof(IJK) );
   self->drawWalls       = drawWalls;
   self->sampleGlobal    = sampleGlobal;
   memcpy( &self->mask, mask, sizeof(lucDrawingObjectMask) );

   self->trianglesAlloced = 100;
   self->triangleList = Memory_Alloc_Array( Surface_Triangle, self->trianglesAlloced, "triangleList" );
}

void _lucIsosurface_Delete( void* drawingObject )
{
   lucIsosurface*  self = (lucIsosurface*)drawingObject;

   Memory_Free( self->triangleList );

   _lucDrawingObject_Delete( self );
}

void _lucIsosurface_Print( void* drawingObject, Stream* stream )
{
   lucIsosurface*  self = (lucIsosurface*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucIsosurface_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucIsosurface);
   Type                                                             type = lucIsosurface_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucIsosurface_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucIsosurface_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucIsosurface_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucIsosurface_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucIsosurface_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucIsosurface_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucIsosurface_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucIsosurface_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucIsosurface_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucIsosurface_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucIsosurface_New(  LUCISOSURFACE_PASSARGS  );
}

void _lucIsosurface_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucIsosurface*         self               = (lucIsosurface*)drawingObject;
   Index                  defaultRes;
   IJK                    resolution;
   double                 isovalue;
   lucDrawingObjectMask   mask;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   self->elementRes[I_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResI"  );
   self->elementRes[J_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResJ"  );
   self->elementRes[K_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResK"  );

   defaultRes = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", 1.0);
   resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", defaultRes);
   resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", defaultRes);
   resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", defaultRes);

   /* Get fields */
   self->isosurfaceField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"IsosurfaceField", FieldVariable, True, data  );
   self->colourField     = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ColourField", FieldVariable, False, data  );
   self->maskField       = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"MaskField", FieldVariable, False, data  );

   if (defaultRes == 1 && (resolution[I_AXIS] > 2 || resolution[J_AXIS] > 2 || resolution[K_AXIS] > 2))
   {
      Journal_Printf( lucInfo, "** WARNING ** excessive isosurface resolution: samples per element reduced to 2,2,2 - was %d,%d,%d\n", resolution[I_AXIS], resolution[J_AXIS], resolution[K_AXIS]);
      resolution[I_AXIS] = resolution[J_AXIS] = resolution[K_AXIS] = 2;
   }

   lucDrawingObjectMask_Construct( &mask, self->name, cf, data );

   isovalue = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"isovalue", 0.0  );

   _lucIsosurface_Init(
      self,
      isovalue,
      resolution,
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"drawWalls", False  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"sampleGlobal", False  ),
      &mask );
}

void _lucIsosurface_Build( void* drawingObject, void* data )
{
   lucIsosurface*             self = (lucIsosurface*)drawingObject;

   /* Legacy, probably not necessary... */
   Stg_Component_Build( self->isosurfaceField, data, False );
   Stg_Component_Build( self->colourField, data, False );
   Stg_Component_Build( self->maskField, data, False );
}

void _lucIsosurface_Initialise( void* drawingObject, void* data ) {
   lucIsosurface*             self = (lucIsosurface*)drawingObject;

   if (self->resolution[ I_AXIS ] >= self->elementRes[I_AXIS]) self->resolution[ I_AXIS ] /= self->elementRes[I_AXIS];
   if (self->resolution[ J_AXIS ] >= self->elementRes[J_AXIS]) self->resolution[ J_AXIS ] /= self->elementRes[J_AXIS];
   if (self->resolution[ K_AXIS ] >= self->elementRes[K_AXIS]) self->resolution[ K_AXIS ] /= self->elementRes[K_AXIS];

   if (self->isosurfaceField->dim == 2) self->resolution[K_AXIS] = 0;

   if (self->sampleGlobal)
   {
      self->resolution[I_AXIS] *= self->elementRes[I_AXIS];
      self->resolution[J_AXIS] *= self->elementRes[J_AXIS];
      self->resolution[K_AXIS] *= self->elementRes[K_AXIS];
   }

   self->nx = self->resolution[I_AXIS] + 1;
   self->ny = self->resolution[J_AXIS] + 1;
   self->nz = self->resolution[K_AXIS] + 1;
}

void _lucIsosurface_Execute( void* drawingObject, void* data ) {}
void _lucIsosurface_Destroy( void* drawingObject, void* data ) {}

void _lucIsosurface_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucIsosurface*             self = (lucIsosurface*)drawingObject;

   lucDrawingObject_SyncShadowValues( self, self->isosurfaceField );
   lucDrawingObject_SyncShadowValues( self, self->colourField );
   lucDrawingObject_SyncShadowValues( self, self->maskField );

   FieldVariable_GetMinAndMaxGlobalCoords(self->isosurfaceField, self->globalMin, self->globalMax);

   self->triangleCount = 0;

   Journal_Printf( lucInfo, "Sampling %s for %s (global=%d)\n", self->isosurfaceField->name, self->name, self->sampleGlobal);

   /* Anything to sample? */
   double min = FieldVariable_GetMinGlobalFieldMagnitude(self->isosurfaceField);
   double max = FieldVariable_GetMaxGlobalFieldMagnitude(self->isosurfaceField);
   if (min == max) return;
   
   if (self->sampleGlobal)
      lucIsosurface_SampleGlobal(drawingObject);
   else
      lucIsosurface_SampleLocal(drawingObject);
}

/* New method: sample & surface each element in local coords, faster, handles deformed meshes */
void lucIsosurface_SampleLocal( void* drawingObject)
{
   lucIsosurface*             self               = (lucIsosurface*)drawingObject;
   FeVariable*                feVariable         = (FeVariable*) self->isosurfaceField;
   FeMesh*    		            mesh               = feVariable->feMesh;
   Element_LocalIndex         lElement_I;
   Element_LocalIndex         elementLocalCount  = FeMesh_GetElementLocalSize( mesh );
   int                        i, j, k;
   Vertex***                  vertex;

   vertex = Memory_Alloc_3DArray( Vertex, self->nx, self->ny, self->nz, (Name)"Vertex array" );

   for ( lElement_I = 0 ; lElement_I < elementLocalCount ; lElement_I++ )
   {
      for (i = 0 ; i < self->nx; i++)
      {
         for (j = 0 ; j < self->ny; j++)
         {
            for (k = 0 ; k < self->nz; k++)
            {
               /* Calc position within element in local coords */
               Coord local = {-1.0 + (2.0 * i / self->resolution[ I_AXIS ]), 
                              -1.0 + (2.0 * j / self->resolution[ J_AXIS ]),
                              -1.0 + (2.0 * k / self->resolution[ K_AXIS ])};

               /* Get value at coords (faster using element and local coords) */
               FeVariable_InterpolateWithinElement( feVariable, lElement_I, local, &(vertex[i][j][k].value));

               /* Save local coords and element index for fast interpolation */
               memcpy( vertex[i][j][k].pos, local, 3 * sizeof(double) );
               vertex[i][j][k].element_I = lElement_I;
            }
         }
      }

      if (self->isosurfaceField->dim == 3)
         lucIsosurface_MarchingCubes( self, vertex );

      if (self->isosurfaceField->dim == 2 || self->drawWalls)
         lucIsosurface_DrawWalls( self, vertex );
   }

   /* Free memory */
   Memory_Free( vertex );
}

/* Old method: sampling in global coords, slower, assumes regular grid */
void lucIsosurface_SampleGlobal( void* drawingObject)
{
   lucIsosurface*             self = (lucIsosurface*)drawingObject;
   FieldVariable*             isosurfaceField = self->isosurfaceField;
   int                        i, j, k;
   double                     dx, dy, dz;
   Coord                      pos;
   Coord                      min, max;
   Vertex***                  vertex;
   double         lengths[3];

   /* Calculate a resolution for this local domain based on a sample size over the entire domain */
   FieldVariable_GetMinAndMaxLocalCoords( isosurfaceField, min, max );
   for (i=0; i<3; i++)
      lengths[i] = (self->globalMax[i] - self->globalMin[i])/((double)self->resolution[i]);
   /* force round up */
   self->nx = ceil((max[0] - min[0]) / lengths[0]) + 1;
   self->ny = ceil((max[1] - min[1]) / lengths[1]) + 1;
   self->nz = ceil((max[2] - min[2]) / lengths[2]) + 1;

   dx = (max[0] - min[0])/((double) self->nx - 1);
   dy = (max[1] - min[1])/((double) self->ny - 1);
   dz = (max[2] - min[2])/((double) self->nz - 1);
   if (self->isosurfaceField->dim == 2)
   {
      dz = 0.0;
      self->nz = 1;
   }

   vertex = Memory_Alloc_3DArray( Vertex , self->nx, self->ny, self->nz , (Name)"Vertex array" );

   /* Sample Field in in regular grid */
   for ( i = 0 ; i < self->nx ; i++ )
   {
      for ( j = 0 ; j < self->ny ; j++ )
      {
         for ( k = 0 ; k < self->nz ; k++ )
         {
            pos[ I_AXIS ] = min[ I_AXIS ] + dx * (double) i;
            pos[ J_AXIS ] = min[ J_AXIS ] + dy * (double) j;
            pos[ K_AXIS ] = min[ K_AXIS ] + dz * (double ) k;

            memcpy( vertex[i][j][k].pos, pos, 3 * sizeof(double) );

            if (!FieldVariable_InterpolateValueAt( isosurfaceField, pos, &vertex[i][j][k].value ))
               /* FieldVariable_InterpolateValueAt returns OTHER_PROC if point not found in mesh, so zero value */
               vertex[i][j][k].value = 0;
         }
      }
   }

   if (self->isosurfaceField->dim == 3)
      /* Find Surface with Marching Cubes */
      lucIsosurface_MarchingCubes( self, vertex );

   if (self->isosurfaceField->dim == 2 || self->drawWalls)
      lucIsosurface_DrawWalls( self, vertex );

   /* Free memory */
   Memory_Free( vertex );
}

void _lucIsosurface_Write( void* drawingObject, lucDatabase* database, Bool walls )
{
   /* Export surface triangles */
   lucIsosurface* self = (lucIsosurface*)drawingObject;
   Index triangle_I;
   int i;
   for ( triangle_I = 0 ; triangle_I < self->triangleCount ; triangle_I++)
   {
      if (self->triangleList[triangle_I].wall != walls) continue;
      for (i=0; i<3; i++)
      {
         /* Dump vertex pos, [value] */
         float coordf[3] = {self->triangleList[triangle_I].pos[i][0],
                            self->triangleList[triangle_I].pos[i][1],
                            self->triangleList[triangle_I].pos[i][2]};
         float value = self->triangleList[triangle_I].value[i];
         lucDatabase_AddVertices(database, 1, lucTriangleType, coordf);
         if (self->colourField && self->colourMap)
            lucDatabase_AddValues(database, 1, lucTriangleType, lucColourValueData, self->colourMap, &value);
      }
   }
}

void _lucIsosurface_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucIsosurface*           self          = (lucIsosurface*)drawingObject;

   /* Calibrate Colour Map */
   if (self->colourMap)
   {
      //if (self->colourField )
      //   lucColourMap_CalibrateFromFieldVariable( self->colourMap, self->colourField );
      //else
      //   /* Colour based on isovalue */
      //   lucColourMap_CalibrateFromFieldVariable( self->colourMap, self->isosurfaceField );
   }

   /* Export triangles, separating walls and surface into two geometry objects */
   /* Export surface triangles */
   _lucIsosurface_Write(self, database, False);

   /* Start new geometry section */
   lucDatabase_OutputGeometry(database, self->id);

   /* Export wall triangles */
   _lucIsosurface_Write(self, database, True);
}

/*
   Given a grid cell and an isolevel, calculate the triangular
   facets required to represent the isosurface through the cell.
   Return the number of triangular facets, the array "triangleList"
   will be loaded up with the vertices at most 5 triangular facets.
   0 will be returned if the grid cell is either totally above
   of totally below the isolevel.
*/

/* This algorithm for constructing an isosurface is taken from:
Lorensen, William and Harvey E. Cline. Marching Cubes: A High Resolution 3D Surface Construction Algorithm. Computer Graphics (SIGGRAPH 87 Proceedings) 21(4) July 1987, p. 163-170) http://www.cs.duke.edu/education/courses/fall01/cps124/resources/p163-lorensen.pdf
The lookup table is taken from http://astronomy.swin.edu.au/~pbourke/modelling/polygonise/
*/
void lucIsosurface_MarchingCubes( lucIsosurface* self, Vertex*** vertex )
{
   double isolevel = self->isovalue;
   int i,j,k,n;
   int cubeindex;
   Vertex points[12];

   int edgeTable[256]=
   {
      0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
      0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
      0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
      0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
      0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
      0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
      0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
      0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
      0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
      0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
      0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
      0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
      0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
      0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
      0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
      0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
      0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
      0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
      0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
      0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
      0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
      0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
      0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
      0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
      0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
      0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
      0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
      0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
      0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
      0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
      0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
      0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
   };
   int triTable[256][16] =
   {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
      {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
      {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
      {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
      {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
      {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
      {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
      {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
      {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
      {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
      {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
      {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
      {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
      {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
      {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
      {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
      {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
      {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
      {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
      {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
      {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
      {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
      {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
      {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
      {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
      {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
      {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
      {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
      {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
      {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
      {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
      {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
      {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
      {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
      {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
      {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
      {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
      {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
      {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
      {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
      {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
      {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
      {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
      {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
      {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
      {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
      {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
      {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
      {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
      {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
      {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
      {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
      {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
      {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
      {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
      {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
      {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
      {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
      {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
      {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
      {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
      {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
      {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
      {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
      {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
      {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
      {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
      {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
      {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
      {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
      {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
      {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
      {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
      {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
      {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
      {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
      {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
      {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
      {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
      {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
      {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
      {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
      {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
      {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
      {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
      {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
      {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
      {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
      {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
      {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
      {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
      {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
      {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
      {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
      {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
      {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
      {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
      {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
      {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
      {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
      {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
      {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
      {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
      {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
      {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
      {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
      {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
      {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
      {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
      {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
      {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
      {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
      {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
      {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
      {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
      {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
      {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
      {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
      {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
      {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
      {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
      {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
      {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
      {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
      {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
      {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
      {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
      {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
      {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
      {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
      {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
      {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
      {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
      {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
      {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
      {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
      {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
      {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
      {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
      {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
      {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
      {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
      {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
      {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
      {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
      {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
      {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
      {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
      {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
      {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
      {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
      {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
      {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
      {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
      {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
      {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
      {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
      {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
      {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
      {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
      {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
      {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
      {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
      {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
      {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
      {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
      {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
      {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
      {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
      {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
      {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
      {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
      {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
      {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
      {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
      {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
      {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
      {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
      {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
      {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
   };


   for ( i = 0 ; i < self->nx - 1  ; i++ )
   {
      for ( j = 0 ; j < self->ny - 1 ; j++ )
      {
         for ( k = 0 ; k < self->nz - 1 ; k++ )
         {
            /* Determine the index into the edge table which tells us which vertices are inside of the surface */
            cubeindex = 0;
            if (vertex[i][j][k].value       < isolevel) cubeindex |= 1;
            if (vertex[i+1][j][k].value     < isolevel) cubeindex |= 2;
            if (vertex[i+1][j][k+1].value   < isolevel) cubeindex |= 4;
            if (vertex[i][j][k+1].value     < isolevel) cubeindex |= 8;
            if (vertex[i][j+1][k].value     < isolevel) cubeindex |= 16;
            if (vertex[i+1][j+1][k].value   < isolevel) cubeindex |= 32;
            if (vertex[i+1][j+1][k+1].value < isolevel) cubeindex |= 64;
            if (vertex[i][j+1][k+1].value   < isolevel) cubeindex |= 128;

            /* Cube is entirely in/out of the surface */
            if (edgeTable[cubeindex] == 0) continue;

            /* Find the vertices where the surface intersects the cube */
            if (edgeTable[cubeindex] & 1)
               VertexInterp(self, &points[0], &vertex[i][j][k] , &vertex[i+1][j][k]);
            if (edgeTable[cubeindex] & 2)
               VertexInterp(self, &points[1], &vertex[i+1][j][k] , &vertex[i+1][j][k+1] );
            if (edgeTable[cubeindex] & 4)
               VertexInterp(self, &points[2], &vertex[i+1][j][k+1] , &vertex[i][j][k+1] );
            if (edgeTable[cubeindex] & 8)
               VertexInterp(self, &points[3], &vertex[i][j][k+1] , &vertex[i][j][k] );
            if (edgeTable[cubeindex] & 16)
               VertexInterp(self, &points[4], &vertex[i][j+1][k] , &vertex[i+1][j+1][k] );
            if (edgeTable[cubeindex] & 32)
               VertexInterp(self, &points[5], &vertex[i+1][j+1][k] , &vertex[i+1][j+1][k+1] );
            if (edgeTable[cubeindex] & 64)
               VertexInterp(self, &points[6], &vertex[i+1][j+1][k+1] , &vertex[i][j+1][k+1] );
            if (edgeTable[cubeindex] & 128)
               VertexInterp(self, &points[7], &vertex[i][j+1][k+1] , &vertex[i][j+1][k] );
            if (edgeTable[cubeindex] & 256)
               VertexInterp(self, &points[8], &vertex[i][j][k] , &vertex[i][j+1][k] );
            if (edgeTable[cubeindex] & 512)
               VertexInterp(self, &points[9], &vertex[i+1][j][k] , &vertex[i+1][j+1][k] );
            if (edgeTable[cubeindex] & 1024)
               VertexInterp(self, &points[10], &vertex[i+1][j][k+1] , &vertex[i+1][j+1][k+1] );
            if (edgeTable[cubeindex] & 2048)
               VertexInterp(self, &points[11], &vertex[i][j][k+1] , &vertex[i][j+1][k+1] );

            /* Create the triangles */
            for ( n = 0 ; triTable[cubeindex][n] != -1 ; n += 3 )
            {
               CreateTriangle(self, &points[ triTable[cubeindex][n  ] ], 
                                    &points[ triTable[cubeindex][n+1] ], 
                                    &points[ triTable[cubeindex][n+2] ], False);
            }
         }
      }
   }
}

/* Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value */
void VertexInterp(lucIsosurface* self, Vertex* point, Vertex* vertex1, Vertex* vertex2 )
{
   double mu;
   double isolevel = self->isovalue;

   /* Don't try and interpolate if isovalue not between vertices */
   if (vertex1->value > isolevel && vertex2->value > isolevel)
   {
      if (vertex1->value < vertex2->value)
         memcpy( point, vertex1, sizeof(Vertex) );
      else
         memcpy( point, vertex2, sizeof(Vertex) );
      return;
   }
   if (vertex1->value < isolevel && vertex2->value < isolevel) 
   {
      if (vertex1->value > vertex2->value)
         memcpy( point, vertex1, sizeof(Vertex) );
      else
         memcpy( point, vertex2, sizeof(Vertex) );
      return;
   }

   memcpy( point, vertex1, sizeof(Vertex) );
   mu = (isolevel - vertex1->value) / (vertex2->value - vertex1->value);
   point->pos[0] = vertex1->pos[0] + mu * (vertex2->pos[0] - vertex1->pos[0]);
   point->pos[1] = vertex1->pos[1] + mu * (vertex2->pos[1] - vertex1->pos[1]);
   point->pos[2] = vertex1->pos[2] + mu * (vertex2->pos[2] - vertex1->pos[2]);
}

void CreateTriangle(lucIsosurface* self, Vertex* point1, Vertex* point2, Vertex* point3, Bool wall)
{
   Vertex* points[3] = {point1, point2, point3};
   int i;

   if (self->triangleCount >= self->trianglesAlloced)
   {
      self->trianglesAlloced += 100;
      self->triangleList = Memory_Realloc_Array( self->triangleList, Surface_Triangle, self->trianglesAlloced );
   }

   /* Wall flag */
   self->triangleList[self->triangleCount].wall = wall;
 
   for (i=0; i<3; i++)
   {
      double value = 0;
      float *pos = self->triangleList[self->triangleCount].pos[i]; 

      if (self->sampleGlobal)
      {
         /* Check maskField values */
         if ( self->maskField )
         {
            double mvalue;
            FieldVariable_InterpolateValueAt( self->maskField, points[i]->pos, &mvalue);
            /* Don't bother calculating if vertex is masked */
            if (lucDrawingObjectMask_Test( &self->mask, mvalue ) == False) return;
         }

         /* Get global coordinates */
         pos[0] = points[i]->pos[0];
         pos[1] = points[i]->pos[1];
         pos[2] = points[i]->pos[2];
         if (self->isosurfaceField->dim == 2) pos[2] = 0;

         /* Get colourField value */
         if ( self->colourField && self->colourMap)
            FieldVariable_InterpolateValueAt( self->colourField, points[i]->pos, &value);
      }
      else
      {
         FeVariable* field = (FeVariable*)self->isosurfaceField;
         double dpos[3] = {0};

         /* Check maskField values */
         if ( self->maskField )
         {
            double mvalue;
            FeVariable_InterpolateWithinElement( self->maskField, points[i]->element_I, points[i]->pos, &mvalue);
            /* Don't bother calculating if vertex is masked */
            if (lucDrawingObjectMask_Test( &self->mask, mvalue ) == False) return;
         }

         /* Get global coordinates */
         FeMesh_CoordLocalToGlobal( field->feMesh, points[i]->element_I, points[i]->pos, dpos );
         pos[0] = dpos[0];
         pos[1] = dpos[1];
         pos[2] = dpos[2];

         /* Get colourField value */
         if ( self->colourField && self->colourMap)
            FeVariable_InterpolateWithinElement( self->colourField, points[i]->element_I, points[i]->pos, &value);
      }

      /* Copy value */
      self->triangleList[self->triangleCount].value[i] = value;
   }

   self->triangleCount++;
}

#define LEFT_BOTTOM     0
#define RIGHT_BOTTOM    1
#define LEFT_TOP        2
#define RIGHT_TOP       3
#define LEFT            4
#define RIGHT           5
#define TOP             6
#define BOTTOM          7

void lucIsosurface_DrawWalls( lucIsosurface* self, Vertex ***vertex )
{
   int i, j, k;
   Vertex ** points;
   Vertex * midVertices;
   /* Allocate Memory */
   points = Memory_Alloc_Array( Vertex* , 8, "array for marching squares");
   midVertices = Memory_Alloc_Array( Vertex , 4, "array for marching squares");
   points[LEFT] = &midVertices[0];
   points[RIGHT] = &midVertices[1];
   points[TOP] = &midVertices[2];
   points[BOTTOM] = &midVertices[3];

   /* Check boundaries of area this vertex array covers, 
    * only draw walls if aligned to outer edges of global domain */ 
   int min[3], max[3], range[3] = {self->nx-1, self->ny-1, self->nz-1};
   int axis;
   if (self->sampleGlobal || self->isosurfaceField->dim == 2)
   {
      /* Array covers the entire domain, don't need to check */
      for (axis=0; axis<3; axis++)
      {
         min[axis] = 0;
         max[axis] = range[axis];
         if (range[axis] == 0) range[axis] = 1;
      }
   }
   else
   {
      /* Compare global min/max to min/max of our sample region and set ranges to process accordingly */
      FeVariable* field = (FeVariable*)self->isosurfaceField;
      Coord posMin, posMax;
      FeMesh_CoordLocalToGlobal( field->feMesh, vertex[0][0][0].element_I, vertex[0][0][0].pos, posMin );
      FeMesh_CoordLocalToGlobal( field->feMesh, vertex[self->nx-1][self->ny-1][self->nz-1].element_I, vertex[self->nx-1][self->ny-1][self->nz-1].pos, posMax );

      for (axis=0; axis<3; axis++)
      {
         if (posMin[axis] == self->globalMin[axis]) min[axis] = 0; else min[axis] = range[axis];
         if (posMax[axis] == self->globalMax[axis]) max[axis] = range[axis]; else max[axis] = 0;
      }
   }

   /* Generate front/back walls (Z=min/max, sample XY) */
   if (range[K_AXIS] > 0) 
   {
      for ( i = 0 ; i < self->nx - 1 ; i++ )
      {
         for ( j = 0 ; j < self->ny - 1 ; j++ )
         {
            for ( k = min[K_AXIS]; k <= max[K_AXIS]; k += range[K_AXIS])
            {
               points[LEFT_BOTTOM]  = &vertex[ i ][ j ][k];
               points[RIGHT_BOTTOM] = &vertex[i+1][ j ][k];
               points[LEFT_TOP]     = &vertex[ i ][j+1][k];
               points[RIGHT_TOP]    = &vertex[i+1][j+1][k];
               lucIsosurface_WallElement( self, points );
            }
         }
      }
   }

   /* Generate left/right walls (X=min/max, sample YZ) */
   if (self->isosurfaceField->dim == 3 && range[I_AXIS] > 0) 
   {
      for ( k = 0 ; k < self->nz - 1 ; k++ )
      {
         for ( j = 0 ; j < self->ny - 1 ; j++ )
         {
            for ( i = min[I_AXIS]; i <= max[I_AXIS]; i += range[I_AXIS])
            {
               points[LEFT_BOTTOM]  = &vertex[i][ j ][ k ];
               points[RIGHT_BOTTOM] = &vertex[i][j+1][ k ];
               points[LEFT_TOP]     = &vertex[i][ j ][k+1];
               points[RIGHT_TOP]    = &vertex[i][j+1][k+1];
               lucIsosurface_WallElement( self, points );
            }
         }
      }
   }

   /* Generate top/bottom walls (Y=min/max, sample XZ) */
   if (self->isosurfaceField->dim == 3 && range[J_AXIS] > 0) 
   {
      for ( i = 0 ; i < self->nx - 1 ; i++ )
      {
         for ( k = 0 ; k < self->nz - 1 ; k++ )
         {
            for ( j = min[J_AXIS]; j <= max[J_AXIS]; j += range[J_AXIS])
            {
               points[LEFT_BOTTOM]  = &vertex[ i ][j][ k ];
               points[RIGHT_BOTTOM] = &vertex[i+1][j][ k ];
               points[LEFT_TOP]     = &vertex[ i ][j][k+1];
               points[RIGHT_TOP]    = &vertex[i+1][j][k+1];
               lucIsosurface_WallElement( self, points );
            }
         }
      }
   }

   Memory_Free( points );
   Memory_Free( midVertices );
}

void lucIsosurface_WallElement( lucIsosurface* self, Vertex** points )
{
   double value  = self->isovalue;
   char   squareType = 0;

   /* find cube type */
   if (points[LEFT_BOTTOM]->value  > value) squareType += 1;
   if (points[RIGHT_BOTTOM]->value > value) squareType += 2;
   if (points[LEFT_TOP]->value     > value) squareType += 4;
   if (points[RIGHT_TOP]->value    > value) squareType += 8;

   if (squareType == 0) return; /* No triangles here */

   /* Create Intermediate Points */
   VertexInterp(self, points[ LEFT ], points[ LEFT_BOTTOM ] , points[LEFT_TOP]);
   VertexInterp(self, points[ RIGHT ], points[ RIGHT_BOTTOM ] , points[RIGHT_TOP]);
   VertexInterp(self, points[ BOTTOM ], points[ LEFT_BOTTOM ] , points[RIGHT_BOTTOM]);
   VertexInterp(self, points[ TOP ], points[ LEFT_TOP ] , points[RIGHT_TOP]);

   lucIsosurface_MarchingRectangles( self, points, squareType );
}

void lucIsosurface_MarchingRectangles( lucIsosurface* self, Vertex** points, char squareType)
{
   switch (squareType)
   {
   case 0:
      /*  @@  */
      /*  @@  */
      break;
   case 1:
      /*  @@  */
      /*  #@  */
      CreateTriangle(self, points[LEFT_BOTTOM], points[LEFT], points[BOTTOM], True);
      break;
   case 2:
      /*  @@  */
      /*  @#  */
      CreateTriangle(self, points[BOTTOM], points[RIGHT], points[RIGHT_BOTTOM], True);
      break;
   case 3:
      /*  @@  */
      /*  ##  */
      CreateTriangle(self, points[LEFT_BOTTOM], points[LEFT], points[RIGHT], True);
      CreateTriangle(self, points[LEFT_BOTTOM], points[RIGHT], points[RIGHT_BOTTOM], True);
      break;
   case 4:
      /*  #@  */
      /*  @@  */
      CreateTriangle(self, points[LEFT_TOP], points[TOP], points[LEFT], True);
      break;
   case 5:
      /*  #@  */
      /*  #@  */
      CreateTriangle(self, points[LEFT_TOP], points[TOP], points[LEFT_BOTTOM], True);
      CreateTriangle(self, points[TOP], points[BOTTOM], points[LEFT_BOTTOM], True);
      break;
   case 6:
      /*  #@  */
      /*  @#  */
      CreateTriangle(self, points[LEFT_TOP], points[TOP], points[LEFT], True);
      CreateTriangle(self, points[BOTTOM], points[RIGHT], points[RIGHT_BOTTOM], True);
      break;
   case 7:
      /*  #@  */
      /*  ##  */
      CreateTriangle(self, points[RIGHT], points[RIGHT_BOTTOM], points[TOP], True);
      CreateTriangle(self, points[TOP], points[RIGHT_BOTTOM], points[LEFT_TOP], True);
      CreateTriangle(self, points[LEFT_TOP], points[RIGHT_BOTTOM], points[LEFT_BOTTOM], True);
      break;
   case 8:
      /*  @#  */
      /*  @@  */
      CreateTriangle(self, points[TOP], points[RIGHT_TOP], points[RIGHT], True);
      break;
   case 9:
      /*  @#  */
      /*  #@  */
      CreateTriangle(self, points[TOP], points[RIGHT_TOP], points[RIGHT], True);
      CreateTriangle(self, points[LEFT_BOTTOM], points[LEFT], points[BOTTOM], True);
      break;
   case 10:
      /*  @#  */
      /*  @#  */
      CreateTriangle(self, points[TOP], points[RIGHT_TOP], points[RIGHT_BOTTOM], True);
      CreateTriangle(self, points[BOTTOM], points[TOP], points[RIGHT_BOTTOM], True);

      break;
   case 11:
      /*  @#  */
      /*  ##  */
      CreateTriangle(self, points[TOP], points[LEFT_BOTTOM], points[LEFT], True);
      CreateTriangle(self, points[RIGHT_TOP], points[LEFT_BOTTOM], points[TOP], True);
      CreateTriangle(self, points[RIGHT_BOTTOM], points[LEFT_BOTTOM], points[RIGHT_TOP], True);
      break;
   case 12:
      /*  ##  */
      /*  @@  */
      CreateTriangle(self, points[LEFT], points[LEFT_TOP], points[RIGHT], True);
      CreateTriangle(self, points[RIGHT], points[LEFT_TOP], points[RIGHT_TOP], True);
      break;
   case 13:
      /*  ##  */
      /*  #@  */
      CreateTriangle(self, points[BOTTOM], points[RIGHT_TOP], points[RIGHT], True);
      CreateTriangle(self, points[LEFT_BOTTOM], points[RIGHT_TOP], points[BOTTOM], True);
      CreateTriangle(self, points[LEFT_TOP], points[RIGHT_TOP], points[LEFT_BOTTOM], True);
      break;
   case 14:
      /*  ##  */
      /*  @#  */
      CreateTriangle(self, points[LEFT], points[LEFT_TOP], points[BOTTOM], True);
      CreateTriangle(self, points[BOTTOM], points[LEFT_TOP], points[RIGHT_BOTTOM], True);
      CreateTriangle(self, points[RIGHT_BOTTOM], points[LEFT_TOP], points[RIGHT_TOP], True);
      break;
   case 15:
      /*  ##  */
      /*  ##  */
      CreateTriangle(self, points[LEFT_TOP], points[RIGHT_TOP], points[RIGHT_BOTTOM], True);
      CreateTriangle(self, points[RIGHT_BOTTOM], points[LEFT_BOTTOM], points[LEFT_TOP], True);
      break;
   default:
      Journal_Printf( lucError, "In func %s: Cannot understand square type %d\n", __func__, squareType );
      abort();
   }
}


