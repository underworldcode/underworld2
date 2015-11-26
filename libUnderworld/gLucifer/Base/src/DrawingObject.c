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
#include <StgFEM/StgFEM.h>
#include <stdarg.h>
#include <string.h>

#include "types.h"
#include "ColourMap.h"
#include "Viewport.h"
#include "Window.h"
#include "DrawingObject.h"
#include "Init.h"

#include <gLucifer/Base/Base.h>


#ifndef MASTER
#define MASTER 0
#endif

const Type lucDrawingObject_Type = "lucDrawingObject";

lucDrawingObject* _lucDrawingObject_New(  LUCDRAWINGOBJECT_DEFARGS  )
{
   lucDrawingObject*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucDrawingObject) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucDrawingObject*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   self->_setup   = _setup;
   self->_draw    = _draw;
   self->_cleanUp = _cleanUp;

   return self;
}

void _lucDrawingObject_Init( 
   lucDrawingObject*    self,
   AbstractContext*     context,
   char*                properties,
   Bool                 disabled,
   Name                 colourName,
   lucColourMap*        colourMap,
   float                opacity)
{
   self->isConstructed = True;
   self->context = context;

   /* Position for depth sort */
   self->depthSort = False;
   self->position[I_AXIS] = 0;
   self->position[J_AXIS] = 0;
   self->position[K_AXIS] = 0;

   self->needsToCleanUp = False;
   self->needsToDraw = True;
   self->disabled = disabled || (context && !self->context->vis);      /* Drawing disabled, defaults to false */
   lucColour_FromString( &self->colour, colourName );
   self->colourMap = colourMap;
   self->opacity = opacity;        /* Transparency override, -1 to disable */

   /* Write property string */
   self->properties = Memory_Alloc_Array(char, 4096, "properties");
   memset(self->properties, 0, 4086);
   strncpy(self->properties, properties, 4096);

   self->id = 0;
}

void _lucDrawingObject_Delete( void* drawingObject )
{
   lucDrawingObject* self        = drawingObject;

   Memory_Free(self->properties);

   _Stg_Component_Delete( self );
}

void _lucDrawingObject_Print( void* drawingObject, Stream* stream )
{
   lucDrawingObject* self        = drawingObject;

   Journal_Printf( stream, "lucDrawingObject: %s\n", self->name );

   Stream_Indent( stream );

   /* Print Parent */
   _Stg_Component_Print( self, stream );

   Journal_PrintPointer( stream, self->_setup );
   Journal_PrintPointer( stream, self->_draw );

   Stream_UnIndent( stream );
}

void* _lucDrawingObject_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap )
{
   lucDrawingObject* self        = drawingObject;
   lucDrawingObject* newDrawingObject;

   newDrawingObject = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );

   return (void*) newDrawingObject;
}

void* _lucDrawingObject_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(lucDrawingObject);
   Type                                                      type = lucDrawingObject_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucDrawingObject_Delete;
   Stg_Class_PrintFunction*                                _print = _lucDrawingObject_Print;
   Stg_Class_CopyFunction*                                  _copy = _lucDrawingObject_Copy;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucDrawingObject_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucDrawingObject_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucDrawingObject_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucDrawingObject_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucDrawingObject_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucDrawingObject_Destroy;
   lucDrawingObject_SetupFunction*                       _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                         _draw = NULL;
   lucDrawingObject_CleanUpFunction*                   _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );
}

void _lucDrawingObject_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucDrawingObject*        self            = (lucDrawingObject*) drawingObject ;
   AbstractContext* context;

   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   if ( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data );

   if ( context ) {
      self->comm  = context->communicator;
      self->rank  = context->rank;
      self->nproc = context->nproc;
   } else {
      self->comm  = MPI_COMM_WORLD;
      MPI_Comm_rank( MPI_COMM_WORLD, &self->rank );
      MPI_Comm_size( MPI_COMM_WORLD, &self->nproc );
   }
   
   _lucDrawingObject_Init( self, context,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"properties", ""  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"disable", False  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colour", "black"  ),
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ColourMap", lucColourMap, False, data),
      (float)Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"opacity", -1.0));
}

void _lucDrawingObject_Build( void* camera, void* data ) { }
void _lucDrawingObject_Initialise( void* camera, void* data ) { }
void _lucDrawingObject_Execute( void* camera, void* data ) { }
void _lucDrawingObject_Destroy( void* camera, void* data ) { }

/* Prepare the object for rendering - default empty implementation */
void lucDrawingObject_Setup( void* drawingObject, lucDatabase* database, void* context ) {} 

/* Render the object */
void lucDrawingObject_Draw( void* drawingObject, lucDatabase* database, void* context )
{
   lucDrawingObject*   self       = (lucDrawingObject*) drawingObject ;

   if (self->disabled) return;

   if (self->needsToDraw && self->_draw)
   {
      /* Use the object's saved draw function  */
      self->_draw(drawingObject, database, context); 
      self->needsToDraw = False;
   }
}

/* Clean up after rendering */
void lucDrawingObject_CleanUp( void* drawingObject ) 
{
   lucDrawingObject*   self       = (lucDrawingObject*) drawingObject ;

   self->needsToCleanUp = False;
}

void lucDrawingObjectMask_Construct( lucDrawingObjectMask* self, Name drawingObjectName, Stg_ComponentFactory* cf, void* mask )
{
   Name                   maskTypeName;

   self->value     = Stg_ComponentFactory_GetDouble( cf, drawingObjectName, (Dictionary_Entry_Key)"maskValue", 0.0  );
   self->tolerance = Stg_ComponentFactory_GetDouble( cf, drawingObjectName, (Dictionary_Entry_Key)"maskTolerance", 0.001  );

   maskTypeName = Stg_ComponentFactory_GetString( cf, drawingObjectName, (Dictionary_Entry_Key)"maskType", "GreaterThan"  );
   if ( strcasecmp( maskTypeName, "GreaterThan" ) == 0 )
      self->type = GreaterThan;
   else if ( strcasecmp( maskTypeName, "LesserThan" ) == 0 || strcasecmp( maskTypeName, "LessThan" ) == 0 )
      self->type = LessThan;
   else if ( strcasecmp( maskTypeName, "EqualTo" ) == 0 )
      self->type = EqualTo;
   else
   {
      Journal_Printf( lucError, "In func %s: Cannot understand 'maskType' '%s'.\n", __func__, maskTypeName );
      abort();
   }
}

Bool lucDrawingObjectMask_Test( lucDrawingObjectMask* self, double value )
{
   double maskValue = self->value;

   switch (self->type)
   {
   case GreaterThan:
      if (value > maskValue)
         return True;
      return False;
   case LessThan:
      if (value < maskValue)
         return True;
      return False;
   case EqualTo:
      if (fabs( maskValue - value ) < self->tolerance )
         return True;
      return False;
   }
   return True;
}

/* HACK - a function to check whether a field is an FeVariable or not before it does an FeVariable_SyncShadowValues */
void lucDrawingObject_SyncShadowValues( void* drawingObject, void* field )
{
   if ( field && Stg_Class_IsInstance( field, FeVariable_Type ) )
      FeVariable_SyncShadowValues( field );
}

void lucDrawingObject_SetProperties(void* drawingObject, char *props)
{
   lucDrawingObject* self = (lucDrawingObject*)drawingObject ;
   strncpy(self->properties, props, 4096);
}


