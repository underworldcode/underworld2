/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucMeshViewer_h__
#define __lucMeshViewer_h__

/* Textual name of this class - This is a global pointer which is used for
   times when you need to refer to class and not a particular instance of a class */
extern const Type lucMeshViewer_Type;

/* Class contents - this is defined as a macro so that sub-classes of
   this class can use this macro at the start of the definition of their struct */
#define __lucMeshViewer                     \
      /* Macro defining parent goes here - This means you can */   \
      /* cast this class as its parent */               \
      __lucDrawingObject                                \
      /* Virtual functions go here */                   \
      Mesh*                           mesh;             \
      /* Other info */                                  \
      /* Node Colour */\
      FieldVariable*                  colourVariable;         \
      /* Node Size */\
      FieldVariable*                  sizeVariable;           \
      /* Stg_Class info */                              \
      unsigned                        nEdges;           \
      unsigned**                      edges;            \
                                                        \
      Bool                            nodeNumbers;      \
      Bool                            elementNumbers;   \
      Bool                            displayNodes;     \
      Bool                            displayEdges;     \
      float                           pointSize;        \
      Bool                            pointSmooth;      \
      unsigned                        segments;
 
struct lucMeshViewer
{
   __lucMeshViewer
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCMESHVIEWER_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCMESHVIEWER_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucMeshViewer* _lucMeshViewer_New(  LUCMESHVIEWER_DEFARGS  );

void _lucMeshViewer_Delete( void* drawingObject ) ;
void _lucMeshViewer_Print( void* drawingObject, Stream* stream ) ;
void* _lucMeshViewer_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;

/* 'Stg_Component' implementations */
void* _lucMeshViewer_DefaultNew( Name name ) ;
void _lucMeshViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucMeshViewer_Build( void* drawingObject, void* data ) ;
void _lucMeshViewer_Initialise( void* drawingObject, void* data ) ;
void _lucMeshViewer_Execute( void* drawingObject, void* data );
void _lucMeshViewer_Destroy( void* drawingObject, void* data ) ;

void _lucMeshViewer_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;
void _lucMeshViewer_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

void lucMeshViewer_RenderEdges( lucMeshViewer* self, lucDatabase* database);
void lucMeshViewer_PrintAllElementsNumber( void* drawingObject, lucDatabase* database);

#endif

