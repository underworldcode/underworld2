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
#include "VectorArrowMeshCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucVectorArrowMeshCrossSection_Type = "lucVectorArrowMeshCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucVectorArrowMeshCrossSection* _lucVectorArrowMeshCrossSection_New(  LUCVECTORARROWMESHCROSSSECTION_DEFARGS  )
{
   lucVectorArrowMeshCrossSection*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucVectorArrowMeshCrossSection) );
   self = (lucVectorArrowMeshCrossSection*) _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_PASSARGS  );

   return self;
}

void _lucVectorArrowMeshCrossSection_Init(
   lucVectorArrowMeshCrossSection*                                  self,
   double                                                       maximum,
   Bool                                                         dynamicRange)
{
   self->maximum = maximum;
   self->dynamicRange = dynamicRange;

   /* Get the sampler to move away from boundaries */
   self->offsetEdges = True;
}

void _lucVectorArrowMeshCrossSection_Delete( void* drawingObject )
{
   lucVectorArrowMeshCrossSection*  self = (lucVectorArrowMeshCrossSection*)drawingObject;
   _lucMeshCrossSection_Delete( self );
}

void _lucVectorArrowMeshCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucVectorArrowMeshCrossSection*  self = (lucVectorArrowMeshCrossSection*)drawingObject;

   _lucMeshCrossSection_Print( self, stream );
}

void* _lucVectorArrowMeshCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucVectorArrowMeshCrossSection);
   Type                                                             type = lucVectorArrowMeshCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucVectorArrowMeshCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucVectorArrowMeshCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucVectorArrowMeshCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucVectorArrowMeshCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucVectorArrowMeshCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucVectorArrowMeshCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucVectorArrowMeshCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucVectorArrowMeshCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucVectorArrowMeshCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucVectorArrowMeshCrossSection_New(  LUCVECTORARROWMESHCROSSSECTION_PASSARGS  );
}

void _lucVectorArrowMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucVectorArrowMeshCrossSection* self = (lucVectorArrowMeshCrossSection*)drawingObject;

   /* Construct Parent */
   _lucMeshCrossSection_AssignFromXML( self, cf, data );
   self->gatherData = False;     /* Drawn in parallel */

   _lucVectorArrowMeshCrossSection_Init(
      self,
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maximum", 1.0  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"dynamicRange", True  ));
}

void _lucVectorArrowMeshCrossSection_Build( void* drawingObject, void* data )
{
   lucVectorArrowMeshCrossSection*  self           = (lucVectorArrowMeshCrossSection*)drawingObject;
   /* Build field variable in parent */
   _lucMeshCrossSection_Build(drawingObject, data);

}

void _lucVectorArrowMeshCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucVectorArrowMeshCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucVectorArrowMeshCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucVectorArrowMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucVectorArrowMeshCrossSection*       self            = (lucVectorArrowMeshCrossSection*)drawingObject;
   DomainContext*            context         = (DomainContext*) _context;

   _lucVectorArrowMeshCrossSection_DrawCrossSection( self, database );
}

void _lucVectorArrowMeshCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database )
{
   lucVectorArrowMeshCrossSection*  self           = (lucVectorArrowMeshCrossSection*)drawingObject;
   double            min = 0.0, max = self->maximum;

   if ( True == self->dynamicRange )
   {
//      min = FieldVariable_GetMinGlobalFieldMagnitude( vectorVariable );  
//      max = FieldVariable_GetMaxGlobalFieldMagnitude( vectorVariable );
   }

   /* Force 3d vectors */
   self->fieldDim = 3;

   /* Sample the 2d cross-section */
   lucMeshCrossSection_Sample(self, False);

   if (self->rank == 0 && database)
   {
      int count = self->dims[1] * self->dims[2];
      lucDatabase_AddVertices(database, count, lucVectorType, &self->vertices[0][0][0]);
      lucDatabase_AddVectors(database, count, lucVectorType, min, max, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}


