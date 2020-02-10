/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucHistoricalSwarmTrajectory_h__
#define __lucHistoricalSwarmTrajectory_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucHistoricalSwarmTrajectory_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucHistoricalSwarmTrajectory \
		/* Parent info */ \
		__lucDrawingObject \
		/* Virtual functions go here */ \
		/* Other info */\
		Swarm*                              swarm;                  \
		ExtensionInfo_Index                 particleIdExtHandle;    \
		/* Other Stuff */ \
      Bool                                flat;                   \
      float                               scaling;                \
      float                               arrowHead;              \
      unsigned int                        steps;                  \
      double                              time;                   \

struct lucHistoricalSwarmTrajectory
{
   __lucHistoricalSwarmTrajectory
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCHISTORICALSWARMTRAJECTORY_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCHISTORICALSWARMTRAJECTORY_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucHistoricalSwarmTrajectory* _lucHistoricalSwarmTrajectory_New(  LUCHISTORICALSWARMTRAJECTORY_DEFARGS  );

void _lucHistoricalSwarmTrajectory_Delete( void* drawingObject ) ;
void _lucHistoricalSwarmTrajectory_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucHistoricalSwarmTrajectory_DefaultNew( Name name ) ;
void _lucHistoricalSwarmTrajectory_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucHistoricalSwarmTrajectory_Build( void* drawingObject, void* data ) ;
void _lucHistoricalSwarmTrajectory_Initialise( void* drawingObject, void* data ) ;
void _lucHistoricalSwarmTrajectory_Execute( void* drawingObject, void* data );
void _lucHistoricalSwarmTrajectory_Destroy( void* drawingObject, void* data ) ;

void _lucHistoricalSwarmTrajectory_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

#endif

