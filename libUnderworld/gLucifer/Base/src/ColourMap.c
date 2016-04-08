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
#include "X11Colours.h"
#include <float.h>

const Type lucColourMap_Type = "lucColourMap";

/* Colour utility functions */
void lucColour_SetColour( lucColour* c, float opacity )
{
   /* Disable opacity override by setting to negative value */
   if (opacity >= 0) c->opacity = opacity;

   /*glColor4f(c->red, c->green, c->blue, c->opacity );*/
}

void lucColour_SetComplimentaryColour( lucColour* c, float opacity )
{
   /* Disable opacity override by setting to negative value */
   if (opacity >= 0) c->opacity = opacity;

   /*glColor4f(1.0 - c->red, 1.0 - c->green, 1.0 - c->blue, c->opacity );*/
}

void lucColour_SetXOR( Bool switchOn, float opacity)
{
   if (opacity < 0) opacity = 1.0;
/*
   if (switchOn)
   {
      glColor4f(1.0, 1.0, 1.0, opacity);
      glLogicOp(GL_XOR);
      glEnable(GL_COLOR_LOGIC_OP);
   }
   else
      glDisable(GL_COLOR_LOGIC_OP);
*/
}

void lucColourMap_SetColourFromValue( lucColourMap* cmap, double value, float opacity )
{
   lucColour colour;
   lucColourMap_GetColourFromValue( cmap, value, &colour, opacity);
   lucColour_SetColour( &colour, opacity );
}

void lucColour_SetFromRGBA( lucColour* colour, float red, float green, float blue, float opacity )
{
   colour->red = red;
   colour->green = green;
   colour->blue = blue;
   colour->opacity = opacity;
}

int lucColour_ToInt(lucColour* colour) 
{
   return (int)(colour->red*255)
             + ((int)(colour->green*255) << 8)
             + ((int)(colour->blue*255) << 16)
             + ((int)(colour->opacity*255) << 24);
}

lucColourMap* lucColourMap_New(
   Name                                               name,
   char*                                              colourMapString,
   double                                             minimum,
   double                                             maximum,
   Bool                                               logScale,
   Bool                                               dynamicRange,
   Bool                                               discrete,
   Bool                                               centreOnFixedValue,
   double											            centreValue	 )
{
   lucColourMap* self = (lucColourMap*) _lucColourMap_DefaultNew( name );

   _lucColourMap_Init( self, colourMapString, minimum, maximum, logScale, dynamicRange, discrete, centreOnFixedValue, centreValue);

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
   Bool                          dynamicRange,
   Bool                          discrete,
   Bool                          centreOnFixedValue,
   double                        centreValue )
{
   char*          charPtr;
   char*          breakChars = " \t\n;,";
   //char*          savePtr;
   char           temp[64];
   char*          colourStr;
   double         value;

   /* Find number of colours */
   self->colourList = NULL;
   self->colourCount = 0;

   /* Write property string */
   self->properties = Memory_Alloc_Array(char, 4096, "properties");
   memset(self->properties, 0, 4086);
   snprintf(self->properties, 4096, "colours=%s", _colourMapString);

   char* colourString = StG_Strdup(_colourMapString);
   char* colourMap_ptr = colourString;
   //charPtr = strtok_r( colourMap_ptr , breakChars, &savePtr);
   charPtr = strtok(colourMap_ptr , breakChars);
   while ( charPtr != NULL )
   {
      self->colourCount++;

      /* Re-allocate space for colour map */
      self->colourList = Memory_Realloc_Array( self->colourList, lucColourMapping, self->colourCount );

      /* Create colour */
      self->colourList[self->colourCount-1].colour = Memory_Alloc(lucColour, "Colour");
      self->colourList[self->colourCount-1].position = -1;
      self->colourList[self->colourCount-1].value = NULL;
      lucColour_SetFromRGBA(self->colourList[self->colourCount-1].colour, 0, 0, 0, 0);
      
      /* Parse out value if provided, otherwise assign a default */
      colourStr = temp;
      /* Parse the input string (OPTIONAL-VALUE)colour */
      if ( sscanf( charPtr, "(%lf)%s", &value, colourStr ) == 2 )
      {
         /* Save the value */
         self->colourList[self->colourCount-1].value = Memory_Alloc(double, "Colour Value");
         *self->colourList[self->colourCount-1].value = value;
      }
      else
         colourStr = charPtr;

      /* Set parsed colour */
      lucColour_FromString( self->colourList[self->colourCount-1].colour, colourStr );

      //printf("%f  %s\n", self->colourList[self->colourCount-1].value ? value : (log(-1)), colourStr);

      //charPtr = strtok_r( NULL, breakChars, &savePtr);
      charPtr = strtok(NULL, breakChars);
   }
   Memory_Free(colourString);

   Journal_Firewall( self->colourCount >= 1, lucError,
                     "ColourMap, in %s for %s '%s' - Require at least one colour.\n", __func__, self->type, self->name );

   /* If maximum and minimum provided as colour values, use them over passed max/min */
   if (self->colourCount > 0 && self->colourList[0].value != NULL)
   {
      minimum = *self->colourList[0].value;
      Memory_Free(self->colourList[0].value);
   }
   if (self->colourCount > 0 && self->colourList[self->colourCount-1].value != NULL) 
   {
      maximum = *self->colourList[self->colourCount-1].value;
      Memory_Free(self->colourList[self->colourCount-1].value);
   }

   /* Set first and last colours to point to minimum and maximum */
   self->colourList[0].value = &self->minimum;
   self->colourList[0].position = 0;
   self->colourList[self->colourCount-1].value = &self->maximum;
   self->colourList[self->colourCount-1].position = 1;

   self->logScale     = logScale;
   self->dynamicRange = dynamicRange;
   self->discrete     = discrete;
   self->centreOnFixedValue = dynamicRange & centreOnFixedValue;  /* Only valid when using dynamic range */
   self->centreValue = centreValue;
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

   /* Free any colours and values allocated */
   int idx;
   for (idx=0; idx < self->colourCount; idx++)
   {
      if (idx > 0 && idx < self->colourCount-1)
         Memory_Free(self->colourList[idx].value);
      Memory_Free(self->colourList[idx].colour);
   }

   Memory_Free( self->colourList );
   Memory_Free(self->properties);

   _Stg_Component_Delete( self );
}

