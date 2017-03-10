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
#include "ScalarField.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucScalarField_Type = "lucScalarField";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucScalarField* _lucScalarField_New(  LUCSCALARFIELD_DEFARGS  )
{
   lucScalarField*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucScalarField) );
   self = (lucScalarField*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucScalarField_Init(lucScalarField* self, char* drawSides)
{
   self->flipNormals = False;
   int i;
   for (i=0; i<3; i++) {
      self->drawSides[i][0] = True;
      self->drawSides[i][1] = True;
   } 
      
   /* Specify axis-aligned sides that should be plotted with character string representing axis */
   /* Small letter represents minimum, large maximum */
   if (!strchr(drawSides, 'x')) self->drawSides[I_AXIS][0] = False;
   if (!strchr(drawSides, 'X')) self->drawSides[I_AXIS][1] = False;
   if (!strchr(drawSides, 'y')) self->drawSides[J_AXIS][0] = False;
   if (!strchr(drawSides, 'Y')) self->drawSides[J_AXIS][1] = False;
   if (!strchr(drawSides, 'z')) self->drawSides[K_AXIS][0] = False;
   if (!strchr(drawSides, 'Z')) self->drawSides[K_AXIS][1] = False;
}

void _lucScalarField_Delete( void* drawingObject )
{
   lucScalarField*  self = (lucScalarField*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucScalarField_Print( void* drawingObject, Stream* stream )
{
   lucScalarField*  self = (lucScalarField*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucScalarField_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucScalarField);
   Type                                                             type = lucScalarField_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucScalarField_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucScalarField_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucScalarField_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucScalarField_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucScalarField_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucScalarField_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucScalarField_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucScalarField_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucScalarField_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucScalarField_New(  LUCSCALARFIELD_PASSARGS  );
}

void _lucScalarField_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucScalarField*  self = (lucScalarField*)drawingObject;

   /* Construct Parent */
   self->defaultResolution = 100;   /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );

   _lucScalarField_Init(self, Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"drawSides", "xyzXYZ"));
}

void _lucScalarField_Build( void* drawingObject, void* data )
{
   lucScalarField*     self        = (lucScalarField*)drawingObject;

   /* Build field variable in parent */
   _lucCrossSection_Build(self, data);
}

void _lucScalarField_Initialise( void* drawingObject, void* data ) {}
void _lucScalarField_Execute( void* drawingObject, void* data ) {}
void _lucScalarField_Destroy( void* drawingObject, void* data ) {}

void _lucScalarField_Draw( void* drawingObject, lucDatabase* database, void* data )
{
   lucScalarField*   self          = (lucScalarField*)drawingObject;

   if (!self->onMesh) 
   {
     if (self->isSet) 
     {
        /* Just draw at given position if provided */
        lucScalarField_DrawCrossSection(self, database, False); //True);
     }
     else if (self->dim == 2)
     {
        lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, False), database, False);
     }
     else
     {
        /* Cross sections at minimums, default winding for faces */
        if (self->drawSides[I_AXIS][0])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 0.0, I_AXIS, True), database, False);
        if (self->drawSides[J_AXIS][0])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 0.0, J_AXIS, True), database, False);
        if (self->drawSides[K_AXIS][0])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, True), database, False);

        /* Cross sections at maximums, reverse winding for faces */
        if (self->drawSides[I_AXIS][1])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 1.0, I_AXIS, True), database, True);
        if (self->drawSides[J_AXIS][1])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 1.0, J_AXIS, True), database, True);
        if (self->drawSides[K_AXIS][1])
           lucScalarField_DrawCrossSection( lucCrossSection_Set(self, 1.0, K_AXIS, True), database, True);
     }
   }
   else
   {
     if (self->isSet) 
     {
        /* Just draw at given position if provided */
        lucScalarField_DrawMeshCrossSection(self, database, False);
     }
     else if (self->dim == 2)
     {
        lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, False), database, False);
     }
     else
     {
        /* Cross sections at minimums, default winding for faces */
        self->flipNormals = False;
        if (self->drawSides[I_AXIS][0])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, 0.0, I_AXIS, False), database, False);
        if (self->drawSides[J_AXIS][0])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, 0.0, J_AXIS, False), database, False);
        if (self->drawSides[K_AXIS][0])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, 0.0, K_AXIS, False), database, False);

        /* Cross sections at maximums, reverse winding for faces and flip normals to face backwards */
        self->flipNormals = True;
        if (self->drawSides[I_AXIS][1])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, self->dims[ I_AXIS ] - 1, I_AXIS, False), database, True);
        if (self->drawSides[J_AXIS][1])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, self->dims[ J_AXIS ] - 1, J_AXIS, False), database, True);
        if (self->drawSides[K_AXIS][1])
           lucScalarField_DrawMeshCrossSection( lucCrossSection_Set(self, self->dims[ K_AXIS ] - 1, K_AXIS, False), database, True);
     }
  }
}

void lucScalarField_DrawCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing)
{
   lucScalarField* self = (lucScalarField*)drawingObject;

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, backFacing);

   if (self->rank == 0 && database)
   {
      int d;
      int count = self->resolutionA * self->resolutionB;
      lucDatabase_AddGridVertices(database, count, self->resolutionB, &self->vertices[0][0][0]);
      lucDatabase_AddValues(database, count, lucGridType, lucColourValueData, self->colourMap, &self->values[0][0][0]);

      /* Flip normal if back facing */
      if (backFacing)
         for (d=0; d<3; d++)
            self->normal[d] = 0 - self->normal[d];

      /* Add a single normal value */
      lucDatabase_AddNormal(database, lucGridType, self->normal);
   }

   /* Free memory */
   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}

void lucScalarField_DrawMeshCrossSection( void* drawingObject, lucDatabase* database, Bool backFacing)
{
   lucScalarField*       self            = (lucScalarField*)drawingObject;

   lucCrossSection_SampleMesh(self, backFacing);

   if (self->rank == 0 && database)
   {
      int count = self->dims[1] * self->dims[2];
      lucDatabase_AddGridVertices(database, count, self->dims[2], self->vertices[0][0]);
      lucDatabase_AddValues(database, count, lucGridType, lucColourValueData, self->colourMap, &self->values[0][0][0]);
   }

   lucCrossSection_FreeSampleData(self);

   /* Start new geometry section - when used with multiple sections */
   lucDatabase_OutputGeometry(database, self->id);
}
