/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __lucTitle_h__
#define __lucTitle_h__

extern const Type lucTitle_Type;

#define __lucTitle                              \
		__lucDrawingObject \
		char*                                              titleString;  \
		int                                                yPos;

struct lucTitle
{
   __lucTitle
};

#ifndef ZERO
#define ZERO 0
#endif

#define LUCTITLE_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCTITLE_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucTitle* _lucTitle_New(  LUCTITLE_DEFARGS  );

void lucTitle_Init(
   lucTitle*                                          self,
   char*                                              titleString,
   int                                                yPos );

/** Virtual Functions */
void _lucTitle_Delete( void* title ) ;
void _lucTitle_Print( void* title, Stream* stream ) ;
void* _lucTitle_DefaultNew( Name name ) ;
void _lucTitle_AssignFromXML( void* title, Stg_ComponentFactory* cf, void* data ) ;
void _lucTitle_Build( void* title, void* data );
void _lucTitle_Initialise( void* title, void* data );
void _lucTitle_Execute( void* title, void* data );
void _lucTitle_Destroy( void* title, void* data );
void _lucTitle_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

