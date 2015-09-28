/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucScalarFieldCrossSection_h__
#define __lucScalarFieldCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucScalarFieldCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucScalarFieldCrossSection \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
 
struct lucScalarFieldCrossSection
{
   __lucScalarFieldCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCSCALARFIELDCROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCSCALARFIELDCROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucScalarFieldCrossSection* _lucScalarFieldCrossSection_New(  LUCSCALARFIELDCROSSSECTION_DEFARGS  );

void _lucScalarFieldCrossSection_Delete( void* drawingObject ) ;
void _lucScalarFieldCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucScalarFieldCrossSection_DefaultNew( Name name ) ;
void _lucScalarFieldCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucScalarFieldCrossSection_Build( void* drawingObject, void* data ) ;
void _lucScalarFieldCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucScalarFieldCrossSection_Execute( void* drawingObject, void* data );
void _lucScalarFieldCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucScalarFieldCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );

void lucScalarFieldCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing);

#endif

