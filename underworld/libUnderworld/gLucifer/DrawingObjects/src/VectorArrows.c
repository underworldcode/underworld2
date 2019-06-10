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
#include "CrossSection.h"
#include "VectorArrows.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucVectorArrows_Type = "lucVectorArrows";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucVectorArrows* _lucVectorArrows_New(  LUCVECTORARROWS_DEFARGS  )
{
   lucVectorArrows*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucVectorArrows) );
   self = (lucVectorArrows*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucVectorArrows_Init( lucVectorArrows*                 self,
                            IJK                              resolution,
                            double                           maximum,
                            Bool                             dynamicRange)
{
   memcpy( self->resolution, resolution, sizeof(IJK) );
   /* Set cross section resolutions to the X & Y resolution */
   self->resolutionA = resolution[I_AXIS];
   self->resolutionB = resolution[J_AXIS];

   self->maximum = maximum;
   self->dynamicRange = dynamicRange;

   /* Get the sampler to move away from boundaries */
   self->offsetEdges = True;
}

void _lucVectorArrows_Delete( void* drawingObject )
{
   lucVectorArrows*  self = (lucVectorArrows*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucVectorArrows_Print( void* drawingObject, Stream* stream )
{
   lucVectorArrows*  self = (lucVectorArrows*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucVectorArrows_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucVectorArrows);
   Type                                                             type = lucVectorArrows_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucVectorArrows_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucVectorArrows_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucVectorArrows_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucVectorArrows_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucVectorArrows_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucVectorArrows_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucVectorArrows_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucVectorArrows_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucVectorArrows_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucVectorArrows_New(  LUCVECTORARROWS_PASSARGS  );
}

void _lucVectorArrows_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucVectorArrows* self = (lucVectorArrows*)drawingObject;
   IJK resolution;

   /* Construct Parent */
   //self->defaultResolution = 8;  /* Default sampling res */
   self->defaultResolution = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", 8);
   _lucCrossSection_AssignFromXML( self, cf, data );

   resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", self->defaultResolution);
   resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", self->defaultResolution);
   resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", self->defaultResolution);

   self->gatherData = False;     /* Drawn in parallel */

   _lucVectorArrows_Init( self, resolution, 
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maximum", 1.0  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"dynamicRange", True  ));
}

void _lucVectorArrows_Build( void* drawingObject, void* data )
{
   /* Build field variable in parent */
   _lucCrossSection_Build(drawingObject, data);
}

void _lucVectorArrows_Initialise( void* drawingObject, void* data ) {}
void _lucVectorArrows_Execute( void* drawingObject, void* data ) {}
void _lucVectorArrows_Destroy( void* drawingObject, void* data ) {}

void _lucVectorArrows_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucVectorArrows*       self    = (lucVectorArrows*)drawingObject;
   Dimension_Index        dim     = self->dim;

   if (!self->onMesh) 
   {
     if (self->isSet) 
     {
        /* Just draw at given position if provided */
        _lucVectorArrows_DrawCrossSection(self, database);
     }
     else if ( dim == 2 )
     {
        _lucVectorArrows_DrawCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, False), database);
     }
     else
     {
        int idx;
        for ( idx=0; idx <= self->resolution[K_AXIS]; idx++)
        {
           _lucVectorArrows_DrawCrossSection( lucCrossSection_Set(self, idx / (double)self->resolution[ K_AXIS ], K_AXIS, True), database);
        }
     }
   }
   else
   {
     if (self->isSet) 
     {
        /* Just draw at given position if provided */
        _lucVectorArrows_DrawMeshCrossSection(self, database);
     }
     else if ( dim == 2 )
     {
        _lucVectorArrows_DrawMeshCrossSection( lucCrossSection_Set(self, 0, K_AXIS, False), database);
     }
     else
     {
        int idx;
        for ( idx=0; idx < self->dims[K_AXIS]; idx++)
        {
           _lucVectorArrows_DrawMeshCrossSection( lucCrossSection_Set(self, idx, K_AXIS, False), database);
        }
     }
   }
}

void _lucVectorArrows_DrawCrossSection( void* drawingObject, lucDatabase* database)
{
   lucVectorArrows*  self           = (lucVectorArrows*)drawingObject;
   double min = HUGE_VAL, max = -HUGE_VAL;
   Index aIndex, bIndex;

   if ( False == self->dynamicRange )
   {
     min = 0.0;
     max = self->maximum;
   }

   /* Force 3d vectors */
   lucCrossSection_AllocateSampleData(self, 3);

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, False);

   /* Write only values that have data on this processor! */
   for ( aIndex = 0 ; aIndex < self->resolutionA ; aIndex++ )
   {
      for ( bIndex = 0 ; bIndex < self->resolutionB ; bIndex++ )
      {
         if (self->values[aIndex][bIndex][0] != HUGE_VAL)
         {
            if ( True == self->dynamicRange )
            {
              double mag = self->values[aIndex][bIndex][0]*self->values[aIndex][bIndex][0];
              mag += self->values[aIndex][bIndex][1]*self->values[aIndex][bIndex][1];
              mag += self->values[aIndex][bIndex][2]*self->values[aIndex][bIndex][2];
              mag = sqrt(mag);
              if (mag < min) min = mag;
              if (mag > max) max = mag;

            }

            lucDatabase_AddVertices(database, 1, lucVectorType, &self->vertices[aIndex][bIndex][0]);
            lucDatabase_AddVectors(database, 1, lucVectorType, min, max, &self->values[aIndex][bIndex][0]);
         }
      }
   }

   lucCrossSection_FreeSampleData(self);
}

void _lucVectorArrows_DrawMeshCrossSection( void* drawingObject, lucDatabase* database )
{
   lucVectorArrows*  self           = (lucVectorArrows*)drawingObject;
   double            min = 0.0, max = self->maximum;

   /* Force 3d vectors */
   self->fieldDim = 3;

   /* Sample the 2d cross-section */
   lucCrossSection_SampleMesh(self, False);

   if ( True == self->dynamicRange )
   {
      min = self->valueMin;
      max = self->valueMax;
   }


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
