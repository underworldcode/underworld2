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
#include "CrossSection.h"

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/MeshCoordinate.hpp>
extern "C" {
#include <ctype.h>
#include "ScalarField.h"
#include "IsosurfaceCrossSection.h"
#include "VectorArrows.h"
#include "FieldSampler.h"
}

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucCrossSection_Type = "lucCrossSection";

void _lucCrossSection_SetFn( void* _self, Fn::Function* fn ){
    lucCrossSection*  self = (lucCrossSection*)_self;
    
    // record fn to struct
    lucCrossSection_cppdata* cppdata = (lucCrossSection_cppdata*) self->cppdata;
    // record fn, and also wrap with a MinMax function so that we can record
    // the min & max encountered values for the colourbar.
    cppdata->fn = std::make_shared<Fn::MinMax>(fn);
    
    // setup fn
    self->dim = Mesh_GetDimSize( self->mesh );
    if (self->onMesh)
    {
        std::shared_ptr<MeshCoordinate> meshCoord = std::make_shared<MeshCoordinate>( self->mesh );
        // set first coord
        meshCoord->index() = 0;
        // get the function.. note that we use 'get' to extract the raw pointer from the smart pointer.
        cppdata->func = cppdata->fn->getFunction(meshCoord.get());
        
        const FunctionIO* io = dynamic_cast<const FunctionIO*>(cppdata->func(meshCoord.get()));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        self->fieldComponentCount = io->size();
    }
    else
    {
        std::shared_ptr<IO_double> globalCoord = std::make_shared<IO_double>( self->dim, FunctionIO::Vector );
        // grab first node for sample node
        memcpy( globalCoord->data(), Mesh_GetVertex( self->mesh, 0 ), self->dim*sizeof(double) );
        // get the function.. note that we use 'get' to extract the raw pointer from the smart pointer.
        cppdata->func = cppdata->fn->getFunction(globalCoord.get());
        
        const FunctionIO* io = dynamic_cast<const FunctionIO*>(cppdata->func(globalCoord.get()));
        if( !io )
            throw std::invalid_argument("Provided function does not appear to return a valid result.");
        self->fieldComponentCount = io->size();
    }

    self->fieldDim = self->fieldComponentCount;

    if( ( Stg_Class_IsInstance( self, lucScalarField_Type )       ||
          Stg_Class_IsInstance( self, lucFieldSampler_Type ) ||
          Stg_Class_IsInstance( self, lucIsosurfaceCrossSection_Type )          )
        && self->fieldComponentCount != 1 )
    {
        throw std::invalid_argument("Provided function must return a scalar result.");
    }

    if( ( Stg_Class_IsInstance( self, lucVectorArrows_Type )      )
             && self->fieldComponentCount != self->dim )
    {
        throw std::invalid_argument("Provided function must return a vector result.");
    }
    
}

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucCrossSection* _lucCrossSection_New(  LUCCROSSSECTION_DEFARGS  )
{
   lucCrossSection*                    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucCrossSection) );
   self = (lucCrossSection*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   self->cppdata = (void*) new lucCrossSection_cppdata;

   return self;
}

void _lucCrossSection_Init(
   lucCrossSection*        self,
   Index                   resolutionA,
   Index                   resolutionB,
   Bool                    onMesh,
   XYZ                     coord1,
   XYZ                     coord2,
   XYZ                     coord3,
   double                  value,
   Axis                    axis,
   Bool                    interpolate)
{
   Journal_Firewall( resolutionA > 1 && resolutionB > 1, lucError,
                     "Error - in %s(): Resolution below 2x2 ==> %d x %d\n", __func__, resolutionA, resolutionB);
   self->resolutionA = resolutionA;
   self->resolutionB = resolutionB;
   self->onMesh = onMesh;
   memcpy( self->coord1, coord1, sizeof(XYZ) );
   memcpy( self->coord2, coord2, sizeof(XYZ) );
   memcpy( self->coord3, coord3, sizeof(XYZ) );
   self->value = value;
   self->axis = axis;
   self->interpolate = interpolate;
   self->gatherData = True;   /* Default behaviour for sampling is to gather all data on root proc */
   self->offsetEdges = False; /* Pushes min/max edges by half of sample size to avoid sampling boundaries */
   self->values = NULL;
   self->vertices = NULL;
}

