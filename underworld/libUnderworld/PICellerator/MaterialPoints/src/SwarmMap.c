/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include "types.h"

#include "IntegrationPointsSwarm.h"
#include "SwarmMap.h"



/* Textual name of this class */
const Type SwarmMap_Type = "SwarmMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

SwarmMap* SwarmMap_New( IntegrationPointsSwarm* swarm ) {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(SwarmMap);
	Type                              type = SwarmMap_Type;
	Stg_Class_DeleteFunction*      _delete = _SwarmMap_Delete;
	Stg_Class_PrintFunction*        _print = _SwarmMap_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

    return _SwarmMap_New(  SWARMMAP_PASSARGS  );
}

SwarmMap* _SwarmMap_New(  SWARMMAP_DEFARGS  ) {
	SwarmMap*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SwarmMap) );
	self = (SwarmMap*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */
    self->mapArray = NULL;
    self->swarm = swarm;
    self->elementCount = 0;

	/* SwarmMap info */
	_SwarmMap_Init( self );

	return self;
}

void _SwarmMap_Init( SwarmMap* self ) {
	assert( self && Stg_CheckType( self, SwarmMap ) );
    
    /* lets get number of elements */
    self->elementCount = Mesh_GetDomainSize( self->swarm->mesh, Mesh_GetDimSize( self->swarm->mesh ) );
    
    /* alloc int array */
    self->mapArray = (int**)malloc(sizeof(int*)*self->elementCount);
    
    int ii;
    for (ii=0; ii<self->elementCount; ii++) {
        int cell = CellLayout_MapElementIdToCellId( self->swarm->cellLayout, ii );
        int cellParticleCount = self->swarm->cellParticleCountTbl[cell];
        self->mapArray[ii] = (int*) malloc(sizeof(int)*cellParticleCount);
        int jj;
        for (jj=0; jj<cellParticleCount; jj++) {
            /* lets init to -1 */
            self->mapArray[ii][jj] = -1;
        }
    }
   
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Public functions
*/

void _SwarmMap_Delete( void* swarmMap ) {
	SwarmMap*	self = (SwarmMap*)swarmMap;

	assert( self && Stg_CheckType( self, SwarmMap ) );

    SwarmMap_Clear(self);

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _SwarmMap_Print( void* swarmMap, Stream* stream ) {
	SwarmMap*	self = (SwarmMap*)swarmMap;
	Stream* 	swarmMapStream;

	assert( self && Stg_CheckType( self, SwarmMap ) );
	
	/* Set the Journal for printing informations */
	swarmMapStream = Journal_Register( InfoStream_Type, (Name)"SwarmMapStream"  );

	/* Print parent */
	Journal_Printf( stream, "SwarmMap (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void SwarmMap_Clear( SwarmMap* self ) {
	assert( self && Stg_CheckType( self, SwarmMap ) );

    /* lets get number of elements */
    if (! self->mapArray )
        return;
    int ii;
    for (ii=0; ii<self->elementCount; ii++) {
        free(self->mapArray[ii]);
        self->mapArray[ii] = NULL;
    }
    free(self->mapArray);
    self->mapArray=NULL;

}


void SwarmMap_Insert( SwarmMap* self, unsigned keyElementId, unsigned keyParticleCellId, unsigned insertValue ) {
    Journal_Firewall( keyElementId < self->elementCount, NULL,
        "Error - in %s() - Attempting to insert map entry for elementId (%u) greater than element count (%u).",
        __func__, keyElementId, self->elementCount );

    int cell = CellLayout_MapElementIdToCellId( self->swarm->cellLayout, keyElementId );
    int cellParticleCount = self->swarm->cellParticleCountTbl[cell];
    Journal_Firewall( keyParticleCellId < cellParticleCount, NULL,
        "Error - in %s() - Attempting to insert map entry for particleId (%u) greater than particle cell count (%u).",
        __func__, keyParticleCellId, cellParticleCount );
    
    self->mapArray[keyElementId][keyParticleCellId] = insertValue;


}

Bool SwarmMap_Map( SwarmMap* self, unsigned keyElementId, unsigned keyParticleCellId, unsigned *returnValue ) {
    if (! self->mapArray )
        _SwarmMap_Init( self );
    Journal_Firewall( keyElementId < self->elementCount, NULL,
        "Error - in %s() - Attempting to insert map entry for elementId (%u) greater than element count (%u).",
        __func__, keyElementId, self->elementCount );
    int cell = CellLayout_MapElementIdToCellId( self->swarm->cellLayout, keyElementId );
    int cellParticleCount = self->swarm->cellParticleCountTbl[cell];
    Journal_Firewall( keyParticleCellId < cellParticleCount, NULL,
        "Error - in %s() - Attempting to insert map entry for particleId (%u) greater than particle cell count (%u).",
        __func__, keyParticleCellId, cellParticleCount );

    int mapVal = self->mapArray[keyElementId][keyParticleCellId];
    if (mapVal == -1)
        return False;
    else {
        *returnValue = mapVal;
        return True;
    }

}
