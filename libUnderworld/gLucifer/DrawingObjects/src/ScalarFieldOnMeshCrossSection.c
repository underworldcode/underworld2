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
#include <StgFEM/StgFEM.h>

#include <gLucifer/Base/Base.h>


#include "types.h"
#include "CrossSection.h"
#include "ScalarFieldOnMeshCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucScalarFieldOnMeshCrossSection_Type = "lucScalarFieldOnMeshCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucScalarFieldOnMeshCrossSection* _lucScalarFieldOnMeshCrossSection_New(  LUCSCALARFIELDONMESHCROSSSECTION_DEFARGS  )
{
   lucScalarFieldOnMeshCrossSection*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucScalarFieldOnMeshCrossSection) );
   self = (lucScalarFieldOnMeshCrossSection*) _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_PASSARGS  );

   return self;
}

void _lucScalarFieldOnMeshCrossSection_Init(
   lucScalarFieldOnMeshCrossSection*                            self)
{
   self->flipNormals = False;
}

void _lucScalarFieldOnMeshCrossSection_Delete( void* drawingObject )
{
   lucScalarFieldOnMeshCrossSection*  self = (lucScalarFieldOnMeshCrossSection*)drawingObject;

   _lucMeshCrossSection_Delete( self );
}

void _lucScalarFieldOnMeshCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucScalarFieldOnMeshCrossSection*  self = (lucScalarFieldOnMeshCrossSection*)drawingObject;

   _lucMeshCrossSection_Print( self, stream );
}

void* _lucScalarFieldOnMeshCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucScalarFieldOnMeshCrossSection);
   Type                                                             type = lucScalarFieldOnMeshCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucScalarFieldOnMeshCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucScalarFieldOnMeshCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucScalarFieldOnMeshCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucScalarFieldOnMeshCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucScalarFieldOnMeshCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucScalarFieldOnMeshCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucScalarFieldOnMeshCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucScalarFieldOnMeshCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucScalarFieldOnMeshCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucScalarFieldOnMeshCrossSection_New(  LUCSCALARFIELDONMESHCROSSSECTION_PASSARGS  );
}

void _lucScalarFieldOnMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucScalarFieldOnMeshCrossSection*     self = (lucScalarFieldOnMeshCrossSection*)drawingObject;

   /* Construct Parent */
   _lucMeshCrossSection_AssignFromXML( self, cf, data );

   _lucScalarFieldOnMeshCrossSection_Init(self);
}

void _lucScalarFieldOnMeshCrossSection_Build( void* drawingObject, void* data )
{
   lucScalarFieldOnMeshCrossSection*     self    = (lucScalarFieldOnMeshCrossSection*)drawingObject;
   /* Build field variable in parent */
   _lucMeshCrossSection_Build(self, data);

}

void _lucScalarFieldOnMeshCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucScalarFieldOnMeshCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucScalarFieldOnMeshCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucScalarFieldOnMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucScalarFieldOnMeshCrossSection*       self            = (lucScalarFieldOnMeshCrossSection*)drawingObject;
   lucScalarFieldOnMeshCrossSection_DrawCrossSection( self, database, False);
}

void lucScalarFieldOnMeshCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing)
{
   lucScalarFieldOnMeshCrossSection*       self            = (lucScalarFieldOnMeshCrossSection*)drawingObject;

   lucMeshCrossSection_Sample(self, backFacing);

   /* Calibrate Colour Map */
   if (self->colourMap)
      //lucColourMap_CalibrateFromFieldVariable(self->colourMap, self->fieldVariable);

   if (self->rank == 0 && database)
   {
      int count = self->dims[1] * self->dims[2];
      lucDatabase_AddGridVertices(database, count, self->dims[2], self->vertices[0][0]);
      lucDatabase_AddValues(database, count, lucGridType, lucColourValueData, self->colourMap, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}