void _lucColourMap_Print( void* colourMap, Stream* stream )
{
   lucColourMap* self        = colourMap;
   Colour_Index  colour_I;
   lucColour*    colour;
   double scaled, *value;

   Journal_Printf( stream, "lucColourMap: %s\n", self->name );

   Journal_PrintValue( stream, self->colourCount );
   for ( colour_I = 0 ; colour_I < self->colourCount ; colour_I++ )
   {
      colour = lucColourMap_GetColourFromList( self, colour_I );

      /* Get scaled position and actual value of colour */
      scaled = self->colourList[colour_I].position;
      value = self->colourList[colour_I].value;
      Journal_Printf( stream, "\tColour %u: Position %f RGBA %02x %02x %02x %02x",
                      colour_I, scaled,
                      (int)(255 * colour->red), (int)(255 * colour->green),
                      (int)(255 * colour->blue), (int)(255 * colour->opacity));
      if (value)
         Journal_Printf( stream, "\t Value: %f\n", *value);
      else
         Journal_Printf( stream, "\t Value: VARYING\n");
   }

   Journal_PrintValue( stream, self->minimum );
   Journal_PrintValue( stream, self->maximum );
   Journal_PrintBool( stream, self->logScale );
   Journal_PrintBool( stream, self->dynamicRange );
   Journal_PrintBool( stream, self->discrete );
   Journal_PrintBool( stream, self->centreOnFixedValue );
   if (self->centreOnFixedValue) Journal_PrintValue( stream, self->centreValue );
}

void* _lucColourMap_Copy( void* colourMap, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap )
{
   lucColourMap* self        = colourMap;
   lucColourMap* newColourMap;

   newColourMap = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );

   newColourMap->colourCount         = self->colourCount;
   newColourMap->minimum             = self->minimum;
   newColourMap->maximum             = self->maximum;
   newColourMap->logScale            = self->logScale;
   newColourMap->dynamicRange        = self->dynamicRange;
   newColourMap->discrete            = self->discrete;
   newColourMap->centreOnFixedValue  = self->centreOnFixedValue;
   newColourMap->centreValue         = self->centreValue;

   if (deep)
      memcpy( newColourMap->colourList, self->colourList, self->colourCount * sizeof(lucColourMapping) );
   else
      newColourMap->colourList = self->colourList;

   return (void*) newColourMap;
}

