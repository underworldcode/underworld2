/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucViewport_h__
#define __lucViewport_h__

#include "Base.h"

extern const Type lucViewport_Type;

#define __lucViewport                                          \
      __Stg_Component                                          \
      AbstractContext*              context;                   \
      Bool                          needsToDraw;               \
      lucCamera*                    camera;                    \
      lucDrawingObject_Register*    drawingObject_Register;    \
      Pixel_Index                   margin;                    \
      double                        nearClipPlane;             \
      double                        farClipPlane;              \
      double                        scaleX;                    \
      double                        scaleY;                    \
      double                        scaleZ;                    \
      char*                         title;                     \
      Bool                          axis;                      \
      double                        axisLength;                \
      int                           border;                    \
      Bool                          antialias;                 \
      Bool                          rulers;                    \
      Bool                          timestep;                  \
      lucColour                     borderColour;              \
      char*                         properties;                \

struct lucViewport
{
   __lucViewport
};

#ifndef ZERO
#define ZERO 0
#endif

#define LUCVIEWPORT_DEFARGS \
                STG_COMPONENT_DEFARGS

#define LUCVIEWPORT_PASSARGS \
                STG_COMPONENT_PASSARGS

#include "Database.h"

lucViewport* _lucViewport_New(  LUCVIEWPORT_DEFARGS  );

void _lucViewport_Delete( void* viewport ) ;
void _lucViewport_Print( void* viewport, Stream* stream ) ;

void* _lucViewport_DefaultNew( Name name ) ;
void _lucViewport_AssignFromXML( void* viewport, Stg_ComponentFactory* cf, void* data ) ;
void _lucViewport_Build( void* viewport, void* data );
void _lucViewport_Initialise( void* viewport, void* data );
void _lucViewport_Execute( void* viewport, void* data );
void _lucViewport_Destroy( void* viewport, void* data );

/* +++ Public Functions +++ */
void lucViewport_Update( void* viewport);
void lucViewport_Setup( void* viewport, lucDatabase* database) ;
void lucViewport_Draw( void* viewport, lucDatabase* database) ;
void lucViewport_CleanUp( void* viewport ) ;
#endif

