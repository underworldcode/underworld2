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
#include "ScalarFieldCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucScalarFieldCrossSection_Type = "lucScalarFieldCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucScalarFieldCrossSection* _lucScalarFieldCrossSection_New(  LUCSCALARFIELDCROSSSECTION_DEFARGS  )
{
   lucScalarFieldCrossSection*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucScalarFieldCrossSection) );
   self = (lucScalarFieldCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucScalarFieldCrossSection_Init(lucScalarFieldCrossSection* self)
{
}

void _lucScalarFieldCrossSection_Delete( void* drawingObject )
{
   lucScalarFieldCrossSection*  self = (lucScalarFieldCrossSection*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucScalarFieldCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucScalarFieldCrossSection*  self = (lucScalarFieldCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucScalarFieldCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucScalarFieldCrossSection);
   Type                                                             type = lucScalarFieldCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucScalarFieldCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucScalarFieldCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucScalarFieldCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucScalarFieldCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucScalarFieldCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucScalarFieldCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucScalarFieldCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucScalarFieldCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucScalarFieldCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucScalarFieldCrossSection_New(  LUCSCALARFIELDCROSSSECTION_PASSARGS  );
}

void _lucScalarFieldCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucScalarFieldCrossSection*     self = (lucScalarFieldCrossSection*)drawingObject;

   /* Construct Parent */
   self->defaultResolution = 100;   /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );

   _lucScalarFieldCrossSection_Init(self);
}

void _lucScalarFieldCrossSection_Build( void* drawingObject, void* data )
{
   lucScalarFieldCrossSection*     self        = (lucScalarFieldCrossSection*)drawingObject;

   /* Build field variable in parent */
   _lucCrossSection_Build(self, data);
}

void _lucScalarFieldCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucScalarFieldCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucScalarFieldCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucScalarFieldCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucScalarFieldCrossSection*       self            = (lucScalarFieldCrossSection*)drawingObject;
   lucScalarFieldCrossSection_DrawCrossSection( self, database, False );
}

void lucScalarFieldCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing)
{
   lucScalarFieldCrossSection* self = (lucScalarFieldCrossSection*)drawingObject;

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, backFacing);

   /* Calibrate Colour Map */
   if (self->colourMap)
      //lucColourMap_CalibrateFromFieldVariable(self->colourMap, self->fieldVariable);

   if (self->rank == 0)
   {
      int d;
      int count = self->resolutionA * self->resolutionB;
      lucDatabase_AddGridVertices(database, count, self->resolutionB, &self->vertices[0][0][0]);
      lucDatabase_AddValues(database, count, lucGridType, lucColourValueData, self->colourMap, &self->values[0][0][0]);

      /* Flip normal if back facing */
      if (backFacing)
         for (d=0; d<3; d++)
            self->normal[d] = 0 - self->normal[d];

      /* Add a single normal value */
      lucDatabase_AddNormal(database, lucGridType, self->normal);
   }

   /* Free memory */
   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}

