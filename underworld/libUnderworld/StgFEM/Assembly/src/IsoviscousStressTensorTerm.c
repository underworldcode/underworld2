/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/Discretisation/src/Discretisation.h>
#include <StgFEM/SLE/src/SLE.h>

#include "types.h"
#include "IsoviscousStressTensorTerm.h"

#include <assert.h>
#include <string.h>
/* Textual name of this class */
const Type IsoviscousStressTensorTerm_Type = "IsoviscousStressTensorTerm";

IsoviscousStressTensorTerm* IsoviscousStressTensorTerm_New( 
	Name							name,
	FiniteElementContext*	context,
	StiffnessMatrix*			stiffnessMatrix,
	Swarm*						integrationSwarm,
	double						viscosity )
{
	IsoviscousStressTensorTerm* self = (IsoviscousStressTensorTerm*) _IsoviscousStressTensorTerm_DefaultNew( name );

	self->isConstructed = True;
	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, integrationSwarm, NULL );
	_IsoviscousStressTensorTerm_Init( self, viscosity );

	return self;
}

/* Creation implementation / Virtual constructor */
IsoviscousStressTensorTerm* _IsoviscousStressTensorTerm_New(  ISOVISCOUSSTRESSTENSORTERM_DEFARGS  )
{
	IsoviscousStressTensorTerm* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IsoviscousStressTensorTerm) );
	self = (IsoviscousStressTensorTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
	
	/* Virtual info */
	
	return self;
}

void _IsoviscousStressTensorTerm_Init( void* matrixTerm, double viscosity ) {
	IsoviscousStressTensorTerm* self = (IsoviscousStressTensorTerm*)matrixTerm;
	self->viscosity = viscosity;
}

void _IsoviscousStressTensorTerm_Delete( void* matrixTerm ) {
	IsoviscousStressTensorTerm* self = (IsoviscousStressTensorTerm*)matrixTerm;

	_StiffnessMatrixTerm_Delete( self );
}

void _IsoviscousStressTensorTerm_Print( void* matrixTerm, Stream* stream ) {
	IsoviscousStressTensorTerm* self = (IsoviscousStressTensorTerm*)matrixTerm;
	
	_StiffnessMatrixTerm_Print( self, stream );

	/* General info */
	Journal_PrintValue( stream, self->viscosity );
}

void* _IsoviscousStressTensorTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(IsoviscousStressTensorTerm);
	Type                                                         type = IsoviscousStressTensorTerm_Type;
	Stg_Class_DeleteFunction*                                 _delete = _IsoviscousStressTensorTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _IsoviscousStressTensorTerm_Print;
	Stg_Class_CopyFunction*                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _IsoviscousStressTensorTerm_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _IsoviscousStressTensorTerm_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _IsoviscousStressTensorTerm_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _IsoviscousStressTensorTerm_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _IsoviscousStressTensorTerm_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _IsoviscousStressTensorTerm_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _IsoviscousStressTensorTerm_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_IsoviscousStressTensorTerm_New(  ISOVISCOUSSTRESSTENSORTERM_PASSARGS  );
}

void _IsoviscousStressTensorTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
	IsoviscousStressTensorTerm*            self             = (IsoviscousStressTensorTerm*)matrixTerm;

	/* Construct Parent */
	_StiffnessMatrixTerm_AssignFromXML( self, cf, data );

	_IsoviscousStressTensorTerm_Init( self, Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"viscosity", 1.0 )  );
}

void _IsoviscousStressTensorTerm_Build( void* matrixTerm, void* data ) {
	IsoviscousStressTensorTerm*             self             = (IsoviscousStressTensorTerm*)matrixTerm;

	_StiffnessMatrixTerm_Build( self, data );
}

void _IsoviscousStressTensorTerm_Initialise( void* matrixTerm, void* data ) {
	IsoviscousStressTensorTerm*             self             = (IsoviscousStressTensorTerm*)matrixTerm;

	_StiffnessMatrixTerm_Initialise( self, data );
}

