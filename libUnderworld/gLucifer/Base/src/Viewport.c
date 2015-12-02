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

#include "types.h"
#include "ColourMap.h"
#include "Window.h"
#include "DrawingObject_Register.h"

#include "Viewport.h"

#include "DrawingObject.h"
#include "Camera.h"
#include "Init.h"

const Type lucViewport_Type = "lucViewport";

lucViewport* _lucViewport_New(  LUCVIEWPORT_DEFARGS  )
{
   lucViewport*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucViewport) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucViewport*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   return self;
}

void _lucViewport_Init(
   lucViewport*                  self,
   lucCamera*                    camera,
   lucDrawingObject**            drawingObjectList,
   DrawingObject_Index           drawingObjectCount,
   char*                         properties,
   double                        nearClipPlane,
   double                        farClipPlane,
   double                        scaleX,
   double                        scaleY,
   double                        scaleZ)
{
   DrawingObject_Index object_I;

   self->camera                   = camera;
   self->nearClipPlane            = nearClipPlane;
   self->farClipPlane             = farClipPlane;
   self->scaleX                   = scaleX;
   self->scaleY                   = scaleY;
   self->scaleZ                   = scaleZ;

   /* Write property string */
   self->properties = Memory_Alloc_Array(char, 4096, "properties");
   memset(self->properties, 0, 4086);
   strncpy(self->properties, properties, 4096);

   self->drawingObject_Register = lucDrawingObject_Register_New();

   for ( object_I = 0 ; object_I < drawingObjectCount ; object_I++ )
      lucDrawingObject_Register_Add( self->drawingObject_Register, drawingObjectList[ object_I ] );
}

void _lucViewport_Delete( void* viewport )
{
   lucViewport* self        = viewport;

   if (self->properties) Memory_Free(self->properties);

   _Stg_Component_Delete( self );
}

void _lucViewport_Print( void* viewport, Stream* stream )
{
   lucViewport*          self        = viewport;

   Journal_Printf( stream, "lucViewport: %s\n", self->name );

   Stream_Indent( stream );

   /* Print Parent */
   _Stg_Component_Print( self, stream );

   lucDrawingObject_Register_PrintAllObjects( self->drawingObject_Register, stream );

   Stg_Class_Print( self->camera, stream );

   Journal_PrintValue( stream, self->margin );
   Journal_PrintValue( stream, self->nearClipPlane );
   Journal_PrintValue( stream, self->farClipPlane );

   Stream_UnIndent( stream );
}

void* _lucViewport_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucViewport );
   Type                                                      type = lucViewport_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucViewport_Delete;
   Stg_Class_PrintFunction*                                _print = _lucViewport_Print;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucViewport_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucViewport_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucViewport_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucViewport_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucViewport_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucViewport_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucViewport_New(  LUCVIEWPORT_PASSARGS  );
}

void _lucViewport_AssignFromXML( void* viewport, Stg_ComponentFactory* cf, void* data )
{
   lucViewport*        self               = (lucViewport*) viewport;
   DrawingObject_Index drawingObjectCount;
   lucDrawingObject**  drawingObjectList;
   lucCamera*          camera;

   camera =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Camera", lucCamera, True, data  ) ;

   drawingObjectList = Stg_ComponentFactory_ConstructByList( cf, self->name, (Dictionary_Entry_Key)"DrawingObject", Stg_ComponentFactory_Unlimited, lucDrawingObject, False, &drawingObjectCount, data  );

   _lucViewport_Init(
      self,
      camera,
      drawingObjectList,
      drawingObjectCount,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"properties", ""  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"nearClipPlane", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"farClipPlane", 0 ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"scaleX", 1.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"scaleY", 1.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"scaleZ", 1.0 ));

   Memory_Free( drawingObjectList );
}

void _lucViewport_Build( void* viewport, void* data ) { }
void _lucViewport_Initialise( void* viewport, void* data ) {}
void _lucViewport_Execute( void* viewport, void* data ) { }
void _lucViewport_Destroy( void* viewport, void* data ) { }

void lucViewport_Draw( void* viewport, lucDatabase* database )
{
   lucViewport*          self = (lucViewport*) viewport ;

   lucDrawingObject_Register_DrawAll( self->drawingObject_Register, database);
}

void lucViewport_CleanUp( void* viewport )
{
   lucViewport*          self = (lucViewport*) viewport ;

   lucDrawingObject_Register_CleanUpAll( self->drawingObject_Register );
}

void lucViewport_Setup( void* viewport, lucDatabase* database )
{
   lucViewport*       self = (lucViewport*) viewport ;
   lucDrawingObject_Register_SetupAll( self->drawingObject_Register, database );
}

void lucViewport_SetProperties(void* viewport, char *props)
{
   lucViewport* self = (lucViewport*)viewport ;
   strncpy(self->properties, props, 4096);
}


