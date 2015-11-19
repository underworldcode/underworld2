/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucVectorArrowCrossSection_h__
#define __lucVectorArrowCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucVectorArrowCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucVectorArrowCrossSection \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
		double                                             maximum;                \
		Bool                                               dynamicRange;           \
 
struct lucVectorArrowCrossSection
{
   __lucVectorArrowCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCVECTORARROWCROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCVECTORARROWCROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucVectorArrowCrossSection* _lucVectorArrowCrossSection_New(  LUCVECTORARROWCROSSSECTION_DEFARGS  );

void _lucVectorArrowCrossSection_Delete( void* drawingObject ) ;
void _lucVectorArrowCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucVectorArrowCrossSection_DefaultNew( Name name ) ;
void _lucVectorArrowCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucVectorArrowCrossSection_Build( void* drawingObject, void* data ) ;
void _lucVectorArrowCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucVectorArrowCrossSection_Execute( void* drawingObject, void* data );
void _lucVectorArrowCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucVectorArrowCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );
void _lucVectorArrowCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database);

#endif

