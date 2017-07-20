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
#include "SwarmViewer.h"
#include "SwarmViewer.h"
#include "SwarmRGBColourViewer.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucSwarmRGBColourViewer_Type = "lucSwarmRGBColourViewer";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucSwarmRGBColourViewer* _lucSwarmRGBColourViewer_New(  LUCSWARMRGBCOLOURVIEWER_DEFARGS  )
{
   lucSwarmRGBColourViewer*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucSwarmRGBColourViewer) );
   self = (lucSwarmRGBColourViewer*) _lucSwarmViewer_New(  LUCSWARMVIEWER_PASSARGS  );

   return self;
}

void _lucSwarmRGBColourViewer_Init(
   lucSwarmRGBColourViewer*            self,
   lucColourMap*                       redColourMap,
   lucColourMap*                       greenColourMap,
   lucColourMap*                       blueColourMap,
   Name                                colourRedVariableName,
   Name                                colourGreenVariableName,
   Name                                colourBlueVariableName )
{
   self->redColourMap = redColourMap;
   self->greenColourMap = greenColourMap;
   self->blueColourMap = blueColourMap;
   self->colourRedVariableName = colourRedVariableName;
   self->colourGreenVariableName = colourGreenVariableName;
   self->colourBlueVariableName = colourBlueVariableName;
}

void _lucSwarmRGBColourViewer_Delete( void* drawingObject )
{
   lucSwarmRGBColourViewer*  self = (lucSwarmRGBColourViewer*)drawingObject;

   _lucSwarmViewer_Delete( self );
}

void _lucSwarmRGBColourViewer_Print( void* drawingObject, Stream* stream )
{
   lucSwarmRGBColourViewer*  self = (lucSwarmRGBColourViewer*)drawingObject;

   _lucSwarmViewer_Print( self, stream );
}

void* _lucSwarmRGBColourViewer_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucSwarmRGBColourViewer);
   Type                                                             type = lucSwarmRGBColourViewer_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucSwarmRGBColourViewer_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucSwarmRGBColourViewer_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucSwarmRGBColourViewer_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucSwarmRGBColourViewer_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucSwarmRGBColourViewer_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucSwarmRGBColourViewer_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucSwarmRGBColourViewer_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucSwarmRGBColourViewer_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucSwarmRGBColourViewer_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucSwarmViewer_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;
   lucSwarmViewer_PlotParticleFunction*                _plotParticle = _lucSwarmViewer_PlotParticle;
   lucSwarmViewer_SetParticleColourFunction*      _setParticleColour = _lucSwarmRGBColourViewer_SetParticleColour;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucSwarmRGBColourViewer_New(  LUCSWARMRGBCOLOURVIEWER_PASSARGS  );
}

void _lucSwarmRGBColourViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucSwarmRGBColourViewer* self = (lucSwarmRGBColourViewer*)drawingObject;
   Name                     colourRedVariableName;
   Name                     colourGreenVariableName;
   Name                     colourBlueVariableName;

   /* Construct Parent */
   _lucSwarmViewer_AssignFromXML( self, cf, data );

   colourRedVariableName   = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"ColourRedVariable", ""  );
   colourGreenVariableName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"ColourGreenVariable", ""  );
   colourBlueVariableName  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"ColourBlueVariable", ""  );

   _lucSwarmRGBColourViewer_Init(
      self,
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"RedColourMap", lucColourMap, False, data),
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"GreenColourMap", lucColourMap, False, data),
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"BlueColourMap", lucColourMap, False, data),
      colourRedVariableName,
      colourGreenVariableName,
      colourBlueVariableName);
}

void _lucSwarmRGBColourViewer_Build( void* drawingObject, void* data ) {}

void _lucSwarmRGBColourViewer_Initialise( void* drawingObject, void* data )
{
   lucSwarmRGBColourViewer*	self                   = (lucSwarmRGBColourViewer*)drawingObject;

   _lucSwarmViewer_Initialise( self, data );

   self->colourRedVariable = lucSwarmViewer_InitialiseVariable(self, self->colourRedVariableName, True, data);
   self->colourGreenVariable = lucSwarmViewer_InitialiseVariable(self, self->colourGreenVariableName, True, data);
   self->colourBlueVariable = lucSwarmViewer_InitialiseVariable(self, self->colourBlueVariableName, True, data);
}


void _lucSwarmRGBColourViewer_Execute( void* drawingObject, void* data ) {}
void _lucSwarmRGBColourViewer_Destroy( void* drawingObject, void* data ) {}

void _lucSwarmRGBColourViewer_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSwarmRGBColourViewer*          self                = (lucSwarmRGBColourViewer*)drawingObject;

   lucSwarmRGBColourViewer_UpdateVariables( self );

   _lucSwarmViewer_Setup( self, database, _context );

   /* Scale Colour Maps /
   if ( self->colourRedVariable && self->redColourMap )
      lucColourMap_CalibrateFromSwarmVariable( self->redColourMap, self->colourRedVariable );
   if ( self->colourGreenVariable && self->greenColourMap )
      lucColourMap_CalibrateFromSwarmVariable( self->greenColourMap, self->colourGreenVariable );
   if ( self->colourBlueVariable && self->blueColourMap )
      lucColourMap_CalibrateFromSwarmVariable( self->blueColourMap, self->colourBlueVariable );
    */
}

void _lucSwarmRGBColourViewer_SetParticleColour( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I )
{
   lucSwarmRGBColourViewer* self                  = (lucSwarmRGBColourViewer*)drawingObject;
   
   _lucSwarmViewer_SetParticleColour(drawingObject, database, lParticle_I );

   lucSwarmViewer_SetColourComponent(self, database, self->colourRedVariable, lParticle_I, lucRedValueData, self->redColourMap);
   lucSwarmViewer_SetColourComponent(self, database, self->colourGreenVariable, lParticle_I, lucGreenValueData, self->greenColourMap);
   lucSwarmViewer_SetColourComponent(self, database, self->colourBlueVariable, lParticle_I, lucBlueValueData, self->blueColourMap);

   //lucColour_SetColour( &self->colour, self->opacity );
}

void lucSwarmRGBColourViewer_UpdateVariables( void* drawingObject )
{
   lucSwarmRGBColourViewer*          self                = (lucSwarmRGBColourViewer*)drawingObject;

   lucSwarmViewer_UpdateVariables( drawingObject ) ;

   if ( self->colourRedVariable && self->colourRedVariable->variable )
      Variable_Update( self->colourRedVariable->variable );
   if ( self->colourGreenVariable && self->colourGreenVariable->variable )
      Variable_Update( self->colourGreenVariable->variable );
   if ( self->colourBlueVariable && self->colourBlueVariable->variable )
      Variable_Update( self->colourBlueVariable->variable );
}


