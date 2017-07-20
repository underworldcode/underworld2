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
#include <float.h>
#include "Sampler.h"
#include <iostream>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/MeshCoordinate.hpp>
extern "C" {
#include <ctype.h>
}

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucSampler_Type = "lucSampler";

void _lucSampler_SetFn( void* _self, Fn::Function* fn ){
    lucSampler*  self = (lucSampler*)_self;
    
    // record fn to struct
    lucSampler_cppdata* cppdata = (lucSampler_cppdata*) self->cppdata;
    // record fn, and also wrap with a MinMax function so that we can record
    // the min & max encountered values for the colourbar.
    cppdata->fn = std::make_shared<Fn::MinMax>(fn);
    
    // setup fn
    self->dim = Mesh_GetDimSize( self->mesh );

    std::shared_ptr<IO_double> globalCoord = std::make_shared<IO_double>( self->dim, FunctionIO::Vector );
    // grab first node for sample node
    memcpy( globalCoord->data(), Mesh_GetVertex( self->mesh, 0 ), self->dim*sizeof(double) );
    // get the function.. note that we use 'get' to extract the raw pointer from the smart pointer.
    cppdata->func = cppdata->fn->getFunction(globalCoord.get());
    
    const FunctionIO* io = dynamic_cast<const FunctionIO*>(cppdata->func(globalCoord.get()));
    if( !io )
        throw std::invalid_argument("Provided function does not appear to return a valid result.");
    self->fieldComponentCount = io->size();

    self->fieldDim = self->fieldComponentCount;

    /*if( ( Stg_Class_IsInstance( self, lucScalarField_Type )       ||
          Stg_Class_IsInstance( self, lucFieldSampler_Type ) ||
          Stg_Class_IsInstance( self, lucIsosurfaceSampler_Type )          )
        && self->fieldComponentCount != 1 )
    {
        throw std::invalid_argument("Provided function must return a scalar result.");
    }

    if( ( Stg_Class_IsInstance( self, lucVectorArrows_Type )      )
             && self->fieldComponentCount != self->dim )
    {
        throw std::invalid_argument("Provided function must return a vector result.");
    }*/
    
}

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucSampler* _lucSampler_New(  LUCSAMPLER_DEFARGS  )
{
   lucSampler*                    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucSampler) );
   self = (lucSampler*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   self->cppdata = (void*) new lucSampler_cppdata;

   return self;
}

void _lucSampler_Init(lucSampler* self)
{
}

void _lucSampler_Delete( void* drawingObject )
{
   lucSampler*  self = (lucSampler*)drawingObject;

   if (self->cppdata)
       delete (lucSampler_cppdata*)self->cppdata;

   _lucDrawingObject_Delete( self );
}

void _lucSampler_Print( void* drawingObject, Stream* stream )
{
   lucSampler*  self = (lucSampler*)drawingObject;
   _lucDrawingObject_Print( self, stream );
}

void* _lucSampler_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucSampler);
   Type                                                             type = lucSampler_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucSampler_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucSampler_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucSampler_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucSampler_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucSampler_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucSampler_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucSampler_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucSampler_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucSampler_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucSampler_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucSampler_New(  LUCSAMPLER_PASSARGS  );
}

void _lucSampler_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucSampler*     self = (lucSampler*)drawingObject;
   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   self->mesh =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Mesh", Mesh, True, data  );

   _lucSampler_Init(self);
}

void _lucSampler_Build( void* drawingObject, void* data )
{
   lucSampler*     self = (lucSampler*)drawingObject;
   /* Build field variable in parent */
   Stg_Component_Build( self->mesh, data, False );
}

void _lucSampler_Initialise( void* drawingObject, void* data ) 
{
}

void _lucSampler_Execute( void* drawingObject, void* data ) {}

void _lucSampler_Destroy( void* drawingObject, void* data ) {}

void _lucSampler_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSampler* self = (lucSampler*)drawingObject;
}

void _lucSampler_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucSampler* self = (lucSampler*)drawingObject;
}

