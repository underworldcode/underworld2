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

#include "types.h"
#include "RBFManager.h"
#include "RBFFieldVariable.h"

#include <assert.h>
#include <string.h>

const Type RBFFieldVariable_Type = (const Type)"RBFFieldVariable";


void* _RBFFieldVariable_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                      _sizeOfSelf = sizeof(RBFFieldVariable);
   Type                                                              type = RBFFieldVariable_Type;
   Stg_Class_DeleteFunction*                                      _delete = _FieldVariable_Delete;
   Stg_Class_PrintFunction*                                        _print = _FieldVariable_Print;
   Stg_Class_CopyFunction*                                          _copy = _FieldVariable_Copy;
   Stg_Component_DefaultConstructorFunction*          _defaultConstructor = _RBFFieldVariable_DefaultNew;
   Stg_Component_ConstructFunction*                            _construct = _RBFFieldVariable_AssignFromXML;
   Stg_Component_BuildFunction*                                    _build = _RBFFieldVariable_Build;
   Stg_Component_InitialiseFunction*                          _initialise = _RBFFieldVariable_Initialise;
   Stg_Component_ExecuteFunction*                                _execute = _FieldVariable_Execute;
   Stg_Component_DestroyFunction*                                _destroy = _RBFFieldVariable_Destroy;
   AllocationType                                      nameAllocationType = NON_GLOBAL;
   FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt = _RBFFieldVariable_InterpolateValueAt;
   FieldVariable_GetValueFunction*            _getMinGlobalFieldMagnitude = FieldVariable_GetMinGlobalFieldMagnitude;
   FieldVariable_GetValueFunction*            _getMaxGlobalFieldMagnitude = FieldVariable_GetMaxGlobalFieldMagnitude;
   FieldVariable_CacheValuesFunction*    _cacheMinMaxGlobalFieldMagnitude = _RBFFieldVariable_CacheMinMaxGlobalFieldMagnitude;
   FieldVariable_GetCoordFunction*               _getMinAndMaxLocalCoords = FieldVariable_GetMinAndMaxLocalCoords;
   FieldVariable_GetCoordFunction*              _getMinAndMaxGlobalCoords = FieldVariable_GetMinAndMaxGlobalCoords;

   return _RBFFieldVariable_New(  RBFFIELDVARIABLE_PASSARGS  );
}

RBFFieldVariable* _RBFFieldVariable_New(  RBFFIELDVARIABLE_DEFARGS  ) {
   RBFFieldVariable* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(RBFFieldVariable) );
   self = (RBFFieldVariable*)_FieldVariable_New(  FIELDVARIABLE_PASSARGS  );

   /* Virtual functions */

   /* General info */

   /* RBFFieldVariable info */

   return self;
}

void _RBFFieldVariable_Init(
   RBFFieldVariable*          self,
   RBFManager*                rbfManager,
   Bool                       useShepardCorrection,
   double                     defaultValue,
   double                     offset,
   double                     shepardCorrectionThreshold,
   SwarmVariable*             swarmVariable ) {

   self->rbfManager                 = rbfManager;
   self->useShepardCorrection       = useShepardCorrection;
   self->defaultValue               = defaultValue;
   self->offset                     = offset;
   self->shepardCorrectionThreshold = shepardCorrectionThreshold;
   self->swarmVariable              = swarmVariable;
   
   /** reset below according to RBFManager config */
   self->dim                 = rbfManager->RBFdim;
   self->fieldComponentCount = 1;

   if(self->swarmVariable){
      self->fieldComponentCount = swarmVariable->dofCount;
   } else {
      /** disable shepard correction here */
      self->useShepardCorrection = False;
   }
   /** force RBFfieldvariable to use cached values */
   self->useCacheMaxMin      = True;

}

