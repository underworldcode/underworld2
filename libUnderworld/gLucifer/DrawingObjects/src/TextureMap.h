/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucTextureMap_h__
#define __lucTextureMap_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucTextureMap_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucTextureMap \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucDrawingObject \
		/* Virtual functions go here */ \
		/* Other info */\
		Coord                                              bottomLeftCoord;  \
		Coord                                              bottomRightCoord; \
		Coord                                              topRightCoord;    \
		Coord                                              topLeftCoord;     \
		unsigned int                                       texture;          \
	
struct lucTextureMap
{
   __lucTextureMap
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCTEXTUREMAP_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCTEXTUREMAP_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucTextureMap* _lucTextureMap_New(  LUCTEXTUREMAP_DEFARGS  );

void _lucTextureMap_Delete( void* drawingObject ) ;
void _lucTextureMap_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucTextureMap_DefaultNew( Name name ) ;
void _lucTextureMap_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucTextureMap_Build( void* drawingObject, void* data ) ;
void _lucTextureMap_Initialise( void* drawingObject, void* data ) ;
void _lucTextureMap_Execute( void* drawingObject, void* data );
void _lucTextureMap_Destroy( void* drawingObject, void* data ) ;

void _lucTextureMap_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

#endif

