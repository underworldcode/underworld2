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
#include "VectorArrowCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucVectorArrowCrossSection_Type = "lucVectorArrowCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucVectorArrowCrossSection* _lucVectorArrowCrossSection_New(  LUCVECTORARROWCROSSSECTION_DEFARGS  )
{
   lucVectorArrowCrossSection*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucVectorArrowCrossSection) );
   self = (lucVectorArrowCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucVectorArrowCrossSection_Init(
   lucVectorArrowCrossSection*      self,
   double                           maximum,
   Bool                             dynamicRange)
{
   self->maximum = maximum;
   self->dynamicRange = dynamicRange;

   /* Get the sampler to move away from boundaries */
   self->offsetEdges = True;
}

void _lucVectorArrowCrossSection_Delete( void* drawingObject )
{
   lucVectorArrowCrossSection*  self = (lucVectorArrowCrossSection*)drawingObject;
   _lucCrossSection_Delete( self );
}

void _lucVectorArrowCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucVectorArrowCrossSection*  self = (lucVectorArrowCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucVectorArrowCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucVectorArrowCrossSection);
   Type                                                             type = lucVectorArrowCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucVectorArrowCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucVectorArrowCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucVectorArrowCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucVectorArrowCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucVectorArrowCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucVectorArrowCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucVectorArrowCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucVectorArrowCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucVectorArrowCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucVectorArrowCrossSection_New(  LUCVECTORARROWCROSSSECTION_PASSARGS  );
}

void _lucVectorArrowCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucVectorArrowCrossSection* self = (lucVectorArrowCrossSection*)drawingObject;

   /* Construct Parent */
   self->defaultResolution = 8;  /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );
   self->gatherData = False;     /* Drawn in parallel */

   _lucVectorArrowCrossSection_Init(
      self,
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maximum", 1.0  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"dynamicRange", True  ));
}

void _lucVectorArrowCrossSection_Build( void* drawingObject, void* data )
{
   /* Build field variable in parent */
   _lucCrossSection_Build(drawingObject, data);
}

void _lucVectorArrowCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucVectorArrowCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucVectorArrowCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucVectorArrowCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucVectorArrowCrossSection*       self            = (lucVectorArrowCrossSection*)drawingObject;
   DomainContext*            context         = (DomainContext*) _context;

   _lucVectorArrowCrossSection_DrawCrossSection( self, database);
}

void _lucVectorArrowCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database)
{
   lucVectorArrowCrossSection*  self = (lucVectorArrowCrossSection*)drawingObject;
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


