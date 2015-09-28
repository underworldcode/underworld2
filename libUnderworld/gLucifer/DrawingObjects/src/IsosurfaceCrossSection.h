/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucIsosurfaceCrossSection_h__
#define __lucIsosurfaceCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucIsosurfaceCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucIsosurfaceCrossSection \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucCrossSection \
      /* Virtual functions go here */ \
      /* Other info */\
      IJK                        resolution;             \
      XYZ                        minCropValues;          \
      XYZ                        maxCropValues;          \
      lucIsosurface*             isosurface;             \
      double                     interval;               \
      double                     minIsovalue;            \
      double                     maxIsovalue;            \
 
struct lucIsosurfaceCrossSection
{
   __lucIsosurfaceCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCISOSURFACECROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCISOSURFACECROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucIsosurfaceCrossSection* _lucIsosurfaceCrossSection_New(  LUCISOSURFACECROSSSECTION_DEFARGS  );

void _lucIsosurfaceCrossSection_Delete( void* drawingObject ) ;
void _lucIsosurfaceCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucIsosurfaceCrossSection_DefaultNew( Name name ) ;
void _lucIsosurfaceCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucIsosurfaceCrossSection_Build( void* drawingObject, void* data ) ;
void _lucIsosurfaceCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucIsosurfaceCrossSection_Execute( void* drawingObject, void* data );
void _lucIsosurfaceCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucIsosurfaceCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context );
void _lucIsosurfaceCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );
#endif

