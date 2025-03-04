/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


/****************************************************************************************************************

  The algorithm here-in computes a discrete voronoi diagram per FEM cell given a set of local
  particle positions, in 3D and 2D. The volumes of the Voronoi regions are used as integration weights for
  the integration point swarm and the integration points are the centroids of the same volumes.

  For a description of this algorithm, see the article by Velic et.al.
     "A Fast Robust Algorithm for computing Discrete Voronoi Diagrams in N-dimensions"
  
  Warning:

  Be very careful of making changes here. It may have undesirable consequences in regard to the
  speed of this implementation. Speed is very important here. You may be tempted to merge the
  2D and 3D functions. Don't do it unless you can do it without using control statements.
  This implementation uses von-Neumann neighbourhoods for boundary chain growth. Do not be tempted 
  to implement "diagonal" neighbourhood growth cycles. For this is an abomination unto me.
*****************************************************************************************************************/

#include <StGermain/libStGermain/src/StGermain.h>
#include <mpi.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include "types.h"
#include "WeightsCalculator.h"
#include "DVCWeights.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type DVCWeights_Type = "DVCWeights";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
DVCWeights* DVCWeights_New( Name name, int *res ) {
	DVCWeights *self = _DVCWeights_DefaultNew( name );

	self->isConstructed = True;
	_DVCWeights_Init( self, res );

	return self;
}

DVCWeights* _DVCWeights_New(  DVCWEIGHTS_DEFARGS  ) {
    DVCWeights* self;
        
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(DVCWeights) );
    self = (DVCWeights*)_WeightsCalculator_New(  WEIGHTSCALCULATOR_PASSARGS  );
        
    /* General info */

    /* Virtual Info */
    self->visiteddvcweights = 0;
    self->pList = NULL;
    self->bchain  = NULL;
    self->plistSize  = 0;
    self->bchainSize = 0;

    return self;
}

void _DVCWeights_Init( void* dvcWeights, int *res ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;

    self->resX = res[I_AXIS];
    self->resY = res[J_AXIS];
    self->resZ = res[K_AXIS];

}

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _DVCWeights_Delete_bchain( DVCWeights* dvcWeights ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    if (self->bchain) {
        int i;
        for(i=0;i<self->bchainSize;i++){
            free((self->bchain)[i].new_claimed_cells);
            free((self->bchain)[i].new_bound_cells);
        }
        self->bchainSize = 0;
        free(self->bchain);
        self->bchain = NULL;
    }
}
void _DVCWeights_Delete_plist( DVCWeights* dvcWeights ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    if (self->pList) {
        free(self->pList);
        self->pList = NULL;
    }
    self->plistSize  = 0;
}

void _DVCWeights_Delete( void* dvcWeights ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    self->visiteddvcweights = 0;
    
    _DVCWeights_Delete_bchain(self);
    _DVCWeights_Delete_plist(self);

    /* Delete parent */
    _WeightsCalculator_Delete( self );
}

void _DVCWeights_Print( void* dvcWeights, Stream* stream ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    /* Print parent */
    _WeightsCalculator_Print( self, stream );
}



void* _DVCWeights_Copy( void* dvcWeights, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    DVCWeights* newDVCWeights;
        
    newDVCWeights = (DVCWeights*)_WeightsCalculator_Copy( self, dest, deep, nameExt, ptrMap );
    return (void*)newDVCWeights;
}

void* _DVCWeights_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(DVCWeights);
	Type                                                      type = DVCWeights_Type;
	Stg_Class_DeleteFunction*                              _delete = _DVCWeights_Delete;
	Stg_Class_PrintFunction*                                _print = _DVCWeights_Print;
	Stg_Class_CopyFunction*                                  _copy = _DVCWeights_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _DVCWeights_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _DVCWeights_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _DVCWeights_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _DVCWeights_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _DVCWeights_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _DVCWeights_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	WeightsCalculator_CalculateFunction*                _calculate = _DVCWeights_Calculate;

    return (void*) _DVCWeights_New(  DVCWEIGHTS_PASSARGS  );
}


void _DVCWeights_AssignFromXML( void* dvcWeights, Stg_ComponentFactory* cf, void *data ) {

    DVCWeights*      self          = (DVCWeights*) dvcWeights;

    int defaultResolution;
    int resolution[3];

    _WeightsCalculator_AssignFromXML( self, cf, data );

    defaultResolution = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", 10  );
    resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", defaultResolution  );
    resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", defaultResolution  );
    resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", defaultResolution  );
       
    _DVCWeights_Init( self, resolution );
}

