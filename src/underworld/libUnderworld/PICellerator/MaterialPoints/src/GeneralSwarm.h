/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_GeneralSwarm_h__
#define __PICellerator_MaterialPoints_GeneralSwarm_h__

#ifdef __cplusplus
extern "C" {
#endif

#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL stg_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <numpy/arrayobject.h>

#if NPY_API_VERSION < 0x00000007
#define NPY_ARRAY_OWNDATA  NPY_OWNDATA
#endif


/* Textual name of this class */
extern const Type GeneralSwarm_Type;

/* GeneralSwarm information */
#define __GeneralSwarm \
      __Swarm \
      \
      SwarmAdvector*                        swarmAdvector;        \
      EscapedRoutine*                       escapedRoutine;       \
      SwarmVariable*                        particleCoordVariable; /** Set only if a global coord system swarm. */ \
      SwarmMap*                             previousIntSwarmMap; \
      List*                                 intSwarmMapList;  \
      void*                                 index;            \
      void*                                 index_int; 
 
struct GeneralSwarm
{
   __GeneralSwarm
};


/*---------------------------------------------------------------------------------------------------------------------
** Constructors
*/

#ifndef ZERO
#define ZERO 0
#endif

#define GENERALSWARM_DEFARGS \
                SWARM_DEFARGS

#define GENERALSWARM_PASSARGS \
                SWARM_PASSARGS

GeneralSwarm* _GeneralSwarm_New(  GENERALSWARM_DEFARGS  );

void _GeneralSwarm_Delete( void* swarm );
#define GeneralSwarm_Copy( self ) \
   (GeneralSwarm*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
#define GeneralSwarm_DeepCopy( self ) \
   (GeneralSwarm*) Stg_Class_Copy( self, NULL, True, NULL, NULL )

void* _GeneralSwarm_DefaultNew( Name name ) ;

void _GeneralSwarm_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
void _GeneralSwarm_Build( void* swarm, void* data ) ;
void _GeneralSwarm_Initialise( void* swarm, void* data ) ;
void _GeneralSwarm_Execute( void* swarm, void* data );
void _GeneralSwarm_Destroy( void* swarm, void* data ) ;


void _GeneralSwarm_Init(
   void*                                 swarm,
   EscapedRoutine*                       escapedRoutine );

/* Public functions */

/** Returns the particle extension at given point index */
void* GeneralSwarm_GetExtensionAt( void* swarm, Index point_I, Index extHandle );

PyObject* GeneralSwarm_AddParticlesFromCoordArray( void* _swarm, Index count, Index dim, double* array );

int GeneralSwarm_AddParticle( void* swarm, Index dim, double xI, double xJ, double xK );

unsigned GeneralSwarm_IntegrationPointMap( void* _self, void* intSwarm, unsigned elementId, unsigned intPtCellId );

void GeneralSwarm_ClearSwarmMaps( void* swarm ) ;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C++" {
void GeneralSwarm_DeleteIndex( void* swarm );
size_t GeneralSwarm_GetClosestParticles( void* swarm, const double* coord, int num_parts );
}
#endif

#endif
