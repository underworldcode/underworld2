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
#include "MeshCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucMeshCrossSection_Type = "lucMeshCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucMeshCrossSection* _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_DEFARGS  )
{
   lucMeshCrossSection*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucMeshCrossSection) );
   self = (lucMeshCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucMeshCrossSection_Init(lucMeshCrossSection* self)
{
}

void _lucMeshCrossSection_Delete( void* drawingObject )
{
   lucMeshCrossSection*  self = (lucMeshCrossSection*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucMeshCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucMeshCrossSection*  self = (lucMeshCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucMeshCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucMeshCrossSection);
   Type                                                             type = lucMeshCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucMeshCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucMeshCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucMeshCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucMeshCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucMeshCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucMeshCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucMeshCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucMeshCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucMeshCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucMeshCrossSection_New(  LUCMESHCROSSSECTION_PASSARGS  );
}

void _lucMeshCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucMeshCrossSection*     self = (lucMeshCrossSection*)drawingObject;

   /* Construct Parent */
   _lucCrossSection_AssignFromXML( self, cf, data );
   self->gatherData = False;     /* Force allocate on all procs */

   _lucMeshCrossSection_Init(self);
}

void _lucMeshCrossSection_Build( void* drawingObject, void* data )
{
   lucMeshCrossSection*     self    = (lucMeshCrossSection*)drawingObject;

   /* Build field variable in parent */
   _lucCrossSection_Build(self, data);

   /* Store the Vertex Grid */
   self->vertexGridHandle = self->mesh->vertGridId;
   if ( self->vertexGridHandle == (ExtensionInfo_Index)-1 )

      Journal_Firewall( self->vertexGridHandle != (ExtensionInfo_Index )-1, lucError,
                        "Error - in %s(): provided Mesh \"%s\" doesn't have a Vertex Grid.\n"
                        "Try visualising with lucScalarField instead.\n", __func__, self->mesh->name );

   self->fieldDim = self->fieldComponentCount;
}

void _lucMeshCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucMeshCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucMeshCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucMeshCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   /* Default draw routine, not yet implemented, draw as points? */
}

void lucMeshCrossSection_Sample( void* drawingObject, Bool reverse)
{
   lucMeshCrossSection* self          = (lucMeshCrossSection*)drawingObject;
   FeVariable*          fieldVariable = (FeVariable*) NULL;  // JM need to fix this guy
      assert(fieldVariable);
   Mesh*                mesh          = (Mesh*) fieldVariable->feMesh;
   Grid*                vertGrid;
   Node_LocalIndex      crossSection_I;
   IJK                  node_ijk;
   Node_GlobalIndex     node_gI;
   Node_DomainIndex     node_dI;
   int                  i,j, d, sizes[3] = {1,1,1};
   Coord                globalMin, globalMax, min, max;

      int localcount = 0;

   vertGrid = *(Grid**)ExtensionManager_Get( mesh->info, mesh, self->vertexGridHandle );
   for (d=0; d<fieldVariable->dim; d++) sizes[d] = vertGrid->sizes[d];
   self->dims[0] = sizes[ self->axis ];
   self->dims[1] = sizes[ self->axis1 ];
   self->dims[2] = sizes[ self->axis2 ];

   crossSection_I = lucCrossSection_GetValue(self, 0, self->dims[0]-1);

   Mesh_GetLocalCoordRange(self->mesh, min, max );
   Mesh_GetGlobalCoordRange(self->mesh, globalMin, globalMax );

   Journal_Printf( lucDebug, "%s called on field %s, with axis of cross section as %d, crossSection_I as %d (dims %d,%d,%d) field dim %d\n",
                    __func__, fieldVariable->name, self->axis, crossSection_I, self->dims[0], self->dims[1], self->dims[2], self->fieldDim);

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
            double value[self->fieldDim];
            FeVariable_GetValueAtNode( fieldVariable, node_dI, value );
            double* pos = Mesh_GetVertex( mesh, node_dI );
            /*fprintf(stderr, "[%d] (%d,%d) Node %d %f,%f,%f value %f\n", self->rank, i, j, node_gI, pos[0], pos[1], pos[2], value);*/
         
            for (d=0; d<fieldVariable->dim; d++)
               self->vertices[i][j][d] = pos[d];

            for (d=0; d<self->fieldDim; d++)
               self->values[i][j][d] = (float)value[d];

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

