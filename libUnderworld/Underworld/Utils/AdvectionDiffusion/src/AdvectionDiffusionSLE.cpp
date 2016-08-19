/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <assert.h>
#include <string.h>
#include <sstream>
#include <math.h>

#include <mpi.h>
#include <petsc.h>
/*
#include "mpi.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"

#include "types.h"
#include "AdvectionDiffusionSLE.h"
#include "Residual.h"
#include "Multicorrector.h"
#include "Timestep.h"
*/
#include "AdvectionDiffusionSLE.h"

const Type AdvectionDiffusionSLE_Type = "AdvectionDiffusionSLE";
#define SMALL_VALUE 1.0e-99

AdvectionDiffusionSLE* AdvectionDiffusionSLE_New( 
	Name							name,
	FiniteElementContext*	context,
	SLE_Solver*					solver,
	Bool							removeBCs,
	Bool							isNonLinear,
	double						nonLinearTolerance,
	Iteration_Index			nonLinearMaxIterations,
	Bool							killNonConvergent,		
	EntryPoint_Register*		entryPoint_Register,
	MPI_Comm						comm,
	FeVariable*					phiField,
	ForceVector*				residual,
	Stg_Component*				massMatrix,
	Dimension_Index			dim,
	double						courantFactor,
	Variable_Register*		variable_Register,
	FieldVariable_Register*	fieldVariable_Register ) 
{	
	AdvectionDiffusionSLE* self = (AdvectionDiffusionSLE*) _AdvectionDiffusionSLE_DefaultNew( name );

	self->isConstructed = True;	
	_SystemLinearEquations_Init( self, solver, removeBCs, NULL, context, False, isNonLinear, nonLinearTolerance,
		nonLinearMaxIterations, killNonConvergent, 1, "", "", entryPoint_Register, comm );
	_AdvectionDiffusionSLE_Init( self, phiField, residual, massMatrix, dim, courantFactor, variable_Register, fieldVariable_Register );

	 return self;
}

/* Creation implementation / Virtual constructor */
AdvectionDiffusionSLE* _AdvectionDiffusionSLE_New(  ADVECTIONDIFFUSIONSLE_DEFARGS  )
{
	AdvectionDiffusionSLE* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(AdvectionDiffusionSLE) );
	self = (AdvectionDiffusionSLE*) _SystemLinearEquations_New(  SYSTEMLINEAREQUATIONS_PASSARGS  );

	return self;
}

void _AdvectionDiffusionSLE_Init(
	void*							sle,
	FeVariable*					phiField,
	ForceVector*				residual,
	Stg_Component*				massMatrix,
	Dimension_Index			dim,
	double						courantFactor,
	Variable_Register*		variable_Register,  
	FieldVariable_Register*	fieldVariable_Register )		
{
	AdvectionDiffusionSLE* self = (AdvectionDiffusionSLE*)sle;

	/* Assign values */
	self->phiField = phiField;
	self->residual = residual;
	self->massMatrix = massMatrix;
	self->dim = dim;
	self->courantFactor = courantFactor;

	/* Solution Vectors are loaded up as part of the algorithm so we can remove this one */
	EP_Remove( self->executeEP, "UpdateSolutionOntoNodes" );
	EP_Remove( self->executeEP, "MatrixSetup" );
	EP_Remove( self->executeEP, "VectorSetup" );

	/* Put Pointer of Solver onto vectors */
	if (residual) {
		residual->applicationDepExtraInfo   = (Stg_Component*) self;
		SystemLinearEquations_AddForceVector( self, residual );
	}
	if (massMatrix && Stg_Class_IsInstance( massMatrix, ForceVector_Type ) ) {
		((ForceVector*) massMatrix)->applicationDepExtraInfo = (Stg_Component*) self;
		SystemLinearEquations_AddForceVector( self, massMatrix );
	}
	else if (massMatrix && Stg_Class_IsInstance( massMatrix, StiffnessMatrix_Type ) ) {
		((StiffnessMatrix*) massMatrix)->applicationDepInfo = (Stg_Component*) self;
		SystemLinearEquations_AddStiffnessMatrix( self, massMatrix );
	}

	self->variableReg = variable_Register;
	self->fieldVariableReg = fieldVariable_Register;

    self->maxDiffusivity = SMALL_VALUE;

}	