void* _lucColourMap_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucColourMap );
   Type                                                      type = lucColourMap_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucColourMap_Delete;
   Stg_Class_PrintFunction*                                _print = _lucColourMap_Print;
   Stg_Class_CopyFunction*                                  _copy = _lucColourMap_Copy;
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
   double centreValue, minimum, maximum;
   Bool centreOnFixedValue, dynamicRange, logScale, discrete;

   dynamicRange = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"dynamicRange", False  );
   centreOnFixedValue = False;
   centreValue = 0;
   if (dynamicRange)
      centreOnFixedValue = Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"centreValue", &centreValue);
   logScale = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"logScale", False  );
   minimum = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minimum", logScale ? 1 : 0  );
   maximum = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maximum", 1  );
   discrete = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"discrete", False  );

   _lucColourMap_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colours", "Blue;White;Red" ),
      minimum, maximum, logScale, dynamicRange, discrete, centreOnFixedValue, centreValue
   );
}

void _lucColourMap_Build( void* colourMap, void* data ) { }
void _lucColourMap_Initialise( void* colourMap, void* data ) { }
void _lucColourMap_Execute( void* colourMap, void* data ) { }
void _lucColourMap_Destroy( void* colourMap, void* data ) { }

void lucColourMap_GetColourFromValue( void* colourMap, double value, lucColour* colour, float opacity )
{
   lucColourMap* self        = colourMap;
   Colour_Index  colour_I;
   lucColourMapping *Below, *Above;
   double        interpolate, scaledValue;
   Colour_Index  colourCount = self->colourCount;

   /* Scale to a position on colourMap [0,1] */
   //printf("Scaling from %f to ", value);
   scaledValue = lucColourMap_ScaleValue(colourMap, value);

   /* Check within range */
   if (scaledValue <= 0.0 || colourCount == 1)
   {
      memcpy( colour, lucColourMap_GetColourFromList( self, 0 ), sizeof(lucColour) );
      return;
   }
   if (scaledValue >= 1.0)
   {
      memcpy( colour, lucColourMap_GetColourFromList( self, colourCount - 1 ), sizeof(lucColour) );
      return;
   }

   /* Find the colour/values our value lies between */
   for (colour_I=0; colour_I < colourCount; colour_I++)
   {
      if (self->colourList[colour_I].position > scaledValue)
      {
         Above = &self->colourList[colour_I];
         break;
      }
      else
         Below = &self->colourList[colour_I];
   }
   Journal_Firewall( colour_I > 0 && colour_I < self->colourCount, lucError,
                     "ColourMap, in %s for %s '%s' - Value %f out of range %f -> %f (Scales to %f)\n", 
                     __func__, self->type, self->name, value, self->minimum, self->maximum, scaledValue);

   /* Calculate interpolation factor [0,1] */
   interpolate = (scaledValue - Below->position) / (Above->position - Below->position);

   /* Discrete colourmap option does not interpolate between colours */
   if (self->discrete)
   {
      /* Copy the nearest colour to value */
      if (interpolate < 0.5)
         memcpy( colour, Below->colour, sizeof(lucColour) );
      else
         memcpy( colour, Above->colour, sizeof(lucColour) );
   }
   else
   {
      /* Do linear interpolation between colours */
      //printf("%f - above %f, below %f colour indexes %d %d interpolate %f\n", scaledValue, 
      //      Above->value ? *Above->value : log10(-1), 
      //      Below->value ? *Below->value : log10(-1), colour_I - 1, colour_I, interpolate);
      colour->red     = ( Above->colour->red     - Below->colour->red     ) * interpolate + Below->colour->red;
      colour->green   = ( Above->colour->green   - Below->colour->green   ) * interpolate + Below->colour->green;
      colour->blue    = ( Above->colour->blue    - Below->colour->blue    ) * interpolate + Below->colour->blue;
      colour->opacity = ( Above->colour->opacity - Below->colour->opacity ) * interpolate + Below->colour->opacity;
   }

   /* Disable opacity override by setting to negative value */
   if (opacity >= 0) colour->opacity = opacity;
}

double lucColourMap_ScaleValue( void* colourMap, double value )
{
   lucColourMap* self        = colourMap;
   double 	      scaledValue, max, min, sampleValue;

   if (self->maximum == self->minimum) return 0.5;   /* Force central value if no range */
   if (value <= self->minimum) return 0.0;
   if (value >= self->maximum) return 1.0;

   /* To get a log scale, transform each value to log10(value) */
   if (self->logScale == True)
   {
      max 	      = log10(self->maximum);
      min  	      = log10(self->minimum);
      sampleValue = log10(value);
   }
   else
   {
      max 	      = self->maximum;
      min  	      = self->minimum;
      sampleValue = value;
   }
   if (max == min) return 0.5;   /* Force central value if no range */

   /* Scale value so that it is between 0 and 1 */
   scaledValue = (sampleValue - min) / (max - min);

   return scaledValue;
}

