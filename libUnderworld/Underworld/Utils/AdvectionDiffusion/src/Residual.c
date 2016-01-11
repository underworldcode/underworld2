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
#include <StgFEM/Discretisation/Discretisation.h>
#include <StgFEM/SLE/SystemSetup/SystemSetup.h>

#include "types.h"
#include "AdvectionDiffusionSLE.h"
#include "Residual.h"
#include "UpwindParameter.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type AdvDiffResidualForceTerm_Type = "AdvDiffResidualForceTerm";

AdvDiffResidualForceTerm* AdvDiffResidualForceTerm_New( 
	Name							name,
	FiniteElementContext*	context,
	ForceVector*				forceVector,
	Swarm*						integrationSwarm,
	Stg_Component*				sle, 
	FeVariable*					velocityField,
	Variable*					diffusivityVariable,
	double						defaultDiffusivity,
	AdvDiffResidualForceTerm_UpwindParamFuncType upwindFuncType )
{
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*) _AdvDiffResidualForceTerm_DefaultNew( name );

	self->isConstructed = True;
	_ForceTerm_Init( self, context, forceVector, integrationSwarm, sle );
	_AdvDiffResidualForceTerm_Init( self, velocityField, diffusivityVariable, defaultDiffusivity, upwindFuncType );

	return self;
}

void* _AdvDiffResidualForceTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                  _sizeOfSelf = sizeof(AdvDiffResidualForceTerm);
	Type                                                          type = AdvDiffResidualForceTerm_Type;
	Stg_Class_DeleteFunction*                                  _delete = _AdvDiffResidualForceTerm_Delete;
	Stg_Class_PrintFunction*                                    _print = _AdvDiffResidualForceTerm_Print;
	Stg_Class_CopyFunction*                                      _copy = NULL;
	Stg_Component_DefaultConstructorFunction*      _defaultConstructor = _AdvDiffResidualForceTerm_DefaultNew;
	Stg_Component_ConstructFunction*                        _construct = _AdvDiffResidualForceTerm_AssignFromXML;
	Stg_Component_BuildFunction*                                _build = _AdvDiffResidualForceTerm_Build;
	Stg_Component_InitialiseFunction*                      _initialise = _AdvDiffResidualForceTerm_Initialise;
	Stg_Component_ExecuteFunction*                            _execute = _AdvDiffResidualForceTerm_Execute;
	Stg_Component_DestroyFunction*                            _destroy = _AdvDiffResidualForceTerm_Destroy;
	ForceTerm_AssembleElementFunction*                _assembleElement = _AdvDiffResidualForceTerm_AssembleElement;
	AdvDiffResidualForceTerm_UpwindParamFunction*         _upwindParam = _AdvDiffResidualForceTerm_UpwindParam;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_AdvDiffResidualForceTerm_New(  ADVDIFFRESIDUALFORCETERM_PASSARGS  );
}

/* Creation implementation / Virtual constructor */
AdvDiffResidualForceTerm* _AdvDiffResidualForceTerm_New(  ADVDIFFRESIDUALFORCETERM_DEFARGS  )
{
	AdvDiffResidualForceTerm* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(AdvDiffResidualForceTerm) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (AdvDiffResidualForceTerm*) _ForceTerm_New(  FORCETERM_PASSARGS  );
	
	/* Virtual info */
	self->_upwindParam = _upwindParam;
	
	return self;
}

void _AdvDiffResidualForceTerm_Init(
	void*														residual,
	FeVariable*												velocityField,
	Variable*												diffusivityVariable,
	double													defaultDiffusivity,
	AdvDiffResidualForceTerm_UpwindParamFuncType	upwindFuncType ) //WHY IS THIS LINE HERE???
{
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	self->velocityField = velocityField;
	self->diffusivityVariable = diffusivityVariable;
	self->defaultDiffusivity = defaultDiffusivity;
	self->upwindParamType = upwindFuncType;
   self->last_maxNodeCount = 0;
}

void _AdvDiffResidualForceTerm_Delete( void* residual ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	_ForceTerm_Delete( self );
}

