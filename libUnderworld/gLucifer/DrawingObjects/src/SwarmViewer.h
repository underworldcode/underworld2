/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucSwarmViewer_h__
#define __lucSwarmViewer_h__

typedef void (lucSwarmViewer_PlotParticleFunction) ( void* object, lucDatabase* database, Particle_Index lParticle_I );
typedef void (lucSwarmViewer_SetParticleColourFunction) ( void* object, lucDatabase* database, Particle_Index lParticle_I );

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucSwarmViewer_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucSwarmViewer \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucDrawingObject \
      /* Virtual functions go here */ \
      lucSwarmViewer_PlotParticleFunction*           _plotParticle;          \
      lucSwarmViewer_SetParticleColourFunction*      _setParticleColour;     \
      /* Colour stuff */\
      Name                                               colourVariableName;     \
      SwarmVariable*                                     colourVariable;         \
      /* Size */\
      Name                                               sizeVariableName;       \
      SwarmVariable*                                     sizeVariable;           \
      /* Other info */\
      Swarm*                                             swarm;                  \
      /* Opacity Stuff */ \
      lucColourMap*                                      opacityColourMap;       \
      Name                                               opacityVariableName;    \
      SwarmVariable*                                     opacityVariable;        \
      /* Mask Info */ \
      Name                                               maskVariableName;       \
      SwarmVariable*                                     maskVariable;           \
      lucDrawingObjectMask                               mask;                   \
      /* Other Stuff */ \
      Bool                                               drawParticleNumber;     \
      Bool                                               sameParticleColour;     \
      int                                                subSample;              \
      Bool                                               positionRange;          \
      Coord                                              minPosition;            \
      Coord                                              maxPosition;            \
      lucGeometryType                                    geomType;               \
      float                                              scaling;                \
      float                                  pointSize; \
      Bool                                   pointSmoothing;

struct lucSwarmViewer
{
   __lucSwarmViewer
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSWARMVIEWER_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS, \
                lucSwarmViewer_PlotParticleFunction*            _plotParticle, \
                lucSwarmViewer_SetParticleColourFunction*  _setParticleColour

#define LUCSWARMVIEWER_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS, \
           _plotParticle,      \
           _setParticleColour

lucSwarmViewer* _lucSwarmViewer_New(  LUCSWARMVIEWER_DEFARGS  );

void _lucSwarmViewer_Delete( void* drawingObject ) ;
void _lucSwarmViewer_Print( void* drawingObject, Stream* stream ) ;
void* _lucSwarmViewer_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;
void* _lucSwarmViewer_DefaultNew( Name name );

/* 'Stg_Component' implementations */
void _lucSwarmViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucSwarmViewer_Build( void* drawingObject, void* data ) ;
void _lucSwarmViewer_Initialise( void* drawingObject, void* data ) ;
void _lucSwarmViewer_Execute( void* drawingObject, void* data );
void _lucSwarmViewer_Destroy( void* drawingObject, void* data ) ;

SwarmVariable* lucSwarmViewer_InitialiseVariable(void* object, Name variableName, Bool scalarRequired, void* data );
float lucSwarmViewer_GetScalar(SwarmVariable* variable, Particle_Index lParticle_I, float defaultVal);

void _lucSwarmViewer_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;
void _lucSwarmViewer_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

void lucSwarmViewBase_DrawParticleNumbers( void* drawingObject, void* _context ) ;

void lucSwarmViewer_UpdateVariables( void* drawingObject ) ;

void lucSwarmViewer_FindParticleLocalIndex(void *drawingObject, Coord coord, Particle_Index  *lParticle_I);

void lucSwarmViewer_SetColourComponent(void* object, lucDatabase* database, SwarmVariable* var, Particle_Index lParticle_I, lucGeometryDataType type, lucColourMap* colourMap);
void _lucSwarmViewer_SetParticleColour( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I ) ;

void _lucSwarmViewer_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I );
#endif

