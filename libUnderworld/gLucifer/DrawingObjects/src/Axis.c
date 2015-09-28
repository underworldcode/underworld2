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


#ifdef HAVE_GL2PS
#include <gl2ps.h>
#endif

#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
#include "Axis.h"

const Type lucAxis_Type = "lucAxis";

lucAxis* _lucAxis_New(  LUCAXIS_DEFARGS  )
{
   lucAxis*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucAxis) );
   self = (lucAxis*)  _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );


   return self;
}

void _lucAxis_Init(
   lucAxis*                            self,
   Coord                               origin,
   float                               length,
   char*                               axes,
   char*                               labelX,
   char*                               labelY,
   char*                               labelZ,
   lucColour                           colourX,
   lucColour                           colourY,
   lucColour                           colourZ)
{
   int i;
   for (i=0; i<3; i++)
      self->axes[i] = False;
   if (strchr(axes, 'x')) self->axes[I_AXIS] = True;
   if (strchr(axes, 'y')) self->axes[J_AXIS] = True;
   if (strchr(axes, 'z')) self->axes[K_AXIS] = True;
   if (strchr(axes, 'X')) self->axes[I_AXIS] = True;
   if (strchr(axes, 'Y')) self->axes[J_AXIS] = True;
   if (strchr(axes, 'Z')) self->axes[K_AXIS] = True;

   self->length = length;

   self->labels[0] = StG_Strdup(labelX);
   self->labels[1] = StG_Strdup(labelY);
   self->labels[2] = StG_Strdup(labelZ);

   memcpy( self->origin, origin, sizeof(Coord) );
   memcpy( &(self->colours[0]), &colourX, sizeof(lucColour) );
   memcpy( &(self->colours[1]), &colourY, sizeof(lucColour) );
   memcpy( &(self->colours[2]), &colourZ, sizeof(lucColour) );
}

void _lucAxis_Delete( void* drawingObject )
{
   lucAxis*  self = (lucAxis*)drawingObject;

   _lucDrawingObject_Delete( self );
}

void _lucAxis_Print( void* drawingObject, Stream* stream )
{
   lucAxis*  self = (lucAxis*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucAxis_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof( lucAxis );
   Type                                                             type = lucAxis_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucAxis_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucAxis_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucAxis_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucAxis_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucAxis_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucAxis_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucAxis_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucAxis_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucAxis_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucAxis_New(  LUCAXIS_PASSARGS  );
}

void _lucAxis_AssignFromXML( void* axis, Stg_ComponentFactory* cf, void* data )
{
   lucAxis*             self               = (lucAxis*) axis;
   Name colourNameX;
   Name colourNameY;
   Name colourNameZ;

   Coord origin;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   colourNameX  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colourX", "Red" ) ;
   colourNameY  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colourY", "Green" ) ;
   colourNameZ  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colourZ", "Blue" ) ;

   lucColour_FromString( &self->colours[0], colourNameX );
   lucColour_FromString( &self->colours[1], colourNameY );
   lucColour_FromString( &self->colours[2], colourNameZ );

   origin[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"originX", 32.0  );
   origin[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"originY", 32.0  );
   origin[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"originZ", 0.25  );

   _lucAxis_Init( self,
                  origin,
                  Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"length", 0.2 ),
                  Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"axes", "xyz" ),
                  Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelX", "X" ),
                  Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelY", "Y" ),
                  Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"labelZ", "Z" ),
                  self->colours[0],
                  self->colours[1],
                  self->colours[2]);
}

void _lucAxis_Build( void* Axis, void* data ) { }
void _lucAxis_Initialise( void* Axis, void* data ) { }
void _lucAxis_Execute( void* Axis, void* data ) { }
void _lucAxis_Destroy( void* Axis, void* data ) { }

void _lucAxis_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   /* Deprecated: axis + axisLength now options on viewport */
}







