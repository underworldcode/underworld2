/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucSwarmShapes_h__
#define __lucSwarmShapes_h__

#include "SwarmViewer.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucSwarmShapes_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucSwarmShapes \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucSwarmViewer \
		/* Virtual functions go here */ \
		/* Other info */\
		/* Colour Stuff */ \
		SwarmVariable*                                     planeVectorVariable;          \
		Name                                               planeVectorVariableName;      \
		SwarmVariable*                                     lengthVariable;               \
		Name                                               lengthVariableName;           \
		SwarmVariable*                                     widthVariable;                \
		Name                                               widthVariableName;            \
		SwarmVariable*                                     heightVariable;               \
		Name                                               heightVariableName;           \
      double                                             length;                       \
      double                                             width;                        \
      double                                             height;                       \
      Bool                                               square;                       \
 
struct lucSwarmShapes
{
   __lucSwarmShapes
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSwarmShapes_DEFARGS \
                LUCSWARMVIEWER_DEFARGS

#define LUCSwarmShapes_PASSARGS \
                LUCSWARMVIEWER_PASSARGS

lucSwarmShapes* _lucSwarmShapes_New(  LUCSwarmShapes_DEFARGS  );

void _lucSwarmShapes_Delete( void* drawingObject ) ;
void _lucSwarmShapes_Print( void* drawingObject, Stream* stream ) ;
void* _lucSwarmShapes_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;

/* 'Stg_Component' implementations */
void* _lucSwarmShapes_DefaultNew( Name name ) ;
void _lucSwarmShapes_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucSwarmShapes_Build( void* drawingObject, void* data ) ;
void _lucSwarmShapes_Initialise( void* drawingObject, void* data ) ;
void _lucSwarmShapes_Execute( void* drawingObject, void* data );
void _lucSwarmShapes_Destroy( void* drawingObject, void* data ) ;

void _lucSwarmShapes_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;

/* SwarmViewer Implementations */
void _lucSwarmShapes_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I ) ;

void lucSwarmShapes_UpdateVariables( void* drawingObject ) ;
#endif

