/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucSwarmVectors_h__
#define __lucSwarmVectors_h__

#include "SwarmViewer.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucSwarmVectors_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucSwarmVectors \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucSwarmViewer \
		/* Virtual functions go here */ \
		/* Other info */\
		SwarmVariable*                                     directionVariable;           \
		Name                                               directionVariableName;       \
		double                                             arrowHeadSize;               \
		SwarmVariable*                                     thicknessVariable;           \
		Name                                               thicknessVariableName;       \
		double                                             thickness;                   \
		SwarmVariable*                                     lengthVariable;              \
		Name                                               lengthVariableName;          \
		double                                             length;                      \
 
struct lucSwarmVectors
{
   __lucSwarmVectors
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSWARMVECTORS_DEFARGS \
                LUCSWARMVIEWER_DEFARGS

#define LUCSWARMVECTORS_PASSARGS \
                LUCSWARMVIEWER_PASSARGS

lucSwarmVectors* _lucSwarmVectors_New(  LUCSWARMVECTORS_DEFARGS  );

void _lucSwarmVectors_Delete( void* drawingObject ) ;
void _lucSwarmVectors_Print( void* drawingObject, Stream* stream ) ;
void* _lucSwarmVectors_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;

/* 'Stg_Component' implementations */
void* _lucSwarmVectors_DefaultNew( Name name ) ;
void _lucSwarmVectors_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucSwarmVectors_Build( void* drawingObject, void* data ) ;
void _lucSwarmVectors_Initialise( void* drawingObject, void* data ) ;
void _lucSwarmVectors_Execute( void* drawingObject, void* data );
void _lucSwarmVectors_Destroy( void* drawingObject, void* data ) ;

/* Drawing Object Implementations */
void _lucSwarmVectors_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;

/* SwarmViewer Base Implementations */
void _lucSwarmVectors_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I ) ;

void lucSwarmVectors_UpdateVariables( void* drawingObject ) ;
#endif

