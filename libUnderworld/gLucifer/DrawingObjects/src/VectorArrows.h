/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucVectorArrows_h__
#define __lucVectorArrows_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucVectorArrows_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucVectorArrows \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
      IJK                     resolution;   \
		  double                  maximum;                \
		  Bool                    dynamicRange;           \
 
struct lucVectorArrows
{
   __lucVectorArrows
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCVECTORARROWS_DEFARGS \
                LUCCROSSSECTION_DEFARGS

#define LUCVECTORARROWS_PASSARGS \
                LUCCROSSSECTION_PASSARGS

lucVectorArrows* _lucVectorArrows_New(  LUCVECTORARROWS_DEFARGS  );

void _lucVectorArrows_Delete( void* drawingObject ) ;
void _lucVectorArrows_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucVectorArrows_DefaultNew( Name name ) ;
void _lucVectorArrows_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucVectorArrows_Build( void* drawingObject, void* data ) ;
void _lucVectorArrows_Initialise( void* drawingObject, void* data ) ;
void _lucVectorArrows_Execute( void* drawingObject, void* data );
void _lucVectorArrows_Destroy( void* drawingObject, void* data ) ;

void _lucVectorArrows_Draw( void* drawingObject, lucDatabase* database, void* _context );

void _lucVectorArrows_DrawCrossSection( void* drawingObject, lucDatabase* database);
void _lucVectorArrows_DrawMeshCrossSection( void* drawingObject, lucDatabase* database );
#endif

