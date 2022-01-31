/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <StGermain/libStGermain/src/StGermain.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcu/pcu.h"
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include "PICellerator/PopulationControl/src/PopulationControl.h"
#include "PICellerator/Weights/src/Weights.h"

/* the ranges of the local coordinates of a FEM cell.*/
const double BBXMIN = -1.0; 
const double BBXMAX = 1.0;
const double BBYMIN = -1.0;
const double BBYMAX = 1.0;
const double BBZMIN = -1.0;
const double BBZMAX = 1.0;

typedef struct {
   struct cell*       cells;   /* the 3D connected grid */ 
   struct particle*   pList;   /* 3D particle List */
   struct chain*      bchain;  /*3D boundary chain */
   struct cell*     cells2D; /* the 2D connected grid */
   struct particle* pList2D; /*2D particle List */
   struct chain*      bchain2D;/*2D boundary chain */
   unsigned int       numx,numy,numz;
   unsigned int       nump,nump2D;
   unsigned int       px, py, pz;
   double             dx,dy,dz;
   double             da,da2D;
   DVCWeights*        dvcWeights;
} DVCWeightsSuiteData;

void DVCWeightsSuite_Setup( DVCWeightsSuiteData* data ) {
   data->cells = NULL;
   data->pList = NULL;
   data->bchain = NULL;
   data->cells2D = NULL;
   data->pList2D = NULL;
   data->bchain2D = NULL;

   /*Define the resolution */
   data->numx = 10;/* the data are hard-coded at this time, so don't change this from 10 */
   data->numy = 10;/* the data are hard-coded at this time, so don't change this from 10 */
   data->numz = 10;/* the data are hard-coded at this time, so don't change this from 10 */

   /*Define the number of particles */
   data->px = 2;
   data->py = 2; 
   data->pz = 2;
   //data->nump = data->px * data->py * data->pz;
   data->nump = 10; /* the data are hard-coded at this time, so don't change this from 10 */
   //data->nump2D = data->px * data->py;
   data->nump2D = 10; /* the data are hard-coded at this time, so don't change this from 10 */

   data->dx = (BBXMAX - BBXMIN)/data->numx;
   data->dy = (BBYMAX - BBYMIN)/data->numy;
   data->dz = (BBZMAX - BBZMIN)/data->numz;
   data->da = data->dx*data->dy*data->dz;
   data->da2D = data->dx*data->dy;
   
   int res[3] = {10,10,10};
   data->dvcWeights = DVCWeights_New( "", (int*) res );
}

void DVCWeightsSuite_Teardown( DVCWeightsSuiteData* data ) {
   Stg_Class_Delete( data->dvcWeights );
   if(data->cells) { free(data->cells); }
   if(data->cells2D) { free(data->cells2D); }
}

/* Do the testing of the 3D functions*/
void DVCWeightsSuite_TestConstructGrid( DVCWeightsSuiteData* data ) {
   FILE         *fp;
   unsigned int ii = 0;
   const char*  gridFilename = "DVCWeightsSuite_testConstructGrid.txt";
   char         expectedGridFilename[PCU_PATH_MAX];
   //int passed = 0;
   char         line[100];
   float        x,y,z,dx,dy,dz;
   int          N,S,E,W,U,D;
   float        tol = 1e-6;
   /* We'll use an expected file for this test, as visual checking of the correct parameters is probably
    *  most logical way to maintain it */

   _DVCWeights_ConstructGrid(&data->cells, data->numz, data->numy, data->numx, BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);      
   pcu_filename_expected( gridFilename, expectedGridFilename );/* get the expected filename to open */
   fp = fopen(expectedGridFilename,"r");

   if( !fp ) {
      pcu_check_true(0);
      Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVCWeightsSuite_TestConstructGrid" ),"expected test File %s Not Found in function %s\n" ,expectedGridFilename, __func__);
   }
   else {
     /* Print out the grid */
      for (ii = 0; ii < (data->numx * data->numy * data->numz); ii++ ) {
         pcu_check_true( data->cells[ii].p == -1 );   /* Particle index: shouldn't be set up yet */
         pcu_check_true( data->cells[ii].done == 0 );
         fgets(line,100,fp);
         fgets(line,100,fp);
         sscanf(line,"\t\tValues: (N: %d, S: %d, E: %d, W: %d, U: %d, D: %d)\n",&N,&S,&E,&W,&U,&D);
         pcu_check_true( data->cells[ii].N == N );
         pcu_check_true( data->cells[ii].S == S );
         pcu_check_true( data->cells[ii].E == E );
         pcu_check_true( data->cells[ii].W == W );
         pcu_check_true( data->cells[ii].U == U );
         pcu_check_true( data->cells[ii].D == D );
         fgets(line,100,fp);
         sscanf(line,"\t\tCoords: ( %f, %f, %f)\n",&x,&y,&z);
         dx = (data->cells[ii].x - x)*(data->cells[ii].x - x); 
         dy = (data->cells[ii].y - y)*(data->cells[ii].y - y); 
         dz = (data->cells[ii].z - z)*(data->cells[ii].z - z); 
         pcu_check_true( dx < tol );
         pcu_check_true( dy < tol );
         pcu_check_true( dz < tol );
      }
   }
   fclose(fp);
}