/** Virtual Functions from "Class" Class */
void _AdvectionDiffusionSLE_Delete( void* sle ) {
	AdvectionDiffusionSLE* self = (AdvectionDiffusionSLE*)sle;

	_SystemLinearEquations_Delete( self );
}

void _AdvectionDiffusionSLE_Print( void* sle, Stream* stream ) {}


void* _AdvectionDiffusionSLE_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                       _sizeOfSelf = sizeof(AdvectionDiffusionSLE);
	Type                                                               type = AdvectionDiffusionSLE_Type;
	Stg_Class_DeleteFunction*                                       _delete = _AdvectionDiffusionSLE_Delete;
	Stg_Class_PrintFunction*                                         _print = _AdvectionDiffusionSLE_Print;
	Stg_Class_CopyFunction*                                           _copy = NULL;
	Stg_Component_DefaultConstructorFunction*           _defaultConstructor = _AdvectionDiffusionSLE_DefaultNew;
	Stg_Component_ConstructFunction*                             _construct = _AdvectionDiffusionSLE_AssignFromXML;
	Stg_Component_BuildFunction*                                     _build = _AdvectionDiffusionSLE_Build;
	Stg_Component_InitialiseFunction*                           _initialise = _AdvectionDiffusionSLE_Initialise;
	Stg_Component_ExecuteFunction*                                 _execute = _AdvectionDiffusionSLE_Execute;
	Stg_Component_DestroyFunction*                                 _destroy = _AdvectionDiffusionSLE_Destroy;
	SystemLinearEquations_LM_SetupFunction*                       _LM_Setup = _SystemLinearEquations_LM_Setup;
	SystemLinearEquations_MatrixSetupFunction*                 _matrixSetup = _SystemLinearEquations_MatrixSetup;
	SystemLinearEquations_VectorSetupFunction*                 _vectorSetup = _SystemLinearEquations_VectorSetup;
	SystemLinearEquations_MG_SelectStiffMatsFunc*        _mgSelectStiffMats = _SystemLinearEquations_MG_SelectStiffMats;

	AllocationType                                            nameAllocationType = GLOBAL;
	SystemLinearEquations_UpdateSolutionOntoNodesFunc*  _updateSolutionOntoNodes = NULL;

	return (void*) _AdvectionDiffusionSLE_New(  ADVECTIONDIFFUSIONSLE_PASSARGS  );
}

void _AdvectionDiffusionSLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data ) {
	AdvectionDiffusionSLE*	self = (AdvectionDiffusionSLE*) sle;
	Stream*						error = Journal_Register( Error_Type, (Name)self->type  );
	FeVariable*					phiField;
	ForceVector*				residual;
	Stg_Component*				massMatrix;
	Dimension_Index			dim;
	double						courantFactor;
	FieldVariable_Register*	fieldVariable_Register = NULL;
	Variable_Register*		variable_Register = NULL;

	/* Construct Parent */
	_SystemLinearEquations_AssignFromXML( self, cf, data );

	/* Get Registers */
    if (self->context) {
        variable_Register = self->context->variable_Register;
        assert( variable_Register );
        fieldVariable_Register = self->context->fieldVariable_Register; 
        assert( fieldVariable_Register );
    }

	/* Get Dependency Stg_Components */
	phiField    =  Stg_ComponentFactory_ConstructByKey( cf, self->name,    (Dictionary_Entry_Key)"PhiField", FeVariable, True, data  );
	residual   =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Residual", ForceVector, True, data  );
	massMatrix =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"MassMatrix", Stg_Component, True, data  );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
    dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim  );

	courantFactor = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"courantFactor", 0.5  );
	Journal_Firewall( 0.0 < courantFactor && courantFactor <= 1.0, 
		error, "In func %s: CourantFactor read in from dictionary = %2.4f - This must be from 0 - 1.\n", 
		__func__, courantFactor );

        self->pureDiffusion = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"pureDiffusion", False  );

    self->phiDotField =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"PhiDotField", FeVariable, True, data  );

	_AdvectionDiffusionSLE_Init(
		self,
		phiField,
		residual,
		massMatrix,
		dim,
		courantFactor,
		variable_Register, 
		fieldVariable_Register );
}

