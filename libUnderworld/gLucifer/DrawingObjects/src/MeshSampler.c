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
#include <gLucifer/Base/DrawingObject.h>
#include "MeshSampler.h"

void lucMeshSampler_SampleLocal( void* drawingObject, void* _context );
void lucMeshSampler_SampleGlobal( void* drawingObject, void* _context );

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucMeshSampler_Type = "lucMeshSampler";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucMeshSampler* _lucMeshSampler_New(  LUCMESHSAMPLER_DEFARGS  )
{
   lucMeshSampler*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucMeshSampler) );
   self = (lucMeshSampler*) _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_PASSARGS  );

   return self;
}

void _lucMeshSampler_Init(lucMeshSampler*           self)
{
}

void _lucMeshSampler_Delete( void* drawingObject )
{
   lucMeshSampler*  self = (lucMeshSampler*)drawingObject;
   _lucMeshCrossSection_Delete( self );
}

void _lucMeshSampler_Print( void* drawingObject, Stream* stream )
{
   lucMeshSampler*  self = (lucMeshSampler*)drawingObject;

   _lucMeshCrossSection_Print( self, stream );
}

void* _lucMeshSampler_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucMeshSampler);
   Type                                                             type = lucMeshSampler_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucMeshSampler_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucMeshSampler_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucMeshSampler_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucMeshSampler_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucMeshSampler_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucMeshSampler_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucMeshSampler_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucMeshSampler_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucMeshSampler_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucMeshSampler_New(  LUCMESHSAMPLER_PASSARGS  );
}

void _lucMeshSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucMeshSampler*         self               = (lucMeshSampler*)drawingObject;
   Index                  defaultRes;

   self->elementRes[I_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResI"  );
   self->elementRes[J_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResJ"  );
   self->elementRes[K_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResK"  );

   /* Construct Parent */
   _lucMeshCrossSection_AssignFromXML( self, cf, data );

   _lucMeshSampler_Init(self);

   /* No lighting */
   self->lit = False;
}

void _lucMeshSampler_Build( void* drawingObject, void* data ) 
{
   /* Build field variable in parent */
   _lucMeshCrossSection_Build(drawingObject, data);
}

void _lucMeshSampler_Initialise( void* drawingObject, void* data ) 
{
   lucMeshSampler*  self = (lucMeshSampler*)drawingObject;
   /* Calculate number of samples */
   self->total = (self->elementRes[I_AXIS]+1) * (self->elementRes[J_AXIS]+1) * (self->elementRes[K_AXIS]+1);
}

void _lucMeshSampler_Execute( void* drawingObject, void* data ) {}
void _lucMeshSampler_Destroy( void* drawingObject, void* data ) {}

void lucMeshSampler_DrawSlice(lucMeshSampler* self, lucDatabase* database)
{
   /* Corners */
   float corners[6] = {self->min[0], self->min[1], self->min[2],
                       self->max[0], self->max[1], self->max[2]};

   /* Sample the 2d cross-section */
   lucMeshCrossSection_Sample(self, False);

   if (self->rank == 0 && database)
   {
      /* Write slice values on root processor */
      lucDatabase_AddVolumeSlice(database, self->elementRes[I_AXIS]+1, self->elementRes[J_AXIS]+1, corners, self->colourMap, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}

void _lucMeshSampler_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucMeshSampler* self = (lucMeshSampler*)drawingObject;
   Dimension_Index dim  = self->dim;

   if (self->isSet) 
   {
      /* Just draw at given position if provided */
      lucMeshSampler_DrawSlice(self, database);
   }
   else if (dim == 2)
   {
      lucMeshSampler_DrawSlice(lucCrossSection_Slice(self, 0.0, False), database);
   }
   else
   {
      int idx;
      for ( idx=0; idx <= self->elementRes[K_AXIS]; idx++)
      {
         lucMeshSampler_DrawSlice(lucCrossSection_Slice(self, idx, False), database);
      }
   }
}