void DVCWeightsSuite_TestInitialiseStructs( DVCWeightsSuiteData* data ) {
   unsigned int ii;   

   _DVCWeights_InitialiseStructs( data->dvcWeights, data->nump);
   data->bchain = data->dvcWeights->bchain;
   data->pList  = data->dvcWeights->pList;
   
   for (ii = 0; ii < data->nump; ii++) {
      pcu_check_true( data->bchain[ii].new_claimed_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain[ii].new_bound_cells_malloced == DVC_INC );
   }
}

void DVCWeightsSuite_TestResetGrid( DVCWeightsSuiteData* data ) {
   unsigned int i;
   
   _DVCWeights_ConstructGrid(&data->cells, data->numz, data->numy, data->numx, BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);      
   _DVCWeights_ResetGrid3D(data->cells, data->numz*data->numy*data->numx);

   for ( i = 0; i < data->numz*data->numy*data->numx; i++) {
      pcu_check_true( data->cells[i].p == -1 );
      pcu_check_true( data->cells[i].done == 0 );
   }
}

void _DVCWeightsSuite_InitialiseParticleCoords( DVCWeightsSuiteData* data ) {
   int i;

   /*Initialise particle coords */
   i = 0;
   data->pList[i].x = -0.64320939127356 ; i++;
   data->pList[i].x = -0.57575625646859 ; i++;
   data->pList[i].x = -0.44866901542991 ; i++;
   data->pList[i].x = -0.45453263074160 ; i++;
   data->pList[i].x = -0.56068608816713 ; i++;
   data->pList[i].x = -0.31215981673449 ; i++;
   data->pList[i].x = -0.94066464062780 ; i++;
   data->pList[i].x = +0.05528775136918 ; i++;
   data->pList[i].x = -0.62383012939245 ; i++;
   data->pList[i].x = +0.43985756300390 ; i++;
   
   i = 0;
   data->pList[i].y = -0.20064506959170 ; i++;
   data->pList[i].y = -0.87612858321518 ; i++;
   data->pList[i].y = -0.90448551066220 ; i++;
   data->pList[i].y = -0.26452634204179 ; i++;
   data->pList[i].y = -0.59652493428439 ; i++;
   data->pList[i].y = +0.29530759342015 ; i++;
   data->pList[i].y = +0.75698726624250 ; i++;
   data->pList[i].y = +0.01171378418803 ; i++;
   data->pList[i].y = -0.76168798375875 ; i++;
   data->pList[i].y = +0.27507007215172 ; i++;  

   i = 0;
   data->pList[i].z = -0.66680242400616; i++;
   data->pList[i].z = -0.89170031249523; i++;
   data->pList[i].z = -0.35793361812830; i++;
   data->pList[i].z = -0.55526655819267; i++;
   data->pList[i].z = -0.91176854260266; i++;
   data->pList[i].z = -0.70107179880142; i++;
   data->pList[i].z = +0.31083723250777; i++;
   data->pList[i].z = +0.83138376288116; i++;
   data->pList[i].z = -0.88380649406463; i++;
   data->pList[i].z = +0.76169187109917; i++;  

}

