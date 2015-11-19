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

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucSwarmViewer_Type = "lucSwarmViewer";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucSwarmViewer* _lucSwarmViewer_New(  LUCSWARMVIEWER_DEFARGS  )
{

   lucSwarmViewer*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucSwarmViewer) );
   self = (lucSwarmViewer*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   self->_plotParticle = _plotParticle;
   self->_setParticleColour = _setParticleColour;

   return self;
}

void _lucSwarmViewer_Init(
   lucSwarmViewer*                                          self,
   Swarm*                                                       swarm,
   Name                                                         colourVariableName,
   Name                                                         sizeVariableName,
   Name                                                         opacityVariableName,
   Name                                                         maskVariableName,
   lucDrawingObjectMask*                                        mask,
   lucColourMap*                                                opacityColourMap,
   Bool                                                         drawParticleNumber,
   Bool                                                         particleColour,
   int                                                          subSample,
   Bool                                                         positionRange,
   Coord                                                        minPosition,
   Coord                                                        maxPosition)
{
   self->swarm               = swarm;
   self->colourVariableName  = colourVariableName;
   self->sizeVariableName    = sizeVariableName;
   self->opacityVariableName = opacityVariableName;
   self->maskVariableName    = maskVariableName;
   self->drawParticleNumber  = drawParticleNumber;
   self->sameParticleColour  = particleColour;
   self->subSample           = subSample;
   self->positionRange       = positionRange;

   /* Create a default colour component mapping, full range black->white */
   self->opacityColourMap = opacityColourMap ? opacityColourMap : LUC_DEFAULT_ALPHAMAP;

   memcpy( &self->mask, mask, sizeof( lucDrawingObjectMask ) );
   memcpy( &self->minPosition, minPosition , sizeof( Coord ) );
   memcpy( &self->maxPosition, maxPosition , sizeof( Coord ) );

   self->geomType = lucPointType;   /* Draws points by default */
}

void _lucSwarmViewer_Delete( void* drawingObject )
{
   lucSwarmViewer*  self = (lucSwarmViewer*)drawingObject;

   _lucDrawingObject_Delete( self );

}

void _lucSwarmViewer_Print( void* drawingObject, Stream* stream )
{
   lucSwarmViewer*  self = (lucSwarmViewer*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucSwarmViewer_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap)
{
   lucSwarmViewer*  self = (lucSwarmViewer*)drawingObject;
   lucSwarmViewer* newDrawingObject;

   newDrawingObject = _lucDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newDrawingObject;
}

void* _lucSwarmViewer_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucSwarmViewer);
   Type                                                             type = lucSwarmViewer_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucSwarmViewer_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucSwarmViewer_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucSwarmViewer_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucSwarmViewer_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucSwarmViewer_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucSwarmViewer_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucSwarmViewer_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucSwarmViewer_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucSwarmViewer_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucSwarmViewer_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;
   lucSwarmViewer_PlotParticleFunction*                _plotParticle = _lucSwarmViewer_PlotParticle;
   lucSwarmViewer_SetParticleColourFunction*      _setParticleColour = _lucSwarmViewer_SetParticleColour;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucSwarmViewer_New(  LUCSWARMVIEWER_PASSARGS  );
}

