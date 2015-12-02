/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <petsc.h>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/ParticleCoordinate.hpp>
#include <Underworld/Function/Function.hpp>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <gLucifer/Base/Base.h>
#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
}

#include "SwarmViewer.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucSwarmViewer_Type = "lucSwarmViewer";


void _lucSwarmViewer_SetFn( void* _self, Fn::Function* fn_colour, Fn::Function* fn_mask, Fn::Function* fn_size, Fn::Function* fn_opacity ){
    lucSwarmViewer*  self = (lucSwarmViewer*)_self;
    
    lucSwarmViewer_cppdata* cppdata = (lucSwarmViewer_cppdata*) self->cppdata;
    // setup fn input.  first make particle coordinate
    std::shared_ptr<ParticleCoordinate> particleCoord = std::make_shared<ParticleCoordinate>( self->swarm->particleCoordVariable );
    // grab first particle for sample data
    particleCoord->index() = 0;

    // record fns, and also throw in a min/max guy where required
    if (fn_colour)
    {
        cppdata->fn_colour   = std::make_shared<Fn::MinMax>(fn_colour);
        cppdata->func_colour = cppdata->fn_colour->getFunction(particleCoord);
        std::shared_ptr<const FunctionIO> io = std::dynamic_pointer_cast<const FunctionIO>(cppdata->func_colour(particleCoord));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        if( io->size() != 1 )
            throw std::invalid_argument("Provided function must return a scalar result.");
    }

    if (fn_mask)
    {
        cppdata->fn_mask   = fn_mask;
        cppdata->func_mask = cppdata->fn_mask->getFunction(particleCoord);
        std::shared_ptr<const FunctionIO> io = std::dynamic_pointer_cast<const FunctionIO>(cppdata->func_mask(particleCoord));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        if( io->size() != 1 )
            throw std::invalid_argument("Provided function must return a scalar result.");
    }

    if (fn_size)
    {
        cppdata->fn_size   = std::make_shared<Fn::MinMax>(fn_size);
        cppdata->func_size = cppdata->fn_size->getFunction(particleCoord);
        std::shared_ptr<const FunctionIO> io = std::dynamic_pointer_cast<const FunctionIO>(cppdata->func_size(particleCoord));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        if( io->size() != 1 )
            throw std::invalid_argument("Provided function must return a scalar result.");
    }

    if (fn_opacity)
    {
        cppdata->fn_opacity   = std::make_shared<Fn::MinMax>(fn_opacity);
        cppdata->func_opacity = cppdata->fn_opacity->getFunction(particleCoord);
        std::shared_ptr<const FunctionIO> io = std::dynamic_pointer_cast<const FunctionIO>(cppdata->func_opacity(particleCoord));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        if( io->size() != 1 )
            throw std::invalid_argument("Provided function must return a scalar result.");
    }
    
}

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucSwarmViewer* _lucSwarmViewer_New(  LUCSWARMVIEWER_DEFARGS  )
{
   lucSwarmViewer* self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucSwarmViewer) );
   self = (lucSwarmViewer*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   self->_plotParticle = _plotParticle;
   self->_setParticleColour = _setParticleColour;
   
   self->cppdata = (void*) new lucSwarmViewer_cppdata;

   return self;
}

void _lucSwarmViewer_Init(
   lucSwarmViewer*            self,
   GeneralSwarm*              swarm,
   lucColourMap*              opacityColourMap)
{
   self->swarm               = swarm;

   /* Create a default colour component mapping, full range black->white */
   self->opacityColourMap = opacityColourMap ? opacityColourMap : LUC_DEFAULT_ALPHAMAP;

   self->geomType = lucPointType;   /* Draws points by default */
}

void _lucSwarmViewer_Delete( void* drawingObject )
{
   lucSwarmViewer*  self = (lucSwarmViewer*)drawingObject;

   if (self->cppdata)
       delete (lucSwarmViewer_cppdata*)self->cppdata;

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

   newDrawingObject = (lucSwarmViewer*) _lucDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

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
   GeneralSwarm*           swarm;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   swarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", GeneralSwarm, True, data  ) ;

   _lucSwarmViewer_Init(
      self,
      swarm,
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"OpacityColourMap", lucColourMap, False, data) );
}

void _lucSwarmViewer_Build( void* drawingObject, void* data )
{
   lucSwarmViewer*          self                   = (lucSwarmViewer*)drawingObject;

   _lucDrawingObject_Build( self, data );

}

void _lucSwarmViewer_Initialise( void* drawingObject, void* data )
{
   lucSwarmViewer* self = (lucSwarmViewer*)drawingObject;

   /* Initialise Parent */
   _lucDrawingObject_Initialise( self, data );

}


