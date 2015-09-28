/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "CrossSection.h"

#ifndef __lucEigenvectorsCrossSection_h__
#define __lucEigenvectorsCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucEigenvectorsCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucEigenvectorsCrossSection                                                        \
		/* Macro defining parent goes here - This means you can cast this class as its parent */  \
		__lucCrossSection                                                                         \
		/* Virtual functions go here */                                                           \
		/* Other info */                                                                          \
		lucColour                                          colours[3];                            \
      float                                              scaling;                               \
      float                                              arrowHead;                             \
      int                                                glyphs;              \
		/* Colour used to display negative EigenValues */                                         \
		lucColour                                          colourForNegative[3];                  \
		/* Specifies if the eigenvalue is used to draw the vector - default true */               \
		Bool 						                              useEigenValue;                         \
		/* Specifies if the EigenVector and/or EigenValues are to be drawn */                     \
		/* Default is True for EigenVector, False for EigenValues */                              \
      Bool                                               plotEigenVector;                       \
		Bool                                               plotEigenValue;                        \
 

struct lucEigenvectorsCrossSection
{
   __lucEigenvectorsCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCEIGENVECTORSCROSSSECTION_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCEIGENVECTORSCROSSSECTION_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucEigenvectorsCrossSection* _lucEigenvectorsCrossSection_New(  LUCEIGENVECTORSCROSSSECTION_DEFARGS  );

void _lucEigenvectorsCrossSection_Delete( void* drawingObject ) ;
void _lucEigenvectorsCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucEigenvectorsCrossSection_DefaultNew( Name name ) ;
void _lucEigenvectorsCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucEigenvectorsCrossSection_Build( void* drawingObject, void* data ) ;
void _lucEigenvectorsCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucEigenvectorsCrossSection_Execute( void* drawingObject, void* data );
void _lucEigenvectorsCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucEigenvectorsCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );
void _lucEigenvectorsCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database );

#endif

