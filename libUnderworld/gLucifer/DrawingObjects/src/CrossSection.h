/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __lucCrossSection_h__
#define __lucCrossSection_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/MinMax.hpp>

struct lucCrossSection_cppdata
{
    Fn::Function::func func;
    std::shared_ptr<Fn::MinMax> fn;
};

void _lucCrossSection_SetFn( void* _self, Fn::Function* fn );

}

extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include <gLucifer/Base/Base.h>

#include "types.h"


/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucCrossSection                    \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucDrawingObject                  \
      /* Virtual functions go here */           \
      /* Other info */                          \
      Mesh*                mesh;                \
      double               value;               \
      Index                defaultResolution;   \
      Index                resolutionA;         \
      Index                resolutionB;         \
      Bool                 onMesh;              \
      XYZ                  normal;              \
      XYZ                  coord1;              \
      XYZ                  coord2;              \
      XYZ                  coord3;              \
      Axis                 axis;                \
      Axis                 axis1;               \
      Axis                 axis2;               \
      Bool                 isSet;               \
      Bool                 interpolate;         \
      Bool                 axisAligned;         \
      Bool                 gatherData;          \
      Bool                 offsetEdges;         \
      float***             vertices;            \
      float***             values;              \
      Coord                min;                 \
      Coord                max;                 \
      unsigned             fieldComponentCount; \
      void*                cppdata;             \
      unsigned             dim;                 \
      ExtensionInfo_Index  vertexGridHandle;    \
      int                  dims[3];             \
      int                  fieldDim;            \
 
struct lucCrossSection
{
   __lucCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCROSSSECTION_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCCROSSSECTION_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucCrossSection* _lucCrossSection_New(  LUCCROSSSECTION_DEFARGS  );

void _lucCrossSection_Delete( void* drawingObject ) ;
void _lucCrossSection_Print( void* drawingObject, Stream* stream ) ;

void* _lucCrossSection_DefaultNew( Name name ) ;
void _lucCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucCrossSection_Build( void* drawingObject, void* data ) ;
void _lucCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucCrossSection_Execute( void* drawingObject, void* data );
void _lucCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context );
void _lucCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );
void lucCrossSection_Interpolate2d(void* crossSection, double factorA, double factorB, XYZ coord);
void lucCrossSection_SetPosition(void* crossSection);
double lucCrossSection_GetValue(void* crossSection, double min, double max);
lucCrossSection* lucCrossSection_Set(void* crossSection, double val, Axis axis, Bool interpolate);
lucCrossSection* lucCrossSection_Slice(void* crossSection, double val, Bool interpolate);

void lucCrossSection_AllocateSampleData(void* drawingObject, int dims);
void lucCrossSection_SampleField(void* drawingObject, Bool reverse);
void lucCrossSection_SampleMesh( void* drawingObject, Bool reverse);
void lucCrossSection_FreeSampleData(void* drawingObject);

#ifdef __cplusplus
}
#endif

#endif

