/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucEigenvectors_h__
#define __lucEigenvectors_h__

#include "EigenvectorsCrossSection.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucEigenvectors_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucEigenvectors \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucEigenvectorsCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
		IJK                                                resolution;                            \
 
struct lucEigenvectors
{
   __lucEigenvectors
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCEIGENVECTORS_DEFARGS \
                LUCEIGENVECTORSCROSSSECTION_DEFARGS

#define LUCEIGENVECTORS_PASSARGS \
                LUCEIGENVECTORSCROSSSECTION_PASSARGS

lucEigenvectors* _lucEigenvectors_New(  LUCEIGENVECTORS_DEFARGS  );

void _lucEigenvectors_Delete( void* drawingObject ) ;
void _lucEigenvectors_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucEigenvectors_DefaultNew( Name name ) ;
void _lucEigenvectors_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucEigenvectors_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

