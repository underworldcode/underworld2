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
#include "MeshViewer.h"

/* Textual name of this class - This is a global pointer which is used for
   times when you need to refer to class and not a particular instance of a class */
const Type lucMeshViewer_Type = "lucMeshViewer";


/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucMeshViewer* _lucMeshViewer_New(  LUCMESHVIEWER_DEFARGS  )
{
   lucMeshViewer*					self;

   /* Call private constructor of parent - this will set virtual functions of
      parent and continue up the hierarchy tree. At the beginning of the tree
      it will allocate memory of the size of object and initialise all the
      memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucMeshViewer) );
   self = (lucMeshViewer*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucMeshViewer_Init(
   lucMeshViewer*         self,
   Mesh*                  mesh,
   char*                  skipEdges,
   float                  pointSize,
   Bool                   pointSmooth )
{
   self->mesh = mesh;

   /* Specify axis-aligned mesh edges that should not be plotted with character string representing axis */
   if (strchr(skipEdges, 'x') || strchr(skipEdges, 'X')) self->skipXedges = True;
   else self->skipXedges = False;
   if (strchr(skipEdges, 'y') || strchr(skipEdges, 'Y')) self->skipYedges = True;
   else self->skipYedges = False;
   if (strchr(skipEdges, 'z') || strchr(skipEdges, 'Z')) self->skipZedges = True;
   else self->skipZedges = False;

   self->pointSmooth = pointSmooth;
   self->pointSize = pointSize;
   /* Append to property string */
   lucDrawingObject_AppendProps(self, "pointsmooth=%d\npointsize=%g\n", pointSmooth, pointSize); 
}

void _lucMeshViewer_Delete( void* drawingObject )
{
   lucMeshViewer*  self = (lucMeshViewer*)drawingObject;

   if ( self->edges )
      Memory_Free( self->edges );

   _lucDrawingObject_Delete( self );
}

void _lucMeshViewer_Print( void* drawingObject, Stream* stream )
{
   lucMeshViewer*  self = (lucMeshViewer*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucMeshViewer_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap)
{
   lucMeshViewer*  self = (lucMeshViewer*)drawingObject;
   lucMeshViewer* newDrawingObject;
   newDrawingObject = _lucDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newDrawingObject;
}


void* _lucMeshViewer_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucMeshViewer);
   Type                                                             type = lucMeshViewer_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucMeshViewer_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucMeshViewer_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucMeshViewer_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucMeshViewer_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucMeshViewer_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucMeshViewer_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucMeshViewer_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucMeshViewer_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucMeshViewer_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucMeshViewer_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucMeshViewer_New(  LUCMESHVIEWER_PASSARGS  );
}

void _lucMeshViewer_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucMeshViewer*         self = (lucMeshViewer*)drawingObject;
   Mesh*                  mesh;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Mesh", Mesh, True, data  );

   self->nodeNumbers = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"nodeNumbers", False );
   self->elementNumbers = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"elementNumbers", False );
   self->displayNodes = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"displayNodes", False );
   self->displayEdges = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"displayEdges", True );

   self->colourVariable = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ColourField", FieldVariable, False, data  );
   self->sizeVariable = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"SizeField", FieldVariable, False, data  );

   _lucMeshViewer_Init(
      self,
      mesh,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"skipEdges", ""),
      (float) Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"pointSize", self->displayNodes ? 5.0 : 1.0 ),
      (Bool ) Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"pointSmoothing", True )
   );

   /* Drawing settings for this component */
   self->lit = False;
}

void _lucMeshViewer_Build( void* drawingObject, void* data )
{
   lucMeshViewer*	self = (lucMeshViewer*)drawingObject;

   Stg_Component_Build( self->colourVariable, data, False );
   Stg_Component_Build( self->sizeVariable, data, False );
}

