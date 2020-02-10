/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __lucSampler_h__
#define __lucSampler_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/MinMax.hpp>

struct lucSampler_cppdata
{
    Fn::Function::func func;
    std::shared_ptr<Fn::MinMax> fn;
};

void _lucSampler_SetFn( void* _self, Fn::Function* fn );

}

extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include <gLucifer/Base/Base.h>

#include "types.h"


/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucSampler_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucSampler                    \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucDrawingObject                  \
      /* Virtual functions go here */           \
      /* Other info */                          \
      Mesh*                mesh;                \
      unsigned             fieldComponentCount; \
      void*                cppdata;             \
      unsigned             dim;                 \
      int                  fieldDim;            \
 
struct lucSampler
{
   __lucSampler
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSAMPLER_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCSAMPLER_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucSampler* _lucSampler_New(  LUCSAMPLER_DEFARGS  );

void _lucSampler_Delete( void* drawingObject ) ;
void _lucSampler_Print( void* drawingObject, Stream* stream ) ;

void* _lucSampler_DefaultNew( Name name ) ;
void _lucSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucSampler_Build( void* drawingObject, void* data ) ;
void _lucSampler_Initialise( void* drawingObject, void* data ) ;
void _lucSampler_Execute( void* drawingObject, void* data );
void _lucSampler_Destroy( void* drawingObject, void* data ) ;

void _lucSampler_Setup( void* drawingObject, lucDatabase* database, void* _context );
void _lucSampler_Draw( void* drawingObject, lucDatabase* database, void* _context );

void lucSampler_SampleField(void* drawingObject, float* vertices, int V, float* values, int N);

#ifdef __cplusplus
}
#endif

#endif

