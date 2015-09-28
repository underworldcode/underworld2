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
#include <Underworld/Underworld.h>

#include "types.h"
#include "RBFManager.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type RBFManager_Type = (const Type)"RBFManager";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

RBFManager* _RBFManager_New(  RBFMANAGER_DEFARGS  )
{
   RBFManager* self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(RBFManager) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */

   self = (RBFManager*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );


   /* General info */

   /* Virtual Info */

   return self;
}

void _RBFManager_Init(
   void*                  _self,
   PICelleratorContext*   context,
   Dimension_Index        RBFdim,
   double                 particleSupportRadius,
   Swarm*                 rbfParticleSwarm )
{
   RBFManager* self = (RBFManager*)_self;

   self->context               = context;
   self->particleMass          = 1.;
   self->RBFdim                = RBFdim;
   self->particleSupportRadius = particleSupportRadius;
   self->rbfParticleSwarm      = rbfParticleSwarm;
}


/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _RBFManager_Delete( void* _self ) {
   RBFManager* self = (RBFManager*)_self;
   _Stg_Component_Delete( self );
}

void  _RBFManager_Print( void* RBFManager, Stream* stream ) { assert(0); }
void* _RBFManager_Copy( void* RBFManager, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) { assert(0); }

void* _RBFManager_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                _sizeOfSelf = sizeof(RBFManager);
   Type                                                        type = RBFManager_Type;
   Stg_Class_DeleteFunction*                                _delete = _RBFManager_Delete;
   Stg_Class_PrintFunction*                                  _print = _RBFManager_Print;
   Stg_Class_CopyFunction*                                    _copy = _RBFManager_Copy;
   Stg_Component_DefaultConstructorFunction*    _defaultConstructor = _RBFManager_DefaultNew;
   Stg_Component_ConstructFunction*                      _construct = _RBFManager_AssignFromXML;
   Stg_Component_BuildFunction*                              _build = _RBFManager_Build;
   Stg_Component_InitialiseFunction*                    _initialise = _RBFManager_Initialise;
   Stg_Component_ExecuteFunction*                          _execute = _RBFManager_Execute;
   Stg_Component_DestroyFunction*                          _destroy = _RBFManager_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _RBFManager_New(  RBFMANAGER_PASSARGS  );
}


void _RBFManager_AssignFromXML( void* _self, Stg_ComponentFactory* cf, void* data ) {
   RBFManager*          self          = (RBFManager*) _self;
   PICelleratorContext* context;
   Dimension_Index      RBFdim;
   double               particleSupportRadius;
   Swarm*               rbfParticleSwarm;

   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", PICelleratorContext, False, data );
   if( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", PICelleratorContext, True, data  );

   /** RBF swarm dimensionality.  a 2d swarm can live within a 3d space */
   RBFdim             = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"RBFdim", 2  );
   /** particleSupportRadius is the RBF particle support radius */
   particleSupportRadius = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"ParticleSupportRadius", 0.  );
   /** of course we need a swarm of particles, the rbf swarm */
   rbfParticleSwarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"RBFSwarm", Swarm, True, data );

   _RBFManager_Init( self, context, RBFdim, particleSupportRadius, rbfParticleSwarm );
}

void _RBFManager_Build( void* _self, void* data ) {
   RBFManager*                     self = (RBFManager*) _self;
   RBFParticle                   RBFParticle;

   if(self->RBFdim < self->context->dim)
      self->particleCoordVariable = Swarm_NewVectorVariable( self->rbfParticleSwarm, (Name)"Position", GetOffsetOfMember( RBFParticle, coord ),
         Variable_DataType_Double,
         self->RBFdim,
         "PositionX",
         "PositionZ" );
   else
      self->particleCoordVariable = Swarm_NewVectorVariable( self->rbfParticleSwarm, (Name)"Position", GetOffsetOfMember( RBFParticle, coord ),
         Variable_DataType_Double,
         self->RBFdim,
         "PositionX",
         "PositionY",
         "PositionZ" );
   self->particleDensity       = Swarm_NewScalarVariable( self->rbfParticleSwarm, (Name)"RBFDensity"  , GetOffsetOfMember( RBFParticle, density ),
      Variable_DataType_Double);
   /** disable checkpointing of particle density, as this will be recalculated as required */
   self->particleDensity->isCheckpointedAndReloaded = False;
   Stg_Component_Build( self->rbfParticleSwarm     , NULL, False );
   Stg_Component_Build( self->particleCoordVariable, NULL, False );
   Stg_Component_Build( self->particleDensity      , NULL, False );

}