void _AdvectionDiffusionSLE_Destroy( void* sle, void* data ) {
   AdvectionDiffusionSLE* self = (AdvectionDiffusionSLE*) sle;

   __AdvDiffResidualForceTerm_FreeLocalMemory( self );
   Stg_Component_Destroy( self->phiDotVector, data, False );
   Stg_Component_Destroy( self->phiVector, data, False );

   _SystemLinearEquations_Destroy( self, data );
}

/*******************************************************************************
  The following function scans all the elements of the mesh associated with
  the residual forceterm phi to find the element with the most nodes.
  Once the maximum number of nodes is found we then may allocate memory for
  GNx etc that now live on the AdvDiffResidualForceTerm struct. This way we 
  do not reallocate memory for these arrays for every element.
 *******************************************************************************/
void __AdvDiffResidualForceTerm_UpdateLocalMemory( AdvectionDiffusionSLE* sle ){
	FeVariable*				phiField = sle->phiField;
	FeMesh*					phiMesh = phiField->feMesh;
	Dimension_Index 		dim = phiField->dim;
	Element_LocalIndex	e, n_elements;
	Node_Index				max_elementNodeCount;

   if( sle->advDiffResidualForceTerm == NULL ) return;

	n_elements = FeMesh_GetElementLocalSize(phiMesh);//returns number of elements in a mesh

	/* Scan all elements in Mesh to get max node count */
	max_elementNodeCount = 0;

	for(e=0;e<n_elements;e++){
		ElementType *elementType = FeMesh_GetElementType( phiMesh, e );
		Node_Index elementNodeCount = elementType->nodeCount;
	
		if( elementNodeCount > max_elementNodeCount){
			max_elementNodeCount = elementNodeCount;
		}
	}

   /* logic to see if we need to allocate memory */
   if( sle->advDiffResidualForceTerm->last_maxNodeCount == 0 ) {
      /* first time in this function, so allocate */
      sle->advDiffResidualForceTerm->GNx = Memory_Alloc_2DArray( double, dim, max_elementNodeCount, (Name)"(SUPG): Global Shape Function Derivatives" );
      sle->advDiffResidualForceTerm->phiGrad = Memory_Alloc_Array(double, dim, "(SUPG): Gradient of the Advected Scalar");
      sle->advDiffResidualForceTerm->Ni = Memory_Alloc_Array(double, max_elementNodeCount, "(SUPG): Gradient of the Advected Scalar");
      sle->advDiffResidualForceTerm->SUPGNi = Memory_Alloc_Array(double, max_elementNodeCount, "(SUPG): Upwinded Shape Function");
      sle->advDiffResidualForceTerm->incarray=IArray_New();
   } else if (sle->advDiffResidualForceTerm->last_maxNodeCount < max_elementNodeCount) {
      /* free existing memory and re-allocate */
      __AdvDiffResidualForceTerm_FreeLocalMemory( sle );
      sle->advDiffResidualForceTerm->GNx = Memory_Alloc_2DArray( double, dim, max_elementNodeCount, (Name)"(SUPG): Global Shape Function Derivatives" );
      sle->advDiffResidualForceTerm->phiGrad = Memory_Alloc_Array(double, dim, "(SUPG): Gradient of the Advected Scalar");
      sle->advDiffResidualForceTerm->Ni = Memory_Alloc_Array(double, max_elementNodeCount, "(SUPG): Gradient of the Advected Scalar");
      sle->advDiffResidualForceTerm->SUPGNi = Memory_Alloc_Array(double, max_elementNodeCount, "(SUPG): Upwinded Shape Function");
      sle->advDiffResidualForceTerm->incarray=IArray_New();
   }

   sle->advDiffResidualForceTerm->last_maxNodeCount = max_elementNodeCount;
}

