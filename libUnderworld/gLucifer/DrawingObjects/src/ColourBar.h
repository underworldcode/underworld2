/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __lucColourBar_h__
#define __lucColourBar_h__

#include <gLucifer/Base/DrawingObject.h>

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucColourBar_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucColourBar \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucDrawingObject \
      /* Virtual functions go here */ \
      /* Other info */\
      double                                              lengthFactor;          \
      Pixel_Index                                         height;                \
      Pixel_Index                                         margin;                \
      int                                                 borderWidth;           \
      int                                                 precision;             \
      Bool                                                scientific;            \
      int                                                 ticks;                 \
      Bool                                                printTickValue;        \
      Bool                                                printUnits;            \
      double                                              scaleValue;            \
      double                                              tickValues[11];        \

struct lucColourBar
{
   __lucColourBar
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCOLOURBAR_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCCOLOURBAR_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucColourBar* _lucColourBar_New(  LUCCOLOURBAR_DEFARGS  );

void _lucColourBar_Delete( void* drawingObject ) ;
void _lucColourBar_Print( void* drawingObject, Stream* stream ) ;
void* _lucColourBar_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;

/* 'Stg_Component' implementations */
void* _lucColourBar_DefaultNew( Name name ) ;
void _lucColourBar_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucColourBar_Build( void* drawingObject, void* data ) ;
void _lucColourBar_Initialise( void* drawingObject, void* data ) ;
void _lucColourBar_Execute( void* drawingObject, void* data );
void _lucColourBar_Destroy( void* drawingObject, void* data ) ;

void _lucColourBar_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

#endif