void _DVCWeights_Build( void* dvcWeights, void* data ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    _WeightsCalculator_Build( self, data );
}
void _DVCWeights_Initialise( void* dvcWeights, void* data ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    _WeightsCalculator_Initialise( self, data );
}
void _DVCWeights_Execute( void* dvcWeights, void* data ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    _WeightsCalculator_Execute( self, data );
}

void _DVCWeights_Destroy( void* dvcWeights, void* data ) {
    DVCWeights* self = (DVCWeights*)dvcWeights;
    _WeightsCalculator_Destroy( self, data );
}

/*-------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

/** Get centroids of each voronoi region */
void _DVCWeights_GetCentroids3D( struct cell *cells,struct particle *pList,
                               int n, int m, int l,int nump,double vol){
    int i;
    int *count;

    count=(int *)malloc(sizeof(int)*nump);

    for(i=0;i<nump;i++){
        count[i] = 0;
        pList[ i ].cx = 0.0;
        pList[ i ].cy = 0.0;
        pList[ i ].cz = 0.0;    
    }
    for(i=0;i<n*m*l;i++){
        pList[ cells[i].p ].cx += cells[i].x;
        pList[ cells[i].p ].cy += cells[i].y;
        pList[ cells[i].p ].cz += cells[i].z;
        count[ cells[i].p ]++;//for total volume of a cell
    }
    for(i=0;i<nump;i++){
        pList[ i ].w = count[i]*vol;
        if(count[i] != 0){
            pList[ i ].cx /= count[i];
            pList[ i ].cy /= count[i];
            pList[ i ].cz /= count[i];
        } 
    }
    free(count);
}
/** Get centroids of each voronoi region in 2D*/
void _DVCWeights_GetCentroids2D( struct cell *cells,struct particle *pList,
                                 int n, int m, int nump,double vol){
    int i;
    int *count;

    count=(int *)malloc(sizeof(int)*nump);
  
    for(i=0;i<nump;i++){
        count[i] = 0;
        pList[ i ].cx = 0.0;
        pList[ i ].cy = 0.0;
    }
    for(i=0;i<n*m;i++){
        pList[ cells[i].p ].cx += cells[i].x;
        pList[ cells[i].p ].cy += cells[i].y;
        count[ cells[i].p ]++;//for total volume of a cell
    }
    for(i=0;i<nump;i++){
        pList[ i ].w = count[i]*vol;
        if(count[i] != 0){
            pList[ i ].cx /= count[i];
            pList[ i ].cy /= count[i];
        } 
    }  
    free(count);
}

/** Claim a cell for a particle in the list */
void _DVCWeights_ClaimCells(struct chain *bbchain,struct cell *cells,struct particle *pList,int p_i){
    int i,count;
    int cell_num0;
    double x0,y0,x1,y1,x2,y2,z0,z1,z2,dist1;
    struct chain *bchain = &(bbchain[p_i]);
    int *temp;

    count = 0;
    bchain->numclaimed = 0;


    for(i=0;i<bchain->sizeofboundary;i++){
        cell_num0 = bchain->new_bound_cells[i];// cell number we are trying to claim
        if(cells[cell_num0].p == -1){//if cell unowned then claim cell
            /* This is the bit needed for mallocing */
            /* do a test here to see if we need to realloc bchain->new_claimed_cells and bchain->new_bound_cells */
            if( count > bchain->new_claimed_cells_malloced - 1 ){
                temp = (int *)realloc( bchain->new_claimed_cells, (bchain->new_claimed_cells_malloced + DVC_INC)*sizeof(int) );
                bchain->new_claimed_cells = temp;
                bchain->new_claimed_cells_malloced += DVC_INC;
                temp = (int *)realloc( bchain->new_bound_cells, (bchain->new_bound_cells_malloced + DVC_INC)*sizeof(int) );
                bchain->new_bound_cells = temp;
                bchain->new_bound_cells_malloced += DVC_INC;      
            }
            /* end of bit needed for mallocing */
            bchain->new_claimed_cells[count] = cell_num0;
            bchain->numclaimed++;
            count++;
            cells[cell_num0].p = p_i;// this cell is now owned by particle p_i
        }
        else{
            if(cells[cell_num0].p != p_i){
                //we need a contest between particles for the cell.
                x2 = (pList)[p_i].x;
                y2 = (pList)[p_i].y;
                z2 = (pList)[p_i].z;
                x1 = (pList)[cells[cell_num0].p].x;
                y1 = (pList)[cells[cell_num0].p].y;
                z1 = (pList)[cells[cell_num0].p].z;
                x0 = cells[cell_num0].x;
                y0 = cells[cell_num0].y;
                z0 = cells[cell_num0].z;
        
                dist1 = _DVCWeights_DistanceTest(x0,y0,z0,x1,y1,z1,x2,y2,z2);
                if(dist1 > 0.0){
                    bchain->new_claimed_cells[count] = cell_num0;
                    bchain->numclaimed++;
                    count++;
                    cells[cell_num0].p = p_i;// this cell is now owned by particle p_i
                }
            }//if
        }//else
    }
    bchain->new_claimed_cells[count] = -1;// end of list
}

