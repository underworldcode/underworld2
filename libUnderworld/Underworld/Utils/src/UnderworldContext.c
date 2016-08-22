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
#include <PICellerator/PICellerator.h>
#include <Underworld/Rheology/Rheology.h>

#include "types.h"
#include "UnderworldContext.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type UnderworldContext_Type = "UnderworldContext";

/* Constructors ------------------------------------------------------------------------------------------------*/

UnderworldContext* UnderworldContext_New( 
	Name			name,
	double		start,
	double		stop,
	MPI_Comm		communicator,
	Dictionary*	dictionary )
{
	UnderworldContext* self = (UnderworldContext*)_UnderworldContext_DefaultNew( name );

	self->isConstructed = True;
	_AbstractContext_Init( (AbstractContext*) self );
	_DomainContext_Init( (DomainContext*) self );	
	_FiniteElementContext_Init( (FiniteElementContext*) self );
	_PICelleratorContext_Init( (PICelleratorContext*) self );
	_UnderworldContext_Init( self );

	return self;
}	

void* _UnderworldContext_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(UnderworldContext);
	Type                                                      type = UnderworldContext_Type;
	Stg_Class_DeleteFunction*                              _delete = _UnderworldContext_Delete;
	Stg_Class_PrintFunction*                                _print = _UnderworldContext_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _UnderworldContext_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _UnderworldContext_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _AbstractContext_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _AbstractContext_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _AbstractContext_Execute;
	Stg_Component_DestroyFunction*                        _destroy = (Stg_Component_DestroyFunction*)_UnderworldContext_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	AbstractContext_SetDt*                                  _setDt = _PICelleratorContext_SetDt;
	double                                               startTime = 0;
	double                                                stopTime = 0;
	MPI_Comm                                          communicator = MPI_COMM_WORLD;
	Dictionary*                                         dictionary = NULL;

	return (void*) _UnderworldContext_New(  UNDERWORLDCONTEXT_PASSARGS  );
}

UnderworldContext* _UnderworldContext_New(  UNDERWORLDCONTEXT_DEFARGS  ) {
	UnderworldContext* self;
	
	/* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. 
		At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
	self = (UnderworldContext*)_PICelleratorContext_New(  PICELLERATORCONTEXT_PASSARGS  );
	
	/* General info */
	
	/* Function pointers for this class that are not on the parent class should be set here */
	
	return self;
}

void _UnderworldContext_Init( UnderworldContext* self ) {}

/* Virtual Functions -------------------------------------------------------------------------------------------------------------*/

void _UnderworldContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data ) {
	UnderworldContext* self = (UnderworldContext*)context;

	_PICelleratorContext_AssignFromXML( context, cf, data );

	_UnderworldContext_Init( self );
}

void _UnderworldContext_Delete( void* context ) {
	UnderworldContext* self = (UnderworldContext*)context;
	
	Journal_DPrintf( self->debug, "In: %s()\n", __func__ );

	/* Stg_Class_Delete parent */
	_PICelleratorContext_Delete( self );
}

void _UnderworldContext_Destroy( void* component, void* data ) {
	UnderworldContext* self = (UnderworldContext*)component;

   _UnderworldContext_GenerateOptionsFile( self );
	
	_PICelleratorContext_Destroy( self, data );
}

void _UnderworldContext_GenerateOptionsFile( UnderworldContext* context ) {
   char*              optionsFilename;
   #if ((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=2))
   PetscViewer        options;
   #else
   FILE*              options;
   #endif

   Stg_asprintf( &optionsFilename, "%s/options-%s.txt", context->outputPath, context->timeStamp );

   /* Generate PETSc options file. */
   #if ((PETSC_VERSION_MAJOR==3) && (PETSC_VERSION_MINOR>=2))
   PetscViewerASCIIOpen( context->communicator, optionsFilename, &options );
   PetscOptionsView( options );
   PetscViewerDestroy( &options );
   #else
   options = fopen( optionsFilename, "w" );
   PetscOptionsPrint( options );
   fclose( options );
   #endif
   Memory_Free( optionsFilename );
}

void _UnderworldContext_Print( void* context, Stream* stream ) {
	UnderworldContext* self = (UnderworldContext*)context;
	
	/* General info */
	Journal_Printf( (void*) stream, "UnderworldContext (ptr): %p\n", self );
	
	/* Print parent */
	_PICelleratorContext_Print( self, stream );

#if 0
	Journal_PrintPointer( stream, self->stokesSLE );
	Journal_PrintPointer( stream, self->energySLE );
	Journal_PrintPointer( stream, self->compositionSLE );
	Journal_PrintPointer( stream, self->constitutiveMatrix );
#endif
}


/* Public Functions ----------------------------------------------------------------------------------------------------*/


/* EntryPoint Hooks ----------------------------------------------------------------------------------------------------*/


