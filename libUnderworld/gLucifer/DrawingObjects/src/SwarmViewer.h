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

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>

struct lucSwarmViewer_cppdata
{
    std::shared_ptr<Fn::MinMax>   fn_colour  = NULL;
    Fn::Function::func          func_colour;
    Fn::Function*                 fn_mask    = NULL;
    Fn::Function::func          func_mask;
    std::shared_ptr<Fn::MinMax>   fn_size    = NULL;
    Fn::Function::func          func_size;
    std::shared_ptr<Fn::MinMax>   fn_opacity = NULL;
    Fn::Function::func          func_opacity;
};

void _lucSwarmViewer_SetFn( void* _self, Fn::Function* fn_colour, Fn::Function* fn_mask, Fn::Function* fn_size, Fn::Function* fn_opacity );

}

extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include <gLucifer/Base/Base.h>

#include "types.h"

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
      /* Other info */\
      GeneralSwarm*                                      swarm;                  \
      /* Opacity Stuff */ \
      lucColourMap*                                      opacityColourMap;       \
      lucGeometryType                                    geomType;               \
      float                                              scaling;                \
      float                                              pointSize;              \
      void*                                              cppdata;                \

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

void _lucSwarmViewer_Setup( void* drawingObject, lucDatabase* database, void* _context ) ;
void _lucSwarmViewer_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

void lucSwarmViewer_SetColourComponent(void* object, lucDatabase* database, SwarmVariable* var, Particle_Index lParticle_I, lucGeometryDataType type, lucColourMap* colourMap);
void _lucSwarmViewer_SetParticleColour( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I ) ;

void _lucSwarmViewer_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I );


/* keep these dummy functions for now */
float lucSwarmViewer_GetScalar(SwarmVariable* variable, Particle_Index lParticle_I, float defaultVal);
SwarmVariable* lucSwarmViewer_InitialiseVariable(void* object, Name variableName, Bool scalarRequired, void* data );
void lucSwarmViewer_UpdateVariables( void* drawingObject );



#ifdef __cplusplus
}
#endif

#endif

