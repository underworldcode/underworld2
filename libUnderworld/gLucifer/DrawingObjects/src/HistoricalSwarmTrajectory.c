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
#include "HistoricalSwarmTrajectory.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucHistoricalSwarmTrajectory_Type = "lucHistoricalSwarmTrajectory";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucHistoricalSwarmTrajectory* _lucHistoricalSwarmTrajectory_New(  LUCHISTORICALSWARMTRAJECTORY_DEFARGS  )
{
   lucHistoricalSwarmTrajectory*					self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucHistoricalSwarmTrajectory) );
   self = (lucHistoricalSwarmTrajectory*) _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );

   return self;
}

void _lucHistoricalSwarmTrajectory_Init(
   lucHistoricalSwarmTrajectory*       self,
   Swarm*                              swarm,
   Bool                                flat,
   double                              lineScaling,
   double                              arrowHead,
   unsigned int 						      historySteps,
   double							         historyTime )
{
   self->swarm             = swarm;
   self->flat              = flat;
   self->scaling           = lineScaling;
   self->arrowHead         = arrowHead;
   self->steps	            = historySteps;
   self->time	            = historyTime;

   /* Append to property string */
   //OK: DEPRECATED, SET VIA PYTHON
   //lucDrawingObject_AppendProps(self, "scaling=%g\nflat=%d\narrowhead=%g\nsteps=%d\ntime=%g\n", lineScaling, flat, arrowHead, historySteps, historyTime); 

   /* Create the particle extension */
   /* in this case we must assign a global id to each particle in the swarm; so we add an extension to the particles */
   self->particleIdExtHandle = ExtensionManager_Add( swarm->particleExtensionMgr, "particleID", sizeof(unsigned int) );
}

void _lucHistoricalSwarmTrajectory_Delete( void* drawingObject )
{
   lucHistoricalSwarmTrajectory*  self = (lucHistoricalSwarmTrajectory*)drawingObject;

   _lucDrawingObject_Delete( self );
}

void _lucHistoricalSwarmTrajectory_Print( void* drawingObject, Stream* stream )
{
   lucHistoricalSwarmTrajectory*  self = (lucHistoricalSwarmTrajectory*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucHistoricalSwarmTrajectory_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucHistoricalSwarmTrajectory);
   Type                                                             type = lucHistoricalSwarmTrajectory_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucHistoricalSwarmTrajectory_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucHistoricalSwarmTrajectory_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucHistoricalSwarmTrajectory_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucHistoricalSwarmTrajectory_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucHistoricalSwarmTrajectory_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucHistoricalSwarmTrajectory_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucHistoricalSwarmTrajectory_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucHistoricalSwarmTrajectory_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucHistoricalSwarmTrajectory_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucHistoricalSwarmTrajectory_New(  LUCHISTORICALSWARMTRAJECTORY_PASSARGS  );
}

void _lucHistoricalSwarmTrajectory_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucHistoricalSwarmTrajectory*   self	= (lucHistoricalSwarmTrajectory*)drawingObject;
   Swarm*            swarm;
   unsigned int      historySteps;
   double            historyTime;
   unsigned int      defaultSteps = 100;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   swarm         =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", Swarm, True, data  );

   historySteps  =  Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"historySteps", defaultSteps);
   historyTime   =  Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"historyTime", 0  );

   Journal_Firewall(
      swarm->particleLayout->coordSystem == GlobalCoordSystem,
      Journal_MyStream( Error_Type, self ),
      "In func %s, unable to visualise swarm %s because it uses a local coord system layout %s of type %s.\n",
      __func__,
      swarm->name,
      swarm->particleLayout->name,
      swarm->particleLayout->type );

   _lucHistoricalSwarmTrajectory_Init(
      self,
      swarm,
      (Bool) Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"flat", False  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"lineScaling", 0.05  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"arrowHead", 2.0  ),
      historySteps,
      historyTime );

   /* Disable lighting when flat option specified */
   //TODO: Set via python properties
   //self->lit = self->flat;
}

void _lucHistoricalSwarmTrajectory_Build( void* drawingObject, void* data ) {}

void _lucHistoricalSwarmTrajectory_Initialise( void* drawingObject, void* data )
{
   lucHistoricalSwarmTrajectory*  self = (lucHistoricalSwarmTrajectory*)drawingObject;
   Swarm*                        swarm = self->swarm;
   int p;
   unsigned int offset = 0;
   unsigned int particle_id;
   unsigned int* counts = Memory_Alloc_Array(unsigned int, self->nproc, "particle counts");

   Stg_Component_Initialise( self->swarm, data, False );

   /* Get the count on each proc */
   (void)MPI_Allgather(&swarm->particleLocalCount, 1, MPI_UNSIGNED, counts, 1, MPI_UNSIGNED, self->comm);

   /* Assign a global ID to each particle */
   for (p=0; p < self->nproc; p++)
   {
      /* Sum to get offset */
      if (p > 0) offset += counts[p-1];
      if (self->rank == p)
      {
         for( particle_id = 0 ; particle_id < swarm->particleLocalCount ; particle_id++ )
         {
            GlobalParticle *g_point = (GlobalParticle*)Swarm_ParticleAt( swarm, particle_id );
            unsigned int *part_ext = ExtensionManager_Get( swarm->particleExtensionMgr, g_point, self->particleIdExtHandle );
            *part_ext = offset + particle_id;
         }
      }
   }
   Memory_Free(counts);
}

void _lucHistoricalSwarmTrajectory_Execute( void* drawingObject, void* data ) {}

void _lucHistoricalSwarmTrajectory_Destroy( void* drawingObject, void* data ) {}

void _lucHistoricalSwarmTrajectory_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucHistoricalSwarmTrajectory* self           = (lucHistoricalSwarmTrajectory*)drawingObject;
   Swarm*                        swarm          = self->swarm;
   Particle_Index                lParticle_I;
   GlobalParticle*               particle;

   /* Loop over all particles and draw lines according to where each one has been */
   for ( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount ; lParticle_I++ )
   {
      particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );
      unsigned int* particle_id = ExtensionManager_Get( swarm->particleExtensionMgr, particle, self->particleIdExtHandle );

      /* Export particle position */
      if (database)
      {
         float coordf[3] = {particle->coord[0], particle->coord[1], particle->coord[2]};
         lucDatabase_AddVerticesWidth(database, 1, lucTracerType, swarm->particleLocalCount, coordf);
         lucDatabase_AddIndex(database, lucTracerType, *particle_id);

         /* Export particle colour value - not required when using same value for every particle at timestep /
         if (colourMap)
         {
            lucDatabase_AddValues(database, 1, lucTracerType, lucColourValueData, colourMap, &value);
         }*/
      }
   }
}


