/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucFieldSampler_h__
#define __lucFieldSampler_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucFieldSampler_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucFieldSampler \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucCrossSection \
      /* Virtual functions go here */ \
      /* Other info */\
      IJK                           resolution;       \
      Index                         total;            \

struct lucFieldSampler
{
   __lucFieldSampler
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCFIELDSAMPLER_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCFIELDSAMPLER_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucFieldSampler* _lucFieldSampler_New(  LUCFIELDSAMPLER_DEFARGS  );

void _lucFieldSampler_Delete( void* drawingObject ) ;
void _lucFieldSampler_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucFieldSampler_DefaultNew( Name name ) ;
void _lucFieldSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucFieldSampler_Build( void* drawingObject, void* data ) ;
void _lucFieldSampler_Initialise( void* drawingObject, void* data ) ;
void _lucFieldSampler_Execute( void* drawingObject, void* data );
void _lucFieldSampler_Destroy( void* drawingObject, void* data ) ;

/* Drawing Object Implementations */
void _lucFieldSampler_Setup( void* drawingObject, lucDatabase* database, void* _context );
void _lucFieldSampler_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;
#endif