void _lucMeshViewer_Initialise( void* drawingObject, void* data )
{
   lucMeshViewer*	self = (lucMeshViewer*)drawingObject;

   if ( !Mesh_HasIncidence( self->mesh, MT_EDGE, MT_VERTEX ) )
   {
      /* Edge rendering not defined, not yet implemented, needed? */\
      self->displayEdges = False;
   }
}

void _lucMeshViewer_Execute( void* drawingObject, void* data )
{
}

void _lucMeshViewer_Destroy( void* drawingObject, void* data )
{
}

void _lucMeshViewer_Setup( void* drawingObject, lucDatabase* database, void* _context )
{}

void _lucMeshViewer_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucMeshViewer*          self          = (lucMeshViewer*)drawingObject;
   lucColourMap*           colourMap      = self->colourMap;

   Journal_Firewall( Mesh_GetDomainSize( self->mesh, MT_VERTEX ),
                     NULL, "Error when trying to render mesh. Provided mesh may not be supported." );

   /* Scale Colour Maps */
   if ( self->colourVariable && colourMap )
   {
      /* Colour by provided variable */
      lucColourMap_CalibrateFromFieldVariable( colourMap, self->colourVariable );
   }
   else if (colourMap )
   {
      /* Colour by proc */
      lucColourMap_SetMinMax( colourMap, 0, self->nproc-1);
   }

   /* Pick the correct dimension. */
   int dim = Mesh_GetDimSize( self->mesh );

   /* Render vertices */
   if (self->displayNodes || self->nodeNumbers )
   {
      unsigned	nVerts;
      unsigned	v_i;

      nVerts = Mesh_GetLocalSize( self->mesh, MT_VERTEX );
      for ( v_i = 0; v_i < nVerts; v_i ++ )
      {
         double* vert = Mesh_GetVertex( self->mesh, v_i );
         float pos[3] = {vert[0], vert[1], dim == 3 ? vert[2] : 0};
         /* Add the vertex for the label as a point */
         lucDatabase_AddVertices(database, 1, lucPointType, pos);
         /* Add to the label data */
         if (self->nodeNumbers)
         {
            char label[32];
            sprintf( label, " nl%u", v_i );
            lucDatabase_AddLabel(database, lucPointType, label);
         }

         /* Optional colour and size of nodes */
         double value;
         if (self->colourVariable && FieldVariable_InterpolateValueAt( self->colourVariable, vert, &value ) == LOCAL)
         {
            float val = (float)value;
            lucDatabase_AddValues(database, 1, lucPointType, lucColourValueData, self->colourMap, &val);
         }
         if (self->sizeVariable && FieldVariable_InterpolateValueAt( self->sizeVariable, vert, &value ) == LOCAL)
         {
            float size = (float)value;
            lucDatabase_AddValues(database, 1, lucPointType, lucSizeData, NULL, &size);
         }
      }
   }

   /* Render edges */
   if (self->displayEdges )
      lucMeshViewer_RenderEdges( self, database );

   /* Prints the element numbers */
   if ( self->elementNumbers )
      lucMeshViewer_PrintAllElementsNumber( self, database );
}