void _RBFFieldVariable_AssignFromXML( void* _RBFFieldVariable, Stg_ComponentFactory* cf, void* data ) {
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;
   RBFManager*         rbfManager;
   Bool              useShepardCorrection;
   double            defaultValue;
   double            offset;
   double            shepardCorrectionThreshold;
   SwarmVariable*    swarmVariable = NULL;

   _FieldVariable_AssignFromXML( self, cf, data );

   /** required rbfManager for which we will interpolate values from */
   rbfManager                 = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"RBFManager", RBFManager, True, data  );
   /** shepard correction will improve results near swarm boundaries and in flat regions */
   useShepardCorrection       = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"UseShepardCorrection", True  );
   /** this allows for the definition of a default value where interpolating in regions where no particles exist. */
   /** note that the default value ignores any user defined offset. */
   defaultValue               = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"DefaultValue", 0.0);
   /** a user provided offset to the interpolated value */
   offset                     = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"Offset", 0.0);
   /** this threshold adjusts when to use the correction. setting it to zero means the correction will always be used.       */
   /** it shouldn't be set higher than 0.9, as this may given useless results.  instead you should turn the correction off.  */
   shepardCorrectionThreshold = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"ShepardCorrectionThreshold", 0.25);
   /** the rbf swarmvariable that we would like to evaluate at some location. */
   /** note that if no variable is provide, we will simply return the evaluation of the kernels */
   /** this will give a value of 1 where we have a complete complement of particles (ie, we are within the swarm. */
   /** values less than 1 will be returned otherwise, with a value 0 where the evaluation is not within particleSupportRadius of any particles */
   swarmVariable              = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"SwarmVariable", SwarmVariable, False, data );

   Journal_Firewall( shepardCorrectionThreshold <= 0.9,
        Journal_MyStream( Error_Type, self ),
        "\n\nError in %s for %s '%s' - 'shepardCorrectionThreshold' is %g.  It shouldn't be set higher than 0.9, as this may give useless results.\n"  
        "Instead you should turn the correction off using the 'useShepardCorrection' flag.\n\n\n",
        __func__,
        self->type,
        self->name,
        shepardCorrectionThreshold );

   _RBFFieldVariable_Init( self, rbfManager, useShepardCorrection, defaultValue, offset, shepardCorrectionThreshold, swarmVariable );

}

void _RBFFieldVariable_Build( void* _RBFFieldVariable, void* data ) {
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;
   
   Stg_Component_Build( self->rbfManager, data, False );
   if(self->swarmVariable) Stg_Component_Build( self->swarmVariable, data, False );
   _FieldVariable_Build( self, data );

}

void _RBFFieldVariable_Initialise( void* _RBFFieldVariable, void* data ) {
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;
   
   Stg_Component_Initialise( self->rbfManager, NULL, False );
   if(self->swarmVariable) Stg_Component_Initialise( self->swarmVariable, NULL, False );
   _FieldVariable_Initialise( self, data );
   
   /** do these now to prevent potential hung processes */
   self->_getMinGlobalFieldMagnitude( _RBFFieldVariable );
   self->_getMaxGlobalFieldMagnitude( _RBFFieldVariable );

}

void _RBFFieldVariable_Execute( void* RBFFieldVariable, void* data ) {

}

void _RBFFieldVariable_Destroy( void* RBFFieldVariable, void* data ) {
}