/** Claim a cell for a particle in the list  in 2D*/
void _DVCWeights_ClaimCells2D(struct chain *bbchain,struct cell *cells,struct particle *pList,int p_i){
    int i,count;
    int cell_num0;
    double x0,y0,x1,y1,x2,y2,dist1;
    struct chain *bchain = &(bbchain[p_i]);
    int *temp;

    count = 0;
    bchain->numclaimed = 0;


    for(i=0;i<bchain->sizeofboundary;i++){
        cell_num0 = bchain->new_bound_cells[i];// cell number we are trying to claim
        if(cells[cell_num0].p == -1){//if cell unowned then claim cell
            /* This is the bit needed for mallocing */
            /* do a test here to see if we need to realloc bchain->new_claimed_cells and bchain->new_bound_cells */
            if( count > bchain->new_claimed_cells_malloced - 1 ){
                temp = (int *)realloc( bchain->new_claimed_cells, (bchain->new_claimed_cells_malloced + DVC_INC)*sizeof(int) );
                bchain->new_claimed_cells = temp;
                bchain->new_claimed_cells_malloced += DVC_INC;
                temp = (int *)realloc( bchain->new_bound_cells, (bchain->new_bound_cells_malloced + DVC_INC)*sizeof(int) );
                bchain->new_bound_cells = temp;
                bchain->new_bound_cells_malloced += DVC_INC;      
            }
            /* end of bit needed for mallocing */
            bchain->new_claimed_cells[count] = cell_num0;
            bchain->numclaimed++;
            count++;
            cells[cell_num0].p = p_i;// this cell is now owned by particle p_i
        }
        else{
            if(cells[cell_num0].p != p_i){
                //we need a contest between particles for the cell.
                x2 = (pList)[p_i].x;
                y2 = (pList)[p_i].y;
                x1 = (pList)[cells[cell_num0].p].x;
                y1 = (pList)[cells[cell_num0].p].y;
                x0 = cells[cell_num0].x;
                y0 = cells[cell_num0].y;
        
                dist1 = _DVCWeights_DistanceTest2D(x0,y0,x1,y1,x2,y2);
                if(dist1 > 0.0){
                    bchain->new_claimed_cells[count] = cell_num0;
                    bchain->numclaimed++;
                    count++;
                    cells[cell_num0].p = p_i;// this cell is now owned by particle p_i
                }
            }//if
        }//else
    }
    bchain->new_claimed_cells[count] = -1;// end of list
}

/** Reset the values in the cells so that the grid over the element can be used again */
void _DVCWeights_ResetGrid3D(struct cell *cells, int n){
    int i;

    for(i=0;i<n;i++){
        (cells)[i].p = -1;
        (cells)[i].done = 0;
    }
}

/** Reset the values in the cells so that the grid over the element can be used again, in 2D */
void _DVCWeights_ResetGrid2D(struct cell *cells, int n){
    int i;

    for(i=0;i<n;i++){
        (cells)[i].p = -1;
        (cells)[i].done = 0;
    }
}

/** Update the list of the cells on the boundary of the growing voronoi cells 
    Add new particles on the boundary, and remove particles no longer on the boundary.*/