void _lucCrossSection_Delete( void* drawingObject )
{
   lucCrossSection*  self = (lucCrossSection*)drawingObject;

   if (self->cppdata)
       delete (lucCrossSection_cppdata*)self->cppdata;

   _lucDrawingObject_Delete( self );
}

void _lucCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucCrossSection*  self = (lucCrossSection*)drawingObject;
   _lucDrawingObject_Print( self, stream );
}

void* _lucCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucCrossSection);
   Type                                                             type = lucCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );
}

void _lucCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucCrossSection*     self = (lucCrossSection*)drawingObject;
   Name crossSectionStr;
   char axisChar;
   char crossSectionVal[20];
   char modifierChar = ' ';
   double value = 0.0;
   Axis axis = I_AXIS;
   Bool interpolate = False;
   XYZ  coord1;
   XYZ  coord2;
   XYZ  coord3;

   /* Default to set from input args, flag turned of if none found */
   self->isSet = True;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   /* Get Values which define the cross section 
    * if not specified, use axis parameter instead */
   if (Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"X1", &coord1[ I_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Y1", &coord1[ J_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Z1", &coord1[ K_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"X2", &coord2[ I_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Y2", &coord2[ J_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Z2", &coord2[ K_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"X3", &coord3[ I_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Y3", &coord3[ J_AXIS ] ) &&
       Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"Z3", &coord3[ K_AXIS ] ))
   {
      /* No axis alignment, using specified coords to define a cross-section plane */
      self->axisAligned = False;
   }
   else
   {
      self->axisAligned = True;

      /* If any of these direct value parameters are set use them instead of parsing the crossSection string
       * (added to allow scaling to work as a value in a string will not have scaling applied) */
      if (Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"crossSectionX", &value ))
         axis = I_AXIS;
      else if (Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"crossSectionY", &value ))
         axis = J_AXIS;
      else if (Stg_ComponentFactory_TryDouble( cf, self->name, (Dictionary_Entry_Key)"crossSectionZ", &value ))
         axis = K_AXIS;
      else
      {
         /* Interpolate between max and min value using provided value */
         interpolate = True;
         /* Read the cross section string specification */
         crossSectionStr = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"crossSection", "" );

         /* axis=value    : draw at min + value * range, ie: x=0.25 will be a quarter along the x range
          * axis=min      : draw at minimum of range on axis
          * axis=max      : draw at maximum of range on axis
          * axis=value%   : draw at interpolated percentage value of range on axis
          * Axis is a single character, one of [xyzXYZ] */

         /* Parse the input string */
         if (strlen(crossSectionStr) > 0 && sscanf( crossSectionStr, "%c=%s", &axisChar, crossSectionVal ) == 2 )
         {
            /* Axis X/Y/Z */
            if ( toupper( axisChar ) >= 'X' )
               axis = static_cast<Axis>(toupper( axisChar ) - 'X');   /* x=0 y=1 z=2 */

            if (sscanf( crossSectionVal, "%lf%c", &value, &modifierChar) >= 1)
            {
               /* Found a numeric value  + optional modifier character */
               //fprintf(stderr, "CROSS SECTION VALUE %lf on Axis %c\n",self->value, axisChar);

               /* Interpolate cross section using percentage value */
               if (modifierChar == '%')
                  //fprintf(stderr, "PERCENTAGE %lf %% CROSS SECTION on Axis %c\n", self->value, axisChar);
                  value *= 0.01;
            }
            /* Max or Min specified? */
            else if (strcmp(crossSectionVal, "min") == 0)
               value = 0.0;
               //fprintf(stderr, "MIN CROSS SECTION AT %lf on Axis %c\n", self->value, axisChar);
            else if (strcmp(crossSectionVal, "max") == 0)
               value = 1.0;
               //fprintf(stderr, "MAX CROSS SECTION AT %lf on Axis %c\n", self->value, axisChar);
         }
         else
           self->isSet = False;
      }
   }

   self->mesh =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Mesh", Mesh, True, data  );

   if (self->defaultResolution < 2) self->defaultResolution = 100;   /* Default sampling res */
   self->defaultResolution = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", self->defaultResolution);

   _lucCrossSection_Init(
      self,
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionA", self->defaultResolution),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionB", self->defaultResolution),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"onMesh", True  ),
      coord1,
      coord2,
      coord3,
      value,
      axis,
      interpolate
   );
}

