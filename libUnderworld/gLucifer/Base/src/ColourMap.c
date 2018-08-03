/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <string.h>
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include <gLucifer/Base/Base.h>
#include "types.h"
#include "ColourMap.h"
#include <float.h>

const Type lucColourMap_Type = "lucColourMap";

lucColourMap* lucColourMap_New(
   Name                                               name,
   char*                                              properties)
{
   lucColourMap* self = (lucColourMap*) _lucColourMap_DefaultNew( name );
   _lucColourMap_Init( self, properties);
   return self;
}

lucColourMap* _lucColourMap_New(  LUCCOLOURMAP_DEFARGS  )
{
   lucColourMap*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucColourMap) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucColourMap*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   return self;
}

void _lucColourMap_Init(
   lucColourMap*                 self,
   char*                         properties )

{
   /* Write property string */
   self->properties = Memory_Alloc_Array(char, MAX_PROPERTY_LEN, "properties");
   memset(self->properties, 0, MAX_PROPERTY_LEN);
   strncpy(self->properties, properties, MAX_PROPERTY_LEN);
   lucColourMap_SetMinMax(self, 0, 0);
   self->id = 0;
}

void _lucColourMap_Delete( void* colourMap )
{
   lucColourMap* self        = colourMap;

   Memory_Free(self->properties);

   _Stg_Component_Delete( self );
}

void* _lucColourMap_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucColourMap );
   Type                                                      type = lucColourMap_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucColourMap_Delete;
   Stg_Class_PrintFunction*                                _print = NULL;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucColourMap_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucColourMap_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucColourMap_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucColourMap_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucColourMap_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucColourMap_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucColourMap_New(  LUCCOLOURMAP_PASSARGS  );
}

void _lucColourMap_AssignFromXML( void* colourMap, Stg_ComponentFactory* cf, void* data )
{
   lucColourMap* self             = (lucColourMap*) colourMap;

   _lucColourMap_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"properties", "colours=Blue;White;Red" )
   );
}

void _lucColourMap_Build( void* colourMap, void* data ) { }
void _lucColourMap_Initialise( void* colourMap, void* data ) { }
void _lucColourMap_Execute( void* colourMap, void* data ) { }
void _lucColourMap_Destroy( void* colourMap, void* data ) { }

void lucColourMap_SetProperties( void* colourMap, char *props )
{
   lucColourMap* self = colourMap;
   strncpy(self->properties, props, MAX_PROPERTY_LEN);
}

void lucColourMap_SetMinMax( void* colourMap, double min, double max )
{
   lucColourMap* self       = colourMap;

   /* Copy to colour map */
   self->minimum = min;
   self->maximum = max;
}