void _lucSwarmViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucSwarmViewer*     self = (lucSwarmViewer*)drawingObject;
   Swarm*                  swarm;
   Name                    colourVariableName;
   Name                    sizeVariableName;
   Name                    opacityVariableName;
   Name                    maskVariableName;
   Bool                    drawParticleNumber;
   Bool                    sameParticleColour;
   int                     subSample;
   Bool                    positionRange;
   Coord                   minPosition;
   Coord                   maxPosition;
   lucDrawingObjectMask    mask;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   swarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", Swarm, True, data  ) ;

   colourVariableName  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"ColourVariable", ""  );
   sizeVariableName  = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"SizeVariable", ""  );
   opacityVariableName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"OpacityVariable", ""  );
   maskVariableName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"MaskVariable", ""  );

   drawParticleNumber = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"drawParticleNumber", False  );
   sameParticleColour = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"sameParticleColour", False  );

   subSample = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"subSample", 0  );
   positionRange = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"positionRange", False  );

   /* Memory allocation */
   minPosition[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minPositionX", -HUGE_VAL  );
   minPosition[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minPositionY", -HUGE_VAL  );
   minPosition[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"minPositionZ", -HUGE_VAL  );

   maxPosition[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxPositionX", HUGE_VAL  );
   maxPosition[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxPositionY", HUGE_VAL  );
   maxPosition[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxPositionZ", HUGE_VAL  );


   lucDrawingObjectMask_Construct( &mask, self->name, cf, data );

   _lucSwarmViewer_Init(
      self,
      swarm,
      colourVariableName,
      sizeVariableName,
      opacityVariableName,
      maskVariableName,
      &mask,
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"OpacityColourMap", lucColourMap, False, data),
      drawParticleNumber,
      sameParticleColour,
      subSample,
      positionRange,
      minPosition,
      maxPosition);
}

void _lucSwarmViewer_Build( void* drawingObject, void* data )
{
   lucSwarmViewer*          self                   = (lucSwarmViewer*)drawingObject;

   _lucDrawingObject_Build( self, data );

}

SwarmVariable* lucSwarmViewer_InitialiseVariable(void* object, Name variableName, Bool scalarRequired, void* data )
{
   lucSwarmViewer* self = (lucSwarmViewer*)object;
   SwarmVariable_Register*	swarmVariable_Register = self->swarm->swarmVariable_Register;
   SwarmVariable* variable = NULL;

   if ( 0 != strcmp( variableName, "" ) )
   {
      variable = SwarmVariable_Register_GetByName( swarmVariable_Register, variableName );
      Journal_Firewall( variable != NULL, lucError,
                        "Error - for gLucifer drawing object \"%s\" - in %s(): Colour Variable name given was \"%s\", "
                        "but no corresponding SwarmVariable found in the register for swarm \"%s\".\n",
                        self->name, __func__, variableName, self->swarm->name );

      Stg_Component_Build( variable, data, False );
      Stg_Component_Initialise( variable, data, False );

      /* Check if scalar, when required */
      if ( variable && scalarRequired )
      {
         Journal_Firewall( variable->dofCount <= 1, lucError,
                        "Error - in %s(): provided SwarmVariable \"%s\" has %u components - but %s Component "
                        "can only visualise FieldVariables with 1 component. Did you mean to visualise the "
                        "magnitude of the given field?\n", __func__, variable->name,
                        variable->dofCount, self->type );
      }
   }
   return variable;
}


void _lucSwarmViewer_Initialise( void* drawingObject, void* data )
{
   lucSwarmViewer* self = (lucSwarmViewer*)drawingObject;

   /* Initialise Parent */
   _lucDrawingObject_Initialise( self, data );

//   self->colourVariable = lucSwarmViewer_InitialiseVariable(self, self->colourVariableName, True, data);
//   self->sizeVariable = lucSwarmViewer_InitialiseVariable(self, self->sizeVariableName, True, data);
//   self->opacityVariable = lucSwarmViewer_InitialiseVariable(self, self->opacityVariableName, True, data);
//   self->maskVariable = lucSwarmViewer_InitialiseVariable(self, self->maskVariableName, False, data);
}


void _lucSwarmViewer_Execute( void* drawingObject, void* data ) {}
void _lucSwarmViewer_Destroy( void* drawingObject, void* data ) {}

void _lucSwarmViewer_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSwarmViewer*          self                = (lucSwarmViewer*)drawingObject;
   lucColourMap*                colourMap           = self->colourMap;
   SwarmVariable*               colourVariable      = self->colourVariable;

   lucSwarmViewer_UpdateVariables( self );

   /* Scale Colour Maps */
   if ( colourVariable && colourMap )
      lucColourMap_CalibrateFromSwarmVariable( colourMap, colourVariable );
   if ( self->opacityVariable && self->opacityColourMap )
      lucColourMap_CalibrateFromSwarmVariable( self->opacityColourMap, self->opacityVariable );
}

float lucSwarmViewer_GetScalar(SwarmVariable* variable, Particle_Index lParticle_I, float defaultVal)
{
   double value;
   if (variable)
   {
      Index count = variable->dofCount;
      if (count == 1)
         SwarmVariable_ValueAt( variable, lParticle_I, &value );
      else
      {
         double* var = Memory_Alloc_Array(double, count, (Name)"swarm var");
         SwarmVariable_ValueAt( variable, lParticle_I, var );
         value = StGermain_VectorMagnitude(var, count);
      }
      return value;
   }
   return defaultVal;
}

#define SEED_VAL 12345
void _lucSwarmViewer_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSwarmViewer*      self                = (lucSwarmViewer*)drawingObject;
   Swarm*                   swarm               = self->swarm;
   SwarmVariable*           maskVariable        = self->maskVariable;
   Particle_Index           particleLocalCount  = swarm->particleLocalCount;
   Particle_Index           lParticle_I;
   double                   maskResult;
   int                      subSample           = self->subSample;
   Bool                     positionRange       = self->positionRange;
   GlobalParticle*          particle;
   double*                  coord;
   double*                  minPosition;
   double*                  maxPosition;

   minPosition = self->minPosition;
   maxPosition = self->maxPosition;

   if (subSample == 0)
   {
      int particles;
      MPI_Allreduce(&swarm->particleLocalCount, &particles, 1, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);
      subSample = 1;
      if (particles > 2000000) 
      {
         subSample = particles / 2000000;
         Journal_Printf(lucInfo, "*** Auto sub-sampling %d particle swarm %s by %d to %d particles\n", particles, self->name, subSample, particles / subSample);
      }
   }

   /* Make sure subSampling is deterministic by seeding random generator with a constant value */
   srand(SEED_VAL); 
   for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++)
   {
      /* If subSampling, use a pseudo random distribution to select which particles to draw */
      /* If we just draw every n'th particle, we end up with a whole bunch in one region / proc */
      if (subSample && rand() % subSample > 0) continue;

      /* Test to see if this particle should be drawn */
      if ( maskVariable )
      {
         SwarmVariable_ValueAt( maskVariable, lParticle_I, &maskResult );
         if ( lucDrawingObjectMask_Test( &self->mask, maskResult ) == False )
            continue;
      }

      /* Check if needed that the particle falls into the right position range */
      particle            = (GlobalParticle*)Swarm_ParticleAt( self->swarm, lParticle_I );
      coord               = particle->coord;
      if (positionRange)
      {

         if ( coord[0] <= minPosition[I_AXIS] || coord[1] <= minPosition[J_AXIS] ||
               coord[0] >= maxPosition[I_AXIS] || coord[1] >= maxPosition[J_AXIS] )
            continue;

         if (swarm->dim == 3 && (coord[2] <= minPosition[K_AXIS] || coord[2] >= maxPosition[K_AXIS]))
            continue;
      }

      /* Export particle position */
      float coordf[3] = {coord[0], coord[1], swarm->dim == 3 ? coord[2] : 0.0f};
      lucDatabase_AddVertices(database, 1, self->geomType, coordf);

      /* Sets the colour for the particle */
      self->_setParticleColour( self, database, lParticle_I );

      /* Plot the particle using the function given by the concrete class */
      self->_plotParticle( self, database, lParticle_I );

   }

   /* Go through the list of the particles again and write the text of the numbers next to each other */
   if ( self->drawParticleNumber )
      lucSwarmViewBase_DrawParticleNumbers( self, _context );
}