InterpolationResult _RBFFieldVariable_InterpolateValueAt( void* _RBFFieldVariable, double* coord, double* value ){
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;   
   RBFManager*       rbfManager     = self->rbfManager;
   Swarm*            swarm        = (Swarm*) rbfManager->rbfParticleSwarm;
   RBFParticle*      particle     = NULL;
   Index             lCell_I;
   Index             particle_I;
   Index             nCell_I;
   Index             numSumParticles = 0;
   double            distanceToParticle;
   NeighbourIndex    neighbourCount;
   NeighbourIndex    neighbour_I;
   NeighbourIndex*   neighbourList;
   double            h = self->rbfManager->particleSupportRadius/2.;    /** our smoothing length, h */
   double            maxSepSquared = 4*h*h;
   double            kernelResult;
   IArray*           incFaces = IArray_New();
	GlobalParticle    testParticle;
	double            shepardCorrection = 0;
	double            tempVal;
   unsigned          ii;
   double*           varValue;
   Coord             localCoord;

   /** if we are a 2d surface embedded in 3d space, switch axis accordingly */
   /** note that this is not general! TODO: fix */
   localCoord[0] = coord[I_AXIS];
   if( self->context->dim > self->rbfManager->RBFdim ){
      localCoord[1] = coord[K_AXIS];
   } else {
      localCoord[1] = coord[J_AXIS];
      if(self->rbfManager->RBFdim == 3)
         localCoord[2] = coord[K_AXIS];      
   }
	
	/** init value */
	value[0] = 0;
   for(ii=1; ii<self->fieldComponentCount; ii++)
      value[ii] = 0;

   if(self->swarmVariable)
      varValue = Memory_Alloc_Array( double, self->swarmVariable->dofCount, "swarmvarvalue" );

   /** first find out which cell the coord is  in.  copy it to a test particle which we will then attempt to find an owning cell */
	memcpy( testParticle.coord, localCoord, sizeof(Coord) );
	/** First specify the particle doesn't have an owning cell yet, so as
	not to confuse the search algorithm */
	testParticle.owningCell = swarm->cellDomainCount;
	lCell_I = CellLayout_CellOf( swarm->cellLayout, &testParticle );

	/** Test if this cell is on this processor - if not then bail */
	if (lCell_I >= swarm->cellLocalCount)
		return OTHER_PROC;

   /** scan through cells summing up particle contributions */

   /** Find all neighbours to lCell_I - this Assumes ElementCellLayout */
   Mesh_GetIncidence( ((ElementCellLayout*)swarm->cellLayout)->mesh, rbfManager->RBFdim, lCell_I, rbfManager->RBFdim, incFaces );
   neighbourCount = IArray_GetSize( incFaces );
   neighbourList  = IArray_GetPtr( incFaces );                                         

   /** first sum all contributions for particles within the current cell */
   for( particle_I = 0 ; particle_I < swarm->cellParticleCountTbl[lCell_I] ; particle_I++ ) {
      particle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, lCell_I, particle_I );
      
      /** Calculate distance to particle */
      distanceToParticle     = pow((particle->coord[ I_AXIS ] - localCoord[ I_AXIS ]), 2) + 
                               pow((particle->coord[ J_AXIS ] - localCoord[ J_AXIS ]), 2) ; 

      if ( rbfManager->RBFdim == 3 )
         distanceToParticle += pow((particle->coord[ K_AXIS ] - localCoord[ K_AXIS ]), 2) ;

      
      if ( distanceToParticle < maxSepSquared ) {
         distanceToParticle = sqrt(distanceToParticle);
         kernelResult = ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceToParticle, h) : RBFManager_3D_CubicSplineKernel( distanceToParticle, h) );
         tempVal = (rbfManager->particleMass/particle->density)*kernelResult;
         if(self->swarmVariable){
            SwarmVariable_ValueAt( self->swarmVariable, Swarm_ParticleCellIDtoLocalID(swarm, lCell_I, particle_I), varValue );
            for(ii=0; ii<self->fieldComponentCount; ii++){
               value[ii] += tempVal*varValue[ii];
            }
         }
         shepardCorrection += tempVal;
         numSumParticles++;
      }
   }

   /** now sum contributions from surrounding cells */
   for ( neighbour_I = 0 ; neighbour_I < neighbourCount ; neighbour_I++ ) {
      nCell_I = neighbourList[ neighbour_I ];

      if( nCell_I < swarm->cellDomainCount ) {
         for( particle_I = 0 ; particle_I < swarm->cellParticleCountTbl[nCell_I] ; particle_I++ ) {
            particle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, nCell_I, particle_I );
            
            /** Calculate distance to particle */
            distanceToParticle     = pow((particle->coord[ I_AXIS ] - localCoord[ I_AXIS ]), 2) + 
                                     pow((particle->coord[ J_AXIS ] - localCoord[ J_AXIS ]), 2) ; 
      
            if ( rbfManager->RBFdim == 3 )
               distanceToParticle += pow((particle->coord[ K_AXIS ] - localCoord[ K_AXIS ]), 2) ;
            
            if ( distanceToParticle < maxSepSquared ) {
               distanceToParticle = sqrt(distanceToParticle);
               kernelResult = ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceToParticle, h) : RBFManager_3D_CubicSplineKernel( distanceToParticle, h) );
               tempVal = (rbfManager->particleMass/particle->density)*kernelResult;
               if(self->swarmVariable){
                  SwarmVariable_ValueAt( self->swarmVariable, Swarm_ParticleCellIDtoLocalID(swarm, nCell_I, particle_I), varValue );
                  for(ii=0; ii<self->fieldComponentCount; ii++){
                     value[ii] += tempVal*varValue[ii];
                  }
               }
               shepardCorrection += tempVal;
               numSumParticles++;
            }
         }
      }
   }

   Stg_Class_Delete( incFaces );
   if(self->swarmVariable) Memory_Free( varValue );

   /** if no swarmvariable is provided, the shepard correction value should given an indication of distance to the swarm */
   if(!self->swarmVariable) value[0] = shepardCorrection;
   /** apply shepard correction if required */
   if (self->useShepardCorrection ){
      if ( shepardCorrection > self->shepardCorrectionThreshold )
         for(ii=0; ii<self->fieldComponentCount; ii++){
            value[ii] /= shepardCorrection;
         }
      else{
         value[0] = self->defaultValue;
         return OUTSIDE_GLOBAL;
      }
   }
   
   /** add offset as defined in AssignFromXML */
   for(ii=0; ii<self->fieldComponentCount; ii++){
      value[ii] += self->offset;
   }

   if (numSumParticles > 0)
      return LOCAL;
   else {
      if(self->swarmVariable) value[0] = self->defaultValue;
      return OUTSIDE_GLOBAL;
   }
}


