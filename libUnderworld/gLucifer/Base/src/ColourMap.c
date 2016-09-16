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
   char*                                              colourMapString,
   double                                             minimum,
   double                                             maximum,
   Bool                                               logScale,
   Bool                                               discrete)
{
   lucColourMap* self = (lucColourMap*) _lucColourMap_DefaultNew( name );

   _lucColourMap_Init( self, colourMapString, minimum, maximum, logScale, discrete);

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
   char*                         _colourMapString,
   double                        minimum,
   double                        maximum,
   Bool                          logScale,
   Bool                          discrete)
{
   /* Write property string */
   self->properties = Memory_Alloc_Array(char, 4096, "properties");
   memset(self->properties, 0, 4086);
   snprintf(self->properties, 4096, "colours=%s\nrange=[%f,%f]", _colourMapString, minimum, maximum);

   self->logScale     = logScale;
   self->discrete     = discrete;
   /* Set the range minimum and maximum 
    * also scales the values to find colour bar positions [0,1] */
   lucColourMap_SetMinMax(self, minimum, maximum);
   self->fieldVariable = NULL;
   self->id = 0;
   self->object = NULL;
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
   double minimum, maximum;
   Bool logScale, discrete;

   logScale = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"logScale", False  );
   minimum = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minimum", logScale ? 1 : 0  );
   maximum = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maximum", 1  );
   discrete = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"discrete", False  );

   _lucColourMap_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colours", "Blue;White;Red" ),
      minimum, maximum, logScale, discrete
   );
}

void _lucColourMap_Build( void* colourMap, void* data ) { }
void _lucColourMap_Initialise( void* colourMap, void* data ) { }
void _lucColourMap_Execute( void* colourMap, void* data ) { }
void _lucColourMap_Destroy( void* colourMap, void* data ) { }

void lucColourMap_SetMinMax( void* colourMap, double min, double max )
{
   lucColourMap* self       = colourMap;

   /* Copy to colour map */
   self->minimum = min;
   self->maximum = max;
   if (self->logScale)
   {
      if (self->minimum <= DBL_MIN  )
         self->minimum =  DBL_MIN;
      if (self->maximum <= DBL_MIN )
         self->maximum =  DBL_MIN;

      if (self->minimum <= DBL_MIN || self->maximum == DBL_MIN )
         Journal_DPrintf( lucInfo, "\n WARNING: Field used for logscale colourmap possibly contains non-positive values. \n" );
   }
}

