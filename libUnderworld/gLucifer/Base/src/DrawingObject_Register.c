/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "Base.h"
#include "types.h"
#include "Viewport.h"
#include "Viewport.h"
#include "ColourMap.h"
#include "Window.h"
#include "DrawingObject_Register.h"
#include "DrawingObject.h"

#include "Database.h"

const Type lucDrawingObject_Register_Type = "lucDrawingObject_Register";


lucDrawingObject_Register*	lucDrawingObject_Register_New( void )
{
   /* Variables set in this function */
   SizeT                      _sizeOfSelf = sizeof(lucDrawingObject_Register);
   Type                              type = lucDrawingObject_Register_Type;
   Stg_Class_DeleteFunction*      _delete = _NamedObject_Register_Delete;
   Stg_Class_PrintFunction*        _print = _NamedObject_Register_Print;
   Stg_Class_CopyFunction*          _copy = _NamedObject_Register_Copy;

   lucDrawingObject_Register* self;

   self = (lucDrawingObject_Register*) _NamedObject_Register_New(  NAMEDOBJECT_REGISTER_PASSARGS  );

   return self;
}

void lucDrawingObject_Register_SetupAll( void* drawingObject_Register, lucDatabase* database )
{
   lucDrawingObject_Register* self           = (lucDrawingObject_Register*) drawingObject_Register;
   DrawingObject_Index        object_I;
   DrawingObject_Index        objectCount    = lucDrawingObject_Register_GetCount( self );
   lucDrawingObject*          object;

   for ( object_I = 0 ; object_I < objectCount ; object_I++ )
   {
      object = lucDrawingObject_Register_GetByIndex( self, object_I );

      /* Ensure setup has been called to prepare for rendering (used to set position information) */
      if ( !object->disabled)
      {
         object->_setup( object, database, database->context );
         object->needsToCleanUp = True;
         object->needsToDraw = True;
      }
   }
}

void lucDrawingObject_Register_DrawAll( void* drawingObject_Register, lucDatabase* database)
{
   lucDrawingObject_Register* self          = (lucDrawingObject_Register*) drawingObject_Register;
   DrawingObject_Index        object_I;
   DrawingObject_Index        objectCount   = lucDrawingObject_Register_GetCount( self );
   lucDrawingObject*          object;

   for ( object_I = 0 ; object_I < objectCount ; object_I++ )
   {
      object = lucDrawingObject_Register_GetByIndex( self, object_I );
      lucDrawingObject_Draw( object, database, database->context );
      lucDatabase_OutputGeometry(database, object->id);
   }
}

void lucDrawingObject_Register_CleanUpAll( void* drawingObject_Register )
{
   lucDrawingObject_Register* self          = (lucDrawingObject_Register*) drawingObject_Register;
   DrawingObject_Index        object_I;
   DrawingObject_Index        objectCount   = lucDrawingObject_Register_GetCount( self );
   lucDrawingObject*          object;

   for ( object_I = 0 ; object_I < objectCount ; object_I++ )
   {
      object = lucDrawingObject_Register_GetByIndex( self, object_I );
      if (object->needsToCleanUp )
         object->_cleanUp( object );
   }
}

