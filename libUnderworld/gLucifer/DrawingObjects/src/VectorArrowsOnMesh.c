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
#include "VectorArrowMeshCrossSection.h"
#include "VectorArrowsOnMesh.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucVectorArrowsOnMesh_Type = "lucVectorArrowsOnMesh";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucVectorArrowsOnMesh* _lucVectorArrowsOnMesh_New(  LUCVECTORARROWSONMESH_DEFARGS  )
{
   lucVectorArrowsOnMesh*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucVectorArrowsOnMesh) );
   self = (lucVectorArrowsOnMesh*) _lucVectorArrowMeshCrossSection_New(  LUCVECTORARROWMESHCROSSSECTION_PASSARGS  );

   return self;
}

void _lucVectorArrowsOnMesh_Init( lucVectorArrowsOnMesh* self)
{
}

void _lucVectorArrowsOnMesh_Delete( void* drawingObject )
{
   lucVectorArrowsOnMesh*  self = (lucVectorArrowsOnMesh*)drawingObject;

   _lucVectorArrowMeshCrossSection_Delete( self );
}

void _lucVectorArrowsOnMesh_Print( void* drawingObject, Stream* stream )
{
   lucVectorArrowsOnMesh*  self = (lucVectorArrowsOnMesh*)drawingObject;

   _lucVectorArrowMeshCrossSection_Print( self, stream );
}

void* _lucVectorArrowsOnMesh_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucVectorArrowsOnMesh);
   Type                                                             type = lucVectorArrowsOnMesh_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucVectorArrowsOnMesh_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucVectorArrowsOnMesh_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucVectorArrowsOnMesh_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucVectorArrowsOnMesh_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucVectorArrowMeshCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucVectorArrowMeshCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucVectorArrowMeshCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucVectorArrowMeshCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucVectorArrowsOnMesh_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucVectorArrowsOnMesh_New(  LUCVECTORARROWSONMESH_PASSARGS  );
}

void _lucVectorArrowsOnMesh_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucVectorArrowsOnMesh* self = (lucVectorArrowsOnMesh*)drawingObject;

   self->elementRes[I_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResI"  );
   self->elementRes[J_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResJ"  );
   self->elementRes[K_AXIS] = Dictionary_GetInt( cf->rootDict, (Dictionary_Entry_Key)"elementResK"  );

   /* Construct Parent */
   _lucVectorArrowMeshCrossSection_AssignFromXML( self, cf, data );

   _lucVectorArrowsOnMesh_Init( self );
}

void _lucVectorArrowsOnMesh_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucVectorArrowsOnMesh*       self    = (lucVectorArrowsOnMesh*)drawingObject;
   Dimension_Index dim  = self->dim;

   if ( dim == 2 )
   {
      _lucVectorArrowMeshCrossSection_DrawCrossSection( lucCrossSection_Set(self, 0, K_AXIS, False), database);
   }
   else
   {
      int idx;
      for ( idx=0; idx <= self->elementRes[K_AXIS]; idx++)
      {
         _lucVectorArrowMeshCrossSection_DrawCrossSection( lucCrossSection_Set(self, idx, K_AXIS, False), database);
      }
   }
}