void _RBFManager_Initialise( void* _self, void* data ) {
   RBFManager* self = (RBFManager*) _self;

   Stg_Component_Initialise( self->rbfParticleSwarm      , NULL, False );
   Stg_Component_Initialise( self->particleCoordVariable , NULL, False );
   Stg_Component_Initialise( self->particleDensity       , NULL, False );

   Variable_Update( self->particleCoordVariable->variable );
   Variable_Update( self->particleDensity->variable );

   /** initialise particle density */
   RBFManager_CalculateParticleDensities( self );
   self->particleDensity->isCheckpointedAndReloaded = False;
   
}

void _RBFManager_Execute( void* _self, void* data ) {}
void _RBFManager_Destroy( void* _self, void* data ) {}

/*-------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


/*---------------------------------------------------------------------------------------------------------------------
** Entry Point Hooks
*/

/*-------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void RBFManager_CalculateParticleDensities( void* _self ){
   RBFManager*        self     = (RBFManager*) _self;
   Swarm*           swarm    = (Swarm*) self->rbfParticleSwarm;
   RBFParticle*     oParticle = NULL;
   RBFParticle*     iParticle = NULL;
   RBFParticle*     particle  = NULL;
   Index            cell_I, lParticle_I, oParticle_I, iParticle_I;
   Index            nCell_I;
   double           distanceBetweenParticles;
   NeighbourIndex   neighbourCount;
   NeighbourIndex   neighbour_I;
   NeighbourIndex*  neighbourList;
   double           h = self->particleSupportRadius/2.;    /** our smoothing length, h */
   double           maxSepSquared = 4*h*h;
   double           kernelResult;
   IArray*          incFaces = IArray_New();

   /** init all densities to zero */
   for( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount ; lParticle_I++ ){
         particle = (RBFParticle*)Swarm_ParticleAt( swarm, lParticle_I );
         particle->density = 0;
   }

   /** scan through cells summing up particle densities */

   for( cell_I = 0; cell_I < swarm->cellDomainCount; cell_I++ ) {

      /** Find all neighbours to cell_I - this Assumes ElementCellLayout */
      Mesh_GetIncidence( ((ElementCellLayout*)swarm->cellLayout)->mesh, self->RBFdim, cell_I, self->RBFdim, incFaces );
      neighbourCount = IArray_GetSize( incFaces );
      neighbourList  = IArray_GetPtr( incFaces );

      /** first sum all contributions for particles within the current cell */

      /** get outer loop particle */
      for( oParticle_I = 0 ; oParticle_I < swarm->cellParticleCountTbl[cell_I] ; oParticle_I++ ) {
         oParticle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, cell_I, oParticle_I );

         /** add particles contribution to itself */
         kernelResult = ( self->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( 0, h) : RBFManager_3D_CubicSplineKernel( 0, h) );
         oParticle->density += self->particleMass*kernelResult;

         /** get inner loop particle */
         for( iParticle_I = oParticle_I+1 ; iParticle_I < swarm->cellParticleCountTbl[cell_I] ; iParticle_I++ ) {
            iParticle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, cell_I, iParticle_I );

            /** Calculate distance to particle */
            distanceBetweenParticles     = pow((iParticle->coord[ I_AXIS ] - oParticle->coord[ I_AXIS ]), 2) +
                                           pow((iParticle->coord[ J_AXIS ] - oParticle->coord[ J_AXIS ]), 2) ;

            if (self->RBFdim == 3)
               distanceBetweenParticles += pow((iParticle->coord[ K_AXIS ] - oParticle->coord[ K_AXIS ]), 2) ;

            if ( distanceBetweenParticles < maxSepSquared ) {
               distanceBetweenParticles = sqrt(distanceBetweenParticles);
               kernelResult = ( self->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceBetweenParticles, h) : RBFManager_3D_CubicSplineKernel( distanceBetweenParticles, h) );
               oParticle->density += self->particleMass*kernelResult;
               iParticle->density += self->particleMass*kernelResult;
            }
         }
      }



      /** now sum contributions from surrounding cells */
      for ( neighbour_I = 0 ; neighbour_I < neighbourCount ; neighbour_I++ ) {
         nCell_I = neighbourList[ neighbour_I ];

         if( nCell_I < swarm->cellDomainCount ) {
            /** get outer loop particle */
            for( oParticle_I = 0 ; oParticle_I < swarm->cellParticleCountTbl[cell_I] ; oParticle_I++ ) {
               oParticle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, cell_I, oParticle_I );

               /** get inner loop particle */
               for( iParticle_I = 0 ; iParticle_I < swarm->cellParticleCountTbl[nCell_I] ; iParticle_I++ ) {
                  iParticle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, nCell_I, iParticle_I );

               /** Calculate distance to particle */
               distanceBetweenParticles     = pow((iParticle->coord[ I_AXIS ] - oParticle->coord[ I_AXIS ]), 2) +
                                              pow((iParticle->coord[ J_AXIS ] - oParticle->coord[ J_AXIS ]), 2) ;

               if (self->RBFdim == 3)
                  distanceBetweenParticles += pow((iParticle->coord[ K_AXIS ] - oParticle->coord[ K_AXIS ]), 2) ;

                  if ( distanceBetweenParticles < maxSepSquared ) {
                     distanceBetweenParticles = sqrt(distanceBetweenParticles);
                     kernelResult = ( self->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceBetweenParticles, h) : RBFManager_3D_CubicSplineKernel( distanceBetweenParticles, h) );
                     oParticle->density += self->particleMass*kernelResult;
                  }
               }
            }
         }
      }
   }

   Stg_Class_Delete( incFaces );
}

