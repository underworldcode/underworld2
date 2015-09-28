/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucMeshCrossSection_h__
#define __lucMeshCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucMeshCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucMeshCrossSection \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __lucCrossSection \
      /* Virtual functions go here */ \
      /* Other info */\
      ExtensionInfo_Index     vertexGridHandle;       \
      int                     dims[3];                 \
      int                     fieldDim;               \
 
struct lucMeshCrossSection
{
   __lucMeshCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCMESHCROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCMESHCROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucMeshCrossSection* _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_DEFARGS  );

void _lucMeshCrossSection_Delete( void* drawingObject ) ;
void _lucMeshCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucMeshCrossSection_DefaultNew( Name name ) ;
void _lucMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucMeshCrossSection_Build( void* drawingObject, void* data ) ;
void _lucMeshCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucMeshCrossSection_Execute( void* drawingObject, void* data );
void _lucMeshCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );

void lucMeshCrossSection_Sample(void* drawingObject, Bool reverse);

#endif