void _DVCWeights_UpdateBchain(struct chain *bbchain,struct cell *cells,int p_i){
    int i,k,count;
    int cell_num0,cell_num[6],cell_num1;
    struct chain *bchain = &(bbchain[p_i]);
    int *temp;

    count = 0;
    bchain->sizeofboundary = 0;
    for(i=0;i<bchain->numclaimed;i++){
        cell_num0 =bchain->new_claimed_cells[i];

        cell_num[0] = cells[cell_num0].S;    
        cell_num[1] = cells[cell_num0].N;
        cell_num[2] = cells[cell_num0].E;
        cell_num[3] = cells[cell_num0].W;
        cell_num[4] = cells[cell_num0].U;
        cell_num[5] = cells[cell_num0].D;

        for(k=0;k<6;k++){
            cell_num1 = cell_num[k];
            // if cell does not already belong to the particle and hasn't been
            // marked as being done then add it to new boundary array and mark it
            // as done
            if(cell_num1 != -2){
                if(cells[cell_num1].p != p_i && cells[cell_num1].done != 1){
                    /* This is the bit needed for mallocing */     
                    /* do a test here to see if we need to realloc bchain->new_claimed_cells and bchain->new_bound_cells */
                    if( count > bchain->new_bound_cells_malloced - 1 ){
                        temp = (int *)realloc( bchain->new_claimed_cells, (bchain->new_claimed_cells_malloced + DVC_INC)*sizeof(int) );
                        bchain->new_claimed_cells = temp;
                        bchain->new_claimed_cells_malloced += DVC_INC;
                        temp = (int *)realloc( bchain->new_bound_cells, (bchain->new_bound_cells_malloced + DVC_INC)*sizeof(int) );
                        bchain->new_bound_cells = temp;
                        bchain->new_bound_cells_malloced += DVC_INC; 
                    }
                    /* end of bit needed for mallocing */
                    bchain->new_bound_cells[count] = cell_num1;
                    bchain->sizeofboundary++;
                    count++;
                    cells[cell_num1].done = 1;
                }//if
            }//if cell_num1
        }//for k
    }//for
    // reset the done flags back to zero for next time
    for(i=0;i<count;i++){
        cells[  bchain->new_bound_cells[i]  ].done = 0;
    }
}

/** Update the list of the cells on the boundary of the growing voronoi cells 
    Add new particles on the boundary, and remove particles no longer on the boundary. In 2D*/
void _DVCWeights_UpdateBchain2D(struct chain *bbchain,struct cell *cells,int p_i){
    int i,k,count;
    int cell_num0,cell_num[4],cell_num1;
    struct chain *bchain = &(bbchain[p_i]);
    int *temp;

    count = 0;
    bchain->sizeofboundary = 0;
    for(i=0;i<bchain->numclaimed;i++){
        cell_num0 =bchain->new_claimed_cells[i];

        cell_num[0] = cells[cell_num0].S;    
        cell_num[1] = cells[cell_num0].N;
        cell_num[2] = cells[cell_num0].E;
        cell_num[3] = cells[cell_num0].W;

        for(k=0;k<4;k++){
            cell_num1 = cell_num[k];
            // if cell does not already belong to the particle and hasn't been
            // marked as being done then add it to new boundary array and mark it
            // as done
            if(cell_num1 != -2){
                if(cells[cell_num1].p != p_i && cells[cell_num1].done != 1){
                    /* This is the bit needed for mallocing */     
                    /* do a test here to see if we need to realloc bchain->new_claimed_cells and bchain->new_bound_cells */
                    if( count > bchain->new_bound_cells_malloced - 1 ){
                        temp = (int *)realloc( bchain->new_claimed_cells, (bchain->new_claimed_cells_malloced + DVC_INC)*sizeof(int) );
                        bchain->new_claimed_cells = temp;
                        bchain->new_claimed_cells_malloced += DVC_INC;
                        temp = (int *)realloc( bchain->new_bound_cells, (bchain->new_bound_cells_malloced + DVC_INC)*sizeof(int) );
                        bchain->new_bound_cells = temp;
                        bchain->new_bound_cells_malloced += DVC_INC; 
                    }
                    /* end of bit needed for mallocing */
                    bchain->new_bound_cells[count] = cell_num1;
                    bchain->sizeofboundary++;
                    count++;
                    cells[cell_num1].done = 1;
                }//if
            }//if cell_num1
        }//for k
    }//for
    // reset the done flags back to zero for next time
    for(i=0;i<count;i++){
        cells[  bchain->new_bound_cells[i]  ].done = 0;
    }
}


/** Construct the grid over the element. This grid is fixed. 
    (n, m, l) are (z, x, y) grid resolutions respectively.*/
