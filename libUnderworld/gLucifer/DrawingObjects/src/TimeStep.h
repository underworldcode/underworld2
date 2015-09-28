/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __lucTimeStep_h__
#define __lucTimeStep_h__

extern const Type lucTimeStep_Type;

#define __lucTimeStep                              \
		__lucDrawingObject \
		Bool                                frame;   \
		Bool                                showtime;    \
		char*                               units;    \

struct lucTimeStep
{
   __lucTimeStep
};

#ifndef ZERO
#define ZERO 0
#endif

#define LUCTIMESTEP_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCTIMESTEP_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucTimeStep* _lucTimeStep_New(  LUCTIMESTEP_DEFARGS  );

void _lucTimeStep_Init(
   lucTimeStep*                                       self,
   Bool                                               frame,
   Bool                                               showtime,
   char*                                              units);


/** Virtual Functions */
void _lucTimeStep_Delete( void* timeStep ) ;
void _lucTimeStep_Print( void* timeStep, Stream* stream ) ;
void* _lucTimeStep_Copy( void* timeStep, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
void* _lucTimeStep_DefaultNew( Name name ) ;
void _lucTimeStep_AssignFromXML( void* timeStep, Stg_ComponentFactory* cf, void* data ) ;
void _lucTimeStep_Build( void* timeStep, void* data );
void _lucTimeStep_Initialise( void* timeStep, void* data );
void _lucTimeStep_Execute( void* timeStep, void* data );
void _lucTimeStep_Destroy( void* timeStep, void* data );
void _lucTimeStep_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

