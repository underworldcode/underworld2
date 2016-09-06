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
#define LUC_DEFAULT_COLOURMAP lucColourMap_New("defaultColourMap", "black white", 0, 0, False, False)
#define LUC_DEFAULT_ALPHAMAP lucColourMap_New("defaultAlphaMap", "#000000:0.0 black", 0, 0, False, False)

struct lucColour 
{
   float red;
   float green;
   float blue;
   float opacity;
};

#define __lucColourMap                                            \
      __Stg_Component                                             \
      AbstractContext*                       context;             \
      double                                 minimum;             \
      double                                 maximum;             \
      Bool                                   logScale;            \
      Bool                                   discrete;            \
      FieldVariable*                         fieldVariable;       \
      int                                    id;                  \
      void*                                  object;              \
      char*                                  properties;          \

struct lucColourMap
{
   __lucColourMap
};

/** Constructors */
lucColourMap* lucColourMap_New(
   Name                                         name,
   char*                                        _colourMapString,
   double                                       minimum,
   double                                       maximum,
   Bool                                         logScale,
   Bool                                         discrete
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
   Bool                          discrete );

/** Virtual Functions */
void _lucColourMap_Delete( void* colourMap ) ;

void* _lucColourMap_DefaultNew( Name name ) ;
void _lucColourMap_AssignFromXML( void* colourMap, Stg_ComponentFactory* cf, void* data ) ;
void _lucColourMap_Build( void* colourMap, void* data ) ;
void _lucColourMap_Initialise( void* colourMap, void* data ) ;
void _lucColourMap_Execute( void* colourMap, void* data ) ;
void _lucColourMap_Destroy( void* colourMap, void* data ) ;

void lucColourMap_SetMinMax( void* colourMap, double min, double max ) ;
#endif

