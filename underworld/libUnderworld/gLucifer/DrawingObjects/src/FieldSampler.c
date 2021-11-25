/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>

#include <gLucifer/Base/src/Base.h>


#include "types.h"
#include <gLucifer/Base/src/DrawingObject.h>
#include "FieldSampler.h"

void lucFieldSampler_SampleLocal( void* drawingObject, void* _context );
void lucFieldSampler_SampleGlobal( void* drawingObject, void* _context );

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucFieldSampler_Type = "lucFieldSampler";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucFieldSampler* _lucFieldSampler_New(  LUCFIELDSAMPLER_DEFARGS  )
{
   lucFieldSampler*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucFieldSampler) );
   self = (lucFieldSampler*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucFieldSampler_Init(
   lucFieldSampler*           self,
   IJK                        resolution)
{
   memcpy( self->resolution, resolution, sizeof(IJK) );
}

void _lucFieldSampler_Delete( void* drawingObject )
{
   lucFieldSampler*  self = (lucFieldSampler*)drawingObject;
   _lucCrossSection_Delete( self );
}

void _lucFieldSampler_Print( void* drawingObject, Stream* stream )
{
   lucFieldSampler*  self = (lucFieldSampler*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucFieldSampler_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucFieldSampler);
   Type                                                             type = lucFieldSampler_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucFieldSampler_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucFieldSampler_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucFieldSampler_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucFieldSampler_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucFieldSampler_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucFieldSampler_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucFieldSampler_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucFieldSampler_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucFieldSampler_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucFieldSampler_New(  LUCFIELDSAMPLER_PASSARGS  );
}

void _lucFieldSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucFieldSampler*         self               = (lucFieldSampler*)drawingObject;
   Index                  defaultRes;
   IJK                    resolution;

   /* Construct Parent */
   _lucCrossSection_AssignFromXML( self, cf, data );

   defaultRes = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", 32);
   resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", defaultRes);
   resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", defaultRes);
   resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", defaultRes);

   _lucFieldSampler_Init(self, resolution);
}

void _lucFieldSampler_Build( void* drawingObject, void* data ) 
{
   /* Build field variable in parent */
   _lucCrossSection_Build(drawingObject, data);
}

void _lucFieldSampler_Initialise( void* drawingObject, void* data ) 
{
   _lucCrossSection_Initialise(drawingObject, data);
}

void _lucFieldSampler_Execute( void* drawingObject, void* data ) {}
void _lucFieldSampler_Destroy( void* drawingObject, void* data ) {}

void lucFieldSampler_DrawSlice(void* drawingObject, lucDatabase* database)
{
   lucFieldSampler* self = (lucFieldSampler*)drawingObject;

   /* Corners */
   float corners[6] = {self->min[0], self->min[1], self->min[2],
                       self->max[0], self->max[1], self->max[2]};

   /* Sample the 2d cross-section */
   self->resolutionA = self->resolution[self->axis1];
   self->resolutionB = self->resolution[self->axis2];
   lucCrossSection_SampleField(self, False);

   if (self->rank == 0 && database)
   {
      /* Write slice values on root processor */
      lucDatabase_AddVolumeSlice(database, self->resolution[I_AXIS], self->resolution[J_AXIS], corners, self->colourMap, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}

void lucFieldSampler_DrawMeshSlice(void* drawingObject, lucDatabase* database)
{
   lucFieldSampler* self = (lucFieldSampler*)drawingObject;

   /* Corners */
   float corners[6] = {self->min[0], self->min[1], self->min[2],
                       self->max[0], self->max[1], self->max[2]};

   /* Sample the 2d cross-section */
   lucCrossSection_SampleMesh(self, False);

   if (self->rank == 0 && database)
   {
      /* Write slice values on root processor */
      lucDatabase_AddVolumeSlice(database, self->dims[I_AXIS], self->dims[J_AXIS], corners, self->colourMap, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}



void _lucFieldSampler_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucFieldSampler* self = (lucFieldSampler*)drawingObject;
   Dimension_Index dim   = self->dim;
   if (dim == 2) self->resolution[K_AXIS] = 0;

   if (!self->onMesh)
   {
     if (dim == 2)
     {
        lucFieldSampler_DrawSlice(lucCrossSection_Slice(self, 0.0, True), database);
     }
     else
     {
        int idx;
        for ( idx=0; idx < self->resolution[K_AXIS]; idx++)
        {
           lucFieldSampler_DrawSlice(lucCrossSection_Slice(self, idx / (double)(self->resolution[K_AXIS]-1), True), database);
        }
     }
   }
   else
   {
     if (dim == 2)
     {
        lucFieldSampler_DrawMeshSlice(lucCrossSection_Slice(self, 0.0, False), database);
     }
     else
     {
        int idx;
        for ( idx=0; idx < self->dims[K_AXIS]; idx++)
        {
           lucFieldSampler_DrawMeshSlice(lucCrossSection_Slice(self, idx, False), database);
        }
     }
   }
}