void _lucCrossSection_Build( void* drawingObject, void* data )
{
   lucCrossSection*     self = (lucCrossSection*)drawingObject;
   /* Build field variable in parent */
   Stg_Component_Build( self->mesh, data, False );

   if (self->onMesh)
   {
      /* Store the Vertex Grid */
      self->vertexGridHandle = self->mesh->vertGridId;
      if ( self->vertexGridHandle == (ExtensionInfo_Index)-1 )

      Journal_Firewall( self->vertexGridHandle != (ExtensionInfo_Index )-1, lucError,
                        "Error - in %s(): provided Mesh \"%s\" doesn't have a Vertex Grid.\n"
                        "Try visualising with lucScalarField instead.\n", __func__, self->mesh->name );

   }
}

void _lucCrossSection_Initialise( void* drawingObject, void* data ) 
{

}

void _lucCrossSection_Execute( void* drawingObject, void* data ) {}

void _lucCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucCrossSection_Setup( void* drawingObject, lucDatabase* database, void* _context )
{
   lucCrossSection* self = (lucCrossSection*)drawingObject;
   if (self->onMesh)
   {
      Mesh*                mesh  = (Mesh*) self->mesh;
      Grid*                vertGrid;
      vertGrid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, self->vertexGridHandle );
      int sizes[3] = {1,1,1};
      for (int d=0; d<self->dim; d++)
        sizes[d] = vertGrid->sizes[d];
      self->dims[0] = sizes[ self->axis ];
      self->dims[1] = sizes[ self->axis1 ];
      self->dims[2] = sizes[ self->axis2 ];
   }

   /* Use provided setup function to correctly set axis etc */
   lucCrossSection_Set(self, self->value, self->axis, self->interpolate);
}

/* Default cross-section object allows drawing a cut plane at a specified coord on any axis */
void _lucCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucCrossSection* self = (lucCrossSection*)drawingObject;

   /* Only draw on proc 0 */
   if (database->rank > 0) return;

   /* Create a plane, have 3 coords, get the 4th corner coord */
   double coord4[3];
   lucCrossSection_Interpolate2d(self, 1.0, 1.0, coord4);

   /* Dump vertex pos */
   float coords[12] = {(float)self->coord3[0], (float)self->coord3[1], (float)self->coord3[2],
                       (float)coord4[0], (float)coord4[1], (float)coord4[2],
                       (float)self->coord1[0], (float)self->coord1[1], (float)self->coord1[2],
                       (float)self->coord2[0], (float)self->coord2[1], (float)self->coord2[2]};

   lucDatabase_AddGridVertices(database, 4, 2, coords);

   /* Add a single normal value */
   lucDatabase_AddNormal(database, lucGridType, self->normal);
}

/* Set the position for depth sort to centre of cross section */
void lucCrossSection_SetPosition(void* crossSection)
{
   lucCrossSection* self = (lucCrossSection*)crossSection;
   lucCrossSection_Interpolate2d(self, 0.5, 0.5, self->position);
   self->depthSort = True;
}

/* Interpolate in 2 directions */
void lucCrossSection_Interpolate2d(void* crossSection, double factorA, double factorB, XYZ coord)
{
   lucCrossSection* self = (lucCrossSection*)crossSection;
   int i;
   for (i=0; i<3; i++)
      coord[i] = self->coord1[i] + factorA * (self->coord2[i] - self->coord1[i]) + factorB * (self->coord3[i] - self->coord1[i]);
}

