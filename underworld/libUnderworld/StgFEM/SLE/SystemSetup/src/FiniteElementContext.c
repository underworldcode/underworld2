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
#include "StgFEM/Discretisation/Discretisation.h"

#include "types.h"

#include "FiniteElementContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "SystemLinearEquations.h"
#include "SolutionVector.h"

#define FINISHED_WRITING_TAG 9

/* Textual name of this class */
const Type FiniteElementContext_Type = "FiniteElementContext";
const Name defaultFiniteElementContextETypeRegisterName = "finiteElementContext";
const Name FiniteElementContext_EP_CalcDt = "FiniteElementContext_EP_CalcDt";

/* Constructors ------------------------------------------------------------------------------------------------------------------*/

FiniteElementContext* FiniteElementContext_New(
   Name        name,
   double      start,
   double      stop,
   MPI_Comm    communicator,
   Dictionary* dictionary )
{
   FiniteElementContext* self = FiniteElementContext_DefaultNew( name );

   self->isConstructed = True;
   _AbstractContext_Init( (AbstractContext*) self );
   _DomainContext_Init( (DomainContext*) self );   
   _FiniteElementContext_Init( self );

   return self;
}
   
void* FiniteElementContext_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(FiniteElementContext);
   Type                                                      type = FiniteElementContext_Type;
   Stg_Class_DeleteFunction*                              _delete = _FiniteElementContext_Delete;
   Stg_Class_PrintFunction*                                _print = _FiniteElementContext_Print;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = FiniteElementContext_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _FiniteElementContext_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _AbstractContext_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _AbstractContext_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _AbstractContext_Execute;
   Stg_Component_DestroyFunction*                        _destroy = (Stg_Component_DestroyFunction*)_FiniteElementContext_Destroy;
   AllocationType                              nameAllocationType = NON_GLOBAL;
   AbstractContext_SetDt*                                  _setDt = _FiniteElementContext_SetDt;
   double                                               startTime = 0;
   double                                                stopTime = 0;
   MPI_Comm                                          communicator = MPI_COMM_WORLD;
   Dictionary*                                         dictionary = NULL;

   return _FiniteElementContext_New( FINITEELEMENTCONTEXT_PASSARGS );
}

FiniteElementContext* _FiniteElementContext_New( FINITEELEMENTCONTEXT_DEFARGS ) {
   FiniteElementContext* self;
   
   /* Allocate memory */
   self = (FiniteElementContext*)_DomainContext_New( DOMAINCONTEXT_PASSARGS );
   
   /* General info */
   
   /* Virtual info */
   
   return self;
}

void _FiniteElementContext_Init( FiniteElementContext* self ) {
   /* Set up stream preferences */
   Journal_Enable_NamedStream( InfoStream_Type, "StgFEM_VerboseConfig", False );

   /* register this current stream (the context) as a child of the FE stream */
   /* TODO: Want to be able to recombine this with the Abs context's debug stream at some stage */
   self->debug = Stream_RegisterChild( StgFEM_Debug, FiniteElementContext_Type );
   
   /* set up s.l.e list */
   self->slEquations = Stg_ObjectList_New();

   /* Create Entry Point for Calculating timestep */
   self->calcDtEP = EntryPoint_New( FiniteElementContext_EP_CalcDt, EntryPoint_Minimum_VoidPtr_CastType );
   EntryPoint_Register_Add( self->entryPoint_Register, self->calcDtEP );
   
   /* Add hooks to existing entry points... use name "default" so that plugin, etc can exert same behaviour on other contexts*/
   EntryPoint_Prepend( 
      Context_GetEntryPoint( self, AbstractContext_EP_Build ),
      (Name)"_FiniteElementContext_Build", 
      _FiniteElementContext_Build, 
      FiniteElementContext_Type );
   EntryPoint_Prepend( 
      Context_GetEntryPoint( self, AbstractContext_EP_Initialise ),
      (Name)"_FiniteElementContext_Initialise", 
      _FiniteElementContext_Initialise, 
      FiniteElementContext_Type );
   EntryPoint_Append( 
      Context_GetEntryPoint( self, AbstractContext_EP_Solve ),
      (Name)"_FiniteElementContext_Solve", 
      _FiniteElementContext_Solve, 
      FiniteElementContext_Type );
   EntryPoint_Append( 
      Context_GetEntryPoint( self, AbstractContext_EP_Solve ),
      (Name)"_FiniteElementContext_PostSolve", 
      _FiniteElementContext_PostSolve, 
      FiniteElementContext_Type );
   EntryPoint_Append( 
      Context_GetEntryPoint( self, AbstractContext_EP_Dt ),
      (Name)"_FiniteElementContext_GetDt", 
      _FiniteElementContext_GetDt, 
      FiniteElementContext_Type );
}