void _DVCWeights_ConstructGrid(struct cell **cell_list, int n, int m, int l,
                               double x0,double y0,double z0,double x1,double y1,double z1){
    struct cell *cells;
    int i,j,k;
    double dx,dy,dz,Dx,Dy,Dz,X,Y,Z;

    cells = malloc(n*m*l*sizeof(struct cell));
    for(i=0;i<l*m*n;i++){
        cells[i].S = -2;
        cells[i].N = -2;
        cells[i].E = -2;
        cells[i].W = -2;
        cells[i].U = -2;
        cells[i].D = -2;
        cells[i].p = -1;
        cells[i].done = 0;
    }
    for(k=0;k<n;k++){
        for(i=0;i<l*(m-1);i++){
            cells[i+k*l*m].N=i+k*l*m+l;
            cells[i+l+k*l*m].S = i+k*l*m;
        }
    }
    for(k=0;k<n-1;k++){
        for(i=0;i<l*m;i++){
            cells[i+k*l*m].U=i+k*l*m+l*m;
            cells[i+k*l*m+l*m].D = i+k*l*m;
        }
    }
  
    Dx = x1-x0; Dy = y1-y0; Dz = z1-z0;
    dx = Dx/l;  dy = Dy/m;  dz = Dz/n;
    Z = z0 - dz/2.0;
    for(k=0;k<n;k++){
        Z = Z + dz;
        Y = y0 - dy/2.0;
        for(j=0;j<m;j++){
            Y = Y + dy;
            X = x0 - dx/2.0;
            for(i=0;i<l;i++){
                X=X+dx;
                cells[i+l*j+k*l*m].x = X;
                cells[i+l*j+k*l*m].y = Y;
                cells[i+l*j+k*l*m].z = Z;
                if(i!= l-1){
                    cells[i+l*j+k*l*m].E = i+l*j+1+k*l*m;
                    cells[i+l*j+1+k*l*m].W = i+l*j+k*l*m;
                }
            }//x
        }//y
    }//z
    *cell_list = cells; 
}


/** Construct the grid over the element. This grid is fixed. 
    (m, l) are (x, y) grid resolutions respectively.*/
void _DVCWeights_ConstructGrid2D(struct cell **cell_list, int m, int l,
                                 double x0,double y0,double x1,double y1){
    struct cell *cells;
    int i,j;
    double dx,dy,Dx,Dy,X,Y;

    cells = malloc(m*l*sizeof(struct cell));
    for(i=0;i<l*m;i++){
        cells[i].S = -2;
        cells[i].N = -2;
        cells[i].E = -2;
        cells[i].W = -2;
        cells[i].p = -1;
        cells[i].done = 0;
    }
    for(i=0;i<l*(m-1);i++){
        cells[i].N=i+l;
        cells[i+l].S = i;
    }
    Dx = x1-x0; Dy = y1-y0;
    dx = Dx/l;  dy = Dy/m;
    Y = y0 - dy/2.0;
    for(j=0;j<m;j++){
        Y = Y + dy;
        X = x0 - dx/2.0;
        for(i=0;i<l;i++){
            X=X+dx;
            cells[i+l*j].x = X;
            cells[i+l*j].y = Y;
            if(i!= l-1){
                cells[i+l*j].E = i+l*j+1;
                cells[i+l*j+1].W = i+l*j;
            }
        }
    }
    *cell_list = cells; 
}

/** Calculate the sqaure of the distance between two points */
double _DVCWeights_DistanceSquared(double x0, double y0, double z0, double x1, double y1, double z1){
    return (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0);
}

