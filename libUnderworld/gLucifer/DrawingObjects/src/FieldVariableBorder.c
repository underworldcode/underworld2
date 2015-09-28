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
#include "FieldVariableBorder.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucFieldVariableBorder_Type = "lucFieldVariableBorder";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucFieldVariableBorder* _lucFieldVariableBorder_New(  LUCFIELDVARIABLEBORDER_DEFARGS  )
{
   lucFieldVariableBorder*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucFieldVariableBorder) );
   self = (lucFieldVariableBorder*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucFieldVariableBorder_Init(
   lucFieldVariableBorder*                                      self,
   FieldVariable*                                               fieldVariable)
{
   self->fieldVariable = fieldVariable;
}

void _lucFieldVariableBorder_Delete( void* drawingObject )
{
   lucFieldVariableBorder*  self = (lucFieldVariableBorder*)drawingObject;

   _lucDrawingObject_Delete( self );
}

void _lucFieldVariableBorder_Print( void* drawingObject, Stream* stream )
{
   lucFieldVariableBorder*  self = (lucFieldVariableBorder*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucFieldVariableBorder_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap)
{
   lucFieldVariableBorder*  self = (lucFieldVariableBorder*)drawingObject;
   lucFieldVariableBorder* newDrawingObject;

   newDrawingObject = _lucDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newDrawingObject;
}


void* _lucFieldVariableBorder_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucFieldVariableBorder);
   Type                                                             type = lucFieldVariableBorder_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucFieldVariableBorder_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucFieldVariableBorder_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucFieldVariableBorder_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucFieldVariableBorder_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucFieldVariableBorder_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucFieldVariableBorder_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucFieldVariableBorder_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucFieldVariableBorder_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucFieldVariableBorder_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucFieldVariableBorder_New(  LUCFIELDVARIABLEBORDER_PASSARGS  );
}

void _lucFieldVariableBorder_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucFieldVariableBorder*  self = (lucFieldVariableBorder*)drawingObject;
   FieldVariable*           fieldVariable;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   fieldVariable =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FieldVariable", FieldVariable, True, data  );

   _lucFieldVariableBorder_Init(
      self,
      fieldVariable);

   /* Drawing settings for this component */
   self->lit = False;
   self->wireframe = True;
}

void _lucFieldVariableBorder_Build( void* drawingObject, void* data ) {}
void _lucFieldVariableBorder_Initialise( void* drawingObject, void* data ) {}
void _lucFieldVariableBorder_Execute( void* drawingObject, void* data ) {}
void _lucFieldVariableBorder_Destroy( void* drawingObject, void* data ) {}

void _lucFieldVariableBorder_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   /* Deprecated: border + borderColour now options on viewport */
}


