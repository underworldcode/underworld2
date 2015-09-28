/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "MeshCrossSection.h"

#ifndef __lucMeshSampler_h__
#define __lucMeshSampler_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucMeshSampler_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucMeshSampler \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucMeshCrossSection \
      /* Virtual functions go here */ \
      /* Calculated Values */ \
      Index                         total;            \
      Index                         elementRes[3];    \
      Coord                         cell;             \

struct lucMeshSampler
{
   __lucMeshSampler
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCMESHSAMPLER_DEFARGS \
                LUCMESHCROSSSECTION_DEFARGS

#define LUCMESHSAMPLER_PASSARGS \
                LUCMESHCROSSSECTION_PASSARGS

lucMeshSampler* _lucMeshSampler_New(  LUCMESHSAMPLER_DEFARGS  );

void _lucMeshSampler_Delete( void* drawingObject ) ;
void _lucMeshSampler_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucMeshSampler_DefaultNew( Name name ) ;
void _lucMeshSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucMeshSampler_Build( void* drawingObject, void* data ) ;
void _lucMeshSampler_Initialise( void* drawingObject, void* data ) ;
void _lucMeshSampler_Execute( void* drawingObject, void* data );
void _lucMeshSampler_Destroy( void* drawingObject, void* data ) ;

/* Drawing Object Implementations */
void _lucMeshSampler_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;
#endif

