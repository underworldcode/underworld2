/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "ContourCrossSection.h"

#ifndef __lucContour_h__
#define __lucContour_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucContour_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucContour \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucContourCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
 
struct lucContour
{
   __lucContour
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCONTOUR_DEFARGS \
                LUCCONTOURCROSSSECTION_DEFARGS

#define LUCCONTOUR_PASSARGS \
                LUCCONTOURCROSSSECTION_PASSARGS

lucContour* _lucContour_New(  LUCCONTOUR_DEFARGS  );

void _lucContour_Delete( void* drawingObject ) ;
void _lucContour_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucContour_DefaultNew( Name name ) ;
void _lucContour_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucContour_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;
#endif

