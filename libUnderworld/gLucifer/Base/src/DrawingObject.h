/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucDrawingObject_h__
#define __lucDrawingObject_h__

struct lucDatabase;

#include "types.h"

extern const Type lucDrawingObject_Type;

typedef void (lucDrawingObject_SetupFunction) ( void* object, lucDatabase* database, void* context );
typedef void (lucDrawingObject_DrawFunction)  ( void* object, lucDatabase* database, void* context );
typedef void (lucDrawingObject_CleanUpFunction)  ( void* object );

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucDrawingObject                           \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __Stg_Component                                   \
      AbstractContext*               context;             \
      /* Virtual Functions */ \
      lucDrawingObject_SetupFunction*                    _setup;              \
      lucDrawingObject_DrawFunction*                     _draw;               \
      lucDrawingObject_CleanUpFunction*                  _cleanUp;            \
      /* Other Info */ \
      Bool                                               needsToCleanUp;      \
      Bool                                               needsToDraw;         \
      Bool                                               depthSort;           \
      Coord                                              position;            \
      /* Drawing settings */ \
      Bool                                               disabled;            \
      lucColourMap*                                      colourMap;           \
      lucColour                                          colour;              \
      float                                              opacity;             \
      int                                                id;                  \
      char*                                              properties;          \
      MPI_Comm                                           comm;                \
      int                                                rank;                \
      int                                                nproc;

struct lucDrawingObject
{
   __lucDrawingObject
};


#ifndef ZERO
#define ZERO 0
#endif

#define LUCDRAWINGOBJECT_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                lucDrawingObject_SetupFunction*      _setup, \
                lucDrawingObject_DrawFunction*        _draw, \
                lucDrawingObject_CleanUpFunction*  _cleanUp

#define LUCDRAWINGOBJECT_PASSARGS \
                STG_COMPONENT_PASSARGS, \
           _setup,   \
           _draw,    \
           _cleanUp

lucDrawingObject* _lucDrawingObject_New(  LUCDRAWINGOBJECT_DEFARGS  );


void _lucDrawingObject_Delete( void* drawingObject ) ;
void _lucDrawingObject_Print( void* drawingObject, Stream* stream ) ;
void* _lucDrawingObject_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;

void* _lucDrawingObject_DefaultNew( Name name );
void _lucDrawingObject_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data ) ;
void _lucDrawingObject_Build( void* camera, void* data );
void _lucDrawingObject_Initialise( void* camera, void* data );
void _lucDrawingObject_Execute( void* camera, void* data );
void _lucDrawingObject_Destroy( void* camera, void* data );

/* +++ Public Functions +++ */
void lucDrawingObject_Setup( void* drawingObject, lucDatabase* database, void* context ) ;
void lucDrawingObject_Draw( void* drawingObject, lucDatabase* database, void* context ) ;
void lucDrawingObject_CleanUp( void* drawingObject ) ;

typedef enum { GreaterThan, LessThan, EqualTo } lucDrawingObjectMask_Type;
typedef struct
{
   lucDrawingObjectMask_Type type;
   double                    value;
   double                    tolerance;
} lucDrawingObjectMask;

void lucDrawingObjectMask_Construct( lucDrawingObjectMask* self, Name drawingObjectName, Stg_ComponentFactory* cf, void* data ) ;
Bool lucDrawingObjectMask_Test( lucDrawingObjectMask* self, double value ) ;

/* HACK - a function to check whether a field is an FeVariable or not before it does an FeVariable_SyncShadowValues */
void lucDrawingObject_SyncShadowValues( void* drawingObject, void* field );

void lucDrawingObject_SetProperties(void* drawingObject, char *props);
#endif