void DVCWeightsSuite_TestCreateVoronoi( DVCWeightsSuiteData* data ) {
   int i;

   _DVCWeights_ConstructGrid(&data->cells, data->numz, data->numy, data->numx, BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);      
   _DVCWeights_InitialiseStructs( data->dvcWeights, data->nump);
   data->bchain = data->dvcWeights->bchain;
   data->pList  = data->dvcWeights->pList;

   _DVCWeightsSuite_InitialiseParticleCoords( data );
   _DVCWeights_CreateVoronoi3D( data->bchain, data->pList, data->cells, data->dx, data->dy, data->dz,
      data->nump, data->numx, data->numy, data->numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);
   
   /* data->bchain changes */
   for (i = 0; i < data->nump; i++) {
      //pcu_check_true( data->bchain[i].index == (data->nump-1) ); //This value doesn't matter at the moment: also nump-1 is the wrong number anyway
      pcu_check_true( data->bchain[i].sizeofboundary == 0 );
      pcu_check_true( data->bchain[i].numclaimed == 0 );
      pcu_check_true( data->bchain[i].totalclaimed == 1 );
      pcu_check_true( data->bchain[i].new_bound_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain[i].new_claimed_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain[i].done == 0 );
   }
   /* particle values */
   for (i = 0; i < data->nump; i++) {
      pcu_check_true( (data->pList[i].cx == 0) && (data->pList[i].cy == 0) && (data->pList[i].cz == 0));
      pcu_check_true( data->pList[i].w == 0 );
   }
}

void DVCWeightsSuite_TestGetCentroids( DVCWeightsSuiteData* data ) {
   FILE         *fp;
   unsigned int i;
   const char*  centroidsFilename = "DVCWeightsSuite_testGetCentroids.txt";
   char         expectedCentroidsFilename[PCU_PATH_MAX];
   char         line[100];
   float        x,y,z,dx,dy,dz;
   float        tol = 1e-6;
   /* We'll use an expected file for this test, as visual checking of the correct parameters is probably
    *  most logical way to maintain it */
   
   _DVCWeights_ConstructGrid(&data->cells, data->numz, data->numy, data->numx, BBXMIN,BBYMIN,BBZMIN,BBXMAX,BBYMAX,BBZMAX);
   _DVCWeights_InitialiseStructs( data->dvcWeights, data->nump);
   data->bchain = data->dvcWeights->bchain;
   data->pList  = data->dvcWeights->pList;
   _DVCWeightsSuite_InitialiseParticleCoords( data );
   _DVCWeights_CreateVoronoi3D( data->bchain, data->pList, data->cells, data->dx, data->dy, data->dz,
      data->nump, data->numx, data->numy, data->numz, BBXMIN, BBXMAX, BBYMIN, BBYMAX, BBZMIN, BBZMAX);

   _DVCWeights_GetCentroids3D( data->cells, data->pList,data->numz,data->numy,data->numx,data->nump,data->da);

   pcu_filename_expected(centroidsFilename, expectedCentroidsFilename);/* get the expected filename to open */
   fp = fopen(expectedCentroidsFilename,"r");

   if(!fp){
      pcu_check_true(0);
      Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVCWeightsSuite_TestGetCentroids" ),"expected test File %s Not Found in function %s\n" ,expectedCentroidsFilename, __func__);
   }
   else{
      for (i = 0; i < data->nump; i++) {
         fgets(line,100,fp);
         fgets(line,100,fp);
         sscanf(line,"\t\tcoords: (x, y, z) = ( %f, %f, %f)\n",&x,&y,&z);
         dx = (data->pList[i].x -x)*(data->pList[i].x -x);
         dy = (data->pList[i].y -y)*(data->pList[i].y -y);
         dz = (data->pList[i].z -z)*(data->pList[i].z -z);
         pcu_check_true( dx < tol );
         pcu_check_true( dy < tol );
         pcu_check_true( dz < tol );

         fgets(line,100,fp);
         sscanf(line,"\t\tcentroids: (cx, cy, cz) = ( %f, %f, %f)\n",&x,&y,&z);
         dx = (data->pList[i].cx -x)*(data->pList[i].cx -x);
         dy = (data->pList[i].cy -y)*(data->pList[i].cy -y);
         dz = (data->pList[i].cz -z)*(data->pList[i].cz -z);
         pcu_check_true( dx < tol );
         pcu_check_true( dy < tol );
         pcu_check_true( dz < tol );
       
         fgets(line,100,fp);
         sscanf(line,"\t\tweight = %f\n",&x);
         dx = (data->pList[i].w -x)*(data->pList[i].w -x);
         pcu_check_true( dx < tol );
      }
   }
   fclose(fp);
}

