/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucIsosurface_h__
#define __lucIsosurface_h__

#include <gLucifer/Base/src/Base.h>
#include "types.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucIsosurface_Type;

typedef struct
{
   float pos[3][3];
   float value[3];
   Bool  wall;
} Surface_Triangle;

typedef struct
{
   double value;
   double pos[3];
   int   element_I;
} Vertex;


/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucIsosurface \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucDrawingObject \
      /* Virtual functions go here */ \
      /* Other info */\
      FieldVariable*                      isosurfaceField;        \
      double                              isovalue;               \
      IJK                                 resolution;             \
      Bool                                drawWalls;              \
      Bool                                sampleGlobal;           \
      Coord                               globalMin;              \
      Coord                               globalMax;              \
      /* Colour Parameters */ \
      FieldVariable*                      colourField;            \
      /* Masking parameters */\
      FieldVariable*                      maskField;              \
      lucDrawingObjectMask                mask;                   \
      /* Calculated Values */ \
      Surface_Triangle*                   triangleList;           \
      Index                               triangleCount;          \
      Index                               trianglesAlloced;       \
      Index                               nx;                     \
      Index                               ny;                     \
      Index                               nz;                     \
      Index                               elementRes[3];          \
 
struct lucIsosurface
{
   __lucIsosurface
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCISOSURFACE_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCISOSURFACE_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucIsosurface* _lucIsosurface_New(  LUCISOSURFACE_DEFARGS  );

void _lucIsosurface_Delete( void* drawingObject ) ;
void _lucIsosurface_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucIsosurface_DefaultNew( Name name ) ;
void _lucIsosurface_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucIsosurface_Build( void* drawingObject, void* data ) ;
void _lucIsosurface_Initialise( void* drawingObject, void* data ) ;
void _lucIsosurface_Execute( void* drawingObject, void* data );
void _lucIsosurface_Destroy( void* drawingObject, void* data ) ;

/* Drawing Object Implementations */
void _lucIsosurface_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;
void _lucIsosurface_Write( void* drawingObject, lucDatabase* database, Bool walls );
void _lucIsosurface_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

void lucIsosurface_MarchingCubes( lucIsosurface* self, Vertex*** vertex ) ;
void lucIsosurface_DrawWalls( lucIsosurface* self, Vertex*** array ) ;

void lucIsosurface_MarchingRectangles( lucIsosurface* self, Vertex** points, char squareType ) ;
void lucIsosurface_WallElement( lucIsosurface* self, Vertex** points ) ;

#endif