/** Calculate the sqaure of the distance between two points in 2D*/
double _DVCWeights_DistanceSquared2D(double x0, double y0,double x1, double y1){
    return (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0);
}
double _DVCWeights_DistanceTest2D(double x0, double y0, double x1, double y1,double x2, double y2){
    return (x1+x2-x0-x0)*(x1-x2) + (y1+y2-y0-y0)*(y1-y2);
}
double _DVCWeights_DistanceTest(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2){
    return (x1+x2-x0-x0)*(x1-x2) + (y1+y2-y0-y0)*(y1-y2) + (z1+z2-z0-z0)*(z1-z2);
}
/** Allocate the internal structs for the bchain (boundary chain) and the plist (particle list) */
void _DVCWeights_InitialiseStructs( DVCWeights* self, int nump){
    int i;
    if (nump > self->bchainSize) {
        _DVCWeights_Delete_bchain(self);
        if( (self->bchain = (struct chain *)malloc( nump*sizeof(struct chain ) )) == 0){
            Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVC_Weights" ),
                              "No memory for bchain in '%s'\nCannot continue.\n", __func__);
        }
        for(i=0;i<nump;i++){
            (self->bchain)[i].new_claimed_cells = (int *)malloc(DVC_INC*sizeof(int));
            (self->bchain)[i].new_bound_cells = (int *)malloc(DVC_INC*sizeof(int));
            (self->bchain)[i].new_claimed_cells_malloced = DVC_INC;
            (self->bchain)[i].new_bound_cells_malloced = DVC_INC;
        }
        self->bchainSize = nump;
    }

    if (nump > self->plistSize) {
        _DVCWeights_Delete_plist(self);
        if( (self->pList = (struct particle *)malloc( nump*sizeof(struct particle ) )) == 0){
            Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVC_Weights" ),
                              "No memory for pList in '%s'\nCannot continue.\n", __func__);
        }
        self->plistSize = nump;
    }

    /* Initialise all particle values to zero */
    for (i = 0; i < nump; i++) {
        (self->pList)[i].x  = 0; (self->pList)[i].y  = 0; (self->pList)[i].z  = 0;
        (self->pList)[i].cx = 0; (self->pList)[i].cy = 0; (self->pList)[i].cz = 0;
        (self->pList)[i].w = 0;
        (self->pList)[i].index = 0;
    }
}
/** Allocate the internal structs for the bchain (boundary chain) and the plist (particle list) */
void _DVCWeights_InitialiseStructs2D( DVCWeights* self, int nump){
    int i;
    if (nump > self->bchainSize) {
        _DVCWeights_Delete_bchain(self);
        if( (self->bchain = (struct chain *)malloc( nump*sizeof(struct chain ) )) == 0){
            Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVC_Weights" ),
                              "No memory for bchain in '%s'\nCannot continue.\n", __func__);
        }
        //
        for(i=0;i<nump;i++){
            (self->bchain)[i].new_claimed_cells = (int *)malloc(DVC_INC*sizeof(int));
            (self->bchain)[i].new_bound_cells = (int *)malloc(DVC_INC*sizeof(int));
            (self->bchain)[i].new_claimed_cells_malloced = DVC_INC;
            (self->bchain)[i].new_bound_cells_malloced = DVC_INC;
        }
        self->bchainSize = nump;
    }
    
    if (nump > self->plistSize) {
        _DVCWeights_Delete_plist(self);
        if( (self->pList = (struct particle *)malloc( nump*sizeof(struct particle ) )) == 0){
            Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVC_Weights" ),
                              "No memory for pList in '%s'\nCannot continue.\n", __func__);
        }
        self->plistSize = nump;
    }

    /* Initialise all particle values to zero */
    for (i = 0; i < nump; i++) {
        (self->pList)[i].x  = 0; (self->pList)[i].y  = 0;
        (self->pList)[i].cx = 0; (self->pList)[i].cy = 0;
        (self->pList)[i].w = 0;
        (self->pList)[i].index = 0;
    }
}

/** Create the Voronoi diagram by growing the voronoi cells from the particle locations.
    All the parameters passed into _DVCWeights_CreateVoronoi
    must be initialised already. */
void _DVCWeights_CreateVoronoi3D( struct chain *bchain, struct particle *pList, struct cell *cells, 
                                double dx, double dy, double dz,
                                int nump,
                                int numx, int numy, int numz, 
                                double BBXMIN, double BBXMAX, 
                                double BBYMIN, double BBYMAX,
                                double BBZMIN, double BBZMAX){
    int i,j,k,l;
    int count;
    int claimed;

    for(i=0;i<nump;i++){
        k = ((pList)[i].x - BBXMIN)/dx;
        j = ((pList)[i].y - BBYMIN)/dy;
        l = ((pList)[i].z - BBZMIN)/dz;
        /* If a particle is exactly on the border then make sure it is in a valid cell inside the element */
        if (k == numx){  k--; }
        if (j == numy) { j--; }
        if (l == numz) { l--; }
        (cells)[k+j*numx+l*numx*numy].p = i; //particle number i
        (bchain)[i].numclaimed = 1;// number of most recently claimed cells
        (bchain)[i].sizeofboundary = 0;
        (bchain)[i].totalclaimed = 1;// total of claimed cells so far.
        (bchain)[i].done = 0;
        (bchain)[i].index = k+j*numx+l*numx*numy;// ith particle is in cell # k+j*numx
        (bchain)[i].new_claimed_cells[0] = k+j*numx+l*numx*numy; 
        // ith particle has just claimed cell number k+j*numx+l*numx*numy
        (bchain)[i].new_claimed_cells[1] = -1;// denotes end of claimed_cells list
        // when we have finished claiming cells we call this function.
        _DVCWeights_UpdateBchain(bchain,cells,i);
    }
    count = i;// number of particles
    claimed = 1;
      
    while(claimed != 0){
        claimed = 0 ;
        for(i=0;i<count;i++){
            _DVCWeights_ClaimCells(bchain,cells,pList,i);
            claimed += (bchain)[i].numclaimed;
            _DVCWeights_UpdateBchain(bchain,cells,i);
        }
    }//while
}