void DVCWeightsSuite_TestDistanceSquared( DVCWeightsSuiteData* data ) {
   double particleDistance;      
   double particle0[3], particle1[3];

   particle0[0] = 0.5; particle0[1] = 0.5; particle0[2] = 0.5;
   particle1[0] = 0.25; particle1[1] = 0.25; particle1[2] = 0;       

   particleDistance = _DVCWeights_DistanceSquared(
      particle0[0], particle0[1], particle0[2],
      particle1[0], particle1[1], particle1[2]);

   pcu_check_true( particleDistance == 0.375 );
}

/* 2D Functions */
void DVCWeightsSuite_TestConstructGrid2D( DVCWeightsSuiteData* data ) {
   FILE         *fp;
   unsigned int ii = 0;
   const char*  gridFilename = "DVCWeightsSuite_testConstructGrid2D.txt";
   char         expectedGridFilename[PCU_PATH_MAX];
   char         line[100];
   float        x,y,dx,dy;
   int          N,S,E,W;
   float        tol = 1e-6;

   _DVCWeights_ConstructGrid2D(&data->cells2D,data->numy,data->numx, BBXMIN,BBYMIN,BBXMAX,BBYMAX);      

   pcu_filename_expected( gridFilename, expectedGridFilename );/* get the expected filename to open */
   fp = fopen(expectedGridFilename,"r");

   if(!fp){
      pcu_check_true(0);
      Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVCWeightsSuite_TestConstructGrid2D" ),"expected test File %s Not Found in function %s\n" ,expectedGridFilename, __func__);
   }
   else{
      /* Print out the grid */
      for (ii = 0; ii < (data->numx * data->numy); ii++ ) {
         pcu_check_true( data->cells2D[ii].p == -1 );   /* Particle index: shouldn't be set up yet */
         pcu_check_true( data->cells2D[ii].done == 0 );
         fgets(line,100,fp);  fgets(line,100,fp);
         sscanf(line,"\t\tValues: (N: %d, S: %d, E: %d, W: %d)\n",&N,&S,&E,&W);
         pcu_check_true( data->cells2D[ii].N == N );
         pcu_check_true( data->cells2D[ii].S == S );
         pcu_check_true( data->cells2D[ii].E == E );
         pcu_check_true( data->cells2D[ii].W == W );
         fgets(line,100,fp);
         sscanf(line,"\t\tCoords: ( %f, %f)\n",&x,&y);
         dx = (data->cells2D[ii].x - x)*(data->cells2D[ii].x - x); 
         dy = (data->cells2D[ii].y - y)*(data->cells2D[ii].y - y); 
         pcu_check_true( dx < tol );
         pcu_check_true( dy < tol );
      }
   }
   fclose(fp);
}

void DVCWeightsSuite_TestInitialiseStructs2D( DVCWeightsSuiteData* data ) {
   unsigned int ii;   

   _DVCWeights_InitialiseStructs2D( data->dvcWeights, data->nump);
   data->bchain2D = data->dvcWeights->bchain;
   data->pList2D  = data->dvcWeights->pList;

   for (ii = 0; ii < data->nump2D; ii++) {
      pcu_check_true( data->bchain2D[ii].new_claimed_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain2D[ii].new_bound_cells_malloced == DVC_INC );
   }
}

