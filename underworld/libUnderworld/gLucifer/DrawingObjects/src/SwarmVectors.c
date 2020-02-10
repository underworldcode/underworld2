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
#include "SwarmVectors.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucSwarmVectors_Type = "lucSwarmVectors";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucSwarmVectors* _lucSwarmVectors_New(  LUCSWARMVECTORS_DEFARGS  )
{
   lucSwarmVectors*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucSwarmVectors) );
   self = (lucSwarmVectors*) _lucSwarmViewer_New(  LUCSWARMVIEWER_PASSARGS  );

   return self;
}

void _lucSwarmVectors_Init(
   lucSwarmVectors*                                   self,
   Name                                               directionVariableName,
   double                                             arrowHeadSize,
   Name                                               thicknessVariableName,
   double                                             thickness,
   Name                                               lengthVariableName,
   double                                             length )
{
   self->directionVariableName = directionVariableName;
   self->arrowHeadSize         = arrowHeadSize;
   self->thicknessVariableName = thicknessVariableName;
   self->thickness             = thickness;
   self->lengthVariableName    = lengthVariableName;
   self->length                = length;

   self->geomType = lucVectorType;   /* Draw vectors */

   /* Append scaling factors to property string */
   //OK: DEPRECATED, SET VIA PYTHON
   //lucDrawingObject_AppendProps(self, "length=%g\nthickness=%g\narrowhead=%g\n", length, thickness, arrowHeadSize);
}

void _lucSwarmVectors_Delete( void* drawingObject )
{
   lucSwarmVectors*  self = (lucSwarmVectors*)drawingObject;

   _lucSwarmViewer_Delete( self );
}

void _lucSwarmVectors_Print( void* drawingObject, Stream* stream )
{
   lucSwarmVectors*  self = (lucSwarmVectors*)drawingObject;

   _lucSwarmViewer_Print( self, stream );
}

void* _lucSwarmVectors_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap)
{
   lucSwarmVectors*  self = (lucSwarmVectors*)drawingObject;
   lucSwarmVectors* newDrawingObject;

   newDrawingObject = _lucSwarmViewer_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newDrawingObject;
}


void* _lucSwarmVectors_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucSwarmVectors);
   Type                                                             type = lucSwarmVectors_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucSwarmVectors_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucSwarmVectors_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucSwarmVectors_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucSwarmVectors_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucSwarmVectors_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucSwarmVectors_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucSwarmVectors_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucSwarmVectors_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucSwarmVectors_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucSwarmViewer_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;
   lucSwarmViewer_PlotParticleFunction*                _plotParticle = _lucSwarmVectors_PlotParticle;
   lucSwarmViewer_SetParticleColourFunction*      _setParticleColour = _lucSwarmViewer_SetParticleColour;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucSwarmVectors_New(  LUCSWARMVECTORS_PASSARGS  );
}

void _lucSwarmVectors_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucSwarmVectors*  self = (lucSwarmVectors*)drawingObject;

   /* Construct Parent */
   _lucSwarmViewer_AssignFromXML( self, cf, data );

   _lucSwarmVectors_Init(
      self,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"DirectionVariable", ""  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"arrowHeadSize", 2.0  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"ThicknessVariable", ""  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"thickness", 0.0  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"LengthVariable", ""  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"length", 1.0 )  );
}

void _lucSwarmVectors_Build( void* drawingObject, void* data )
{
   lucSwarmVectors*         self                   = (lucSwarmVectors*)drawingObject;

   _lucSwarmViewer_Build( self, data );
}
void _lucSwarmVectors_Initialise( void* drawingObject, void* data )
{
   lucSwarmVectors*         self                   = (lucSwarmVectors*)drawingObject;

   _lucSwarmViewer_Initialise( self, data );

   self->directionVariable = lucSwarmViewer_InitialiseVariable(self, self->directionVariableName, False, data);
   self->lengthVariable = lucSwarmViewer_InitialiseVariable(self, self->lengthVariableName, False, data);
   self->thicknessVariable = lucSwarmViewer_InitialiseVariable(self, self->thicknessVariableName, False, data);

}
void _lucSwarmVectors_Execute( void* drawingObject, void* data ) {}
void _lucSwarmVectors_Destroy( void* drawingObject, void* data ) {}

void _lucSwarmVectors_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   _lucSwarmViewer_Setup( drawingObject, database, _context );
   lucSwarmVectors_UpdateVariables( drawingObject );
}

void _lucSwarmVectors_PlotParticle( void* drawingObject, lucDatabase* database, Particle_Index lParticle_I )
{
   lucSwarmVectors*        self                = (lucSwarmVectors*)drawingObject;
   SwarmVariable*          lengthVariable      = self->lengthVariable;
   SwarmVariable*          thicknessVariable   = self->thicknessVariable;
   float                   length;
   XYZ                     direction           = { 1, 0, 0 };

   SwarmVariable_ValueAt( self->directionVariable, lParticle_I, direction );

   length = lucSwarmViewer_GetScalar(lengthVariable, lParticle_I, self->length);

   //float max = self->directionVariable ? FieldVariable_GetMaxGlobalFieldMagnitude( self->directionVariable ) : 0;
   float vecf[3] = {direction[0]*length, direction[1]*length, direction[2]*length};
   lucDatabase_AddVectors(database, 1, lucVectorType, 0, 0, vecf);

   if (thicknessVariable)
   {
      float thickness = lucSwarmViewer_GetScalar(thicknessVariable, lParticle_I, self->thickness);
      lucDatabase_AddValues(database, 1, self->geomType, lucXWidthData, NULL, &thickness);
   }
}

void lucSwarmVectors_UpdateVariables( void* drawingObject )
{
   lucSwarmVectors*          self                = (lucSwarmVectors*)drawingObject;
   lucSwarmViewer_UpdateVariables( drawingObject ) ;

   if ( self->directionVariable && self->directionVariable->variable )
      StgVariable_Update( self->directionVariable->variable );
   if ( self->thicknessVariable && self->thicknessVariable->variable )
      StgVariable_Update( self->thicknessVariable->variable );
   if ( self->lengthVariable && self->lengthVariable->variable )
      StgVariable_Update( self->lengthVariable->variable );
}


