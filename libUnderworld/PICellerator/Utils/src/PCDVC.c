/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


/****************************************************************************************************************

  The algorithm here uses the DVCWeights module to compute a discrete voronoi diagram per FEM cell given a set of local
  particle positions, in 3D and 2D. The volumes of the Voronoi regions are used as integration weights for
  the integration point swarm and the integration points are the centroids of the same volumes.

  The volumes are also used as criteria for splitting and deleting particles. i.e. lowerT and upperT as percentages.

  The Threshold and CentPosRatio parameters are only used when Inflow is turned on.

  For a description of the Voronoi algorithm, see the article by Velic et.al.
     "A Fast Robust Algorithm for computing Discrete Voronoi Diagrams in N-dimensions"

*****************************************************************************************************************/

#include <StGermain/StGermain.h>
#include <mpi.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PopulationControl/PopulationControl.h>
#include <PICellerator/Weights/Weights.h>
#include <PICellerator/MaterialPoints/MaterialPoints.h>

//#include <PICellerator/PICellerator.h>
#include "types.h"

#include "PCDVC.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type PCDVC_Type = "PCDVC";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
void PCDVC_Firewall( int nump, int lCell_I, char* funcguy )
{
    Journal_Firewall( nump , NULL, "Something went wrong in %s: Problem has an under resolved cell (Cell Id = %d).\n"
                                   "You may need to check your initial particle layout configuration. A per cell layout might give better results than a global layout, "
                                   "especially where you have a deformed mesh. Also, if particles are allowed to escape the domain, you will need to set "
                                   "the 'particleEscape=True' swarm constructor parameter, and likely also the 'aggressive=True' population control constructed parameter.", funcguy, lCell_I );
}

PCDVC* PCDVC_New(
	Name						name,
	int*						res,
 	GeneralSwarm*	mps,
	double					upT,
	double					lowT,
	int						maxDeletions,
	int						maxSplits,
	Bool						splitInInterfaceCells,
 	Bool						deleteInInterfaceCells,
	Bool						Inflow,
	double					CentPosRatio,
	int						ParticlesPerCell,
	double					Threshold )
{
    PCDVC *self = _PCDVC_DefaultNew( name );

    self->isConstructed = True;
    _DVCWeights_Init( self, res );
    _PCDVC_Init( self, mps, upT, lowT, maxDeletions, maxSplits, splitInInterfaceCells, deleteInInterfaceCells, Inflow, CentPosRatio, ParticlesPerCell, Threshold );

	return self;
}

PCDVC* _PCDVC_New(  PCDVC_DEFARGS  ) {
    PCDVC* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(PCDVC) );

    /* Initialise the parent class. Every class has a parent, bar Stg_Component, which needs to be called */
    self = (PCDVC*)_DVCWeights_New(  DVCWEIGHTS_PASSARGS  );

    /* set this to zero here */
    self->pcdvcvisited = 0;

    /* General info */

    /* Virtual Info */

    return self;
}

void _PCDVC_Init( void* pcdvc, GeneralSwarm* mps, double upT, double lowT,
                  int maxDeletions, int maxSplits, Bool splitInInterfaceCells,
                  Bool deleteInInterfaceCells, Bool Inflow, double CentPosRatio,
                  int ParticlesPerCell, double Threshold )
{
    PCDVC* self = (PCDVC*)pcdvc;

    self->materialPointsSwarm = mps;
    self->upperT = upT;
    self->lowerT = lowT;
    self->maxDeletions = maxDeletions;
    self->maxSplits    = maxSplits;
    self->Inflow       = Inflow;
    self->splitInInterfaceCells  = splitInInterfaceCells;
    self->deleteInInterfaceCells = deleteInInterfaceCells;
    self->Threshold = Threshold;
    self->CentPosRatio = CentPosRatio;
    self->ParticlesPerCell = ParticlesPerCell;
}

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PCDVC_Delete( void* pcdvc ) {
    PCDVC* self = (PCDVC*)pcdvc;
    /* Delete parent */
    _DVCWeights_Delete( self );
}


void _PCDVC_Print( void* pcdvc, Stream* stream ) {
    PCDVC* self = (PCDVC*)pcdvc;
    /* Print parent */
    _DVCWeights_Print( self, stream );
}



void* _PCDVC_Copy( void* pcdvc, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    PCDVC*	self = (PCDVC*)pcdvc;
    PCDVC*	newPCDVC;

    newPCDVC = (PCDVC*)_DVCWeights_Copy( self, dest, deep, nameExt, ptrMap );
    return (void*)newPCDVC;
}

void* _PCDVC_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(PCDVC);
	Type                                                         type = PCDVC_Type;
	Stg_Class_DeleteFunction*                                 _delete = _PCDVC_Delete;
	Stg_Class_PrintFunction*                                   _print = _PCDVC_Print;
	Stg_Class_CopyFunction*                                     _copy = _PCDVC_Copy;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _PCDVC_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _PCDVC_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _PCDVC_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _PCDVC_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _PCDVC_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _PCDVC_Destroy;
	AllocationType                                 nameAllocationType = NON_GLOBAL;
	WeightsCalculator_CalculateFunction*                   _calculate = _PCDVC_Calculate;

    return (void*) _PCDVC_New(  PCDVC_PASSARGS  );
}


