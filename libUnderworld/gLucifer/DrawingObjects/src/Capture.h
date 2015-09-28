/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <gLucifer/Base/DrawingObject.h>

#ifndef __lucCapture_h__
#define __lucCapture_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucCapture_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucCapture                    \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucDrawingObject                  \
		/* Virtual functions go here */           \
		/* Other info */                          \
      lucDatabase* database;                        \
 
struct lucCapture
{
   __lucCapture
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCAPTURE_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCCAPTURE_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucCapture* _lucCapture_New(  LUCCAPTURE_DEFARGS  );

/* 'Stg_Component' implementations */
void* _lucCapture_DefaultNew( Name name ) ;
void _lucCapture_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucCapture_Draw( void* drawingObject, lucDatabase* database, void* _context );
#endif