/** Create the Voronoi diagram by growing the voronoi cells from the particle locations.
    All the parameters passed into _DVCWeights_CreateVoronoi2D
    must be initialised already. */
void _DVCWeights_CreateVoronoi2D( struct chain *bchain, struct particle *pList, struct cell *cells, 
                                  double dx, double dy,
                                  int nump,
                                  int numx, int numy,
                                  double BBXMIN, double BBXMAX, 
                                  double BBYMIN, double BBYMAX){
    int i,j,k;
    int claimed;

    for(i=0;i<nump;i++){
        k = ((pList)[i].x - BBXMIN)/dx;
        j = ((pList)[i].y - BBYMIN)/dy;
        /* If particle is on the border exactly, make sure it is
           put in a valid cell inside the element */
        if (k == numx){
            k--;
        }
        if ( j == numy) {
            j--;
        }
        
        (cells)[k+j*numx].p = i; //particle number i
      
        (bchain)[i].numclaimed = 1;// number of most recently claimed cells
        (bchain)[i].sizeofboundary = 0;
        (bchain)[i].totalclaimed = 1;// total of claimed cells so far.
        (bchain)[i].done = 0;
        (bchain)[i].index = k+j*numx;// ith particle is in cell # k+j*numx
        (bchain)[i].new_claimed_cells[0] = k+j*numx; // ith particle has just claimed cell number k+j*numx
        (bchain)[i].new_claimed_cells[1] = -1;// denotes end of claimed_cells list
        // when we have finished claiming cells we call this function.
        _DVCWeights_UpdateBchain2D( bchain, cells, i);
    }//nump
   
    claimed = 1;

    while(claimed != 0){
        claimed = 0 ;
        for(i=0;i<nump;i++){
            _DVCWeights_ClaimCells2D( bchain, cells, pList, i);
            claimed += (bchain)[i].numclaimed;
            _DVCWeights_UpdateBchain2D( bchain, cells, i);
        }
    }//while
}


/* Calculate the integration weighting for each particle by contructing
   a voronoi diagram in an element in 3D*/
void _DVCWeights_Calculate3D( void* dvcWeights, void* _swarm, Cell_LocalIndex lCell_I ) {
    DVCWeights*             self            = (DVCWeights*)  dvcWeights;
    Swarm*                       swarm           = (Swarm*) _swarm;
    Particle_InCellIndex         cParticleCount;
    IntegrationPoint**           particle;
    double dx,dy,dz,da;
    static struct cell *cells;// the connected grid
    struct particle *pList;// particle List
    struct chain *bchain;//boundary chain
    int nump,numx,numy,numz;
    double BBXMIN = -1.0; // the ranges of the local coordinates of a FEM cell.
    double BBXMAX = 1.0;
    double BBYMIN = -1.0;
    double BBYMAX = 1.0;
    double BBZMIN = -1.0;
    double BBZMAX = 1.0;
    int i;

    numx = self->resX;
    numy = self->resY;
    numz = self->resZ;

    nump = cParticleCount = swarm->cellParticleCountTbl[lCell_I];

    Journal_Firewall( nump , Journal_Register( Error_Type, (Name)"DVC_Weights" ), "Error in %s: Problem has an under resolved cell (Cell Id = %d), add more particles to your model\n", __func__, lCell_I );

    dx = (BBXMAX - BBXMIN)/numx;
    dy = (BBYMAX - BBYMIN)/numy;
    dz = (BBZMAX - BBZMIN)/numz;
    da = dx*dy*dz;
        
    // Construct the grid for the Voronoi cells only once.
    // If we wanted to call this function again during a job with a different resolution
    // then we should destroy the grid once we have looped through the whole mesh.
    // I am assuming we are not going to do that for now.
    // Easy to implement this anyway, if needed.
    if(!self->visiteddvcweights){
        /* The DVCWeights class should really be a class the next level up here */
        /* We should be able to swap out the WeightsCalculator_CalculateAll instead of just setting
           a pointer inside that function */
        self->visiteddvcweights++;
        _DVCWeights_ConstructGrid(&cells,numz,numy,numx,BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);
    }
        
        
    // init the data structures
    _DVCWeights_InitialiseStructs( self, nump );
    bchain = self->bchain;
    pList  = self->pList;
    _DVCWeights_ResetGrid3D(cells,numz*numy*numx);
        
    particle = (IntegrationPoint**)malloc(nump*sizeof(IntegrationPoint*));
        
    // initialize the particle positions to be the local coordinates of the material swarm particles
    // I am assuming the xi's (local coords) are precalculated somewhere and get reset based on material
    // positions each time step.
    for(i=0;i<nump;i++){
              
        particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, i );
        pList[i].x = particle[i]->xi[0];
        pList[i].y = particle[i]->xi[1];
        pList[i].z = particle[i]->xi[2];
              
    }
    _DVCWeights_CreateVoronoi3D( bchain, pList, cells, dx, dy, dz, nump, numx, numy, numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);
    _DVCWeights_GetCentroids3D( cells, pList,numz,numy,numx,nump,da);
    // We are setting the integration points to be the centroids of the Voronoi regions here and
    // the weight is the volume of each Voronoi region.
    for(i=0;i<nump;i++){

        particle[i]->xi[0] = pList[i].cx;
        particle[i]->xi[1] = pList[i].cy;
        particle[i]->xi[2] = pList[i].cz;
        particle[i]->weight = pList[i].w;

    }   
    free(particle);
}