void _PCDVC_AssignFromXML( void* pcdvc, Stg_ComponentFactory* cf, void *data ) {

    PCDVC*	     self          = (PCDVC*) pcdvc;
    GeneralSwarm*       materialPointsSwarm;
    double upT, lowT;
    int maxD, maxS;
    Bool splitInInterfaceCells;
    Bool deleteInInterfaceCells;
    Bool Inflow;
    double CentPosRatio;
    int ParticlesPerCell;
    double Thresh;

    _DVCWeights_AssignFromXML( self, cf, data );

    materialPointsSwarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"GeneralSwarm", GeneralSwarm, True, data  );
    Stream*  stream = Journal_Register( Info_Type, (Name)materialPointsSwarm->type  );

    stream = Journal_Register( Info_Type, (Name)materialPointsSwarm->type  );
    upT = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"upperT", 25  );
    lowT = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"lowerT", 0.6  );
    maxD = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"maxDeletions", 2 );
    maxS = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"maxSplits", 3 );
    Inflow =  Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"Inflow", False );
    // note that the default for splitInInterfaceCells is True if Inflow is True
    splitInInterfaceCells  = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"splitInInterfaceCells", Inflow );
    deleteInInterfaceCells = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"deleteInInterfaceCells", False );
    Thresh = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"Threshold", 0.8  );
    //CentPosRatio is ratio of allowable distance of a centroid from generating particle to distance across a FEM cell.
    // I think the centroid distance idea is not ideal in the end...can create some weirdness...even thought the code "works"
    // after a while one can get wiggly lines in the cells...the centriods are close to the particles but its all
    // centred in the FEM cell.
    CentPosRatio =  Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"CentPosRatio", 0.01  );
    // just getting the initial PPC for now...maybe could make this a separate parameter yet if needed.
    ParticlesPerCell = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"particlesPerCell", 25 );

    if(upT < lowT){
        lowT = 0.6;
        upT = 25;
        Journal_Printf( stream,"On Proc %d: In func %s(): WARNING!! lowT and upT have been reset to some more reasonable values. (lowT = 0.6, upT = 25) now!",materialPointsSwarm->myRank, __func__);
    }

    _PCDVC_Init( self, materialPointsSwarm,  upT, lowT, maxD, maxS, splitInInterfaceCells,
                 deleteInInterfaceCells, Inflow, CentPosRatio, ParticlesPerCell, Thresh );
}

void _PCDVC_Build( void* pcdvc, void* data ) {
    PCDVC*	self = (PCDVC*)pcdvc;
    _DVCWeights_Build( self, data );
    Stg_Component_Build( self->materialPointsSwarm, data, False );
}
void _PCDVC_Destroy( void* pcdvc, void* data ) {
    PCDVC*	self = (PCDVC*)pcdvc;
    _DVCWeights_Destroy( self, data );
    Stg_Component_Destroy( self->materialPointsSwarm, data, False );
}
void _PCDVC_Initialise( void* pcdvc, void* data ) {
    PCDVC*	self = (PCDVC*)pcdvc;
    Stg_Component_Initialise( self->materialPointsSwarm, data, False );
    _DVCWeights_Initialise( self, data );
}
void _PCDVC_Execute( void* pcdvc, void* data ) {
    PCDVC*	self = (PCDVC*)pcdvc;
    _DVCWeights_Execute( self, data );
}


/*-------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/
/****************************************************************************************************
 This function deletes integration particle number intParticleToRemove_IndexWithinCell in lCell_I
 as well as the corresponding material point.
 It assumes the one to one coincidence mapper is being used. i.e. for every integration point
 there is one material point and vice versa.
******************************************************************************************************/
void deleteIntParticleByIndexWithinCell( IntegrationPointsSwarm*  intSwarm, GeneralSwarm* matSwarm, Cell_LocalIndex lCell_I, Particle_Index intParticleToRemove_IndexWithinCell ) {
    Particle_Index intParticleToRemove_IndexOnCPU = Swarm_ParticleCellIDtoLocalID( intSwarm, lCell_I, intParticleToRemove_IndexWithinCell );
    Swarm_DeleteParticle( intSwarm, intParticleToRemove_IndexOnCPU );
    Swarm_DeleteParticle( matSwarm, intParticleToRemove_IndexOnCPU );
}
/****************************************************************************************************
 This function deletes integration particle number intParticleToRemove_IndexOnCPU
 on integration swarm as well as the corresponding material point.
 It assumes the one to one coincidence mapper is being used. i.e. for every integration point
 there is one material point and vice versa.
******************************************************************************************************/
void deleteIntParticleByIndexOnCPU( IntegrationPointsSwarm*  intSwarm, GeneralSwarm* matSwarm, Particle_Index intParticleToRemove_IndexOnCPU ) {
    Swarm_DeleteParticle( intSwarm, intParticleToRemove_IndexOnCPU );
    Swarm_DeleteParticle( matSwarm, intParticleToRemove_IndexOnCPU );
}
void splitIntParticleByIndexWithinCell( IntegrationPointsSwarm*  intSwarm, GeneralSwarm* matSwarm, Cell_LocalIndex lCell_I, Particle_Index intParticleToSplit_IndexWithinCell, Coord xi ) {
    Particle_Index    intNewParticle_IndexOnCPU;/* the particle number within the swarm on the local CPU */
    Particle_Index    matNewParticle_IndexOnCPU;
    IntegrationPoint* intNewParticle;
    GlobalParticle*   matNewParticle;
    Particle_Index    intNewParticle_IndexWithinCell;/* the number of the particle within the cell */
    IntegrationPoint* intParticleToSplit;
    GlobalParticle*   matParticleToSplit;
    Coord             newCoord;

    FeMesh*  mesh = (FeMesh*)((ElementCellLayout*)matSwarm->cellLayout)->mesh;

    /* Add a new particle to end the end of each swarm */
    intNewParticle     = (IntegrationPoint*) Swarm_CreateNewParticle( intSwarm, &intNewParticle_IndexOnCPU );
    matNewParticle     = (GlobalParticle*)   Swarm_CreateNewParticle( matSwarm, &matNewParticle_IndexOnCPU );

    /* Copy particle information */
    intParticleToSplit = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, intParticleToSplit_IndexWithinCell );
    matParticleToSplit = (  GlobalParticle*) Swarm_ParticleInCellAt( matSwarm, lCell_I, intParticleToSplit_IndexWithinCell );

    memcpy( intNewParticle, intParticleToSplit, intSwarm->particleExtensionMgr->finalSize );
    memcpy( matNewParticle, matParticleToSplit, matSwarm->particleExtensionMgr->finalSize );

    Swarm_AddParticleToCell( intSwarm, lCell_I, intNewParticle_IndexOnCPU );
    Swarm_AddParticleToCell( matSwarm, lCell_I, matNewParticle_IndexOnCPU );

    /* Copy new local position to xi on new int particle */
    memcpy( intNewParticle->xi, xi, sizeof(Coord) );

    /* Get new Global Coordinates from the Local Coordinates */
    FeMesh_CoordLocalToGlobal( mesh, lCell_I, xi, newCoord );

    /* Copy new global position to coord on new mat particle */
    memcpy( matNewParticle->coord, newCoord, sizeof(Coord) );

}
/* sort indexOnCPU in reverse order */
int compare_indexOnCPU(const void * _particleA, const void * _particleB){
    struct deleteParticle * particleA = (struct deleteParticle *) _particleA;
    struct deleteParticle * particleB = (struct deleteParticle *) _particleB;

    if(particleA->indexOnCPU < particleB->indexOnCPU)
        return 1;
    else
        return -1;

}
int compare_ints(const void * _A, const void * _B){
    int *A = (int *) _A;
    int *B = (int *) _B;
    if(*A > *B) return 1; else return -1;
}
/* Calculate the integration weights for each particle by contructing
   a voronoi diagram in an element in 3D*/