/* Returns the cross section value, interpolating where necessary */
double lucCrossSection_GetValue(void* crossSection, double min, double max)
{
   lucCrossSection* self = (lucCrossSection*)crossSection;
   if (self->interpolate)
      /* Interpolation factor 0-1 provided to determine cross-section value */
      return min + self->value * (max - min);
   else
      /* Exact value provided */
      return self->value;
}

/* Function to set cross section parameters and return self for use in passing cross-sections to functions */
lucCrossSection* lucCrossSection_Set(void* crossSection, double val, Axis axis, Bool interpolate)
{
   lucCrossSection* self = (lucCrossSection*)crossSection;
   self->value = val;
   self->axis = axis;
   self->interpolate = interpolate;

   Mesh_GetGlobalCoordRange(self->mesh, self->min, self->max );

   if (self->axisAligned) 
   {
      /* Set other axis directions for drawing cross section:
       * These settings produce consistent polygon winding for cross sections on all axis */
      switch (self->axis)
      {
      case I_AXIS:
         self->axis1 = J_AXIS;
         self->axis2 = K_AXIS;
         break;
      case J_AXIS:
         self->axis1 = K_AXIS;
         self->axis2 = I_AXIS;
         break;
      case K_AXIS:
         self->axis1 = I_AXIS;
         self->axis2 = J_AXIS;
         break;
      }

      double halfSampleA = 0, halfSampleB = 0;
      if (self->offsetEdges)
      {
         halfSampleA = 0.5 * (self->max[self->axis1] - self->min[self->axis1]) / self->resolutionA;
         halfSampleB = 0.5 * (self->max[self->axis2] - self->min[self->axis2]) / self->resolutionB;
      }

      /* Use axis to calculate from min & max */
      double pos = lucCrossSection_GetValue(self, self->min[self->axis], self->max[self->axis]);
      self->coord1[self->axis] = pos;
      self->coord1[self->axis1] = self->min[self->axis1] + halfSampleA;
      self->coord1[self->axis2] = self->min[self->axis2] + halfSampleB;
      self->coord2[self->axis] = pos;
      self->coord2[self->axis1] = self->max[self->axis1] - halfSampleA;
      self->coord2[self->axis2] = self->min[self->axis2] + halfSampleB;
      self->coord3[self->axis] = pos;
      self->coord3[self->axis1] = self->min[self->axis1] + halfSampleA;
      self->coord3[self->axis2] = self->max[self->axis2] - halfSampleB;
   }

   /* Set the position for depth sort to centre of cross section */
   lucCrossSection_SetPosition(self);

   /* Create normal to plane */
   StGermain_NormalToPlane( self->normal, self->coord1, self->coord2, self->coord3);

   return self;
}

/* Function to set cross section parameters for a volume slice */
lucCrossSection* lucCrossSection_Slice(void* crossSection, double val, Bool interpolate)
{
   lucCrossSection* self = (lucCrossSection*)crossSection;
   self->value = val;
   self->interpolate = interpolate;
   self->axis = K_AXIS;
   self->axis1 = J_AXIS;
   self->axis2 = I_AXIS;

   Mesh_GetGlobalCoordRange(self->mesh, self->min, self->max );

   double pos = lucCrossSection_GetValue(self, self->min[K_AXIS], self->max[K_AXIS]);
   self->coord1[self->axis] = self->coord2[self->axis] = self->coord3[self->axis] = pos;
   self->coord1[self->axis1] = self->coord3[self->axis1] = self->min[self->axis1];
   self->coord1[self->axis2] = self->coord2[self->axis2] = self->min[self->axis2];
   self->coord2[self->axis1] = self->max[self->axis1];
   self->coord3[self->axis2] = self->max[self->axis2];

   return self;
}