void _IsoviscousStressTensorTerm_Execute( void* matrixTerm, void* data ) {
	_StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _IsoviscousStressTensorTerm_Destroy( void* matrixTerm, void* data ) {
	_StiffnessMatrixTerm_Destroy( matrixTerm, data );
}


void _IsoviscousStressTensorTerm_AssembleElement( 
		void*                                              matrixTerm,
		StiffnessMatrix*                                   stiffnessMatrix, 
		Element_LocalIndex                                 lElement_I, 
		SystemLinearEquations*                             sle,
		FiniteElementContext*                              context,
		double**                                           elStiffMat ) 
{
	IsoviscousStressTensorTerm*         self         = Stg_CheckType( matrixTerm, IsoviscousStressTensorTerm );
	Swarm*                              swarm        = self->integrationSwarm;
	FeVariable*                         variable1    = stiffnessMatrix->rowVariable;
	Dimension_Index                     dim          = stiffnessMatrix->dim;
	
	IntegrationPoint*                   currIntegrationPoint;
	double*                             xi;
	double                              weight;
	Particle_InCellIndex                cParticle_I, cellParticleCount;
	Index                               nodesPerEl;
	Node_ElementLocalIndex              rowNode_I;
	Node_ElementLocalIndex              colNode_I;
	double**                            GNx;
	double                              detJac;
	double                              visc           = self->viscosity;
	
	Cell_Index                          cell_I;
	ElementType*                        elementType;
	Dof_Index                           diagDof_I;
	Dof_Index                           rowDof_I;
	Dof_Index                           colDof_I;
	Dof_Index                           d;
	Dof_Index                           dofsPerNode;
	Index                               rowIndex;
	Index                               colIndex;
	
	/* Set the element type */
	elementType = FeMesh_GetElementType( variable1->feMesh, lElement_I );
	nodesPerEl = elementType->nodeCount;

	/* assumes constant number of dofs per element */
	dofsPerNode = dim;
	
	if( nodesPerEl > self->max_nElNodes ) {
		/* reallocate */
		self->GNx = ReallocArray2D( self->GNx, double, dim, nodesPerEl );
		self->max_nElNodes = nodesPerEl;
	}
	GNx = self->GNx;

	cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
	cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];
	
	for( cParticle_I=0; cParticle_I < cellParticleCount; cParticle_I++ ) {
		
		currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		xi = currIntegrationPoint->xi;
		weight = currIntegrationPoint->weight;
		
		ElementType_ShapeFunctionsGlobalDerivs( 
			elementType,
			variable1->feMesh, lElement_I,
			xi, dim, &detJac, GNx );
		
		/* Initially just build the diagonal (from a Dof perspective) entries */
		for( rowNode_I=0; rowNode_I < nodesPerEl; rowNode_I++ ) { 		
			for( colNode_I=0; colNode_I < nodesPerEl; colNode_I++ ) {		
				for( diagDof_I=0; diagDof_I < dofsPerNode; diagDof_I++ ) {	
					rowIndex =  rowNode_I*dofsPerNode + diagDof_I;
					colIndex = colNode_I*dofsPerNode + diagDof_I;
					elStiffMat[rowIndex][colIndex] +=
						detJac * visc * weight * ( GNx[diagDof_I][rowNode_I] * GNx[diagDof_I][colNode_I] );
					for( d=0; d<dofsPerNode; d++ ) {	
						elStiffMat[rowIndex][colIndex] +=
							detJac * visc * weight * ( GNx[d][rowNode_I] * GNx[d][colNode_I] );
					}
				}
			}
		}
		
		/* then build one set of the off diagonal blocks of K, and copy to the other off diags using symmetry */
		for( rowNode_I=0; rowNode_I < nodesPerEl; rowNode_I++ ) {
                  for( rowDof_I=0; rowDof_I < dofsPerNode-1; rowDof_I++ ) {	/* dont need to do anything in last row */
				for( colNode_I=0; colNode_I < nodesPerEl; colNode_I++ ) {
					for( colDof_I=rowDof_I+1; colDof_I < dofsPerNode; colDof_I++ ) {
						rowIndex = rowNode_I*dofsPerNode + rowDof_I;
						colIndex = colNode_I*dofsPerNode + colDof_I;
						elStiffMat[rowIndex][colIndex] += detJac * visc * weight * 
								( GNx[colDof_I][rowNode_I] * GNx[rowDof_I][colNode_I] );
						/* symmetry	*/
						elStiffMat[colIndex][rowIndex] = elStiffMat[rowIndex][colIndex];
					}
				}
			}
		}
	}
	return;
}


