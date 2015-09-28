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

#include <gLucifer/Base/Base.h>

#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
#include "Capture.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucCapture_Type = "lucCapture";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucCapture* _lucCapture_New(  LUCCAPTURE_DEFARGS  )
{
   lucCapture*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucCapture) );
   self = (lucCapture*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucCapture_Init(lucCapture* self, lucDatabase* database)
{
   self->database = database;
}

void* _lucCapture_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucCapture);
   Type                                                             type = lucCapture_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucDrawingObject_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucDrawingObject_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucCapture_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucCapture_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucDrawingObject_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucDrawingObject_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucDrawingObject_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucDrawingObject_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucCapture_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucCapture_New(  LUCCAPTURE_PASSARGS  );
}

void _lucCapture_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucCapture* self = (lucCapture*)drawingObject;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   _lucCapture_Init(self, 
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Database", lucDatabase, False, data )
      );
}


void _lucCapture_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   /* Placeholder */
}