void DVCWeightsSuite_TestResetGrid2D( DVCWeightsSuiteData* data ) {
   unsigned int   i;
   
   _DVCWeights_ConstructGrid2D(&data->cells2D,data->numy,data->numx, BBXMIN,BBYMIN,BBXMAX,BBYMAX);
   _DVCWeights_ResetGrid2D(data->cells2D,data->numx*data->numy);

   for ( i = 0; i < data->numx*data->numy; i++) {
      pcu_check_true( data->cells2D[i].p == -1 );
      pcu_check_true( data->cells2D[i].done == 0 );
   }
}

void _DVCWeightsSuite_InitialiseParticleCoords2D( DVCWeightsSuiteData* data ) {
   int i;

   /*Initialise particle coords */
   i = 0;
   data->pList2D[i].x = -0.55376385338604 ; i++;
   data->pList2D[i].x = -0.10488151479512 ; i++;
   data->pList2D[i].x = +0.13873003050685 ; i++;
   data->pList2D[i].x = -0.05016296077520 ; i++;
   data->pList2D[i].x = +0.90808211639524 ; i++;
   data->pList2D[i].x = -0.69216587487608 ; i++;
   data->pList2D[i].x = -0.62051867879927 ; i++;
   data->pList2D[i].x = +0.24515981227160 ; i++;
   data->pList2D[i].x = -0.83195694349706 ; i++;
   data->pList2D[i].x = +0.92260552570224 ; i++;
   
   i = 0;
   data->pList2D[i].y = -0.56640755850822 ; i++;
   data->pList2D[i].y = -0.01476682443172 ; i++;
   data->pList2D[i].y = -0.05242515634745 ; i++;
   data->pList2D[i].y = -0.93268149159849 ; i++;
   data->pList2D[i].y = +0.91700463462621 ; i++;
   data->pList2D[i].y = -0.47783328499645 ; i++;
   data->pList2D[i].y = -0.27844923641533 ; i++;
   data->pList2D[i].y = -0.66626300383359 ; i++;
   data->pList2D[i].y = +0.17968866135925 ; i++;
   data->pList2D[i].y = -0.58136314433068 ; i++;
   
}

void DVCWeightsSuite_TestCreateVoronoi2D( DVCWeightsSuiteData* data ) {
   int i;

   _DVCWeights_ConstructGrid2D(&data->cells2D,data->numy,data->numx, BBXMIN,BBYMIN,BBXMAX,BBYMAX);
   _DVCWeights_InitialiseStructs2D( data->dvcWeights, data->nump);
   data->bchain2D = data->dvcWeights->bchain;
   data->pList2D  = data->dvcWeights->pList;

   _DVCWeightsSuite_InitialiseParticleCoords2D( data );
   _DVCWeights_CreateVoronoi2D( data->bchain2D, data->pList2D, data->cells2D, data->dx, data->dy,
      data->nump2D, data->numx, data->numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);
   
   /* data->bchain2D changes */
   for (i = 0; i < data->nump2D; i++) {
      //pcu_check_true( data->bchain2D[i].index == (data->nump2D-1) );//This value doesn't matter at the moment: also nump2D-1 is the wrong number anyway
      pcu_check_true( data->bchain2D[i].sizeofboundary == 0 );
      pcu_check_true( data->bchain2D[i].numclaimed == 0 );
      pcu_check_true( data->bchain2D[i].totalclaimed == 1 );
      pcu_check_true( data->bchain2D[i].new_bound_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain2D[i].new_claimed_cells_malloced == DVC_INC );
      pcu_check_true( data->bchain2D[i].done == 0 );
   }
   /* particle values */
   for (i = 0; i < data->nump2D; i++) {
      pcu_check_true( (data->pList2D[i].cx == 0) && (data->pList2D[i].cy == 0) );
      pcu_check_true( data->pList2D[i].w == 0 );
   }
}