void lucMeshViewer_RenderEdges( lucMeshViewer* self, lucDatabase* database )
{
   unsigned	nEdges;
   int	nIncVerts, *incVerts;
   IArray*		inc;
   unsigned	e_i;
   int dim = Mesh_GetDimSize( self->mesh );

   Journal_Firewall( Mesh_GetDomainSize( self->mesh, MT_EDGE ) && Mesh_HasIncidence( self->mesh, MT_EDGE, MT_VERTEX ),
                     NULL, "Error when trying to render mesh. Provided mesh may not be supported." );

   nEdges = Mesh_GetLocalSize( self->mesh, MT_EDGE );
   inc = IArray_New();
   for ( e_i = 0; e_i < nEdges; e_i++ )
   {
      Mesh_GetIncidence( self->mesh, MT_EDGE, e_i, MT_VERTEX, inc );
      nIncVerts = IArray_GetSize( inc );
      incVerts = IArray_GetPtr( inc );
      Journal_Firewall( nIncVerts == 2, NULL, "Error when trying to render mesh. Provided mesh may not be supported." );

      double *vertex1, *vertex2;
      vertex1 = Mesh_GetVertex( self->mesh, incVerts[0] );
      vertex2 = Mesh_GetVertex( self->mesh, incVerts[1] );
      if (!EdgeSkip(self, vertex1, vertex2))
      {
         float pos1[3] = {vertex1[0], vertex1[1], dim == 3 ? vertex1[2] : 0};
         float pos2[3] = {vertex2[0], vertex2[1], dim == 3 ? vertex2[2] : 0};
         /* Add the line vertices */
         lucDatabase_AddVertices(database, 1, lucLineType, pos1);
         lucDatabase_AddVertices(database, 1, lucLineType, pos2);
      }
   }

   Stg_Class_Delete( inc );
}


Bool EdgeSkip(lucMeshViewer* self, double* v1, double* v2)
{
   /* Skip where Y+Z unchanging (x-axis aligned horizontal edges) */
   if (self->skipXedges && v1[J_AXIS] == v2[J_AXIS] && v1[K_AXIS] == v2[K_AXIS])
      return True;

   /* Skip where X+Z unchanging (y-axis aligned vertical edges) */
   if (self->skipYedges && v1[I_AXIS] == v2[I_AXIS] && v1[K_AXIS] == v2[K_AXIS])
      return True;

   /* Skip where X+Y unchanging (z-axis aligned horizontal edges) */
   if (self->skipZedges && v1[I_AXIS] == v2[I_AXIS] && v1[J_AXIS] == v2[J_AXIS])
      return True;

   return False;
}

void lucMeshViewer_PrintAllElementsNumber( void* drawingObject, lucDatabase* database )
{
#if 0
   lucMeshViewer*	     self = (lucMeshViewer*)drawingObject;
   Coord                avgCoord;
   Coord                offset;
   char                 elementNumString[100];
   Dimension_Index      dim_I;
   Node_LocalIndex      node_lI;
   Node_Index           elNode_I;
   Element_LocalIndex   element_lI;


   glColor3f( self->colour.red, self->colour.green, self->colour.blue );

   /* Prints the element numbers */
   offset[0] = -0.01;
   offset[1] = -0.01;
   offset[2] = 0;
   for ( element_lI = 0; element_lI < self->mesh->elementLocalCount; element_lI++ )
   {
      sprintf( elementNumString, "el%u", element_lI );

      for ( dim_I=0; dim_I < 3; dim_I++)
      {
         avgCoord[dim_I] = 0;
      }
      for ( elNode_I=0; elNode_I < self->mesh->elementNodeCountTbl[element_lI]; elNode_I++ )
      {
         node_lI = self->mesh->elementNodeTbl[element_lI][elNode_I];
         for ( dim_I=0; dim_I < ((HexaEL*)(self->mesh->layout->elementLayout))->dim; dim_I++)
         {
            avgCoord[dim_I] += self->mesh->nodeCoord[node_lI][dim_I];
         }
      }
      for ( dim_I=0; dim_I < ((HexaEL*)(self->mesh->layout->elementLayout))->dim; dim_I++)
      {
         avgCoord[dim_I] /= (double)self->mesh->elementNodeCountTbl[element_lI];
      }

      if ( ((HexaEL*)(self->mesh->layout->elementLayout))->dim == 2)
      {
         glRasterPos2f( (float)avgCoord[0] + offset[0], (float)avgCoord[1] + offset[1] );
      }
      else
      {
         glRasterPos3f( (float)avgCoord[0] + offset[0], (float)avgCoord[1] + offset[1],
                        (float)avgCoord[2] + offset[2] );
      }

      lucPrintString( elementNumString );
   }
#endif
}