void _lucSwarmViewer_Execute( void* drawingObject, void* data ) {}
void _lucSwarmViewer_Destroy( void* drawingObject, void* data ) {}
void _lucSwarmViewer_Setup( void* drawingObject, lucDatabase* database, void* _context ) {}

void _lucSwarmViewer_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSwarmViewer*      self                = (lucSwarmViewer*)drawingObject;
   GeneralSwarm*        swarm               = self->swarm;
   Particle_Index           particleLocalCount  = swarm->particleLocalCount;
   Particle_Index           lParticle_I;
   
   lucSwarmViewer_cppdata* cppdata = (lucSwarmViewer_cppdata*) self->cppdata;

   /* reset the min/max values */
   if (cppdata->fn_colour)
      cppdata->fn_colour->reset();
   if (cppdata->fn_size)
      cppdata->fn_size->reset();
   if (cppdata->fn_opacity)
      cppdata->fn_opacity->reset();

   // setup fn_io.
   std::shared_ptr<ParticleCoordinate> particleCoord = std::make_shared<ParticleCoordinate>( self->swarm->particleCoordVariable );

   for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++)
   {
      particleCoord->index() = lParticle_I;
      /* note we need to cast object to const version to ensure it selects const data() method */
      const double* coord = std::const_pointer_cast< const ParticleCoordinate>(particleCoord)->data();
      /* Test to see if this particle should be drawn */
      if ( cppdata->fn_mask && !cppdata->func_mask(particleCoord)->at<bool>())
         continue;

      /* Export particle position */
      float coordf[3] = {(float)coord[0],(float) coord[1], swarm->dim == 3 ? (float)coord[2] : 0.0f};
      lucDatabase_AddVertices(database, 1, self->geomType, coordf);

      if (cppdata->fn_colour) {
         /* evaluate function */
         float valuef = cppdata->func_colour(particleCoord)->at<float>();
         lucDatabase_AddValues(database, 1, self->geomType, lucColourValueData, NULL, &valuef);
      }
      if (cppdata->fn_size) {
         /* evaluate function */
         float valuef = cppdata->func_size(particleCoord)->at<float>();
         lucDatabase_AddValues(database, 1, self->geomType, lucSizeData, NULL, &valuef);
      }
   }
   
   /* Scale Colour Maps */
   if ( self->colourMap && self->colourMap->dynamicRange && cppdata->fn_colour ){
      lucColourMap_SetMinMax( self->colourMap, cppdata->fn_colour->getMinGlobal(), cppdata->fn_colour->getMaxGlobal() );
      /* do a size zero add just so we can reset the colourmap */
      lucDatabase_AddValues(database, 0, self->geomType, lucColourValueData, self->colourMap, NULL);
  }

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
//   lucSwarmViewer*      self                = (lucSwarmViewer*) drawingObject;
//   SwarmVariable*           colourVariable      = self->colourVariable;
//   SwarmVariable*           opacityVariable     = self->opacityVariable;
//   lucColourMap*            colourMap           = self->colourMap;
//   double                   colourValue;
//
//   /* Get colour value if there is a colourVariable and a colourMap */
//   if ( colourVariable && colourMap )
//   {
//      SwarmVariable_ValueAt( colourVariable, lParticle_I, &colourValue );
//      lucColourMap_GetColourFromValue( colourMap, colourValue, &self->colour, self->opacity );
//      /* Export particle colour value */
//      float valuef = colourValue;
//      if (database) lucDatabase_AddValues(database, 1, self->geomType, lucColourValueData, colourMap, &valuef);
//   }
//
//   /* Get Opacity Value */
//   lucSwarmViewer_SetColourComponent(self, database, opacityVariable, lParticle_I, lucOpacityValueData, self->opacityColourMap);
//
//   lucColour_SetColour( &self->colour, self->opacity );
}

void _lucSwarmViewer_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I )
{
//   lucSwarmViewer*          self          = (lucSwarmViewer*)drawingObject;
//   float size =  1.0; //= self->pointSize;
//   if (self->sizeVariable)
//   {
//      size = lucSwarmViewer_GetScalar(self->sizeVariable, lParticle_I, size);
//      lucDatabase_AddValues(database, 1, self->geomType, lucSizeData, NULL, &size);
//   }
}

float lucSwarmViewer_GetScalar( SwarmVariable* variable, Particle_Index lParticle_I, float defaultVal ){ abort(); }
SwarmVariable* lucSwarmViewer_InitialiseVariable(void* object, Name variableName, Bool scalarRequired, void* data ){ abort(); }
void lucSwarmViewer_UpdateVariables( void *drawingObject ){ abort(); } ;