void DVCWeightsSuite_TestGetCentroids2D( DVCWeightsSuiteData* data ) {
   FILE         *fp;
   unsigned int i;
   const char*  centroidsFilename = "DVCWeightsSuite_testGetCentroids2D.txt";
   char         expectedCentroidsFilename[PCU_PATH_MAX];
   char         line[100];
   float        x,y,dx,dy;
   float        tol = 1e-6;

   _DVCWeights_ConstructGrid2D(&data->cells2D,data->numy,data->numx, BBXMIN,BBYMIN,BBXMAX,BBYMAX);
   _DVCWeights_InitialiseStructs2D( data->dvcWeights, data->nump);
   data->bchain2D = data->dvcWeights->bchain;
   data->pList2D  = data->dvcWeights->pList;
   _DVCWeightsSuite_InitialiseParticleCoords2D( data );
   _DVCWeights_CreateVoronoi2D( data->bchain2D, data->pList2D, data->cells2D, data->dx, data->dy,
      data->nump2D, data->numx, data->numy, BBXMIN, BBXMAX, BBYMIN, BBYMAX);

   _DVCWeights_GetCentroids2D( data->cells2D, data->pList2D,data->numy,data->numx,data->nump2D,data->da2D);

   pcu_filename_expected( centroidsFilename, expectedCentroidsFilename );
   fp = fopen( expectedCentroidsFilename,"r" );

   if(!fp){
     pcu_check_true(0);
     Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)"DVCWeightsSuite_TestGetCentroids2D" ),"expected test File %s Not Found in function %s\n" ,expectedCentroidsFilename, __func__);
   }
   else{

     for (i = 0; i < data->nump2D; i++) {
       fgets(line,100,fp);
       fgets(line,100,fp);
       sscanf(line,"\t\tcoords: (x, y) = ( %f, %f)\n",&x,&y);
       dx = (data->pList2D[i].x -x)*(data->pList2D[i].x -x);
       dy = (data->pList2D[i].y -y)*(data->pList2D[i].y -y);
       pcu_check_true( dx < tol );
       pcu_check_true( dy < tol );

       fgets(line,100,fp);
       sscanf(line,"\t\tcentroids: (cx, cy) = ( %f, %f)\n",&x,&y);
       dx = (data->pList2D[i].cx -x)*(data->pList2D[i].cx -x);
       dy = (data->pList2D[i].cy -y)*(data->pList2D[i].cy -y);
       pcu_check_true( dx < tol );
       pcu_check_true( dy < tol );
       
       fgets(line,100,fp);
       sscanf(line,"\t\tweight = %f\n",&x);
       dx = (data->pList2D[i].w -x)*(data->pList2D[i].w -x);
       pcu_check_true( dx < tol );
     }
   }
   fclose(fp);
}

void DVCWeightsSuite_TestDistanceSquared2D( DVCWeightsSuiteData* data ) {
   double particleDistance;      
   double particle0[2], particle1[2];
   
   particle0[0] = 0.5; particle0[1] = 0.5;
   particle1[0] = 0.25; particle1[1] = 0.25;

   particleDistance = _DVCWeights_DistanceSquared2D(
      particle0[0], particle0[1],
      particle1[0], particle1[1] );
   
   pcu_check_true( particleDistance == 0.125 );
}

void DVCWeightsSuite( pcu_suite_t* suite ) {
   // These tests are disabled currently
   // They have been updated to reflect changes in DVCWeights 30/8/2015 but
   // have not been tested to ensure they produce the correct results still.
   assert(0);
   pcu_suite_setData( suite, DVCWeightsSuiteData );
   pcu_suite_setFixtures( suite, DVCWeightsSuite_Setup, DVCWeightsSuite_Teardown );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestConstructGrid );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestResetGrid );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestInitialiseStructs );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestCreateVoronoi );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestGetCentroids );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestDistanceSquared );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestConstructGrid2D );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestInitialiseStructs2D );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestResetGrid2D );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestCreateVoronoi2D );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestGetCentroids2D );
   pcu_suite_addTest( suite, DVCWeightsSuite_TestDistanceSquared2D );
}