void _AdvDiffResidualForceTerm_Print( void* residual, Stream* stream ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;
	
	_ForceTerm_Print( self, stream );

	Journal_Printf( stream, "self->calculateUpwindParam = %s\n", 
		self->_upwindParam == AdvDiffResidualForceTerm_UpwindXiExact ? 
			"AdvDiffResidualForceTerm_UpwindXiExact" :
		self->_upwindParam == AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption ? 
			"AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption" :
		self->_upwindParam == AdvDiffResidualForceTerm_UpwindXiCriticalAssumption ? 
		"AdvDiffResidualForceTerm_UpwindXiCriticalAssumption" : "Unknown"  );

	/* General info */
	Journal_PrintPointer( stream, self->velocityField );
	Journal_PrintDouble( stream, self->defaultDiffusivity );
	Journal_Printf( stream, "self->diffusivityVariable = ");

	if ( self->diffusivityVariable )
		Journal_Printf( stream, "%s\n", self->diffusivityVariable->name );
	else
		Journal_Printf( stream, "<Unused>\n");
}

void _AdvDiffResidualForceTerm_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data ) {
	AdvDiffResidualForceTerm*							self = (AdvDiffResidualForceTerm*)residual;
	FeVariable*												velocityField;
	Variable*												diffusivityVariable;
	Name														upwindParamFuncName;
	double													defaultDiffusivity;
	AdvDiffResidualForceTerm_UpwindParamFuncType	upwindFuncType = 0;

	/* Construct Parent */
	_ForceTerm_AssignFromXML( self, cf, data );

	velocityField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityField", FeVariable, True, data  );
	diffusivityVariable = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"DiffusivityVariable", Variable, False, data  );
	upwindParamFuncName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"UpwindXiFunction", "Exact"  );

	if ( strcasecmp( upwindParamFuncName, "DoublyAsymptoticAssumption" ) == 0 )
		upwindFuncType = DoublyAsymptoticAssumption;
	else if ( strcasecmp( upwindParamFuncName, "CriticalAssumption" ) == 0 )
		upwindFuncType = CriticalAssumption;
	else if ( strcasecmp( upwindParamFuncName, "Exact" ) == 0 )
		upwindFuncType = Exact;
	else 
		Journal_Firewall( False, Journal_Register( Error_Type, (Name)self->type  ), "Cannot understand '%s'\n", upwindParamFuncName );

	defaultDiffusivity = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"defaultDiffusivity", 1.0  );

	_AdvDiffResidualForceTerm_Init( self, velocityField, diffusivityVariable, defaultDiffusivity, upwindFuncType );
}

void _AdvDiffResidualForceTerm_Build( void* residual, void* data ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	_ForceTerm_Build( self, data );

	Stg_Component_Build( self->velocityField, data, False );

	if ( self->diffusivityVariable )
		Stg_Component_Build( self->diffusivityVariable, data, False );
}

void _AdvDiffResidualForceTerm_Initialise( void* residual, void* data ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	_ForceTerm_Initialise( self, data );

	Stg_Component_Initialise( self->velocityField, data, False );

	if ( self->diffusivityVariable )
		Stg_Component_Initialise( self->diffusivityVariable, data, False );
}

void _AdvDiffResidualForceTerm_Execute( void* residual, void* data ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	_ForceTerm_Execute( self, data );
}

void _AdvDiffResidualForceTerm_Destroy( void* residual, void* data ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

   self->last_maxNodeCount = 0;
	_ForceTerm_Destroy( self, data );
}