void lucCrossSection_AllocateSampleData(void* drawingObject, int dims)
{
   lucCrossSection* self = (lucCrossSection*)drawingObject;
   Index          aIndex, bIndex, d;
   if (dims <= 0) dims = self->fieldComponentCount;

   if ((!self->vertices && self->rank == 0) || !self->gatherData)
      self->vertices = Memory_Alloc_3DArray( float, self->resolutionA, self->resolutionB, 3, "quad vertices");
   else
      self->vertices = NULL;

   if (!self->values) 
      self->values = Memory_Alloc_3DArray( float, self->resolutionA, self->resolutionB, dims, "vertex values");

   if (dims > self->fieldComponentCount)
   {
      for ( aIndex = 0 ; aIndex < self->resolutionA ; aIndex++ )
         for ( bIndex = 0 ; bIndex < self->resolutionB ; bIndex++ )
            for (d=self->fieldComponentCount; d<dims; d++)
               self->values[aIndex][bIndex][d] = 0;
   }
}

void lucCrossSection_SampleField(void* drawingObject, Bool reverse)
{
   lucCrossSection* self = (lucCrossSection*)drawingObject;
   Coord          localMin, localMax;
   Coord          pos;
   Index          aIndex, bIndex;
   int d;
   int dims = self->fieldComponentCount;

   Mesh_GetLocalCoordRange(self->mesh, localMin, localMax );
   std::shared_ptr<IO_double> globalCoord = std::make_shared<IO_double>( self->dim, FunctionIO::Vector );
   lucCrossSection_cppdata* cppdata = (lucCrossSection_cppdata*) self->cppdata;
   // reset max/min
   cppdata->fn->reset();

   lucCrossSection_AllocateSampleData(self, 0);

   /* Get mesh cross section vertices and values */
   double time = MPI_Wtime();
   Journal_Printf(lucInfo, "Sampling (%s) %d x %d...  0%", self->name, self->resolutionA, self->resolutionB);
   for ( aIndex = 0 ; aIndex < self->resolutionA ; aIndex++ )
   {
      int percent = 100 * (aIndex + 1) / self->resolutionA;
      Journal_Printf(lucInfo, "\b\b\b\b%3d%%", percent);
      fflush(stdout);

      /* Reverse order if requested */
      Index aIndex1 = aIndex;
      if (reverse) aIndex1 = self->resolutionA - aIndex - 1;

      for ( bIndex = 0 ; bIndex < self->resolutionB ; bIndex++ )
      {
         int d;
         /* Get position */
         lucCrossSection_Interpolate2d(self, aIndex1 / (double)(self->resolutionA-1), bIndex / (double)(self->resolutionB-1), pos);
         memcpy( globalCoord->data(), pos, self->dim*sizeof(double) );

         /* Check cross section is within local space, 
          * if outside then skip to avoid wasting time attempting to sample */
         /* This scales horrendously when mesh is irregular as we search for points that are not on the processor */
         /* A local coord sampling routine, such as implemented for isosurfaces would help with this problem */
         /*double TOL = 0.00000001; //((max[self->axis1] - min[self->axis1] + max[self->axis2] - min[self->axis2])/2.0) * 0.000001;
         if (pos[I_AXIS] + TOL > localMin[I_AXIS] && pos[I_AXIS] - TOL < localMax[I_AXIS] &&
             pos[J_AXIS] + TOL > localMin[J_AXIS] && pos[J_AXIS] - TOL < localMax[J_AXIS] &&
            (fieldVariable->dim < 3 || (pos[K_AXIS] + TOL > localMin[K_AXIS] && pos[K_AXIS] - TOL < localMax[K_AXIS])))*/
         if (pos[I_AXIS] > localMin[I_AXIS]-FLT_EPSILON && pos[I_AXIS] < localMax[I_AXIS]+FLT_EPSILON &&
             pos[J_AXIS] > localMin[J_AXIS]-FLT_EPSILON && pos[J_AXIS] < localMax[J_AXIS]+FLT_EPSILON &&
            (self->dim < 3 || (pos[K_AXIS] > localMin[K_AXIS]-FLT_EPSILON && pos[K_AXIS] < localMax[K_AXIS]+FLT_EPSILON)))
         {
            const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(cppdata->func(globalCoord.get()));

            /* Value found locally, save */
            for (d=0; d<dims; d++)
               self->values[aIndex][bIndex][d] = output->at<float>(d);
         }
         else
         {
            /* Flag not found */
            for (d=0; d<dims; d++)
               self->values[aIndex][bIndex][d] = HUGE_VAL;
         }

         /* Copy vertex data */
         if (self->rank == 0 || !self->gatherData)
            for (d=0; d<3; d++)
               self->vertices[aIndex][bIndex][d] = (float)pos[d];
      }
   }
   /* Show each proc as it finishes */
   Journal_Printf(lucInfo, " (%d)", self->rank);
   fflush(stdout);
   MPI_Barrier(self->comm); /* Sync here, then time will show accurately how long sampling took on ALL procs */
   Journal_Printf(lucInfo, " -- %f sec.\n", MPI_Wtime() - time);

   /* This gathers all sampled data back to the root processor,
    * useful for drawing objects that are connected across proc boundaries
    * eg: surfaces, switch this flag off for others (eg: vectors) */
   if (self->gatherData && self->nproc > 1)
   {
      int count = self->resolutionA * self->resolutionB * dims;
      int r;
      time = MPI_Wtime();
      for (r=1; r < self->nproc; r++)
      {
         if (self->rank == r)
         {
            /* Send */
            (void)MPI_Send(&self->values[0][0][0], count, MPI_FLOAT, 0, r, self->comm);
            Memory_Free(self->values);
            self->values = NULL;
         }
         else if (self->rank == 0)
         {
            /* Receive */
            MPI_Status status;
            float*** rvalues = Memory_Alloc_3DArray( float, self->resolutionA, self->resolutionB, dims, "received vertex values");
            (void)MPI_Recv(&rvalues[0][0][0], count, MPI_FLOAT, r, r, self->comm, &status);

            /* If value provided, copy into final data (duplicates overwritten) */
            for ( aIndex = 0 ; aIndex < self->resolutionA ; aIndex++ )
               for ( bIndex = 0 ; bIndex < self->resolutionB ; bIndex++ )
                  for (d=0; d<dims; d++)
                     if (rvalues[aIndex][bIndex][d] != HUGE_VAL)
                        self->values[aIndex][bIndex][d] = rvalues[aIndex][bIndex][d];
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

void lucCrossSection_SampleMesh( void* drawingObject, Bool reverse)
{
   lucCrossSection* self          = (lucCrossSection*)drawingObject;
   Grid*                vertGrid;
   Node_LocalIndex      crossSection_I;
   IJK                  node_ijk;
   Node_GlobalIndex     node_gI;
   Node_DomainIndex     node_dI;
   int                  i,j, d;
   Coord                globalMin, globalMax, min, max;
   Mesh*                mesh  = (Mesh*) self->mesh;


   int localcount = 0;

   std::shared_ptr<MeshCoordinate> meshCoord = std::make_shared<MeshCoordinate>( self->mesh );
   lucCrossSection_cppdata* cppdata = (lucCrossSection_cppdata*) self->cppdata;
   // reset max/min
   cppdata->fn->reset();

   vertGrid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, self->vertexGridHandle );

   crossSection_I = lucCrossSection_GetValue(self, 0, self->dims[0]-1);

   Mesh_GetLocalCoordRange(self->mesh, min, max );
   Mesh_GetGlobalCoordRange(self->mesh, globalMin, globalMax );

   /* Get mesh cross section self->vertices and values */
   self->resolutionA = self->dims[1];
   self->resolutionB = self->dims[2];
   lucCrossSection_AllocateSampleData(self, self->fieldDim);
   int lSize = Mesh_GetLocalSize( mesh, MT_VERTEX );
   double time = MPI_Wtime();
   Journal_Printf(lucInfo, "Sampling mesh (%s) %d x %d...  0%", self->name, self->dims[1], self->dims[2]);
   node_ijk[ self->axis ] = crossSection_I;
   for ( i = 0 ; i < self->dims[1]; i++ )
   {
      int percent = 100 * (i + 1) / self->dims[1];
      Journal_Printf(lucInfo, "\b\b\b\b%3d%%", percent);
      fflush(stdout);

      /* Reverse order if requested */
      int i0 = i;
      if (reverse) i0 = self->dims[1] - i - 1;

      node_ijk[ self->axis1 ] = i0;

      for ( j = 0 ; j < self->dims[2]; j++ )
      {
         self->vertices[i][j][0] = HUGE_VAL;
         self->vertices[i][j][2] = 0;
         node_ijk[ self->axis2 ] = j;
         node_gI = Grid_Project( vertGrid, node_ijk );
         /* Get coord and value if node is local... */
         if (Mesh_GlobalToDomain( mesh, MT_VERTEX, node_gI, &node_dI ) && node_dI < lSize)
         {  
            /* Found on this processor */
            // set index on the FunctionIO object
            meshCoord->index() = node_dI;

            const FunctionIO* io = dynamic_cast<const FunctionIO*>(cppdata->func(meshCoord.get()));

            double* pos = Mesh_GetVertex( mesh, node_dI );
         
            for (d=0; d<self->dim; d++)
               self->vertices[i][j][d] = pos[d];

            for (d=0; d<self->fieldComponentCount; d++)
            {
               self->values[i][j][d] = io->at<float>(d);
               std::cout << self->values[i][j][d] << std::endl;
            }

            localcount++;
         }
      }
   }
   Journal_Printf(lucInfo, " %f sec. ", MPI_Wtime() - time);

   /* Collate */
   time = MPI_Wtime();
   for ( i=0 ; i < self->dims[1]; i++ )
   {
      for ( j=0 ; j < self->dims[2]; j++ )
      {
         /* Receive values at root */
         if (self->rank == 0)
         {
            /* Already have value? */
            if (self->vertices[i][j][0] != HUGE_VAL) {localcount--; continue; }

            /* Recv (pos and value together = (3 + fevar dims)*float) */
            float data[3 + self->fieldDim];
            (void)MPI_Recv(data, 3+self->fieldDim, MPI_FLOAT, MPI_ANY_SOURCE, i*self->dims[2]+j, self->comm, MPI_STATUS_IGNORE);
            /* Copy */
            memcpy(self->vertices[i][j], data, 3 * sizeof(float));
            memcpy(self->values[i][j], &data[3], self->fieldDim * sizeof(float));
         }
         else
         {
            /* Found on this proc? */
            if (self->vertices[i][j][0] == HUGE_VAL) continue;

            /* Copy */
            float data[3 + self->fieldDim];
            memcpy(data, self->vertices[i][j], 3 * sizeof(float));
            memcpy(&data[3], self->values[i][j], self->fieldDim * sizeof(float));

            /* Send values to root (pos & value = 4 * float) */
            MPI_Ssend(data, 3+self->fieldDim, MPI_FLOAT, 0, i*self->dims[2]+j, self->comm);
            localcount--;
         }
      }
   }
   MPI_Barrier(self->comm);    /* Barrier required, prevent subsequent MPI calls from interfering with transfer */
   Journal_Printf(lucInfo, " Gather in %f sec.\n", MPI_Wtime() - time);
   Journal_Firewall(localcount == 0, lucError,
                     "Error - in %s: count of values sampled compared to sent/received by mpi on proc %d does not match (balance = %d)\n",
                     __func__, self->rank, localcount);
}

void lucCrossSection_FreeSampleData(void* drawingObject)
{
   lucCrossSection* self = (lucCrossSection*)drawingObject;
   if (self->values) Memory_Free(self->values);
   self->values = NULL;
   if (self->vertices) Memory_Free(self->vertices);
   self->vertices = NULL;
}