void __AdvDiffResidualForceTerm_FreeLocalMemory( AdvectionDiffusionSLE* sle ){

   if( sle->advDiffResidualForceTerm == NULL ) return;

	Memory_Free(sle->advDiffResidualForceTerm->GNx);
	Memory_Free(sle->advDiffResidualForceTerm->phiGrad);
	Memory_Free(sle->advDiffResidualForceTerm->Ni);
	Memory_Free(sle->advDiffResidualForceTerm->SUPGNi);
  
	Stg_Class_Delete(sle->advDiffResidualForceTerm->incarray);
}


/** Virtual Functions from "Stg_Component" Class */
void _AdvectionDiffusionSLE_Build( void* sle, void* data ) {
	AdvectionDiffusionSLE*	self = (AdvectionDiffusionSLE*) sle;
	Stream*						errorStream = Journal_MyStream( Error_Type, self );
	Index							forceTerm_I;
	Index							forceTermCount = Stg_ObjectList_Count( self->residual->forceTermList );
	ForceTerm*					forceTerm;

    /* Construct Solution Vectors */
    char *phiVecName, *phiDotVecName;
    phiVecName = Memory_Alloc_Array_Unnamed( char, strlen(self->name)+11 );
    sprintf( phiVecName, "%s-phiVector", self->name );
    phiDotVecName = Memory_Alloc_Array_Unnamed( char, strlen(self->name)+14 );
    sprintf( phiDotVecName, "%s-phiDotVector", self->name );
    self->phiVector    = SolutionVector_New(    phiVecName, self->phiField->communicator,    self->phiField );
    self->phiDotVector = SolutionVector_New( phiDotVecName, self->phiDotField->communicator, self->phiDotField );
    Memory_Free(phiVecName);
    Memory_Free(phiDotVecName);

	_SystemLinearEquations_Build( self, data );

	/* Force Vectors */
	if ( self->residual )
		Stg_Component_Build( self->residual, data, False );
	if ( self->massMatrix )
		Stg_Component_Build( self->massMatrix, data, False );
	
	/* Solution Vectors */
	if ( self->phiVector )
		Stg_Component_Build( self->phiVector, data, False );
	if ( self->phiDotVector )
		Stg_Component_Build( self->phiDotVector, data, False );
}

void _AdvectionDiffusionSLE_Initialise( void* sle, void* data ) {
	AdvectionDiffusionSLE* self = (AdvectionDiffusionSLE*) sle;
	FiniteElementContext* context = (FiniteElementContext*) data;

	Journal_DPrintf( self->debug, "In %s()\n", __func__ );

	_SystemLinearEquations_Initialise( self, data );
	
	Stg_Component_Initialise( self->phiDotField, data, False );
	
    DofLayout_SetAllToZero( self->phiDotField->dofLayout );

	/* Force Vectors */
	Stg_Component_Initialise( self->residual, data, False );
	Stg_Component_Initialise( self->massMatrix, data, False );
	
	/* Solution Vectors */
	Stg_Component_Initialise( self->phiVector, data, False );
	Stg_Component_Initialise( self->phiDotVector, data, False );

	AdvectionDiffusionSLE_ResetStoredValues( self );
}

void _AdvectionDiffusionSLE_Execute( void* sle, void* _context ) {
	AdvectionDiffusionSLE*     self  = (AdvectionDiffusionSLE*) sle;
	FiniteElementContext*      context = (FiniteElementContext*) _context;
//	double                     dt      = context->dt;
	
	AdvectionDiffusionSLE_ResetStoredValues( self );
//	self->currentDt = dt;
	
	_SystemLinearEquations_Execute( self, context );
}


//Vector* _AdvectionDiffusionSLE_GetResidual( void* sle, Index fv_I ) {
Vec _AdvectionDiffusionSLE_GetResidual( void* sle, Index fv_I ) {
	AdvectionDiffusionSLE* self  = (AdvectionDiffusionSLE*) sle;
	return self->residual->vector;
}


void AdvectionDiffusionSLE_ResetStoredValues( void* sle ) {
	AdvectionDiffusionSLE* self  = (AdvectionDiffusionSLE*) sle;
	
	self->maxDiffusivity = SMALL_VALUE;
}