void lucSwarmViewBase_DrawParticleNumbers( void* drawingObject, void* _context )
{
   abort();
}

void _lucSwarmViewer_PlotParticleNumber( void* drawingObject, Particle_Index lParticle_I, lucColour colour )
{
/*
   lucSwarmViewer*      self                = (lucSwarmViewer*)drawingObject;
   GlobalParticle*          particle            = (GlobalParticle*)Swarm_ParticleAt( self->swarm, lParticle_I );
   double*                  coord               = particle->coord;
   Name particle_number;
   Stg_asprintf(&particle_number, "%d", lParticle_I );

   if (self->swarm->dim == 2)
      glRasterPos2f( (float)coord[0] + 0.025, (float)coord[1] );
   else
      glRasterPos3f( (float)coord[0] + 0.025, (float)coord[1], (float)coord[2] );

   lucPrintString( particle_number );
   Memory_Free(particle_number);
*/
}

void lucSwarmViewer_FindParticleLocalIndex(void *drawingObject, Coord coord, Particle_Index *lParticle_I)
{
   lucSwarmViewer*      self  = (lucSwarmViewer*) drawingObject;
   Swarm*               swarm = self->swarm;
   Dimension_Index      dim = self->swarm->dim;
   Particle_InCellIndex cParticle_I;
   Cell_LocalIndex      lCell_I;
   GlobalParticle       testParticle;
   double               minDistance;

   /* Find cell this coordinate is in */
   memcpy( testParticle.coord, coord, sizeof(Coord) );
   /* First specify the particle doesn't have an owning cell yet, so as
      not to confuse the search algorithm */
   testParticle.owningCell = swarm->cellDomainCount;
   lCell_I = CellLayout_CellOf( swarm->cellLayout, &testParticle );

   /* Test if this cell is on this processor - if not then bail */
   if (lCell_I >= swarm->cellLocalCount)
   {
      *lParticle_I = (Particle_Index) -1;
      return;
   }

   /* Find Closest Particle in this Cell */
   cParticle_I = Swarm_FindClosestParticleInCell( swarm, lCell_I, dim, coord, &minDistance );

   /* Convert to Local Particle Index */
   *lParticle_I = swarm->cellParticleTbl[ lCell_I ][ cParticle_I ];
}