/* Virtual Functions -------------------------------------------------------------------------------------------------------------*/

void _FiniteElementContext_Delete( void* context ) {
   FiniteElementContext* self = (FiniteElementContext*)context;
   
   Journal_DPrintf( self->debug, "In: %s()\n", __func__ );

   Stream_IndentBranch( StgFEM_Debug );
   Journal_DPrintfL( self->debug, 2, "Deleting the element type register (and hence all element types).\n" );
   Journal_DPrintfL( self->debug, 2, "Deleting all SLEs and the SLE list.\n" );
   /* Disabling the next 2 lines as the slEquations and its object lists are 
   deleted later on from the LiveComponentRegister (via LiveComponentRegister_DeleteAll) 
   Stg_ObjectList_DeleteAllObjects( self->slEquations ); 
   Stg_Class_Delete( self->slEquations ); */
   Stream_UnIndentBranch( StgFEM_Debug );

   /* Stg_Class_Delete parent */
   _DomainContext_Delete( self );
}

void _FiniteElementContext_Destroy( void* component, void* data ) {
   FiniteElementContext* self = (FiniteElementContext*)component;
   
   Stg_Class_Delete( self->slEquations );
   _DomainContext_Destroy( self, data );
}

void _FiniteElementContext_Print( void* context, Stream* stream ) {
   FiniteElementContext* self = (FiniteElementContext*)context;
   
   /* General info */
   Journal_Printf( (void*) stream, "FiniteElementContext (ptr): %p\n", self );
   
   /* Print parent */
   _DomainContext_Print( self, stream );

   Journal_Printf( (void*) stream, "\tslEquations (ptr): %p\n", self->slEquations );
   Stg_Class_Print( self->slEquations, stream );
}

void _FiniteElementContext_SetDt( void* context, double dt ) {
   FiniteElementContext* self = (FiniteElementContext*)context;
   
   self->dt = dt;
}

/* Public Functions --------------------------------------------------------------------------------------------------------------*/

void FiniteElementContext_AddSLE_Func( void* context, void* sle ) {
   FiniteElementContext* self = (FiniteElementContext*)context;
   
   FiniteElementContext_AddSLE_Macro( self, sle );
}

SystemLinearEquations* FiniteElementContext_GetSLE_Func( void* context, Name sleName ) {
   FiniteElementContext* self = (FiniteElementContext*)context;
   
   return FiniteElementContext_GetSLE_Macro( self, sleName );
}

/* EntryPoint Hooks --------------------------------------------------------------------------------------------------------------*/

void _FiniteElementContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data ){
   FiniteElementContext *self = (FiniteElementContext*) context;
   Stream*              errorStream = Journal_Register( Error_Type, (Name)self->type );

   _DomainContext_AssignFromXML( context, cf, data );

   self->dt = 0.0f;
   self->prevTimestepDt = 0.0;
   self->limitTimeStepIncreaseRate = Dictionary_GetBool_WithDefault(
      self->dictionary,
      (Dictionary_Entry_Key)"limitTimeStepIncreaseRate",
      False );

   self->maxTimeStepIncreasePercentage = Dictionary_GetDouble_WithDefault(
      self->dictionary,
      (Dictionary_Entry_Key)"maxTimeStepIncreasePercentage",
      10.0 );

   Journal_Firewall(
      self->maxTimeStepIncreasePercentage >= 0,
      errorStream,
      "Error - in %s(): maxTimeStepIncreasePercentage must be >= 0\n",
      __func__ );

   self->maxTimeStepSize = Dictionary_GetDouble_WithDefault(
      self->dictionary,
      (Dictionary_Entry_Key)"maxTimeStepSize",
      0.0 );

   self->dtGran = Dictionary_GetDouble_WithDefault(
      self->dictionary,
      (Dictionary_Entry_Key)"DtGranularity",
      0.0 );

   _FiniteElementContext_Init( self );
}