void _PCDVC_Calculate3D( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I ) {
    PCDVC*             self            = (PCDVC*)  pcdvc;
    IntegrationPointsSwarm*  intSwarm  = (IntegrationPointsSwarm*) _swarm;
    GeneralSwarm* matSwarm =	(GeneralSwarm*) self->materialPointsSwarm;
    /* CoincidentMapper is a special case of the one to one mapper */
    //CoincidentMapper* mapper  = (CoincidentMapper*)(intSwarm->mapper); /* need the mapper after-all to update the material ref */
    Particle_InCellIndex         cParticleCount;
    IntegrationPoint**           particle;
    double dx,dy,dz,da;
    static struct cell *cells;// the connected grid
    struct particle *pList;// particle List
    struct chain *bchain;//boundary chain
    int nump_orig,nump,numx,numy,numz;
    double BBXMIN = -1.0; // the ranges of the local coordinates of a FEM cell.
    double BBXMAX = 1.0;
    double BBYMIN = -1.0;
    double BBYMAX = 1.0;
    double BBZMIN = -1.0;
    double BBZMAX = 1.0;
    int i,k;


    /*************************************/
    /* stuff for particle removal/adding */
    struct deleteParticle* deleteList;
    double maxW,minW;
    int maxI, minI;
    double lowT = self->lowerT;
    double upT = self->upperT;
    int delete_flag, split_flag;
    Particle_Index  *splitList;
    int maxDeletions = self->maxDeletions;/* hard setting this till I get stuff from xml file */
    int maxSplits = self->maxSplits;
    int splitCount;
    int deleteCount;
    int Count;
    Bool splitInInterfaceCells  = self->splitInInterfaceCells;
    Bool deleteInInterfaceCells = self->deleteInInterfaceCells;
    Bool Inflow = self->Inflow;
    Bool cellIsInterfaceCell = False;
    double Thresh = self->Threshold;
    int ParticlesPerCell = self->ParticlesPerCell;
    double CentPosRatio = self->CentPosRatio;
    Coord                   xi;

//	FiniteElement_Mesh*     mesh              = (FiniteElement_Mesh*)((ElementCellLayout*)matSwarm->cellLayout)->mesh;

    /* end decs needed for particle control */
    /*************************************/

    numx = self->resX;
    numy = self->resY;
    numz = self->resZ;

    nump_orig = nump = cParticleCount = intSwarm->cellParticleCountTbl[lCell_I];

    PCDVC_Firewall( nump, lCell_I, __func__ );

    dx = (BBXMAX - BBXMIN)/numx;
    dy = (BBYMAX - BBYMIN)/numy;
    dz = (BBZMAX - BBZMIN)/numz;
    da = dx*dy*dz;

    // Construct the grid for the Voronoi cells only once.
    // If we wanted to call this function again during a job with a different resolution
    // then we should destroy the grid once we have looped through the whole mesh.
    // I am assuming we are not going to do that for now.
    // Easy to implement this anyway, if needed.
    if(!self->pcdvcvisited){
        /* The PCDVC class should really be a class the next level up here */
        /* We should be able to swap out the WeightsCalculator_CalculateAll instead of just setting
           a pointer inside that function */
        self->pcdvcvisited++;
        _DVCWeights_ConstructGrid(&cells,numz,numy,numx,BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);
    }

    // init the data structures
    _DVCWeights_InitialiseStructs( (DVCWeights*)self, nump );
    pList = self->pList;
    bchain = self->bchain;

    _DVCWeights_ResetGrid3D(cells,numz*numy*numx);

    particle = (IntegrationPoint**)malloc( (nump)*sizeof(IntegrationPoint*));

    // initialize the particle positions to be the local coordinates of the material swarm particles
    // I am assuming the xi's (local coords) are precalculated somewhere and get reset based on material
    // positions each time step.
    for(i=0;i<nump;i++){

        particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
        pList[i].x = particle[i]->xi[0];
        pList[i].y = particle[i]->xi[1];
        pList[i].z = particle[i]->xi[2];

    }
    _DVCWeights_CreateVoronoi3D( bchain, pList, cells, dx, dy, dz, nump, numx, numy, numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);
    _DVCWeights_GetCentroids3D( cells, pList,numz,numy,numx,nump,da);

    /* DISABLE THIS FOR NOW.. NEED A DIFFERENT WAY FOR NEW METHODS */
    /* check to see if we are in an interface cell.
       We never want to delete particles in an interface cell
    matTypeFirst = IntegrationPointMapper_GetMaterialIndexAt(intSwarm->mapper,intSwarm->cellParticleTbl[ lCell_I ][ 0 ]);
    for(i=0;i<nump;i++){
        matType = IntegrationPointMapper_GetMaterialIndexAt(intSwarm->mapper,intSwarm->cellParticleTbl[ lCell_I ][ i ]);
        if(matType != matTypeFirst){
            cellIsInterfaceCell=True;
            break;
        }
    } */

    /************************************/
    /************************************/
    /*    Start 3D Population Control   */
    /************************************/
    /************************************/
    /* todo: put print statements into Journal statements */
    /*********************************/

    /** want to do something special for inflow problems **/
    /** we need to be a lot more aggressive with adding particles in this case **/
    /************************************/
    /************************************/
    /*      Start 3D Inflow Control     */
    /************************************/
    /************************************/
    /**********************************************************************************/
    /** If is interface cell and splitInInterfaceCells is false then turn off Inflow **/
    /**********************************************************************************/
    if(!splitInInterfaceCells && cellIsInterfaceCell){
        Inflow = False;
    }
    int *VCsize;
    int **particleVoronoiCellList;
    int *count; // count of how many cells each particle owns in the Voronoi diagram.
    int flag =0;
    double FEMCEllspan = BBXMAX - BBXMIN;
    /* LOGIC HERE IS BOGUS.  NEED TO HAVE A CLOSER LOOK. DISABLE FOR NOW */
//    double dist;
//    if(Inflow){
//        for(i=0;i<nump_orig;i++){
//            dist = (pList[i].cx - pList[i].x)*(pList[i].cx - pList[i].x) + (pList[i].cy - pList[i].y)*(pList[i].cy - pList[i].y) + (pList[i].cz - pList[i].z)*(pList[i].cz - pList[i].z);
//        }
//        if(dist > CentPosRatio*FEMCEllspan){flag = 1;}
//
//    }
    if(Inflow && (  ((1.0*nump_orig)/ParticlesPerCell < Thresh) || flag  ) ){
        int oneOda = (int)(1.0/da + 0.5);
        int *VCsize=(int *)malloc(sizeof(int)*nump);
        int j;
        int delNum;
        VCsize=(int *)malloc(sizeof(int)*nump_orig);
        count=(int *)malloc(sizeof(int)*nump_orig);
        splitCount = 0;
        particleVoronoiCellList = (int **)malloc(nump_orig * sizeof(int *));// [i][j] is jth cell owned by particle i
        for(i=0;i<nump_orig;i++){
            count[i] = (int)( pList[i].w*oneOda +0.5 ); // add the 0.5 so we don't lose anything from rounding down accidentally
            VCsize[i] = count[i];
            particleVoronoiCellList[i] = (int *)malloc( ( 1 + count[i] ) * sizeof(int));
        }
        for(i=0;i<numx*numy*numz;i++){// traverse the grid
            /** for total volume of a cell i.e. how many cells a particle owns .. this is actually calculated
                internally in the Centroids function and could be also accessed by dividing the weight of a particle by 'da'. **/
            if( count[cells[i].p] > 0 ){// it's possible for the total count to be a little off...this can cause a negative index
                particleVoronoiCellList[ cells[i].p ][ count[cells[i].p]-1 ] = i;
                count[cells[i].p] = count[cells[i].p] - 1;//decrement the count to insert i value into correct slot.
                //countSum++;
            }
        }
        // we now have a list of cells from the grid belonging to each particle that make up each Voronoi cell in the Voronoi diagram
        // next lets compare how far our centroids are from the particle positions.
        // this is my criteria for a voronoi cell having a weird shape...too stretched out for example.
        // this is exactly what happens in inflow situations.
        // Add a random number of new particles...
        // But Need to add them where they are needed.
        for(i=0;i<ParticlesPerCell;i++){
            j  =  (int) ( numx*numy*numz * (rand() / (RAND_MAX + 1.0)));
            xi[0] = cells[ j ].x;
            xi[1] = cells[ j ].y;
            xi[2] = cells[ j ].z;
            splitIntParticleByIndexWithinCell( intSwarm, matSwarm, lCell_I, cells[j].p, xi );
            nump++; splitCount++;
        }
        for(i=0;i<nump_orig;i++){
            free(particleVoronoiCellList[i]);
        }
        free(particleVoronoiCellList);
        free(VCsize); free(count);
        //if(splitCount) printf("\n\e[32mnump is now %d splitCount = %d\n",nump,splitCount);
        delNum = nump - ParticlesPerCell;
        if(delNum > 5){
            for(i=0;i<delNum;i++){
                j =  (int) (nump * (rand() / (RAND_MAX + 1.0)));
                deleteIntParticleByIndexWithinCell( intSwarm,  matSwarm, lCell_I, j );
                nump--;
                splitCount++;
            }
        }
        if(splitCount){// then redo Voronoi diagram.
            free(particle);

            if(nump < 3){
                PCDVC_Firewall( nump, lCell_I, __func__ );
                Journal_Printf(Journal_Register( Info_Type, (Name)intSwarm->type  ),"WARNING in %s: There are only %d particles in cell (Cell Id=%d)",__func__, nump, lCell_I);
            }
            // init the data structures
            _DVCWeights_InitialiseStructs( (DVCWeights*)self, nump );
            pList = self->pList;
            bchain = self->bchain;

            particle = (IntegrationPoint**)malloc( (nump)*sizeof(IntegrationPoint*));

            // re-initialize the particle positions to be the local coordinates of the material swarm particles
            // could do better here..instead of completely destroying these lists I could append to them I suppose.
            for(i=0;i<nump;i++){

                particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
                pList[i].x = particle[i]->xi[0];
                pList[i].y = particle[i]->xi[1];
                pList[i].z = particle[i]->xi[2];

            }
            _DVCWeights_ResetGrid3D(cells,numz*numy*numx);
            _DVCWeights_CreateVoronoi3D( bchain, pList, cells, dx, dy, dz, nump, numx, numy, numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);
            _DVCWeights_GetCentroids3D( cells, pList,numz,numy,numx,nump,da);

        }
        nump_orig = nump;
    }// if Inflow && ...
    if(Inflow){
        int oneOda = (int)(1.0/da + 0.5);
        //recreate the lists.
        particleVoronoiCellList = (int **)malloc(nump * sizeof(int *));// [i][j] is jth cell owned by particle i
        //VCsize = (int*)realloc(VCsize,nump_orig);
        //count = (int*)realloc(count,nump_orig);
        VCsize=(int *)malloc(sizeof(int)*nump);
        count=(int *)malloc(sizeof(int)*nump);

        for(i=0;i<nump;i++){
            count[i] = (int)( pList[i].w*oneOda +0.5 ); // add the 0.5 so we don't lose anything from rounding down accidentally
            VCsize[i] = count[i];
            particleVoronoiCellList[i] = (int *)malloc( ( 1 + count[i] ) * sizeof(int));
        }
        for(i=0;i<numx*numy*numz;i++){// traverse the grid
            //count[ cells[i].p ]++;
            /** for total volume of a cell i.e. how many cells a particle owns .. this is actually calculated
                internally in the Centroids function and could be also accessed by dividing the weight of a particle by 'da'. **/
            //if(VCsize[cells[i].p] != 0){// in case a particle is unresolved
            if( count[cells[i].p] > 0 ){// it's possible for the total count to be a little off...this can cause a negative index
                particleVoronoiCellList[ cells[i].p ][ count[cells[i].p]-1 ] = i;
                count[cells[i].p] = count[cells[i].p] - 1;//decrement the count to insert i value into correct slot.
                //countSum++;
            }
            //}
        }
    }//if Inflow
    /**************************************/
    /**************************************/
    /*        End 3D Inflow Control       */
    /**************************************/
    /**************************************/
    split_flag = 0;
    delete_flag = 0;
    splitCount = 0;
    deleteCount = 0;
    maxW = upT*8/100.0;
    minW = lowT*8/100.0;
    /* check to see if we are in an interface cell.
       We never want to delete particles in an interface cell */
    if(cellIsInterfaceCell){
        if(!deleteInInterfaceCells) maxDeletions = 0; /* no deletions in an interface cell */
        /* this may be inadequate...we may need to do something in the neighbouring cells to interface cells as well */
        if(!splitInInterfaceCells)  maxSplits    = 0;
    }

    /* need a struct for the deleteList because we must sort it by indexOnCPU and delete in reverse order
       so we don't have the potential problem of  deleting a particle from the list that points to the last particle on the swarm */
    deleteList = (struct deleteParticle*)malloc(nump*sizeof(struct deleteParticle));/* I don't think I am going to let you delete more than half the particles in a given cell */
    splitList  = (Particle_Index*)malloc(nump*sizeof(Particle_Index));
    for(i=0;i<nump;i++){
        if(pList[i].w > maxW){ /* maxW = pList[i].w; maxI = i;*/ splitList[splitCount] = i; splitCount++;}
        if(pList[i].w < minW){
            /* minW = pList[i].w; minI = i; */
            deleteList[deleteCount].indexWithinCell = i;
            deleteList[deleteCount].indexOnCPU  = Swarm_ParticleCellIDtoLocalID( intSwarm, lCell_I, i );
            deleteCount++;
        }
    }
    /* sort the deleteList by indexOnCPU so we can delete the list in reverse order */
    qsort(deleteList, (deleteCount), sizeof(struct deleteParticle),compare_indexOnCPU);

//    if(maxDeletions > nump-4){ maxDeletions = nump/2;}

    /* we now have our lists of particles to delete and split */
    Count = maxSplits > splitCount ? splitCount : maxSplits;
    for(i=0;i<Count;i++){
        int j;
        maxI = splitList[i];
        if(!Inflow){
            /* now get local coords from centroid of the cell that particleToSplit lives in */
            xi[0] = pList[maxI].cx;
            xi[1] = pList[maxI].cy;
            xi[2] = pList[maxI].cz;
        }
        else{
            /* lets do something different now..because am getting lines formed on inflow probs */
            j =  (int) (VCsize[maxI] * (rand() / (RAND_MAX + 1.0)));
            xi[0] = cells[ particleVoronoiCellList[maxI][j] ].x;
            xi[1] = cells[ particleVoronoiCellList[maxI][j] ].y;
            xi[2] = cells[ particleVoronoiCellList[maxI][j] ].z;
        }
        split_flag = 1;
        nump++;
        splitIntParticleByIndexWithinCell( intSwarm, matSwarm, lCell_I, maxI, xi );
    }

    if(Inflow){
        for(i=0;i<nump_orig;i++){
            free(particleVoronoiCellList[i]);
        }
        free(particleVoronoiCellList);
        free(VCsize);
        free(count);
    }
    Count = maxDeletions > deleteCount ? deleteCount : maxDeletions;
    for(i=0;i<Count;i++){
        minI = deleteList[i].indexOnCPU;
        deleteIntParticleByIndexOnCPU( intSwarm,  matSwarm, minI );
        delete_flag = 1;
        nump--;
    }

    if(delete_flag || split_flag ){/* then we need to redo the Voronoi diagram */

        free(particle);

        if(nump < 3){
            PCDVC_Firewall( nump, lCell_I, __func__ );
            Journal_Printf(Journal_Register( Info_Type, (Name)intSwarm->type  ),"WARNING in %s: There are only %d particles in cell (Cell Id=%d)",__func__, nump, lCell_I);
        }
        // init the data structures
        _DVCWeights_InitialiseStructs( (DVCWeights*)self, nump );
        pList = self->pList;
        bchain = self->bchain;
        //_DVCWeights_ResetGrid3D(&cells,numz*numy*numx);

        particle = (IntegrationPoint**)malloc( (nump)*sizeof(IntegrationPoint*));

        // re-initialize the particle positions to be the local coordinates of the material swarm particles
        for(i=0;i<nump;i++){

            particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
            pList[i].x = particle[i]->xi[0];
            pList[i].y = particle[i]->xi[1];
            pList[i].z = particle[i]->xi[2];
            //pList[i].index = i; // to track which particle numbers we have after sorting this list */

        }
        //printf("Population of matSwarm is %d\n",matSwarm->particleLocalCount);
        //printf("Population of intSwarm is %d\n",intSwarm->particleLocalCount);
        _DVCWeights_ResetGrid3D(cells,numz*numy*numx);
        _DVCWeights_CreateVoronoi3D( bchain, pList, cells, dx, dy, dz, nump, numx, numy, numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);
        _DVCWeights_GetCentroids3D( cells, pList,numz,numy,numx,nump,da);

    }/* if delete_flag */
    /******************************************/
    /******************************************/
    /*        End 3D Population Control       */
    /******************************************/
    /******************************************/

    // We are setting the integration points to be the centroids of the Voronoi regions here and
    // the weight is the volume of each Voronoi region.
    for(i=0;i<nump;i++){

        particle[i]->xi[0] = pList[i].cx;
        particle[i]->xi[1] = pList[i].cy;
        particle[i]->xi[2] = pList[i].cz;
        particle[i]->weight = pList[i].w;

    }

    free(particle);
    free(deleteList);
    free(splitList);

}