void lucColourMap_GetColourFromScaledValue( void* colourMap, double scaledValue, lucColour* colour )
{
   lucColourMap* self        = colourMap;
   Colour_Index  colourBelow_I;
   lucColour*    colourBelow;
   lucColour*    colourAbove;
   double         remainder;
   Colour_Index  colourCount = self->colourCount;

   /* Check within range */
   if (scaledValue <= 0.0 || colourCount == 1)
   {
      memcpy( colour, lucColourMap_GetColourFromList( self, 0 ), sizeof(lucColour) );
      return;
   }
   if (scaledValue >= 1.0)
   {
      memcpy( colour, lucColourMap_GetColourFromList( self, colourCount - 1 ), sizeof(lucColour) );
      return;
   }

   /* Discrete colourmap option does not interpolate between colours */
   if (!self->discrete)
   {
      colourBelow_I = (Colour_Index) ( ( colourCount - 1 ) * scaledValue );
      colourBelow   = lucColourMap_GetColourFromList( self, colourBelow_I );
      colourAbove   = lucColourMap_GetColourFromList( self, colourBelow_I + 1 );

      remainder = (double)( colourCount - 1 ) * scaledValue - (double) colourBelow_I;

      /* Do linear interpolation between colours */
      colour->red     = ( colourAbove->red     - colourBelow->red     ) * remainder + colourBelow->red;
      colour->green   = ( colourAbove->green   - colourBelow->green   ) * remainder + colourBelow->green;
      colour->blue    = ( colourAbove->blue    - colourBelow->blue    ) * remainder + colourBelow->blue;
      colour->opacity = ( colourAbove->opacity - colourBelow->opacity ) * remainder + colourBelow->opacity;
   }
   else
   {
      colourBelow_I = (Colour_Index) ( (double)colourCount * scaledValue );
      colourBelow = lucColourMap_GetColourFromList( self, colourBelow_I );
      colour->red = colourBelow->red;
      colour->green = colourBelow->green;
      colour->blue = colourBelow->blue;
      colour->opacity = colourBelow->opacity;
   }
}

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

   /* If a centreValue has been imposed on a dynamic problem 
      adjust max / min to contain it correctly while preserving the scale gradient */
   if (self->centreOnFixedValue)
   {
      if (self->minimum > self->centreValue) self->minimum = self->centreValue;
      if (self->maximum < self->centreValue) self->maximum = self->centreValue;
      if (self->logScale) 
      {
         double mindiff = log10(self->centreValue) - log10(self->minimum);
         double maxdiff = log10(self->maximum) - log10(self->centreValue);
         /* Reduce minimum or increase maximum to push value into centre of scale */
         if (mindiff > maxdiff) 
            self->maximum = pow(10, log10(self->centreValue) + mindiff);
         else
            self->minimum = pow(10, log10(self->centreValue) - maxdiff);
      }
      else
      {
         double mindiff = self->centreValue - self->minimum;
         double maxdiff = self->maximum - self->centreValue;
         /* Reduce minimum or increase maximum to push value into centre of scale */
         if (mindiff > maxdiff) 
            self->maximum = self->centreValue + mindiff;
         else
            self->minimum = self->centreValue - maxdiff;
      }
   }
   //printf("Set range to %f minimum --> %f maximum\n", self->minimum, self->maximum);
   
   /* Recalibrate */
   lucColourMap_Calibrate(self);
}

