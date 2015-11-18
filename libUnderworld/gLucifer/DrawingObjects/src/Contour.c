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
#include "Contour.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucContour_Type = "lucContour";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucContour* _lucContour_New(  LUCCONTOUR_DEFARGS  )
{
   lucContour*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucContour) );
   self = (lucContour*) _lucContourCrossSection_New(  LUCCONTOURCROSSSECTION_PASSARGS  );

   return self;
}

void _lucContour_Init(lucContour*      self)
{
}

void _lucContour_Delete( void* drawingObject )
{
   lucContour*  self = (lucContour*)drawingObject;

   _lucContourCrossSection_Delete( self );
}

void _lucContour_Print( void* drawingObject, Stream* stream )
{
   lucContour*  self = (lucContour*)drawingObject;

   _lucContourCrossSection_Print( self, stream );
}

void* _lucContour_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucContour);
   Type                                                             type = lucContour_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucContour_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucContour_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucContour_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucContour_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucContourCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucContourCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucContourCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucContourCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucContourCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucContour_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucContour_New(  LUCCONTOUR_PASSARGS  );
}

void _lucContour_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucContour*      self = (lucContour*)drawingObject;
   /* Construct Parent */
   _lucContourCrossSection_AssignFromXML( self, cf, data );
   _lucContour_Init(self);
}

void _lucContour_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucContour*            self            = (lucContour*)drawingObject;
   Dimension_Index        dim             = Mesh_GetDimSize( self->mesh );

   if (self->isSet || dim == 2 ) 
   {
      /* Just draw at given position if provided */
      _lucContourCrossSection_Draw(self, database, _context);
   }
   else
   {
      self->coordIndex = 0;
      self->printedIndex = -1;
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 0.0, I_AXIS, True), database );
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 1.0, I_AXIS, True), database );
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 0.0, J_AXIS, True), database );
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 1.0, J_AXIS, True), database );
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, True), database );
      lucContourCrossSection_DrawCrossSection( lucCrossSection_Set(self, 1.0, K_AXIS, True), database );
   }
}


