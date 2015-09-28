/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/




#ifndef __PICellerator_Weights_DVCWeightsClass_h__
#define __PICellerator_Weights_DVCWeightsClass_h__

/* Textual name of this class */
extern const Type DVCWeights_Type;

/* DVCWeights information */
#define __DVCWeights                            \
    /* General info */                          \
    __WeightsCalculator                         \
                                                \
    /* Virtual Info */                          \
    /* Parameters that are passed in */         \
    int resX;                                   \
    int resY;                                   \
    int resZ;                                   \
    int nump;  \
    double dx; \
    double dy; \
    double dz; \
    int visiteddvcweights;                      \
    struct particle   *pList;                   \
    struct chain      *bchain;                  \
    struct cell       *cells;                   \
    unsigned    plistSize;                      \
    unsigned    bchainSize;                     \
    double      bbmin;                          \
    double      bbmax;                          \

struct DVCWeights { __DVCWeights };
        
#define DVC_INC 150
        
struct cell{
    int p;/*particle index number*/
    int index;
    int N;
    int S;
    int E;
    int W;
    int U;
    int D;
    double x;
    double y;
    double z;
    int done;
};

struct chain{
    int p;/*particle index number*/
    int index;/*cell number in grid*/
    int sizeofboundary; /* number of cells on boundary so far */
    int numclaimed;
    int totalclaimed;
    int new_bound_cells_malloced;
    int new_claimed_cells_malloced; 
    int *new_bound_cells;
    int *new_claimed_cells;
    int done;
};
struct particle{
    double x;
    double y;
    double z;
    double cx;
    double cy;
    double cz;
    double w;
    int index;
};

/* Private function prototypes in 3D */
void   _DVCWeights_GetCentroids3D(        
    struct cell *cells,
    struct particle *pList,
    int n,
    int m, 
    int l,
    int nump,
    double vol);
void   _DVCWeights_ClaimCells(
    struct chain *bchain,
    struct cell *cells,
    struct particle *pList,
    int p_i);
void   _DVCWeights_UpdateBchain(
    struct chain *bchain,
    struct cell *cells,
    int p_i);
void   _DVCWeights_ResetGrid3D(struct cell *cells, int n );

double _DVCWeights_DistanceSquared(
    double x0, double y0, double z0, 
    double x1, double y1, double z1);
double _DVCWeights_DistanceTest(double x0, double y0, double z0, 
                                double x1, double y1, double z1,
                                double x2, double y2, double z2);       
void   _DVCWeights_ConstructGrid(
    struct cell **cell_list, 
    int n, int m, int l,
    double x0, double y0, double z0,
    double x1, double y1, double z1);
void   _DVCWeights_InitialiseStructs( DVCWeights* self, int nump);
void   _DVCWeights_CreateVoronoi3D( 
    struct chain *bchain, 
    struct particle *pList, 
    struct cell *cells, 
    double dx, double dy, double dz,
    int nump,
    int numx, int numy, int numz, 
    double BBXMIN, double BBXMAX, 
    double BBYMIN, double BBYMAX,
    double BBZMIN, double BBZMAX);
/* Private function prototypes in 2D */
void   _DVCWeights_GetCentroids2D( 
    struct cell       *cells, 
    struct particle *pList,
    int n, 
    int m, 
    int nump,
    double vol);
void   _DVCWeights_ClaimCells2D(
    struct chain *bchain,
    struct cell  *cells,
    struct particle *pList,
    int p_i);
void   _DVCWeights_UpdateBchain2D(
    struct chain *bchain,
    struct cell  *cells,
    int p_i);
void   _DVCWeights_ResetGrid2D(struct cell *cells, int n );
double _DVCWeights_DistanceSquared2D(double x0, double y0, double x1, double y1);
double _DVCWeights_DistanceTest2D(double x0, double y0,
                                  double x1, double y1,
                                  double x2, double y2);
void   _DVCWeights_ConstructGrid2D(
    struct cell **cell_list, 
    int m, int l,
    double x0, double y0,
    double x1, double y1);
void   _DVCWeights_InitialiseStructs2D( DVCWeights* self, int nump);

void   _DVCWeights_CreateVoronoi2D( 
    struct chain *bchain, 
    struct particle *pList, 
    struct cell *cells, 
    double dx, double dy,
    int nump,
    int numx, int numy,
    double BBXMIN, double BBXMAX, 
    double BBYMIN, double BBYMAX);

void   _DVCWeights_Calculate2D( 
    void* dvcWeights, 
    void* _swarm, 
    Cell_LocalIndex lCell_I );
void   _DVCWeights_Calculate3D( 
    void* dvcWeights, 
    void* _swarm, 
    Cell_LocalIndex lCell_I );


/*---------------------------------------------------------------------------------------------------------------------
** Constructors
*/



DVCWeights* DVCWeights_New( Name name, Dimension_Index dim, int *res ) ;

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DVCWEIGHTS_DEFARGS \
                WEIGHTSCALCULATOR_DEFARGS

	#define DVCWEIGHTS_PASSARGS \
                WEIGHTSCALCULATOR_PASSARGS

DVCWeights* _DVCWeights_New(  DVCWEIGHTS_DEFARGS  );

void _DVCWeights_Init( void* dvcWeights, int *res ) ;

/* Stg_Class_Delete DVCWeights implementation */
void _DVCWeights_Delete( void* dvcWeights );
void _DVCWeights_Print( void* dvcWeights, Stream* stream );
#define DVCWeights_Copy( self )                                         \
    (DVCWeights*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
#define DVCWeights_DeepCopy( self )                                     \
    (DVCWeights*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
void* _DVCWeights_Copy( void* dvcWeights, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
        
void* _DVCWeights_DefaultNew( Name name ) ;

void _DVCWeights_AssignFromXML( void* dvcWeights, Stg_ComponentFactory* cf, void* data ) ;

void _DVCWeights_Build( void* dvcWeights, void* data );
void _DVCWeights_Initialise( void* dvcWeights, void* data );
void _DVCWeights_Execute( void* dvcWeights, void* data );
void _DVCWeights_Destroy( void* dvcWeights, void* data );
                
void _DVCWeights_Calculate( void* dvcWeights, void* _swarm, Cell_LocalIndex lCell_I ) ;

#endif

