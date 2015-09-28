/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucContourCrossSection_h__
#define __lucContourCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucContourCrossSection_Type;

typedef struct 
{
   double x;
   double y;
   double z;
   Bool filled;
   double isovalue;
} labelCoord;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucContourCrossSection \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
		Bool                       showValues;             \
		Bool                       printUnits;             \
		/* Interval Stuff */ \
		double                     interval;               \
      double                     minIsovalue;            \
      double                     maxIsovalue;            \
      int                        coordIndex;             \
      int                        printedIndex;           \
 
struct lucContourCrossSection
{
   __lucContourCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCONTOURCROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCCONTOURCROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucContourCrossSection* _lucContourCrossSection_New(  LUCCONTOURCROSSSECTION_DEFARGS  );

void _lucContourCrossSection_Delete( void* drawingObject ) ;
void _lucContourCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucContourCrossSection_DefaultNew( Name name ) ;
void _lucContourCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucContourCrossSection_Build( void* drawingObject, void* data ) ;
void _lucContourCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucContourCrossSection_Execute( void* drawingObject, void* data );
void _lucContourCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucContourCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context );
void _lucContourCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

void lucContourCrossSection_DrawCrossSection(void* drawingObject, lucDatabase* database);

void lucContourCrossSection_DrawContour(void* drawingObject, lucDatabase* database, double isovalue);

void lucContourCrossSection_PlotPoint(lucContourCrossSection* self, lucDatabase* database, char edge, double isovalue, int aIndex, int bIndex);

#endif