void _RBFFieldVariable_CacheMinMaxGlobalFieldMagnitude( void* _RBFFieldVariable ){
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;   
   RBFManager*       rbfManager     = self->rbfManager;
   Swarm*            swarm        = (Swarm*) rbfManager->rbfParticleSwarm;
   Index             lParticle_I;
   unsigned          ii;
   double		      min = HUGE_VAL;
   double		      max = -HUGE_VAL;

   if(self->swarmVariable){
      double* varValue = Memory_Alloc_Array( double, self->swarmVariable->dofCount, "swarmvarvalue" );
      for( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount ; lParticle_I++ ){
         SwarmVariable_ValueAt( self->swarmVariable, lParticle_I, varValue );
         for(ii=0; ii<self->fieldComponentCount; ii++){
            if ( varValue[ii] < min ) min = varValue[ii];
            if ( varValue[ii] > max ) max = varValue[ii];
         }
      }
      Memory_Free( varValue );
   } else {
      min = 0;
      max = 1;
   }
	/** Find upper and lower bounds on all processors */
	MPI_Allreduce( &min, &self->magnitudeMin, 1, MPI_DOUBLE, MPI_MIN, self->context->communicator );
	MPI_Allreduce( &max, &self->magnitudeMax, 1, MPI_DOUBLE, MPI_MAX, self->context->communicator );
}

/* LM: This doesn't seem to be a very stable way to obtain an accurate derivative, but it is 
       consistent with the interpolation kernels used, so, for completeness, here it is */ 