/* Calculate the integration weighting for each particle by contructing
   a voronoi diagram in an element in 2D*/
void _PCDVC_Calculate2D( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I ) {
    PCDVC*                       self      = (PCDVC*)                  pcdvc;
    IntegrationPointsSwarm*      intSwarm  = (IntegrationPointsSwarm*) _swarm;
    GeneralSwarm*                matSwarm  = (GeneralSwarm*)           self->materialPointsSwarm;
    Particle_InCellIndex         cParticleCount;
    IntegrationPoint**           particle;

    double dx,dy,da;
    static struct cell *cells;// the connected grid
    struct particle *pList;
    struct chain *bchain;
    int nump_orig,nump,numx,numy;
    double BBXMIN = -1.0; // the ranges of the local coordinates of a FEM cell.
    double BBXMAX = 1.0;
    double BBYMIN = -1.0;
    double BBYMAX = 1.0;
    int i;

    /*************************************/
    /* stuff for particle removal/adding */
    double maxW,minW;
    int maxI, minI;
    double lowT = self->lowerT;
    double upT = self->upperT;
    int delete_flag, split_flag;
    struct deleteParticle* deleteList;
    Particle_Index  *splitList;
    int splitCount;
    int deleteCount;
    int maxDeletions = self->maxDeletions;/* hard setting this till I get stuff from xml file */
    int maxSplits = self->maxSplits;
    int Count;
    Bool splitInInterfaceCells  = self->splitInInterfaceCells;
    Bool deleteInInterfaceCells = self->deleteInInterfaceCells;
    Bool Inflow = self->Inflow;
    Bool cellIsInterfaceCell = False;
    double Thresh = self->Threshold;
    int ParticlesPerCell = self->ParticlesPerCell;
    double CentPosRatio = self->CentPosRatio;
    Coord                   xi;


    /* end decs needed for particle control */
    /*************************************/


    numx = self->resX;
    numy = self->resY;

    nump_orig = nump = cParticleCount = intSwarm->cellParticleCountTbl[lCell_I];

    PCDVC_Firewall( nump, lCell_I, __func__ );

    dx = (BBXMAX - BBXMIN)/numx;
    dy = (BBYMAX - BBYMIN)/numy;
    da = dx*dy;

    // Construct the grid for the Voronoi cells only once.
    // If we wanted to call this function again during a job with a different resolution
    // then we should destroy the grid once we have looped through the whole mesh.
    // I am assuming we are not going to do that for now.
    // Easy to implement this anyway, if needed.
    if(!self->pcdvcvisited){
        /* The PCDVC class should really be a class the next level up here */
        /* We should be able to swap out the WeightsCalculator_CalculateAll instead of just setting
           a pointer inside that function */
        self->pcdvcvisited++;
        _DVCWeights_ConstructGrid2D(&cells,numy,numx,BBXMIN,BBYMIN,BBXMAX,BBYMAX);
    }


    // init the data structures
    _DVCWeights_InitialiseStructs2D( (DVCWeights*)self, nump);
    bchain = self->bchain;
    pList  = self->pList;

    _DVCWeights_ResetGrid2D(cells,numy*numx);

    particle = (IntegrationPoint**)malloc((nump)*sizeof(IntegrationPoint*));

    // initialize the particle positions to be the local coordinates of the material swarm particles
    // I am assuming the xi's (local coords) are precalculated somewhere and get reset based on material
    // positions each time step.
    for(i=0;i<nump;i++){

        particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
        pList[i].x = particle[i]->xi[0];
        pList[i].y = particle[i]->xi[1];

    }
    _DVCWeights_CreateVoronoi2D( bchain, pList, cells, dx, dy, nump, numx, numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);
    _DVCWeights_GetCentroids2D( cells, pList,numy,numx,nump,da);

    /* DISABLE THIS FOR NOW.. NEED A DIFFERENT WAY FOR NEW METHODS */
    /* check to see if we are in an interface cell.
       We never want to delete particles in an interface cell
    matTypeFirst = IntegrationPointMapper_GetMaterialIndexAt(intSwarm->mapper,intSwarm->cellParticleTbl[ lCell_I ][ 0 ]);
    for(i=0;i<nump;i++){
        matType = IntegrationPointMapper_GetMaterialIndexAt(intSwarm->mapper,intSwarm->cellParticleTbl[ lCell_I ][ i ]);
        if(matType != matTypeFirst){
            cellIsInterfaceCell=True;
            break;
        }
    } */
    /************************************/
    /************************************/
    /*    Start 2D Population Control   */
    /************************************/
    /** want to do something special for inflow problems **/
    /** we need to be a lot more aggressive with adding particles in this case **/
    /************************************/
    /************************************/
    /*      Start 2D Inflow Control     */
    /************************************/
    /************************************/

    /**********************************************************************************/
    /** If is interface cell and splitInInterfaceCells is false then turn off Inflow **/
    /**********************************************************************************/
    if(!splitInInterfaceCells && cellIsInterfaceCell){
        Inflow = False;
    }
//	if(0){
    int *VCsize;
    int **particleVoronoiCellList;
    int *count; // count of how many cells each particle owns in the Voronoi diagram.
    int flag =0;
    double FEMCEllspan = BBXMAX - BBXMIN;
    /* LOGIC HERE IS BOGUS.  NEED TO HAVE A CLOSER LOOK. DISABLE FOR NOW */
//    double dist;
//    if(Inflow){
//        for(i=0;i<nump_orig;i++){
//            dist = (pList[i].x-pList[i].cx)*(pList[i].x-pList[i].cx)+(pList[i].y-pList[i].cy)*(pList[i].y-pList[i].cy);;
//        }
//        if(dist > CentPosRatio*FEMCEllspan){flag = 1;}
//
//    }
    if(Inflow && (  ((1.0*nump_orig)/ParticlesPerCell < Thresh) || flag  ) ){
        int oneOda = (int)(1.0/da + 0.5);
        int j;
        int delNum;

        VCsize=(int *)malloc(sizeof(int)*nump_orig);
        count=(int *)malloc(sizeof(int)*nump_orig);
        splitCount = 0;
        particleVoronoiCellList = (int **)malloc(nump_orig * sizeof(int *));// [i][j] is jth cell owned by particle i
        for(i=0;i<nump_orig;i++){
            count[i] = (int)( pList[i].w*oneOda +0.5 ); // add the 0.5 so we don't lose anything from rounding down accidentally
            VCsize[i] = count[i];
            particleVoronoiCellList[i] = (int *)malloc( ( 1 + count[i] ) * sizeof(int));
        }
        for(i=0;i<numx*numy;i++){// traverse the grid
            //count[ cells[i].p ]++;
            /** for total volume of a cell i.e. how many cells a particle owns .. this is actually calculated
                internally in the Centroids function and could be also accessed by dividing the weight of a particle by 'da'. **/
            if( count[cells[i].p] > 0 ){// it's possible for the total count to be a little off...this can cause a negative index
                particleVoronoiCellList[ cells[i].p ][ count[cells[i].p]-1 ] = i;
                count[cells[i].p] = count[cells[i].p] - 1;//decrement the count to insert i value into correct slot.
                //countSum++;
            }
        }
        //printf("countSum = %d\n",countSum);
        // we now have a list of cells from the grid belonging to each particle that make up each Voronoi cell in the Voronoi diagram
        // next lets compare how far our centroids are from the particle positions.
        // this is my criteria for a voronoi cell having a weird shape...too stretched out for example.
        // this is exactly what happens in inflow situations.
        // Add a random number of new particles...
        // But Need to add them where they are needed.
        for(i=0;i<ParticlesPerCell;i++){
            j  =  (int) ( numx*numy * (rand() / (RAND_MAX + 1.0)));
            xi[0] = cells[ j ].x;
            xi[1] = cells[ j ].y;
            splitIntParticleByIndexWithinCell( intSwarm, matSwarm, lCell_I, cells[j].p, xi );
            nump++; splitCount++;
        }
        for(i=0;i<nump_orig;i++){
            free(particleVoronoiCellList[i]);
        }
        free(particleVoronoiCellList);
        free(VCsize); free(count);
        //if(splitCount) printf("\n\e[32mnump is now %d splitCount = %d\n",nump,splitCount);
        delNum = nump - ParticlesPerCell;
        if(delNum > 5){
            for(i=0;i<delNum;i++){
                j =  (int) (nump * (rand() / (RAND_MAX + 1.0)));
                deleteIntParticleByIndexWithinCell( intSwarm,  matSwarm, lCell_I, j );
                nump--;
                splitCount++;
            }
        }

        if(splitCount){// then redo Voronoi diagram.
            free(particle);

            if(nump < 3){
                PCDVC_Firewall( nump, lCell_I, __func__ );
                Journal_Printf(Journal_Register( Info_Type, (Name)intSwarm->type  ),"WARNING in %s: There are only %d particles in cell (Cell Id=%d)",__func__, nump, lCell_I);
            }
            // init the data structures
            _DVCWeights_InitialiseStructs2D( (DVCWeights*)self, nump);
            pList  = self->pList;
            bchain = self->bchain;
            particle = (IntegrationPoint**)malloc( (nump)*sizeof(IntegrationPoint*));
            // re-initialize the particle positions to be the local coordinates of the material swarm particles
            // could do better here..instead of completely destroying these lists I could append to them I suppose.
            for(i=0;i<nump;i++){
                particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
                pList[i].x = particle[i]->xi[0];
                pList[i].y = particle[i]->xi[1];
            }
            _DVCWeights_ResetGrid2D(cells,numy*numx);
            _DVCWeights_CreateVoronoi2D( bchain, pList, cells, dx, dy, nump, numx, numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);
            _DVCWeights_GetCentroids2D( cells, pList,numy,numx,nump,da);
        }
        nump_orig = nump;
    }// if Inflow && ...
    if(Inflow){
        int oneOda = (int)(1.0/da + 0.5);
        //recreate the lists.
        particleVoronoiCellList = (int **)malloc(nump * sizeof(int *));// [i][j] is jth cell owned by particle i
        VCsize=(int *)malloc(sizeof(int)*nump);
        count=(int *)malloc(sizeof(int)*nump);

        for(i=0;i<nump;i++){
            count[i] = (int)( pList[i].w*oneOda +0.5 ); // add the 0.5 so we don't lose anything from rounding down accidentally
            VCsize[i] = count[i];
            particleVoronoiCellList[i] = (int *)malloc( ( 1 + count[i] ) * sizeof(int));
        }
        for(i=0;i<numx*numy;i++){// traverse the grid
            /** for total volume of a cell i.e. how many cells a particle owns .. this is actually calculated
                internally in the Centroids function and could be also accessed by dividing the weight of a particle by 'da'. **/
            if( count[cells[i].p] > 0 ){// it's possible for the total count to be a little off...this can cause a negative index
                particleVoronoiCellList[ cells[i].p ][ count[cells[i].p]-1 ] = i;
                count[cells[i].p] = count[cells[i].p] - 1;//decrement the count to insert i value into correct slot.
            }
        }
    }//if Inflow
    /************************************/
    /************************************/
    /*        End 2D Inflow Control     */
    /************************************/
    /************************************/
    split_flag = 0;
    delete_flag = 0;
    splitCount = 0;
    deleteCount = 0;
    /* shouldn't need maxI and minI now */
    maxW = upT*4/100.0;
    minW = lowT*4/100.0;
    /* check to see if we are in an interface cell.
       We never want to delete particles in an interface cell */
    if(cellIsInterfaceCell){
        if(!deleteInInterfaceCells) maxDeletions = 0; /* no deletions in an interface cell */
        /* this may be inadequate...we may need to do something in the neighbouring cells to interface cells as well */
        if(!splitInInterfaceCells)  maxSplits    = 0;
    }

    /* need a struct for the deleteList because we must sort it by indexOnCPU and delete in reverse order
       so we don't have the potential problem of  deleting a particle from the list that points to the last particle on the swarm */
    splitList  = (Particle_Index*)malloc(nump*sizeof(Particle_Index));
    deleteList = (struct deleteParticle*)malloc(nump*sizeof(struct deleteParticle));/* I don't think I am going to let you delete more than half the particles in a given cell */
    for(i=0;i<nump;i++){
        if(pList[i].w > maxW){ /* maxW = pList[i].w; maxI = i;*/ splitList[splitCount] = i; splitCount++;}
        if(pList[i].w < minW){
            /* minW = pList[i].w; minI = i; */
            deleteList[deleteCount].indexWithinCell = i;
            deleteList[deleteCount].indexOnCPU  = Swarm_ParticleCellIDtoLocalID( intSwarm, lCell_I, i );
            deleteCount++;
        }
    }
    /* sort the deleteList by indexOnCPU so we can delete the list in reverse order */
    qsort(deleteList, (deleteCount), sizeof(struct deleteParticle),compare_indexOnCPU);
    //deleteCount--; /* is going to be one size too large after the loop */
    /*
      for(i=0;i<deleteCount;i++){
      printf("deleteCount = %d\n",deleteCount);
      printf("indices are indexWithinCell %d indexOnCPU %d\n",deleteList[i].indexWithinCell,deleteList[i].indexOnCPU);
      }
    */

//    if(maxDeletions > nump-4){ maxDeletions = nump/2;}

    /* we now have our lists of particles to delete and split */
    Count = maxSplits > splitCount ? splitCount : maxSplits;
    /* lets do something different now..because am getting lines formed on inflow probs */
    //if(Inflow){ Count = 0;} //turn off ordinary splitting if inflow is on for moment
    for(i=0;i<Count;i++){
        int j;
        maxI = splitList[i];
        if(!Inflow){
            /* now get local coords from centroid of the cell that particleToSplit lives in */
            xi[0] = pList[maxI].cx;
            xi[1] = pList[maxI].cy;
        }
        else{
            /* lets do something different now..because am getting lines formed on inflow probs */
            j =  (int) (VCsize[maxI] * (rand() / (RAND_MAX + 1.0)));
            xi[0] = cells[ particleVoronoiCellList[maxI][j] ].x;
            xi[1] = cells[ particleVoronoiCellList[maxI][j] ].y;
        }
        split_flag = 1;
        nump++;
        splitIntParticleByIndexWithinCell( intSwarm, matSwarm, lCell_I, maxI, xi );
    }

    if(Inflow){
        for(i=0;i<nump_orig;i++){
            free(particleVoronoiCellList[i]);
        }
        free(particleVoronoiCellList);
        free(VCsize);
        free(count);
    }

    Count = maxDeletions > deleteCount ? deleteCount : maxDeletions;
    for(i=0;i<Count;i++){
        minI = deleteList[i].indexOnCPU;
        deleteIntParticleByIndexOnCPU( intSwarm,  matSwarm, minI );
        delete_flag = 1;
        nump--;
    }
    //printf("pList[maxI].w = %lf particle num = %d : %d\n", pList[maxI].w, pList[maxI].index,maxI);
    //printf("pList[minI].w = %lf particle num = %d : %d\n", pList[minI].w, pList[minI].index,minI);
    if(delete_flag || split_flag ){/* then we need to redo the Voronoi diagram */
        free(particle);
        if(nump < 3){
            PCDVC_Firewall( nump, lCell_I, __func__ );
        }
        particle = (IntegrationPoint**)malloc((nump)*sizeof(IntegrationPoint*));
        // init the data structures
        _DVCWeights_InitialiseStructs2D( (DVCWeights*)self, nump);
        pList  = self->pList;
        bchain = self->bchain;
        // re-initialize the particle positions to be the local coordinates of the material swarm particles
        for(i=0;i<nump;i++){

            particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( intSwarm, lCell_I, i );
            pList[i].x = particle[i]->xi[0];
            pList[i].y = particle[i]->xi[1];
            //pList[i].index = i; // to track which particle numbers we have after sorting this list */

        }
        //printf("Population of matSwarm is %d\n",matSwarm->particleLocalCount);
        //printf("Population of intSwarm is %d\n",intSwarm->particleLocalCount);

        _DVCWeights_ResetGrid2D(cells,numy*numx);
        //reset_grid(&cells,numz*numy*numx);/* adding this line fixed memory leak probs */
        _DVCWeights_CreateVoronoi2D( bchain, pList, cells, dx, dy, nump, numx, numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);
        _DVCWeights_GetCentroids2D( cells, pList,numy,numx,nump,da);

    }/* if delete_flag */
    /**************************************/
    /**************************************/
    /*      End 2D Population Control     */
    /**************************************/
    /**************************************/


    // We are setting the integration points to be the centroids of the Voronoi regions here and
    // the weight is the volume of each Voronoi region.
    for(i=0;i<nump;i++){

        particle[i]->xi[0] = pList[i].cx;
        particle[i]->xi[1] = pList[i].cy;
        particle[i]->weight = pList[i].w;

    }
    /* for(k=0;k<nump;k++){ */
/* 	  printf("::In %s O(%10.7lf %10.7lf) C(%10.7lf %10.7lf) W(%.4lf)\n", __func__, pList[k].x, pList[k].y, pList[k].cx, pList[k].cy, pList[k].w); */
/*     } */
    free(particle);
    free(deleteList);
    free(splitList);
    /*
      FILE *fp;
      fp=fopen("nump.txt","a");
      if(lCell_I< 33){
      fprintf(fp,"nump = %d cell = %d\n",nump,lCell_I);
      }
      fclose(fp);
    */
}