/* Calculate the integration weighting for each particle by contructing
   a voronoi diagram in an element in 2D*/
void _DVCWeights_Calculate2D( void* dvcWeights, void* _swarm, Cell_LocalIndex lCell_I ) {
    DVCWeights*             self            = (DVCWeights*)  dvcWeights;
    Swarm*                       swarm           = (Swarm*) _swarm;
    Particle_InCellIndex         cParticleCount;
    IntegrationPoint**           particle;
    double dx,dy,da;
    static struct cell *cells;// the connected grid
    struct particle *pList;
    struct chain *bchain;
    int nump,numx,numy;
    double BBXMIN = -1.0; // the ranges of the local coordinates of a FEM cell.
    double BBXMAX = 1.0;
    double BBYMIN = -1.0;
    double BBYMAX = 1.0;
    int i;

    numx = self->resX;
    numy = self->resY;

    nump = cParticleCount = swarm->cellParticleCountTbl[lCell_I];

    Journal_Firewall( nump , Journal_Register( Error_Type, (Name)"DVC_Weights" ), "Error in %s: Problem has an under resolved cell (Cell Id = %d), add more particles to your model\n", __func__, lCell_I );

    dx = (BBXMAX - BBXMIN)/numx;
    dy = (BBYMAX - BBYMIN)/numy;
    da = dx*dy;
        
    // Construct the grid for the Voronoi cells only once.
    // If we wanted to call this function again during a job with a different resolution
    // then we should destroy the grid once we have looped through the whole mesh.
    // I am assuming we are not going to do that for now.
    // Easy to implement this anyway, if needed.
    if(!self->visiteddvcweights){
        /* The DVCWeights class should really be a class the next level up here */
        /* We should be able to swap out the WeightsCalculator_CalculateAll instead of just setting
           a pointer inside that function */
        self->visiteddvcweights++;
        _DVCWeights_ConstructGrid2D(&cells,numy,numx,BBXMIN,BBYMIN,BBXMAX,BBYMAX);
    }
        
        
    // init the data structures
    _DVCWeights_InitialiseStructs2D( self, nump );
    bchain = self->bchain;
    pList  = self->pList;

    _DVCWeights_ResetGrid2D(cells,numy*numx);
        
    particle = (IntegrationPoint**)malloc(nump*sizeof(IntegrationPoint*));
        
    // initialize the particle positions to be the local coordinates of the material swarm particles
    // I am assuming the xi's (local coords) are precalculated somewhere and get reset based on material
    // positions each time step.
    for(i=0;i<nump;i++){
              
        particle[i] = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, lCell_I, i );
        pList[i].x = particle[i]->xi[0];
        pList[i].y = particle[i]->xi[1];
              
    }
    _DVCWeights_CreateVoronoi2D( bchain, pList, cells, dx, dy, nump, numx, numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);
    _DVCWeights_GetCentroids2D( cells, pList,numy,numx,nump,da);
    // We are setting the integration points to be the centroids of the Voronoi regions here and
    // the weight is the volume of each Voronoi region.
    for(i=0;i<nump;i++){

        particle[i]->xi[0] = pList[i].cx;
        particle[i]->xi[1] = pList[i].cy;
        particle[i]->weight = pList[i].w;

    }   

    free(particle);

}

void _DVCWeights_Calculate( void* dvcWeights, void* _swarm, Cell_LocalIndex lCell_I ){
    Swarm* swarm = (Swarm*) _swarm;

    if(swarm->dim == 3){
        _DVCWeights_Calculate3D( dvcWeights, _swarm, lCell_I);
    }
    else {
        _DVCWeights_Calculate2D( dvcWeights, _swarm, lCell_I);
    }
}
/*-------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


