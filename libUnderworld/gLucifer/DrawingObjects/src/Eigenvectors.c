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
#include "EigenvectorsCrossSection.h"
#include "Eigenvectors.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucEigenvectors_Type = "lucEigenvectors";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucEigenvectors* _lucEigenvectors_New(  LUCEIGENVECTORS_DEFARGS  )
{
   lucEigenvectors*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucEigenvectors) );
   self = (lucEigenvectors*) _lucEigenvectorsCrossSection_New(  LUCEIGENVECTORSCROSSSECTION_PASSARGS  );

   return self;
}

void _lucEigenvectors_Init( lucEigenvectors*                                             self,
                            IJK                                                          resolution)
{
   memcpy( self->resolution, resolution, sizeof(IJK) );
   /* Set cross section resolutions to the X & Y resolution */
   self->resolutionA = resolution[I_AXIS];
   self->resolutionB = resolution[J_AXIS];
}

void _lucEigenvectors_Delete( void* drawingObject )
{
   lucEigenvectors*  self = (lucEigenvectors*)drawingObject;

   _lucEigenvectorsCrossSection_Delete( self );
}

void _lucEigenvectors_Print( void* drawingObject, Stream* stream )
{
   lucEigenvectors*  self = (lucEigenvectors*)drawingObject;

   _lucEigenvectorsCrossSection_Print( self, stream );
}

void* _lucEigenvectors_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucEigenvectors);
   Type                                                             type = lucEigenvectors_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucEigenvectors_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucEigenvectors_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucEigenvectors_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucEigenvectors_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucEigenvectorsCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucEigenvectorsCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucEigenvectorsCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucEigenvectorsCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucEigenvectors_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucEigenvectors_New(  LUCEIGENVECTORS_PASSARGS  );
}

void _lucEigenvectors_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucEigenvectors* self = (lucEigenvectors*)drawingObject;
   IJK              resolution;

   /* Construct Parent */
   self->defaultResolution = 8;
   _lucEigenvectorsCrossSection_AssignFromXML( self, cf, data );

   resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", self->defaultResolution  );
   resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", self->defaultResolution  );
   resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", self->defaultResolution  );

   _lucEigenvectors_Init( self, resolution );
}

void _lucEigenvectors_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucEigenvectors*       self            = (lucEigenvectors*)drawingObject;
   int idx;

   if (self->isSet) 
      /* Just draw at given position if provided */
      _lucEigenvectorsCrossSection_DrawCrossSection(self, database);
   else if (self->dim)
      _lucEigenvectorsCrossSection_DrawCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, False), database);
   else
      for ( idx=0; idx <= self->resolution[K_AXIS]; idx++)
         _lucEigenvectorsCrossSection_DrawCrossSection( lucCrossSection_Set(self, idx / (double)self->resolution[ K_AXIS ], K_AXIS, True), database);
}