InterpolationResult  RBFFieldVariable_InterpolateGradientValueAt( void* _RBFFieldVariable, double* coord, double* value, int axis ){
   RBFFieldVariable* self = (RBFFieldVariable*)_RBFFieldVariable;   
   RBFManager*       rbfManager     = self->rbfManager;
   Swarm*            swarm        = (Swarm*) rbfManager->rbfParticleSwarm;
   RBFParticle*      particle     = NULL;
   Index             lCell_I;
   Index             particle_I;
   Index             nCell_I;
   Index             numSumParticles = 0;
   double            distanceToParticle;
   double            projectedDistance;
   NeighbourIndex    neighbourCount;
   NeighbourIndex    neighbour_I;
   NeighbourIndex*   neighbourList;
   double            h = self->rbfManager->particleSupportRadius/2.;    /** our smoothing length, h */
   double            maxSepSquared = 4*h*h;
   double            kernelResult;
   IArray*           incFaces = IArray_New();
   GlobalParticle    testParticle;
   double            shepardCorrection = 0.0;
   double            tempVal;
   unsigned          ii;
   double*           varValue;
   Coord             localCoord;

   double correction2 = 0.0;

   /** if we are a 2d surface embedded in 3d space, switch axis accordingly */
   /** note that this is not general! TODO: fix */
   localCoord[0] = coord[I_AXIS];
   if( self->context->dim > self->rbfManager->RBFdim ){
      localCoord[1] = coord[K_AXIS];
   } else {
      localCoord[1] = coord[J_AXIS];
      if(self->rbfManager->RBFdim == 3)
         localCoord[2] = coord[K_AXIS];      
   }

   
   /** init value */
   value[0] = 0;
   for(ii=1; ii<self->fieldComponentCount; ii++)
      value[ii] = 0;

   if(self->swarmVariable)
      varValue = Memory_Alloc_Array( double, self->swarmVariable->dofCount, "swarmvarvalue" );

   /** first find out which cell the coord is  in.  copy it to a test particle which we will then attempt to find an owning cell */
   memcpy( testParticle.coord, localCoord, sizeof(Coord) );
   /** First specify the particle doesn't have an owning cell yet, so as
   not to confuse the search algorithm */
   testParticle.owningCell = swarm->cellDomainCount;
   lCell_I = CellLayout_CellOf( swarm->cellLayout, &testParticle );

   /** Test if this cell is on this processor - if not then bail */
   if (lCell_I >= swarm->cellLocalCount)
      return OTHER_PROC;

   /** scan through cells summing up particle contributions */

   /** Find all neighbours to lCell_I - this Assumes ElementCellLayout */
   Mesh_GetIncidence( ((ElementCellLayout*)swarm->cellLayout)->mesh, rbfManager->RBFdim, lCell_I, rbfManager->RBFdim, incFaces );
   neighbourCount = IArray_GetSize( incFaces );
   neighbourList  = IArray_GetPtr( incFaces );                                         

   /** first sum all contributions for particles within the current cell */
   for( particle_I = 0 ; particle_I < swarm->cellParticleCountTbl[lCell_I] ; particle_I++ ) {
      particle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, lCell_I, particle_I );
      
      /** Calculate distance to particle */
      distanceToParticle     = pow((particle->coord[ I_AXIS ] - localCoord[ I_AXIS ]), 2) + 
                               pow((particle->coord[ J_AXIS ] - localCoord[ J_AXIS ]), 2) ; 

      projectedDistance      = localCoord[ axis ] - particle->coord[ axis ];


      if ( rbfManager->RBFdim == 3 )
         distanceToParticle += pow((particle->coord[ K_AXIS ] - localCoord[ K_AXIS ]), 2) ;

      
      if ( distanceToParticle < maxSepSquared ) {
         distanceToParticle = sqrt(distanceToParticle);

         kernelResult = ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceToParticle, h) : 
                                                    RBFManager_3D_CubicSplineKernel( distanceToParticle, h) );      
         correction2  += (rbfManager->particleMass/particle->density)*kernelResult;

         kernelResult = 
         ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineDerivative( distanceToParticle, projectedDistance, h) :
                                     RBFManager_3D_CubicSplineDerivative( distanceToParticle, projectedDistance, h) );

         tempVal = (rbfManager->particleMass/particle->density)*kernelResult;
         if(self->swarmVariable){
            SwarmVariable_ValueAt( self->swarmVariable, Swarm_ParticleCellIDtoLocalID(swarm, lCell_I, particle_I), varValue );
            for(ii=0; ii<self->fieldComponentCount; ii++){
               value[ii] += tempVal*varValue[ii];
            }
         }


         shepardCorrection += projectedDistance * (tempVal);

         numSumParticles++;
      }
   }

   /** now sum contributions from surrounding cells */
   for ( neighbour_I = 0 ; neighbour_I < neighbourCount ; neighbour_I++ ) {
      nCell_I = neighbourList[ neighbour_I ];

      if( nCell_I < swarm->cellDomainCount ) {
         for( particle_I = 0 ; particle_I < swarm->cellParticleCountTbl[nCell_I] ; particle_I++ ) {
            particle = (RBFParticle*)Swarm_ParticleInCellAt( swarm, nCell_I, particle_I );
            
            /** Calculate distance to particle */
            distanceToParticle     = pow((particle->coord[ I_AXIS ] - localCoord[ I_AXIS ]), 2) + 
                                     pow((particle->coord[ J_AXIS ] - localCoord[ J_AXIS ]), 2) ;                  
            if ( rbfManager->RBFdim == 3 )
               distanceToParticle += pow((particle->coord[ K_AXIS ] - localCoord[ K_AXIS ]), 2) ;
            
            if ( distanceToParticle < maxSepSquared ) {
               distanceToParticle = sqrt(distanceToParticle);
               projectedDistance      = localCoord[ axis ] - particle->coord[ axis ];

               kernelResult = ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineKernel( distanceToParticle, h) : 
                                                          RBFManager_3D_CubicSplineKernel( distanceToParticle, h) );      
               correction2 += (rbfManager->particleMass/particle->density)*kernelResult;

               kernelResult = 
                     ( rbfManager->RBFdim == 2 ? RBFManager_2D_CubicSplineDerivative( distanceToParticle, projectedDistance, h) :
                                                 RBFManager_3D_CubicSplineDerivative( distanceToParticle, projectedDistance, h) );
                       tempVal = (rbfManager->particleMass/particle->density)*kernelResult;
              
               if(self->swarmVariable){
                  SwarmVariable_ValueAt( self->swarmVariable, Swarm_ParticleCellIDtoLocalID(swarm, nCell_I, particle_I), varValue );
                  for(ii=0; ii<self->fieldComponentCount; ii++){
                     value[ii] += tempVal*varValue[ii];
                  }
               }

            shepardCorrection += projectedDistance * (tempVal) ;
            numSumParticles++;


            }
         }
      }
   }

   printf("%d particles contributed - gradient %g, %g or %g, sum %g,  %g \n", 
      numSumParticles,value[0], value[0]/correction2, value[0]/shepardCorrection,
      shepardCorrection, correction2
      );


   Stg_Class_Delete( incFaces );
   if(self->swarmVariable) Memory_Free( varValue );

   /** if no swarmvariable is provided, the shepard correction value should given an indication of distance to the swarm */
   if(!self->swarmVariable) value[0] = shepardCorrection;
   /** apply shepard correction if required */
   if (self->useShepardCorrection ){
      if ( fabs(shepardCorrection) > self->shepardCorrectionThreshold )
         for(ii=0; ii<self->fieldComponentCount; ii++){
            value[ii] /= shepardCorrection;
         }
      else{
         value[0] = self->defaultValue;
         return OUTSIDE_GLOBAL;
      }
   }
   
   /** add offset as defined in AssignFromXML */
   for(ii=0; ii<self->fieldComponentCount; ii++){
      value[ii] += self->offset;
   }

   if (numSumParticles > 0)
      return LOCAL;
   else {
      if(self->swarmVariable) value[0] = self->defaultValue;
      return OUTSIDE_GLOBAL;
   }
}







