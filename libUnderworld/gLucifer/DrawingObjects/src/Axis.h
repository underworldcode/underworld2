/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __lucAxis_h__
#define __lucAxis_h__

extern const Type lucAxis_Type;

#define __lucAxis                              \
      __lucDrawingObject \
      Coord                               origin;\
      float                               length;\
      Bool                                axes[3];\
      char*                               labels[3];\
      lucColour                           colours[3];\

struct lucAxis
{
   __lucAxis
};

#ifndef ZERO
#define ZERO 0
#endif

#define LUCAXIS_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCAXIS_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucAxis* _lucAxis_New(  LUCAXIS_DEFARGS  );

/** Virtual Functions */
void _lucAxis_Delete( void* axis ) ;
void _lucAxis_Print( void* axis, Stream* stream ) ;
void* _lucAxis_DefaultNew( Name name ) ;
void _lucAxis_AssignFromXML( void* axis, Stg_ComponentFactory* cf, void* data ) ;
void _lucAxis_Build( void* axis, void* data );
void _lucAxis_Initialise( void* axis, void* data );
void _lucAxis_Execute( void* axis, void* data );
void _lucAxis_Destroy( void* axis, void* data );
void _lucAxis_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

