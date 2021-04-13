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

#include <Underworld/Function/src/FunctionIO.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>
#include <Underworld/Function/src/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/src/Function.hpp>
#include "Residual.h"

#include <assert.h>
#include <string.h>
#include <cmath>

#define ISQRT15 0.25819888974716112567
#define SUPG_MIN_DIFFUSIVITY 1.0e-20

/* Textual name of this class */
Type AdvDiffResidualForceTerm_Type = (char*) "AdvDiffResidualForceTerm";

AdvDiffResidualForceTerm* AdvDiffResidualForceTerm_New(
    Name                    name,
    FiniteElementContext*   context,
    ForceVector*            forceVector,
    Swarm*                  integrationSwarm,
    Stg_Component*          sle,
    FeVariable*             velocityField,
    AdvDiffResidualForceTerm_UpwindParamFuncType upwindFuncType )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*) _AdvDiffResidualForceTerm_DefaultNew( name );

    self->isConstructed = True;
    _ForceTerm_Init( self, context, forceVector, integrationSwarm, sle );
    _AdvDiffResidualForceTerm_Init( self, velocityField, upwindFuncType );

    return self;
}

void* _AdvDiffResidualForceTerm_DefaultNew( Name name )
{
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
    self->diffFn = (void*) new SUPGVectorTerm_NA__Fn_cppdata;
    self->sourceFn = (void*) new SUPGVectorTerm_NA__Fn_cppdata;

    return self;
}

void _AdvDiffResidualForceTerm_Init(
    void*       residual,
    FeVariable* velocityField,
    AdvDiffResidualForceTerm_UpwindParamFuncType	upwindFuncType ) //WHY IS THIS LINE HERE???
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    self->velocityField = velocityField;
    self->upwindParamType = upwindFuncType;
    self->last_maxNodeCount = 0;
}

void _AdvDiffResidualForceTerm_Delete( void* residual )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    delete (SUPGVectorTerm_NA__Fn_cppdata*)self->diffFn;
    delete (SUPGVectorTerm_NA__Fn_cppdata*)self->sourceFn;

    _ForceTerm_Delete( self );
}

void _AdvDiffResidualForceTerm_Print( void* residual, Stream* stream )
{
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

}

void _AdvDiffResidualForceTerm_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data )
{
    AdvDiffResidualForceTerm*   self = (AdvDiffResidualForceTerm*)residual;
    FeVariable*                 velocityField;
    Name                        upwindParamFuncName;
    AdvDiffResidualForceTerm_UpwindParamFuncType	upwindFuncType;

    /* Construct Parent */
    _ForceTerm_AssignFromXML( self, cf, data );

    velocityField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityField", FeVariable, True, data  );
    upwindParamFuncName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"UpwindXiFunction", "Exact"  );

    if ( strcasecmp( upwindParamFuncName, "DoublyAsymptoticAssumption" ) == 0 )
        upwindFuncType = DoublyAsymptoticAssumption;
    else if ( strcasecmp( upwindParamFuncName, "CriticalAssumption" ) == 0 )
        upwindFuncType = CriticalAssumption;
    else if ( strcasecmp( upwindParamFuncName, "Exact" ) == 0 )
        upwindFuncType = Exact;
    else
        Journal_Firewall( False, Journal_Register( Error_Type, (Name)self->type  ), (char*) "Cannot understand '%s'\n", upwindParamFuncName );


    _AdvDiffResidualForceTerm_Init( self, velocityField, upwindFuncType );
}

void _AdvDiffResidualForceTerm_Build( void* residual, void* data )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    _ForceTerm_Build( self, data );

    Stg_Component_Build( self->velocityField, data, False );

}
void _AdvDiffResidualForceTerm_Allocate( AdvDiffResidualForceTerm* self, int dim, int max_elementNodeCount ) {
  if (self->last_maxNodeCount < max_elementNodeCount) {
    _AdvDiffResidualForceTerm_FreeLocalMemory( self );
  }
  self->GNx = Memory_Alloc_2DArray( double, dim, max_elementNodeCount, (Name)(char*)"(SUPG): Global Shape Function Derivatives" );
  self->phiGrad = Memory_Alloc_Array(double, dim, (char*)"(SUPG): Gradient of the Advected Scalar");
  self->Ni = Memory_Alloc_Array(double, max_elementNodeCount, (char*)"(SUPG): Gradient of the Advected Scalar");
  self->SUPGNi = Memory_Alloc_Array(double, max_elementNodeCount, (char*)"(SUPG): Upwinded Shape Function");
  self->incarray=IArray_New();

  self->last_maxNodeCount = max_elementNodeCount;
}
void _AdvDiffResidualForceTerm_FreeLocalMemory( AdvDiffResidualForceTerm* self ) {

  if( self->last_maxNodeCount == 0 ) return;

  Memory_Free(self->GNx);
  Memory_Free(self->phiGrad);
  Memory_Free(self->Ni);
  Memory_Free(self->SUPGNi);

  Stg_Class_Delete(self->incarray);
  self->last_maxNodeCount = 0;
}