double RBFManager_2D_CubicSplineKernel( double distance, double h ) {
   double q;
   double kappa = 10/(7*M_PI);
   double partResult;

   q = distance/h;

   if ( q < 1 ){
      partResult = 1 - 1.5*q*q + 0.75*q*q*q;
   } else if ( q >=1 && q < 2 ) {
      partResult = 0.25*pow((2 - q), 3);
   } else
      return(0.0);

   return (kappa/(h*h)) * partResult;
}


/* The symmetry of the Kernel means the derivative in any direction is only related to the to the
   distance projected along that direction (i.e. for Cartesian, the x, y, or z coordinate) */

double RBFManager_2D_CubicSplineDerivative( double distance, double projected_distance, double h ) {
   double p,q;
   double kappa = 10/(7*M_PI);
   double partResult;

   q = distance/h;
   p = projected_distance / (h*h);

   if ( q < 1 ){
      partResult = 0.75 * p * (  3 * q - 4.0 );
   } else if ( q >=1 && q < 2 ) {
      partResult = -0.75 * p * (2-q)*(2-q) / q; 
   } else
      return(0.0);

   return (kappa/(h*h)) * partResult;
}

double RBFManager_3D_CubicSplineKernel( double distance, double h ) {
   double q;
   double kappa = 1/M_PI;
   double partResult;

   q = distance/h;

   if ( q < 1 ){
      partResult = 1 - 1.5*q*q + 0.75*q*q*q;
   } else if ( q >=1 && q < 2 ) {
      partResult = 0.25*pow((2 - q), 3);
   } else
      return(0.0);

   return (kappa/(h*h*h)) * partResult;
}

double RBFManager_3D_CubicSplineDerivative( double distance, double projected_distance, double h ) {
   double p,q;
   double kappa = 1/M_PI;
   double partResult;

   q = distance/h;
   p = projected_distance / (h*h);

   if ( q < 1 ){
      partResult = 0.75 * p * (  3 * q - 4.0 );

   } else if ( q >=1 && q < 2 ) {
      partResult = -0.75 * p * (2-q)*(2-q) / q; 
   } else
      return(0.0);

   return (kappa/(h*h)) * partResult;
}






