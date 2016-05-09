/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __lucWindow_h__
#define __lucWindow_h__

struct lucViewport;
#include "types.h"

typedef void (lucWindow_DisplayFunction) ( void* object );
typedef int (lucWindow_EventsWaitingFunction) ( void* object );
typedef Bool (lucWindow_EventProcessorFunction) ( void* object );
typedef void (lucWindow_ResizeFunction) ( void* object );

extern lucDatabase* defaultDatabase;

extern const Type lucWindow_Type;

#define __lucWindow                                                     \
      __Stg_Component                                                   \
      /* Virtual Functions */                                           \
      lucWindow_DisplayFunction*          _displayWindow;               \
                                                                        \
      /* Other Info */                                                  \
      lucViewport**                       viewportList;                 \
      Viewport_Index                      viewportCount;                \
      int*                                viewportLayout;               \
      Pixel_Index                         width;                        \
      Pixel_Index                         height;                       \
      DomainContext*                      context;                      \
      Bool                                isMaster;                     \
      char*                               title;                        \
      Bool                                antialias;                    \
      lucDatabase*                        database;                     \
      Bool                                useModelBounds;               \
      Bool                                disabled;                     \
   
struct lucWindow
{
   __lucWindow
};


#ifndef ZERO
#define ZERO 0
#endif

#define LUCWINDOW_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                lucWindow_DisplayFunction*          _displayWindow

#define LUCWINDOW_PASSARGS \
                STG_COMPONENT_PASSARGS, \
           _displayWindow

lucWindow* _lucWindow_New(  LUCWINDOW_DEFARGS  );

void _lucWindow_Delete( void* window ) ;
void _lucWindow_Print( void* window, Stream* stream ) ;
void* _lucWindow_Copy( void* window, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;

/* Stg_Component Virtual Functions */
void* _lucWindow_DefaultNew( Name name ) ;
void _lucWindow_AssignFromXML( void* window, Stg_ComponentFactory* cf, void* data ) ;
void _lucWindow_Build( void* window, void* data ) ;
void _lucWindow_Initialise( void* window, void* data ) ;
void _lucWindow_Execute( void* window, void* data ) ;
void _lucWindow_Destroy( void* window, void* data ) ;

/* Window Virtual Functions */
void lucWindow_Display( void* window );

/* +++ Public Functions +++ */
void lucWindow_CleanUp( void* window ) ;

lucViewport** lucWindow_ConstructViewportList(
   lucWindow* self,
   Stg_ComponentFactory* cf,
   Viewport_Index* viewportCount,
   void* data ) ;

#endif