void _AdvDiffResidualForceTerm_Initialise( void* residual, void* data )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    _ForceTerm_Initialise( self, data );

    Stg_Component_Initialise( self->velocityField, data, False );
}

void _AdvDiffResidualForceTerm_Execute( void* residual, void* data )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    _ForceTerm_Execute( self, data );
}

void _AdvDiffResidualForceTerm_Destroy( void* residual, void* data )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    _AdvDiffResidualForceTerm_FreeLocalMemory( self );
    _ForceTerm_Destroy( self, data );
}

void _AdvDiffResidualForceTerm_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elementResidual )
{
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
    double                     diffusivity         = NAN;
    double                     source              = NAN;
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

    SUPGVectorTerm_NA__Fn_cppdata* diffFn = (SUPGVectorTerm_NA__Fn_cppdata*)(self->diffFn);
    SUPGVectorTerm_NA__Fn_cppdata* sourceFn = (SUPGVectorTerm_NA__Fn_cppdata*)(self->sourceFn);

    debug_dynamic_cast<ParticleInCellCoordinate*>(diffFn->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
    debug_dynamic_cast<ParticleInCellCoordinate*>(sourceFn->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
    diffFn->input->index()   = lElement_I;  // set the elementId for the fem coordinate
    sourceFn->input->index()   = lElement_I;  // set the elementId for the fem coordinate

    GNx     = self->GNx;
    phiGrad = self->phiGrad;
    Ni      = self->Ni;
    SUPGNi  = self->SUPGNi;

    upwindDiffusivity  = AdvDiffResidualForceTerm_UpwindDiffusivity( self, sle, (void*)diffFn, swarm, phiField->feMesh, lElement_I, dim );

    /* Determine number of particles in element */
    cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
    cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

    for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
        debug_dynamic_cast<ParticleInCellCoordinate*>(diffFn->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
        debug_dynamic_cast<ParticleInCellCoordinate*>(sourceFn->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
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

        /* evaluate function for diffusivity */
        const IO_double* funcdiff = debug_dynamic_cast<const IO_double*>(diffFn->func(diffFn->input.get()));
        const IO_double* funcsource = debug_dynamic_cast<const IO_double*>(sourceFn->func(sourceFn->input.get()));
        diffusivity = funcdiff->at();
        source = funcsource->at();
        assert( !std::isnan(diffusivity) );
        assert( !std::isnan(source) );
        totalDerivative -= source;  // as per first term in Eq. 3.2.18

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

void _SUPGVectorTerm_NA__Fn_SetDiffusivityFn( void* _self, Fn::Function* fn ){
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)_self;

    // record fn to struct
    SUPGVectorTerm_NA__Fn_cppdata* diffFn = (SUPGVectorTerm_NA__Fn_cppdata*) self->diffFn;
    diffFn->fn = fn;

    if( !self->forceVector )
        throw std::invalid_argument( "AdvDiffResidualForceTerm does not appear to have 'ForceVector' set. Please contact developers" );

    FeMesh* mesh = self->forceVector->feVariable->feMesh;
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;

    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    diffFn->input = std::make_shared<FEMCoordinate>((void*)mesh, localCoord);
    diffFn->func = fn->getFunction(diffFn->input.get());

    // check output conforms
    const FunctionIO* sampleguy = diffFn->func(diffFn->input.get());
    const IO_double* iodub = dynamic_cast<const IO_double*>(sampleguy);
    if( !iodub )
        throw std::invalid_argument( "Assembly term expects functions to return 'double' type values." );
    if( iodub->size() != self->forceVector->feVariable->fieldComponentCount ){
        std::stringstream ss;
        ss << "Assembly term expects function to return array of size " << self->forceVector->feVariable->fieldComponentCount << ".\n";
        ss << "Provided function returns array of size " << iodub->size() << ".";
        throw std::invalid_argument( ss.str() );
    }
}

void _SUPGVectorTerm_NA__Fn_SetSourceFn( void* _self, Fn::Function* fn ){
    /* Set the source term 'function' */
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)_self;

    // record fn to struct
    SUPGVectorTerm_NA__Fn_cppdata* sourceFn = (SUPGVectorTerm_NA__Fn_cppdata*) self->sourceFn;
    sourceFn->fn = fn;

    if( !self->forceVector )
        throw std::invalid_argument( "AdvDiffResidualForceTerm does not appear to have 'ForceVector' set. Please contact developers" );

    FeMesh* mesh = self->forceVector->feVariable->feMesh;
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;

    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    sourceFn->input = std::make_shared<FEMCoordinate>((void*)mesh, localCoord);
    sourceFn->func = fn->getFunction(sourceFn->input.get());

    // check output conforms
    const FunctionIO* sampleguy = sourceFn->func(sourceFn->input.get());
    const IO_double* iodub = dynamic_cast<const IO_double*>(sampleguy);
    if( !iodub )
        throw std::invalid_argument( "Assembly term expects functions to return 'double' type values." );
    if( iodub->size() != self->forceVector->feVariable->fieldComponentCount ){
        std::stringstream ss;
        ss << "Assembly term expects function to return array of size " << self->forceVector->feVariable->fieldComponentCount << ".\n";
        ss << "Provided function returns array of size " << iodub->size() << ".";
        throw std::invalid_argument( ss.str() );
    }
}
/* Virtual Function Implementations */
double _AdvDiffResidualForceTerm_UpwindParam( void* residual, double pecletNumber )
{
    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;

    switch ( self->upwindParamType ) {
    case Exact:
        self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiExact;
        break;
    case DoublyAsymptoticAssumption:
        self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption;
        break;
    case CriticalAssumption:
        self->_upwindParam = AdvDiffResidualForceTerm_UpwindXiCriticalAssumption;
        break;
    }

    return AdvDiffResidualForceTerm_UpwindParam( self, pecletNumber );
}


/** AdvectionDiffusion_UpwindDiffusivity - See Brooks, Hughes 1982 Section 3.3
 * All equations refer to this paper if not otherwise indicated */
double AdvDiffResidualForceTerm_UpwindDiffusivity(
		AdvDiffResidualForceTerm* self,
		AdvectionDiffusionSLE* sle,
                void* _cppdata,
		Swarm* swarm,
		FeMesh* mesh,
		Element_LocalIndex lElement_I,
		Dimension_Index dim )
{
	FeVariable*                velocityField   = self->velocityField;
	Coord                      xiElementCentre = {0.0,0.0,0.0};
	double                     xiUpwind;
	double                     velocityCentre[3];
	double                     pecletNumber;
	double                     lengthScale;
	double                     upwindDiffusivity;
	Dimension_Index            dim_I;
	double*                    leastCoord;
	double*                    greatestCoord;
	Node_LocalIndex            nodeIndex_LeastValues, nodeIndex_GreatestValues;
	int             *inc;
	IArray*         incArray;

	Cell_Index                 cell_I;
	double                     averageDiffusivity;
	Particle_InCellIndex       cParticle_I;
	Particle_InCellIndex       particleCount;


        SUPGVectorTerm_NA__Fn_cppdata* diffFn = (SUPGVectorTerm_NA__Fn_cppdata*)_cppdata;
	/* Compute the average diffusivity */
	/* Find Number of Particles in Element */
	cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
	particleCount = swarm->cellParticleCountTbl[ cell_I ];

	/* Average diffusivity for element */
        averageDiffusivity = 0.0;
        for ( cParticle_I = 0 ; cParticle_I < particleCount ; cParticle_I++ ) {
                debug_dynamic_cast<ParticleInCellCoordinate*>(diffFn->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
                /* evaluate function for diffusivity */
                const IO_double* funcout = debug_dynamic_cast<const IO_double*>(diffFn->func(diffFn->input.get()));
                averageDiffusivity += funcout->at();
        }
        averageDiffusivity /= (double)particleCount;


	if (sle->maxDiffusivity < averageDiffusivity)
		sle->maxDiffusivity = averageDiffusivity;

	/* Change Diffusivity if it is too small */
	if ( averageDiffusivity < SUPG_MIN_DIFFUSIVITY )
		averageDiffusivity = SUPG_MIN_DIFFUSIVITY;

	/* Calculate Velocity At Middle of Element - See Eq. 3.3.6 */
	FeVariable_InterpolateFromMeshLocalCoord( velocityField, mesh, lElement_I, xiElementCentre, velocityCentre );

	/* Calculate Length Scales - See Fig 3.4 - ASSUMES BOX MESH TODO - fix */
	incArray = self->incarray;
	FeMesh_GetElementNodes( mesh, lElement_I, incArray );
	inc = IArray_GetPtr( incArray );

	nodeIndex_LeastValues = inc[0];
	nodeIndex_GreatestValues = (dim == 2) ? inc[3] : (dim == 3) ? inc[7] : inc[1];
	leastCoord    = Mesh_GetVertex( mesh, nodeIndex_LeastValues );
	greatestCoord = Mesh_GetVertex( mesh, nodeIndex_GreatestValues );

	upwindDiffusivity = 0.0;
	for ( dim_I = 0 ; dim_I < dim ; dim_I++ ) {
		lengthScale = fabs(greatestCoord[ dim_I ] - leastCoord[ dim_I ]);

		/* Calculate Peclet Number (alpha) - See Eq. 3.3.5 */
		pecletNumber = velocityCentre[ dim_I ] * lengthScale / (2.0 * averageDiffusivity);

		/* Calculate Upwind Local Coordinate - See Eq. 3.3.4 and (2.4.2, 3.3.1 and 3.3.2) */
		xiUpwind = AdvDiffResidualForceTerm_UpwindParam( self, pecletNumber );

		/* Calculate Upwind Thermal Diffusivity - See Eq. 3.3.3  */
		upwindDiffusivity += xiUpwind * velocityCentre[ dim_I ] * lengthScale;
	}
	upwindDiffusivity *= ISQRT15;         /* See Eq. 3.3.11 */


	return upwindDiffusivity;
}

double AdvDiffResidualForceTerm_GetMaxDiffusivity( void* residual ) {
    /*
     * Used to evaluate the maximum diffusivity on the local processor
     *
     * As diffusivity has been 'Functionasized' I have written this seemingly isolated function
     * in this file.
     */

    AdvDiffResidualForceTerm* self = (AdvDiffResidualForceTerm*)residual;
    Swarm* swarm = self->integrationSwarm;
    int e_i, cParticle_I, particleCount, cell_I, nEls;
    double maxDiffusivity = -1;

    SUPGVectorTerm_NA__Fn_cppdata* diffFn = (SUPGVectorTerm_NA__Fn_cppdata*)self->diffFn;

    nEls = FeMesh_GetElementLocalSize( self->forceVector->feVariable->feMesh );

    for( e_i = 0 ; e_i < nEls ; e_i++ ) {

        cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, e_i );
        particleCount = swarm->cellParticleCountTbl[ cell_I ];

        // setup diffFn to run
        debug_dynamic_cast<ParticleInCellCoordinate*>(diffFn->input->localCoord())->index() = e_i;  // set the elementId as the owning cell for the particleCoord
        diffFn->input->index()   = e_i;

        for ( cParticle_I = 0 ; cParticle_I < particleCount ; cParticle_I++ ) {
            debug_dynamic_cast<ParticleInCellCoordinate*>(diffFn->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
            // evaluate function for diffusivity
            const IO_double* funcout = debug_dynamic_cast<const IO_double*>(diffFn->func(diffFn->input.get()));
            if ( funcout->at() > maxDiffusivity ) {
                maxDiffusivity = funcout->at();
            };
        }
    }

    return maxDiffusivity;

}



/** AdvectionDiffusion_UpwindXiExact - Brooks, Hughes 1982 equation 2.4.2
 *\f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ */
double AdvDiffResidualForceTerm_UpwindXiExact( void* residual, double pecletNumber ) {
	if (fabs(pecletNumber) < 1.0e-8 )
		return 0.33333333333333 * pecletNumber;
	else if (pecletNumber < -20.0)
		return -1.0 - 1.0/pecletNumber;
	else if (pecletNumber > 20.0)
		return +1.0 - 1.0/pecletNumber;

	return cosh( pecletNumber )/sinh( pecletNumber ) - 1.0/pecletNumber;
}

/** AdvectionDiffusion_UpwindXiDoublyAsymptoticAssumption - Brooks, Hughes 1982 equation 3.3.1
 * Simplification of \f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ from Brooks, Hughes 1982 equation 2.4.2
 * \f[
\bar \xi \sim \left\{ \begin{array}{rl}
             -1                 &for \quad \alpha <= -3 \\
             \frac{\alpha}{3}   &for \quad -3 < \alpha <= 3 \\
             +1                 &for \quad \alpha > +3
             \end{array} \right.

\f]*/
double AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption( void* residual, double pecletNumber ) {
	if (pecletNumber <= -3.0)
		return -1;
	else if (pecletNumber <= 3.0)
		return 0.33333333333333 * pecletNumber;
	else
		return 1.0;
}

/** AdvectionDiffusion_UpwindXiCriticalAssumption - Brooks, Hughes 1982 equation 3.3.2
 * Simplification of \f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ from Brooks, Hughes 1982 equation 2.4.2
 * \f[
  \bar \xi \sim \left\{ \begin{array}{rl}
              -1 - \frac{1}{\alpha}   &for \quad \alpha <= -1 \\
               0                      &for \quad -1 < \alpha <= +1 \\
              +1 - \frac{1}{\alpha}   &for \quad \alpha > +1
              \end{array} \right.
\f]    */

double AdvDiffResidualForceTerm_UpwindXiCriticalAssumption( void* residual, double pecletNumber ) {
	if (pecletNumber <= -1.0)
		return -1.0 - 1.0/pecletNumber;
	else if (pecletNumber <= 1.0)
		return 0.0;
	else
		return 1.0 - 1.0/pecletNumber;
}
