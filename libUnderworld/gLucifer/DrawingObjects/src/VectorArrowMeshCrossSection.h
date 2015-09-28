/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "MeshCrossSection.h"

#ifndef __lucVectorArrowMeshCrossSection_h__
#define __lucVectorArrowMeshCrossSection_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucVectorArrowMeshCrossSection_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucVectorArrowMeshCrossSection \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucMeshCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
		double		maximum;								\
		Bool		  dynamicRange;					  \
    float		  scaling;								\
		float		  arrowHead;							\
 
struct lucVectorArrowMeshCrossSection
{
	 __lucVectorArrowMeshCrossSection
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCVECTORARROWMESHCROSSSECTION_DEFARGS \
								LUCMESHCROSSSECTION_DEFARGS

#define LUCVECTORARROWMESHCROSSSECTION_PASSARGS \
								LUCMESHCROSSSECTION_PASSARGS

lucVectorArrowMeshCrossSection* _lucVectorArrowMeshCrossSection_New(	LUCVECTORARROWMESHCROSSSECTION_DEFARGS	);

void _lucVectorArrowMeshCrossSection_Delete( void* drawingObject ) ;
void _lucVectorArrowMeshCrossSection_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucVectorArrowMeshCrossSection_DefaultNew( Name name ) ;
void _lucVectorArrowMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucVectorArrowMeshCrossSection_Build( void* drawingObject, void* data ) ;
void _lucVectorArrowMeshCrossSection_Initialise( void* drawingObject, void* data ) ;
void _lucVectorArrowMeshCrossSection_Execute( void* drawingObject, void* data );
void _lucVectorArrowMeshCrossSection_Destroy( void* drawingObject, void* data ) ;

void _lucVectorArrowMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context );
void _lucVectorArrowMeshCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database );

#endif

