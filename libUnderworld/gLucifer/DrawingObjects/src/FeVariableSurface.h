/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucFeVariableSurface_h__
#define __lucFeVariableSurface_h__

#include "MeshCrossSection.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucFeVariableSurface_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucFeVariableSurface \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucMeshCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
      float                               scaling;                \

struct lucFeVariableSurface
{
   __lucFeVariableSurface
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCFEVARIABLESURFACE_DEFARGS \
                LUCMESHCROSSSECTION_DEFARGS

#define LUCFEVARIABLESURFACE_PASSARGS \
                LUCMESHCROSSSECTION_PASSARGS

lucFeVariableSurface* _lucFeVariableSurface_New(  LUCFEVARIABLESURFACE_DEFARGS  );

void _lucFeVariableSurface_Delete( void* drawingObject ) ;

/* 'Stg_Component' implementations */
void* _lucFeVariableSurface_DefaultNew( Name name ) ;
void _lucFeVariableSurface_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucFeVariableSurface_Build( void* drawingObject, void* data ) ;
void _lucFeVariableSurface_Initialise( void* drawingObject, void* data ) ;
void _lucFeVariableSurface_Execute( void* drawingObject, void* data );
void _lucFeVariableSurface_Destroy( void* drawingObject, void* data ) ;

void _lucFeVariableSurface_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;
#endif

