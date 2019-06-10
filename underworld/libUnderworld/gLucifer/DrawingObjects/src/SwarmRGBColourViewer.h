/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucSwarmRGBColourViewer_h__
#define __lucSwarmRGBColourViewer_h__

#include "SwarmViewer.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucSwarmRGBColourViewer_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucSwarmRGBColourViewer \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucSwarmViewer \
      /* Virtual functions go here */ \
      /* Other info */\
      /* Colour Info */ \
      lucColourMap*                       redColourMap;              \
      lucColourMap*                       greenColourMap;            \
      lucColourMap*                       blueColourMap;             \
      Name                                colourRedVariableName;     \
      SwarmVariable*                      colourRedVariable;         \
      Name                                colourGreenVariableName;   \
      SwarmVariable*                      colourGreenVariable;       \
      Name                                colourBlueVariableName;    \
      SwarmVariable*                      colourBlueVariable;        \
   
struct lucSwarmRGBColourViewer
{
   __lucSwarmRGBColourViewer
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSWARMRGBCOLOURVIEWER_DEFARGS \
                LUCSWARMVIEWER_DEFARGS

#define LUCSWARMRGBCOLOURVIEWER_PASSARGS \
                LUCSWARMVIEWER_PASSARGS

lucSwarmRGBColourViewer* _lucSwarmRGBColourViewer_New(  LUCSWARMRGBCOLOURVIEWER_DEFARGS  );

void _lucSwarmRGBColourViewer_Delete( void* drawingObject ) ;
void _lucSwarmRGBColourViewer_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucSwarmRGBColourViewer_DefaultNew( Name name ) ;
void _lucSwarmRGBColourViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucSwarmRGBColourViewer_Build( void* drawingObject, void* data ) ;
void _lucSwarmRGBColourViewer_Initialise( void* drawingObject, void* data ) ;
void _lucSwarmRGBColourViewer_Execute( void* drawingObject, void* data );
void _lucSwarmRGBColourViewer_Destroy( void* drawingObject, void* data ) ;

void _lucSwarmRGBColourViewer_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;

/* SwarmViewer Implementations */
void _lucSwarmRGBColourViewer_SetParticleColour( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I ) ;

void lucSwarmRGBColourViewer_UpdateVariables( void* drawingObject ) ;
#endif

