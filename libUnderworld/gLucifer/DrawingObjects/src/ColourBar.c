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

#include <float.h>

#include "types.h"
#include "ColourBar.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucColourBar_Type = "lucColourBar";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucColourBar* _lucColourBar_New(  LUCCOLOURBAR_DEFARGS  )
{
   lucColourBar*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucColourBar) );
   self = (lucColourBar*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucColourBar_Init(
   lucColourBar*                                                self,
   double                                                       lengthFactor,
   Pixel_Index                                                  height,
   Pixel_Index                                                  margin,
   int                                                          borderWidth,
   int                                                          precision,
   Bool                                                         scientific,
   int                                                          ticks,
   Bool                                                         printTickValue,
   Bool                                                         printUnits,
   double                                                       scaleValue,
   double                                                       tickValues[] )
{
   int i;
   self->lengthFactor = lengthFactor;
   self->height = height;
   self->margin = margin;
   self->borderWidth = borderWidth;
   self->precision = precision;
   self->scientific = scientific;
   self->ticks = ticks;
   self->printTickValue = printTickValue;
   self->printUnits = printUnits;
   self->scaleValue = scaleValue;

   /* Append to property string */
   lucDrawingObject_AppendProps(self, "colourbar=1\nheight=%d\nlengthfactor=%g\nmargin=%d\nborder=%d\nprecision=%d\nscientific=%d\nticks=%d\nprintticks=%d\nprintunits=%d\nscalevalue=%g\n", height, lengthFactor, margin, borderWidth, precision, scientific, ticks, printTickValue, printUnits, scaleValue);

   for (i = 1; i <= ticks; i++)
   {
      self->tickValues[i] = tickValues[i];
      if (tickValues[i] != DBL_MIN)
         lucDrawingObject_AppendProps(self, "tick%d=%g\n", i, tickValues[i]);
   }
}

void _lucColourBar_Delete( void* drawingObject )
{
   lucColourBar*  self = (lucColourBar*)drawingObject;

   _lucDrawingObject_Delete( self );
}

void _lucColourBar_Print( void* drawingObject, Stream* stream )
{
   lucColourBar*  self = (lucColourBar*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucColourBar_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap)
{
   lucColourBar*  self = (lucColourBar*)drawingObject;
   lucColourBar* newDrawingObject;

   newDrawingObject = _lucDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newDrawingObject;
}


void* _lucColourBar_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(lucColourBar);
   Type                                                      type = lucColourBar_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucColourBar_Delete;
   Stg_Class_PrintFunction*                                _print = _lucColourBar_Print;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucColourBar_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucColourBar_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucColourBar_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucColourBar_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucColourBar_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucColourBar_Destroy;
   lucDrawingObject_SetupFunction*                       _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                         _draw = _lucColourBar_Draw;
   lucDrawingObject_CleanUpFunction*                   _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucColourBar_New(  LUCCOLOURBAR_PASSARGS  );
}

void _lucColourBar_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucColourBar*   self = (lucColourBar*)drawingObject;
   unsigned int    i, defaultTicks, ticks;
   double          tickValues[11];
   char            tickLabel[10];

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   Journal_Firewall(self->colourMap != NULL, lucError,
      "In func %s, unable to draw colourBar %s because no colourMap provided.\n",
      __func__, self->name);

   /* Default to 0 tick marks for linear, 1 for fixed centre, 2 for logarithmic scale */
   defaultTicks = 0;
   if (self->colourMap->centreOnFixedValue) defaultTicks = 1;
   if (self->colourMap->logScale) defaultTicks = 2;
   ticks = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"ticks", defaultTicks );
   if (ticks > 9) ticks = 9;

   /* Load any provided intermediate tick values (tick1-9) */
   for (i = 1; i <= ticks; i++ )
   {
      sprintf(tickLabel, "tick%d", i);
      tickValues[i] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)tickLabel, DBL_MIN );
   }
   tickValues[0] = self->colourMap->minimum;
   tickValues[ticks+1] = self->colourMap->maximum;

   _lucColourBar_Init(
      self,
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"lengthFactor", 0.8  ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"height", 10  ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"margin", 16  ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"borderWidth", 1  ) ,
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"precision", 2  ) ,
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"scientific", False  ),
      ticks,
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"printTickValue", True  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"printUnits", False  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"scaleValue", 1.0  ),
      tickValues
   );

}

void _lucColourBar_Build( void* drawingObject, void* data ) {}
void _lucColourBar_Initialise( void* drawingObject, void* data ) {}
void _lucColourBar_Execute( void* drawingObject, void* data ) {}
void _lucColourBar_Destroy( void* drawingObject, void* data ) {}

void _lucColourBar_Draw( void* drawingObject, lucDatabase* database, void* _context ) {}
