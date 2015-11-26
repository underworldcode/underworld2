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
#include "TextureMap.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucTextureMap_Type = "lucTextureMap";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucTextureMap* _lucTextureMap_New(  LUCTEXTUREMAP_DEFARGS  )
{
   lucTextureMap*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucTextureMap) );
   self = (lucTextureMap*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucTextureMap_Init(
   lucTextureMap*                                               self,
   Name                                                         imageName,
   double                                                       bottomLeftX,
   double                                                       bottomLeftY,
   double                                                       bottomLeftZ,
   double                                                       bottomRightX,
   double                                                       bottomRightY,
   double                                                       bottomRightZ,
   double                                                       topRightX,
   double                                                       topRightY,
   double                                                       topRightZ,
   double                                                       topLeftX,
   double                                                       topLeftY,
   double                                                       topLeftZ )
{

   self->bottomLeftCoord[ I_AXIS ] = bottomLeftX;
   self->bottomLeftCoord[ J_AXIS ] = bottomLeftY;
   self->bottomLeftCoord[ K_AXIS ] = bottomLeftZ;

   self->bottomRightCoord[ I_AXIS ] = bottomRightX;
   self->bottomRightCoord[ J_AXIS ] = bottomRightY;
   self->bottomRightCoord[ K_AXIS ] = bottomRightZ;

   self->topRightCoord[ I_AXIS ] = topRightX;
   self->topRightCoord[ J_AXIS ] = topRightY;
   self->topRightCoord[ K_AXIS ] = topRightZ;

   self->topLeftCoord[ I_AXIS ] = topLeftX;
   self->topLeftCoord[ J_AXIS ] = topLeftY;
   self->topLeftCoord[ K_AXIS ] = topLeftZ;

   self->texture = 0;

   /* Append to property string */
   //OK: DEPRECATED, SET VIA PYTHON
   //lucDrawingObject_AppendProps(self, "texturefile=%s\ncullface=0\n", imageName); 
}

void _lucTextureMap_Delete( void* drawingObject )
{
   _lucDrawingObject_Delete( drawingObject );
}

void _lucTextureMap_Print( void* drawingObject, Stream* stream )
{
   lucTextureMap*  self = (lucTextureMap*)drawingObject;

   _lucDrawingObject_Print( self, stream );

   Journal_PrintArray( stream, self->bottomLeftCoord,  3 );
   Journal_PrintArray( stream, self->bottomRightCoord, 3 );
   Journal_PrintArray( stream, self->topRightCoord,    3 );
   Journal_PrintArray( stream, self->topLeftCoord,     3 );
}

void* _lucTextureMap_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucTextureMap);
   Type                                                             type = lucTextureMap_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucTextureMap_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucTextureMap_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucTextureMap_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucTextureMap_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucTextureMap_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucTextureMap_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucTextureMap_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucTextureMap_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucTextureMap_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucTextureMap_New(  LUCTEXTUREMAP_PASSARGS  );
}

void _lucTextureMap_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucTextureMap* self = (lucTextureMap*)drawingObject;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   _lucTextureMap_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"image", ""  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomLeftX", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomLeftY", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomLeftZ", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomRightX", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomRightY", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"bottomRightZ", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topRightX", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topRightY", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topRightZ", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topLeftX", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topLeftY", 0.0  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"topLeftZ", 0.0 )  );
}

void _lucTextureMap_Build( void* drawingObject, void* data ) {}
void _lucTextureMap_Initialise( void* drawingObject, void* data ) {}
void _lucTextureMap_Execute( void* drawingObject, void* data ) {}
void _lucTextureMap_Destroy( void* drawingObject, void* data ) {}

void _lucTextureMap_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucTextureMap* self = (lucTextureMap*)drawingObject;

   /* Only draw on proc 0 */
   if (self->rank > 0) return;

   /* Dump vertex pos */
   float coords[12] = {self->topLeftCoord[0], self->topLeftCoord[1], self->topLeftCoord[2],
                       self->topRightCoord[0], self->topRightCoord[1], self->topRightCoord[2],
                       self->bottomLeftCoord[0], self->bottomLeftCoord[1], self->bottomLeftCoord[2],
                       self->bottomRightCoord[0], self->bottomRightCoord[1], self->bottomRightCoord[2]};

   lucDatabase_AddGridVertices(database, 4, 2, coords);

   lucDatabase_AddTexCoord(database, lucGridType, 0.0, 1.0);
   lucDatabase_AddTexCoord(database, lucGridType, 1.0, 1.0);
   lucDatabase_AddTexCoord(database, lucGridType, 0.0, 0.0);
   lucDatabase_AddTexCoord(database, lucGridType, 1.0, 0.0);
}