void lucSampler_SampleField(void* drawingObject, float* vertices, int V, float* values, int N)
{
   lucSampler* self = (lucSampler*)drawingObject;
   Coord          localMin, localMax;
   Coord          pos;
   int dims = self->fieldComponentCount;

   //Verts have components for mesh dim, values components field dim 
   assert(V/self->dim == N/dims);
   int count = V / self->dim;

   Mesh_GetLocalCoordRange(self->mesh, localMin, localMax );
   std::shared_ptr<IO_double> globalCoord = std::make_shared<IO_double>( self->dim, FunctionIO::Vector );
   lucSampler_cppdata* cppdata = (lucSampler_cppdata*) self->cppdata;
   // reset max/min
   cppdata->fn->reset();

   //Flag empty values with Infinity, 
   //clear components of higher dimension values outside dim range (eg: 2d vectors are stored as 3d, z needs to be zero)
   for (int i = 0 ; i < N ; i++)
      values[i] = HUGE_VALF;

   /* Get mesh cross section vertices and values */
   double time = MPI_Wtime();
   Journal_Printf(lucInfo, "Sampling (%s) over %d vertices...  0%", self->name, N);
   for ( int i = 0 ; i < count ; i++ )
   {
      int percent = 100 * (i + 1) / count;
      Journal_Printf(lucInfo, "\b\b\b\b%3d%%", percent);
      fflush(stdout);

      /* Get position */
      pos[0] = vertices[i*3];
      pos[1] = vertices[i*3+1];
      pos[2] = vertices[i*3+2];
      memcpy( globalCoord->data(), pos, self->dim*sizeof(double) );

      /* Check vertex is within local space, 
       * if outside then skip to avoid wasting time attempting to sample */
      if (pos[I_AXIS] > localMin[I_AXIS]-FLT_EPSILON && pos[I_AXIS] < localMax[I_AXIS]+FLT_EPSILON &&
          pos[J_AXIS] > localMin[J_AXIS]-FLT_EPSILON && pos[J_AXIS] < localMax[J_AXIS]+FLT_EPSILON &&
          (self->dim < 3 || (pos[K_AXIS] > localMin[K_AXIS]-FLT_EPSILON && pos[K_AXIS] < localMax[K_AXIS]+FLT_EPSILON)))
      {
         try
         {
            const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(cppdata->func(globalCoord.get()));

            /* Value found locally, save */
            for (int d=0; d<dims; d++)
               values[i*dims+d] = output->at<float>(d);
         }
         catch (std::exception& e)
         {
            //Not found
            std::cerr << e.what() << std::endl;
         }
      }
   }

   /* Show each proc as it finishes */
   Journal_Printf(lucInfo, " (%d)", self->rank);
   fflush(stdout);
   MPI_Barrier(self->comm); /* Sync here, then time will show accurately how long sampling took on ALL procs */
   Journal_Printf(lucInfo, " -- %f sec.\n", MPI_Wtime() - time);

   /* This gathers all sampled data back to the root processor */
   if (self->nproc > 1)
   {
      time = MPI_Wtime();
      for (int r=1; r < self->nproc; r++)
      {
         if (self->rank == r)
         {
            /* Send */
            (void)MPI_Send(&values[0], N, MPI_FLOAT, 0, r, self->comm);
         }
         else if (self->rank == 0)
         {
            /* Receive */
            MPI_Status status;
            float* rvalues = Memory_Alloc_Array( float, N, "received vertex values");
            (void)MPI_Recv(&rvalues[0], N, MPI_FLOAT, r, r, self->comm, &status);

            /* If value provided, copy into final data (duplicates overwritten) */
            for (int i = 0 ; i < N ; i++)
               if (rvalues[i] != HUGE_VALF)
                  values[i] = rvalues[i];
            Memory_Free(rvalues);
         }
      }
      Journal_Printf(lucInfo, " Gather in %f sec.\n", MPI_Wtime() - time);
   }
   
   // finally, record encountered min/max to colourmap
   if ( self->colourMap)
   {
      lucColourMap_SetMinMax( self->colourMap, cppdata->fn->getMinGlobal(), cppdata->fn->getMaxGlobal() );
      //Journal_Printf(lucInfo, "ColourMap min/max range set to %f - %f\n", self->colourMap->minimum, self->colourMap->maximum);
   }
}

