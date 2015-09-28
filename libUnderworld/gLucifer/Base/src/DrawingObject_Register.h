/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __lucDrawingObject_Register_h__
#define __lucDrawingObject_Register_h__

struct lucDatabase;
#include "types.h"
extern const Type lucDrawingObject_Register_Type;

#define __lucDrawingObject_Register \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__NamedObject_Register \
		\
		/* Virtual functions go here */ \
		\
		/* Class info */ \
 
struct lucDrawingObject_Register
{
   __lucDrawingObject_Register
};


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

lucDrawingObject_Register*	lucDrawingObject_Register_New( void );

/*-----------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

/*-----------------------------------------------------------------------------------------------------------------
** Public functions
*/
#define lucDrawingObject_Register_Add NamedObject_Register_Add

#define lucDrawingObject_Register_GetIndex NamedObject_Register_GetIndex

#define lucDrawingObject_Register_GetByName( self, materialName ) \
		( (lucDrawingObject*) NamedObject_Register_GetByName( self, materialName ) )

#define lucDrawingObject_Register_GetByIndex( self, materialIndex ) \
		( (lucDrawingObject*) NamedObject_Register_GetByIndex( self, materialIndex ) )

#define lucDrawingObject_Register_GetCount( self ) \
		(self)->objects->count

#define lucDrawingObject_Register_DeleteAllObjects( self ) \
		Stg_ObjectList_DeleteAllObjects( (self)->objects )
#define lucDrawingObject_Register_PrintAllObjects( self, stream ) \
		Stg_ObjectList_PrintAllObjects( (self)->objects, stream )

/* +++ Public Functions +++ */
void lucDrawingObject_Register_SetupAll( void* drawingObject_Register, lucDatabase* database) ;
void lucDrawingObject_Register_DrawAll( void* drawingObject_Register, lucDatabase* database) ;
void lucDrawingObject_Register_CleanUpAll( void* drawingObject_Register ) ;


#endif