void _FiniteElementContext_Build( void* context ) {
   FiniteElementContext*       self = (FiniteElementContext*)context;
   SystemLinearEquations_Index sle_I;
   
   Stream_IndentBranch( StgFEM_Debug );
   Journal_DPrintf( self->debug, "In: %s()\n", __func__ );

   /* build all the systems of linear equations */
   for ( sle_I = 0; sle_I < self->slEquations->count; sle_I++ ) {
      Stg_Component_Build( self->slEquations->data[sle_I], self, False );
   }

   /* TODO:
   This call shouldn't really be necessary - each variable used should be built
   by the FeVariable that needs it, via its dofLayout. However, unfortunately with
   "Vector" variables that use it, the app fails without this call - since otherwise
   the "velocity" variable doesn't get build, only "vx", "vy" and "vz".
   Need to debug this properly later.
   */
   Variable_Register_BuildAll( self->variable_Register );

   Stream_UnIndentBranch( StgFEM_Debug );
}

void _FiniteElementContext_Initialise( void* context ) {
   FiniteElementContext*       self = (FiniteElementContext*)context;
   SystemLinearEquations_Index sle_I;
   
   Stream_IndentBranch( StgFEM_Debug );
   Journal_DPrintf( self->debug, "In: %s()\n", __func__ );

   /* initialise all the systems of linear equations */
   for ( sle_I = 0; sle_I < self->slEquations->count; sle_I++ ) {
      Stg_Component_Initialise( self->slEquations->data[sle_I], self, False );
   }

   Stream_UnIndentBranch( StgFEM_Debug );
}

void _FiniteElementContext_Solve( void* context ) {
   FiniteElementContext*       self = (FiniteElementContext*)context;
   SystemLinearEquations_Index sle_I;
   
   Journal_DPrintf( self->debug, "In: %s()\n", __func__ );
   Stream_IndentBranch( StgFEM_Debug );

   /* solve all the systems of linear equations */
   for ( sle_I = 0; sle_I < self->slEquations->count; sle_I++ ) {
      SystemLinearEquations* currentSLE = (SystemLinearEquations*)self->slEquations->data[sle_I];
      Journal_DPrintf( self->debug, "Solving for this timestep the %s SLE:\n", self->slEquations->data[sle_I]->name );
      /* TODO: FeVariable should have the option of rebuilding ID and LM, based on sim.
      loop if geometry or BCs change...need to improve interface. */
      /* We set the "force" flag to True here - want the SLE to be re-solved every timestep */
      Stg_Component_Execute( currentSLE, self, True );
   }
   
   Stream_UnIndentBranch( StgFEM_Debug );
}

void _FiniteElementContext_PostSolve( void* context ) {
   FiniteElementContext* self = (FiniteElementContext*)context;

   Journal_DPrintf( self->debug, "In: %s()\n", __func__ );
   Stream_IndentBranch( StgFEM_Debug );

   FiniteElementContext_CalcNewDt( self );

   Stream_UnIndentBranch( StgFEM_Debug );
}

double _FiniteElementContext_GetDt( void* context ) {
   FiniteElementContext* self = (FiniteElementContext*)context;

   return self->dt;
}

double FiniteElementContext_CalcNewDt( void* context ) {
   FiniteElementContext* self = (FiniteElementContext*)context;

   self->prevTimestepDt = self->dt;
   
   if( self->calcDtEP->hooks->count == 0 ) {
      self->dt = 0.0;
   }
   else {
      self->dt = self->dtFactor * _EntryPoint_Run_Class_Minimum_VoidPtr( self->calcDtEP, self );
   }   
      
   if( ( self->timeStep > 0 ) && ( self->limitTimeStepIncreaseRate == True ) ) {
      double  maxAllowedDt = self->prevTimestepDt * ( 1 + self->maxTimeStepIncreasePercentage / 100 );

      if ( self->dt > maxAllowedDt ) {
         int prevContextPrintingRank = Stream_GetPrintingRank( self->info );
         /* We assume the dt calculation will be the same across all procs since its a global
           operation, so only print this once */
         Stream_SetPrintingRank( self->info, 0 );
         Journal_Printf( 
            self->info, 
            "In %s(): dt calculated was %g (time), but prev timestep's dt\n"
            "was %g (time) and max allowed increase percentage is %.2f\n, thus limiting current\n"
            "dt to %g (time).\n", 
            __func__, 
            self->dt, 
            self->prevTimestepDt,
            self->maxTimeStepIncreasePercentage, 
            maxAllowedDt );
         self->dt = maxAllowedDt;
         Stream_SetPrintingRank( self->info, prevContextPrintingRank );
      }
   }

   if( self->maxTimeStepSize > 0.0 ) {
      if( self->dt > self->maxTimeStepSize )
         self->dt = self->maxTimeStepSize;
   }

   if( self->dtGran > 0.0 ) {
       self->dt = floor( self->dt/self->dtGran )*self->dtGran;
   }
   return self->dt;
}