void _PCDVC_Calculate( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I ){
    Swarm* swarm = (Swarm*) _swarm;
    PCDVC*  self = (PCDVC*)  pcdvc;
    /* GeneralSwarm* matSwarm =	(GeneralSwarm*) self->materialPointsSwarm; */
    /* it might be nice to report the total deletions and splits as well as the final population here */
    /* One could set the parameters to be too aggressive and cause "swarm thrashing" where many particles
       are being created and destroyed while maintaining some population that it has converged on */

    if(swarm->dim == 3)
        _PCDVC_Calculate3D( pcdvc, _swarm, lCell_I);
    else
        _PCDVC_Calculate2D( pcdvc, _swarm, lCell_I);

    /* if last cell done, let's realloc to account for deletions */
    if(lCell_I == swarm->cellLocalCount - 1){
        Swarm_Realloc( swarm );
        Swarm_Realloc( self->materialPointsSwarm );
        /* lets also force a variable update now to ensure variables have correct arraySize (might be off due to deletions) */
        unsigned v_i;
        for( v_i = 0; v_i < swarm->nSwarmVars; v_i++ )
        {
            if( swarm->swarmVars[v_i]->variable )
                StgVariable_Update( swarm->swarmVars[v_i]->variable );
        }
        for( v_i = 0; v_i < self->materialPointsSwarm->nSwarmVars; v_i++ )
        {
            if( self->materialPointsSwarm->swarmVars[v_i]->variable )
                StgVariable_Update( self->materialPointsSwarm->swarmVars[v_i]->variable );
        }

    }


}
/*-------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
