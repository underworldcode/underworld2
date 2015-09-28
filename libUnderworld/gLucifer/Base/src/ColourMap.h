/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __lucColour_ColourMap_h__
#define __lucColour_ColourMap_h__

extern const Type lucColourMap_Type;

//A default colour map, used for mapping colour component values where no map provided
#define LUC_DEFAULT_COLOURMAP lucColourMap_New("defaultColourMap", "black white", 0, 1, False, True, False, False, 0)
#define LUC_DEFAULT_ALPHAMAP lucColourMap_New("defaultAlphaMap", "#000000:0.0 black", 0, 1, False, True, False, False, 0)

struct lucColour 
{
   float red;
   float green;
   float blue;
   float opacity;
};

struct lucColourMapping
{
   lucColour*  colour;
   double      position;
   double*     value;
};

#define __lucColourMap                                            \
      __Stg_Component                                             \
      AbstractContext*                       context;             \
      Colour_Index                           colourCount;         \
      lucColourMapping*                      colourList;          \
      double                                 minimum;             \
      double                                 maximum;             \
      Bool                                   logScale;            \
      Bool                                   dynamicRange;        \
      Bool                                   discrete;            \
      Bool                                   centreOnFixedValue;  \
      double                                 centreValue;         \
      FieldVariable*                         fieldVariable;       \
      int                                    id;                  \
      void*                                  object;              \

struct lucColourMap
{
   __lucColourMap
};

/* Colour utility functions */
void lucColour_SetColour( lucColour* colour, float opacity );
void lucColour_SetComplimentaryColour( lucColour* colour, float opacity );
void lucColour_SetXOR( Bool switchOn, float opacity);
void lucColourMap_SetColourFromValue( lucColourMap* cmap, double value, float opacity ) ;
int lucColour_ToInt(lucColour* colour);

/** Constructors */
lucColourMap* lucColourMap_New(
   Name                                         name,
   char*                                        _colourMapString,
   double                                       minimum,
   double                                       maximum,
   Bool                                         logScale,
   Bool                                         dynamicRange,
   Bool                                         discrete,
   Bool                                         centreOnFixedValue,
   double                                       centreValue
);


#ifndef ZERO
#define ZERO 0
#endif

#define LUCCOLOURMAP_DEFARGS \
                STG_COMPONENT_DEFARGS

#define LUCCOLOURMAP_PASSARGS \
                STG_COMPONENT_PASSARGS

lucColourMap* _lucColourMap_New(  LUCCOLOURMAP_DEFARGS  );


void _lucColourMap_Init(
   lucColourMap*                 self,
   char*                         _colourMapString,
   double                        minimum,
   double                        maximum,
   Bool                          logScale,
   Bool                          dynamicRange,
   Bool                          discrete,
   Bool                          centreOnFixedValue,
   double                        centreValue    );

/** Virtual Functions */
void _lucColourMap_Delete( void* colourMap ) ;
void _lucColourMap_Print( void* colourMap, Stream* stream ) ;
void* _lucColourMap_Copy( void* colourMap, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;

void* _lucColourMap_DefaultNew( Name name ) ;
void _lucColourMap_AssignFromXML( void* colourMap, Stg_ComponentFactory* cf, void* data ) ;
void _lucColourMap_Build( void* colourMap, void* data ) ;
void _lucColourMap_Initialise( void* colourMap, void* data ) ;
void _lucColourMap_Execute( void* colourMap, void* data ) ;
void _lucColourMap_Destroy( void* colourMap, void* data ) ;

/** Public Functions */
void lucColourMap_GetColourFromValue( void* colourMap, double value, lucColour* colour, float opacity );
double lucColourMap_ScaleValue( void* colourMap, double value ) ;

#define lucColourMap_GetColourFromList( self, colour_I ) \
      ((self)->colourList[ colour_I ].colour)

void lucColourMap_SetMinMax( void* colourMap, double min, double max ) ;
void lucColourMap_Calibrate( void* colourMap );
void lucColourMap_CalibrateFromVariable( void* colourMap, void* _variable ) ;
void lucColourMap_CalibrateFromFieldVariable( void* colourMap, void* _fieldVariable ) ;
void lucColourMap_CalibrateFromSwarmVariable( void* colourMap, void* swarmVariable ) ;

void lucColour_FromHSV( lucColour* self, float hue, float saturation, float value, float opacity ) ;
void lucColour_FromString( lucColour* self, char* string ) ;

#endif

