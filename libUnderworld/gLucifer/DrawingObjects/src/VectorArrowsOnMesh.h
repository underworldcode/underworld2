/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucVectorArrowsOnMesh_h__
#define __lucVectorArrowsOnMesh_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucVectorArrowsOnMesh_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucVectorArrowsOnMesh \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucVectorArrowMeshCrossSection \
		/* Virtual functions go here */ \
		/* Other info */\
    Index     elementRes[3];          \
 
struct lucVectorArrowsOnMesh
{
   __lucVectorArrowsOnMesh
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCVECTORARROWSONMESH_DEFARGS \
                LUCVECTORARROWMESHCROSSSECTION_DEFARGS

#define LUCVECTORARROWSONMESH_PASSARGS \
                LUCVECTORARROWMESHCROSSSECTION_PASSARGS

lucVectorArrowsOnMesh* _lucVectorArrowsOnMesh_New(  LUCVECTORARROWSONMESH_DEFARGS  );

void _lucVectorArrowsOnMesh_Delete( void* drawingObject ) ;
void _lucVectorArrowsOnMesh_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucVectorArrowsOnMesh_DefaultNew( Name name ) ;
void _lucVectorArrowsOnMesh_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );

void _lucVectorArrowsOnMesh_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