void lucSwarmViewer_UpdateVariables( void* drawingObject )
{
   lucSwarmViewer*          self                = (lucSwarmViewer*)drawingObject;

   if ( self->opacityVariable && self->opacityVariable->variable )
      Variable_Update( self->opacityVariable->variable );
   if ( self->maskVariable && self->maskVariable->variable )
      Variable_Update( self->maskVariable->variable );
}

void lucSwarmViewer_SetColourComponent(void* object, lucDatabase* database, SwarmVariable* var, Particle_Index lParticle_I, lucGeometryDataType type, lucColourMap* colourMap)
{
   lucSwarmViewer* self = (lucSwarmViewer*)object;
   if (var && colourMap)
   {
      double value;
      lucColour colour;
      SwarmVariable_ValueAt( var, lParticle_I, &value );
      lucColourMap_GetColourFromValue( colourMap, value, &colour, self->opacity );

      /* Extract and overwrite component value */
      if (type == lucRedValueData)
         self->colour.red = colour.red;
      if (type == lucGreenValueData)
         self->colour.green = colour.green;
      if (type == lucBlueValueData)
         self->colour.blue = colour.blue;
      if (type == lucOpacityValueData)
         self->colour.opacity = colour.opacity;

      /* Export particle value */
      float valuef = value;
      lucDatabase_AddValues(database, 1, self->geomType, type, colourMap, &valuef);
   }
}

/* Default Swarm Viewer Implementation */
void _lucSwarmViewer_SetParticleColour( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I )
{
   lucSwarmViewer*      self                = (lucSwarmViewer*) drawingObject;
   SwarmVariable*           colourVariable      = self->colourVariable;
   SwarmVariable*           opacityVariable     = self->opacityVariable;
   lucColourMap*            colourMap           = self->colourMap;
   double                   colourValue;

   /* Get colour value if there is a colourVariable and a colourMap */
   if ( colourVariable && colourMap )
   {
      SwarmVariable_ValueAt( colourVariable, lParticle_I, &colourValue );
      lucColourMap_GetColourFromValue( colourMap, colourValue, &self->colour, self->opacity );
      /* Export particle colour value */
      float valuef = colourValue;
      if (database) lucDatabase_AddValues(database, 1, self->geomType, lucColourValueData, colourMap, &valuef);
   }

   /* Get Opacity Value */
   lucSwarmViewer_SetColourComponent(self, database, opacityVariable, lParticle_I, lucOpacityValueData, self->opacityColourMap);

   lucColour_SetColour( &self->colour, self->opacity );
}

void _lucSwarmViewer_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I )
{
   lucSwarmViewer*          self          = (lucSwarmViewer*)drawingObject;
   float size = 1.0; //= self->pointSize;
   if (self->sizeVariable)
   {
      size = lucSwarmViewer_GetScalar(self->sizeVariable, lParticle_I, size);
      lucDatabase_AddValues(database, 1, self->geomType, lucSizeData, NULL, &size);
   }
}