void lucColourMap_Calibrate( void* colourMap )
{
   /* Default calibration - gets scaled positions for colours 
    * Scale the values to find colour bar positions [0,1] */
   lucColourMap* self       = colourMap;
   int i,j,k;
   double inc;
   for (i = 1; i < self->colourCount-1; i++)
   {
      /* Found an empty value */
      if (self->colourList[i].value == NULL)
      {
         /* Search for next provided value */
         //printf("Empty value at %d ", i);
         for (j = i + 1; j < self->colourCount; j++)
         {
            if (self->colourList[j].value != NULL) 
            {
               /* Scale to get new position, unless at max pos */
               if (j < self->colourCount - 1)
                  self->colourList[j].position = lucColourMap_ScaleValue(self, *self->colourList[j].value);

               //printf(", next value found at %d, ", j);
               inc = (self->colourList[j].position - self->colourList[i - 1].position) / (j - i + 1);
               for (k = i; k < j; k++)
               {
                  self->colourList[k].position = self->colourList[k-1].position + inc;
                  //printf("Interpolating at %d from %f by %f to %f\n", k, self->colourList[k-1].position, inc, self->colourList[k].position);
               }
               break;
            }
         }
         /* Continue search from j */
         i = j;
      }
      else
         /* Value found, scale to get position */
         self->colourList[i].position = lucColourMap_ScaleValue(self, *self->colourList[i].value);
   }
}

void lucColourMap_CalibrateFromVariable( void* colourMap, void* _variable )
{
   lucColourMap* self        = colourMap;
   Variable*     variable    = (Variable*)_variable;
   Index         array_I;
   Index         arrayCount  = *variable->arraySizePtr;
   double        value;
   double        localMin    = 1.0e99;
   double        localMax    = -1.0e99;
   double        globalMax;
   double        globalMin;
   self->fieldVariable = NULL;   /* Clear any saved fieldVariable */

   if ( !self->dynamicRange ) return;

   for ( array_I = 0 ; array_I < arrayCount ; array_I++ )
   {
      /* Get scalar value from particle */
      value = Variable_GetValueDouble( variable, array_I ) ;

      if ( value < localMin )
         localMin = value;
      else if ( value > localMax )
         localMax = value;
   }

   (void)MPI_Allreduce( &localMin, &globalMin, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );
   (void)MPI_Allreduce( &localMax, &globalMax, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );

   lucColourMap_SetMinMax( self, globalMin, globalMax );
}

void lucColourMap_CalibrateFromFieldVariable( void* colourMap, void* _fieldVariable )
{
   lucColourMap*   self          = colourMap;
   /* Save the field variable for colourBar label scaling */
   self->fieldVariable = (FieldVariable*)_fieldVariable;
   if ( !self->dynamicRange ) return;

   lucColourMap_SetMinMax(
      self,
      FieldVariable_GetMinGlobalFieldMagnitude( self->fieldVariable ),
      FieldVariable_GetMaxGlobalFieldMagnitude( self->fieldVariable ) );
}

void lucColourMap_CalibrateFromSwarmVariable( void* colourMap, void* swarmVariable )
{
   lucColourMap*   self          = colourMap;
   self->fieldVariable = NULL;   /* Clear any saved fieldVariable */

   if ( !self->dynamicRange ) return;

   lucColourMap_SetMinMax(
      self,
      SwarmVariable_GetCachedMinGlobalMagnitude( swarmVariable ),
      SwarmVariable_GetCachedMaxGlobalMagnitude( swarmVariable ) );
}

void lucColour_FromHSV( lucColour* self, float hue, float saturation, float value, float opacity )
{
   int   Hi     = (int) ( hue/60.0 );
   float f      = hue/60.0 - (float) Hi;
   float p      = value * ( 1.0 - saturation );
   float q      = value * ( 1.0 - saturation * f );
   float t      = value * ( 1.0 - saturation * ( 1.0 - f ));

   switch ( Hi )
   {
   case 0:
      self->red = value;
      self->green = t;
      self->blue = p;
      break;
   case 1:
      self->red = q;
      self->green = value;
      self->blue = p;
      break;
   case 2:
      self->red = p;
      self->green = value;
      self->blue = t;
      break;
   case 3:
      self->red = p;
      self->green = q;
      self->blue = value;
      break;
   case 4:
      self->red = t;
      self->green = p;
      self->blue = value;
      break;
   case 5:
      self->red = value;
      self->green = p;
      self->blue = q;
      break;
   }

   self->opacity = opacity;
}

void lucColour_FromString( lucColour* self, char* string )
{
   char* charPointer;
   float opacity;

   lucColour_FromX11ColourName( self, string );

   /* Get Opacity From String */
   /* Opacity must be read in after the ":" of the name of the colour */
   charPointer = strchr( string, ':' );

   if (charPointer != NULL)
   {
      /* Return full opactity (non-transparent) if no opacity is set */
      if (sscanf( charPointer + 1, "%f", &opacity )	!= 1)
         opacity = 1.0;
   }
   else
      opacity = 1.0;
   self->opacity = opacity;
}