void _AdvDiffResidualForceTerm_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elementResidual ) {
	AdvDiffResidualForceTerm*  self               = Stg_CheckType( forceTerm, AdvDiffResidualForceTerm );
	AdvectionDiffusionSLE*     sle                = Stg_CheckType( self->extraInfo, AdvectionDiffusionSLE );
	Swarm*                     swarm              = self->integrationSwarm;
	Particle_Index             lParticle_I;
	Particle_Index             cParticle_I;
	Particle_Index             cellParticleCount;
	Cell_Index                 cell_I;    
	IntegrationPoint*          particle;
	FeVariable*                phiField           = sle->phiField;
	Dimension_Index            dim                = forceVector->dim;
	double                     velocity[3];
	double                     phi, phiDot;
	double                     detJac;
	double*                    xi;
	double                     totalDerivative, diffusionTerm;
	double                     diffusivity         = self->defaultDiffusivity;
	Variable*                  diffusivityVariable = self->diffusivityVariable;
	ElementType*               elementType         = FeMesh_GetElementType( phiField->feMesh, lElement_I );
	Node_Index                 elementNodeCount    = elementType->nodeCount;
	Node_Index                 node_I;
	double                     factor;

	double**                   GNx;
	double*                    phiGrad;
	double*                    Ni;
	double*                    SUPGNi;
	double                     supgfactor;
	double                     udotu, perturbation;
	double                     upwindDiffusivity;
	GNx     = self->GNx;
	phiGrad = self->phiGrad;
	Ni = self->Ni;
	SUPGNi = self->SUPGNi;
	
	upwindDiffusivity  = AdvDiffResidualForceTerm_UpwindDiffusivity( self, sle, swarm, phiField->feMesh, lElement_I, dim );

	/* Determine number of particles in element */
	cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
	cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];
	
	for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
		lParticle_I     = swarm->cellParticleTbl[cell_I][cParticle_I];

		particle        = (IntegrationPoint*) Swarm_ParticleAt( swarm, lParticle_I );
		xi              = particle->xi;
		
		/* Evaluate Shape Functions */
		ElementType_EvaluateShapeFunctionsAt(elementType, xi, Ni);

		/* Calculate Global Shape Function Derivatives */
		ElementType_ShapeFunctionsGlobalDerivs( 
			elementType,
			phiField->feMesh, lElement_I,
			xi, dim, &detJac, GNx );
		
		/* Calculate Velocity */
		FeVariable_InterpolateFromMeshLocalCoord( self->velocityField, phiField->feMesh, lElement_I, xi, velocity );

		/* Build the SUPG shape functions */
		udotu = velocity[I_AXIS]*velocity[I_AXIS] + velocity[J_AXIS]*velocity[J_AXIS];
		if(dim == 3) udotu += velocity[ K_AXIS ] * velocity[ K_AXIS ];

		supgfactor = upwindDiffusivity / udotu;
		for ( node_I = 0 ; node_I < elementNodeCount ; node_I++ ) {
			/* In the case of per diffusion - just build regular shape functions */
			if ( fabs(upwindDiffusivity) < SUPG_MIN_DIFFUSIVITY ) {
				SUPGNi[node_I] = Ni[node_I];
				continue;
			}
			
			perturbation = velocity[ I_AXIS ] * GNx[ I_AXIS ][ node_I ] + velocity[ J_AXIS ] * GNx[ J_AXIS ][ node_I ];
			if (dim == 3)
					perturbation = perturbation + velocity[ K_AXIS ] * GNx[ K_AXIS ][ node_I ];
			
			/* p = \frac{\bar \kappa \hat u_j w_j }{ ||u|| } -  Eq. 3.2.25 */
			perturbation = supgfactor * perturbation;
			
			SUPGNi[node_I] = Ni[node_I] + perturbation;
		}  
		
		/* Calculate phi on particle */
		_FeVariable_InterpolateNodeValuesToElLocalCoord( phiField, lElement_I, xi, &phi );

		/* Calculate Gradients of Phi */
		FeVariable_InterpolateDerivatives_WithGNx( phiField, lElement_I, GNx, phiGrad );

		/* Calculate time derivative of phi */
		_FeVariable_InterpolateNodeValuesToElLocalCoord( sle->phiDotField, lElement_I, xi, &phiDot );
		
		/* Calculate total derivative (i.e. Dphi/Dt = \dot \phi + u . \grad \phi) */
		totalDerivative = phiDot + StGermain_VectorDotProduct( velocity, phiGrad, dim );

		/* Get Diffusivity */
		/* diffusivityVariable will only be NOT NULL if:
		 * 1) The MaterialDiffusivityPlugin is used. It's in Underworld/Plugins
		 * 2) A special user defined DiffusivityVariable is given during the Construction phase
		 */  
		if ( diffusivityVariable != NULL )
			diffusivity = self->_getDiffusivityFromIntPoint( self, particle );

		/* Add to element residual */
		factor = particle->weight * detJac;
		for ( node_I = 0 ; node_I < elementNodeCount ; node_I++ ) {
			/* Calculate Diffusion Term */
			diffusionTerm = diffusivity * ( GNx[0][node_I] * phiGrad[0] + GNx[1][node_I] * phiGrad[1] );
			if (dim == 3)
				diffusionTerm += diffusivity * GNx[2][ node_I ] * phiGrad[2] ;
			
                        if( sle->pureDiffusion )
                            elementResidual[ node_I ] -=  factor * diffusionTerm;
                        else
                            elementResidual[ node_I ] -=  factor * ( SUPGNi[ node_I ] * totalDerivative + diffusionTerm );
		}
	}
	
}

/* Virtual Function Implementations */
double _AdvDiffResidualForceTerm_UpwindParam( void* residual, double pecletNumber ) {
	AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

	switch ( self->upwindParamType ) {
		case Exact:
			self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiExact; break;
		case DoublyAsymptoticAssumption:
			self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption; break;
		case CriticalAssumption:
			self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiCriticalAssumption; break;
	}

	return AdvDiffResidualForceTerm_UpwindParam( self, pecletNumber );
}


