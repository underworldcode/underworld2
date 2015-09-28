/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucScalarField_h__
#define __lucScalarField_h__

#include "ScalarFieldCrossSection.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucScalarField_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucScalarField \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucScalarFieldCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
      Bool  drawSides[3][2];  \

struct lucScalarField
{
   __lucScalarField
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSCALARFIELD_DEFARGS \
                LUCSCALARFIELDCROSSSECTION_DEFARGS

#define LUCSCALARFIELD_PASSARGS \
                LUCSCALARFIELDCROSSSECTION_PASSARGS

lucScalarField* _lucScalarField_New(  LUCSCALARFIELD_DEFARGS  );

void _lucScalarField_Delete( void* drawingObject ) ;
void _lucScalarField_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucScalarField_DefaultNew( Name name ) ;
void _lucScalarField_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucScalarField_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

