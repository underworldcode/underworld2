/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucScalarFieldOnMesh_h__
#define __lucScalarFieldOnMesh_h__

#include "ScalarFieldOnMeshCrossSection.h"

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucScalarFieldOnMesh_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucScalarFieldOnMesh \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucScalarFieldOnMeshCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
      Bool  drawSides[3][2];  \
 
struct lucScalarFieldOnMesh
{
   __lucScalarFieldOnMesh
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSCALARFIELDONMESH_DEFARGS \
                LUCSCALARFIELDONMESHCROSSSECTION_DEFARGS

#define LUCSCALARFIELDONMESH_PASSARGS \
                LUCSCALARFIELDONMESHCROSSSECTION_PASSARGS

lucScalarFieldOnMesh* _lucScalarFieldOnMesh_New(  LUCSCALARFIELDONMESH_DEFARGS  );

void _lucScalarFieldOnMesh_Delete( void* drawingObject ) ;
void _lucScalarFieldOnMesh_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucScalarFieldOnMesh_DefaultNew( Name name ) ;
void _lucScalarFieldOnMesh_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucScalarFieldOnMesh_Build( void* drawingObject, void* data );
void _lucScalarFieldOnMesh_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

