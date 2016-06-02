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



*****************************************************************************************************************/

#ifndef __PICellerator_Weights_PCDVCClass_h__
#define __PICellerator_Weights_PCDVCClass_h__

/* Textual name of this class */
extern const Type PCDVC_Type;

/* PCDVC information */
//   #define __PCDVC 
/* Who's my daddy */ 
//    __DVCWeights 
/* My Data structures */ 
//  GeneralSwarm* materialPointsSwarm; 
//  int upT; 
//  int lowT;

#define __PCDVC                                                         \
    __DVCWeights                                                        \
                                                                        \
    GeneralSwarm*         materialPointsSwarm;                          \
    double                upperT;                                       \
    double                lowerT;                                       \
    Bool                  splitInInterfaceCells;                        \
    Bool                  deleteInInterfaceCells;                       \
    int                   maxDeletions;                                 \
    int                   maxSplits;                                    \
    Bool                  Inflow;                                       \
    double                CentPosRatio;                                 \
    int                   ParticlesPerCell;                             \
    double                Threshold;                                    \
    /* we also need to store some parameters, as everything is turned up      */ \
    /* for interpolation restarts (which may require significant repopulation */ \
    /* and after the first timestep needs to be set back to correct values    */ \
    int                   maxDeletions_orig;                            \
    int                   maxSplits_orig;                               \
    Bool                  Inflow_orig;                                  \
    Bool                  splitInInterfaceCells_orig;                   \
    Bool                  deleteInInterfaceCells_orig;                  \
    int                   pcdvcvisited;


struct PCDVC { __PCDVC };

struct deleteParticle{
    Particle_Index indexWithinCell;
    Particle_Index indexOnCPU;
};

/*---------------------------------------------------------------------------------------------------------------------
** Constructors
*/



PCDVC* PCDVC_New( Name name, int* res,
                  GeneralSwarm* mps, double upT, double lowT,
                  int maxDeletions, int maxSplits, Bool splitInInterfaceCells,
                  Bool deleteInInterfaceCells, Bool Inflow, double CentPosRatio,
                  int ParticlesPerCell, double Threshold ) ;


	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PCDVC_DEFARGS \
                DVCWEIGHTS_DEFARGS

	#define PCDVC_PASSARGS \
                DVCWEIGHTS_PASSARGS

PCDVC* _PCDVC_New(  PCDVC_DEFARGS  );

void _PCDVC_Init( void* pcdvc, GeneralSwarm* mps, double upT, double lowT,
                  int maxDeletions, int maxSplits, Bool splitInInterfaceCells,
                  Bool deleteInInterfaceCells, Bool Inflow, double CentPosRatio,
                  int ParticlesPerCell, double Threshold ) ;


/* Stg_Class_Delete PCDVC implementation */
void _PCDVC_Delete( void* pcdvc );
void _PCDVC_Print( void* pcdvc, Stream* stream );
#define PCDVC_Copy( self )                                      \
    (PCDVC*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
#define PCDVC_DeepCopy( self )                                  \
    (PCDVC*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
void* _PCDVC_Copy( void* pcdvc, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
        
void* _PCDVC_DefaultNew( Name name ) ;

void _PCDVC_AssignFromXML( void* pcdvc, Stg_ComponentFactory* cf, void* data ) ;

void _PCDVC_Build( void* pcdvc, void* data ) ;
void _PCDVC_Initialise( void* pcdvc, void* data ) ;
void _PCDVC_Destroy( void* pcdvc, void* data )  ;
void _PCDVC_Execute( void* pcdvc, void* data );
void splitIntParticleByIndexWithinCell( IntegrationPointsSwarm* intSwarm,  GeneralSwarm* matSwarm, Cell_LocalIndex lCell_I, Particle_Index intParticleToSplit_IndexOnCPU, Coord xi );
void deleteIntParticleByIndexWithinCell( IntegrationPointsSwarm* intSwarm,  GeneralSwarm* matSwarm,  Cell_LocalIndex lCell_I, Particle_Index intParticleToSplit_IndexWithinCell );
void deleteIntParticleByIndexOnCPU( IntegrationPointsSwarm* intSwarm,  GeneralSwarm* matSwarm, Particle_Index intParticleToSplit_IndexWithinCell );
void _PCDVC_Calculate3D( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I );
void _PCDVC_Calculate2D( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I );  
void _PCDVC_Calculate( void* pcdvc, void* _swarm, Cell_LocalIndex lCell_I ) ;

#endif

