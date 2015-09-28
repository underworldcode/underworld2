/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "MeshCrossSection.h"

#ifndef __lucScalarFieldOnMeshCrossSection_h__
#define __lucScalarFieldOnMeshCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucScalarFieldOnMeshCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucScalarFieldOnMeshCrossSection \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucMeshCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
    Bool                    flipNormals;            \
 
struct lucScalarFieldOnMeshCrossSection
{
   __lucScalarFieldOnMeshCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSCALARFIELDONMESHCROSSSECTION_DEFARGS \
                LUCMESHCROSSSECTION_DEFARGS

#define LUCSCALARFIELDONMESHCROSSSECTION_PASSARGS \
                LUCMESHCROSSSECTION_PASSARGS

lucScalarFieldOnMeshCrossSection* _lucScalarFieldOnMeshCrossSection_New(  LUCSCALARFIELDONMESHCROSSSECTION_DEFARGS  );

void _lucScalarFieldOnMeshCrossSection_Delete( void* drawingObject ) ;
void _lucScalarFieldOnMeshCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucScalarFieldOnMeshCrossSection_DefaultNew( Name name ) ;
void _lucScalarFieldOnMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucScalarFieldOnMeshCrossSection_Build( void* drawingObject, void* data ) ;
void _lucScalarFieldOnMeshCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucScalarFieldOnMeshCrossSection_Execute( void* drawingObject, void* data );
void _lucScalarFieldOnMeshCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucScalarFieldOnMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );

void lucScalarFieldOnMeshCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing) ;

#endif

