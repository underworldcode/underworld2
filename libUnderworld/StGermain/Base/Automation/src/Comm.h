/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Automation_Comm_h__
#define __StGermain_Base_Automation_Comm_h__

extern const Type Comm_Type;
        
#define __Comm                                  \
   __Stg_Class                                  \
   MPI_Comm mpiComm;                            \
   IArray nbrs;                                 \
   IMap inv;                                    \
   MPI_Request* recvs;                          \
   MPI_Request* sends;                          \
   MPI_Status* stats;                           \
   int* recvSizes;                              \
   int itmSize;                                 \
   int srcSize;                                 \
   int* srcSizes;

struct Comm { __Comm };

#ifndef ZERO
#define ZERO 0
#endif

#define COMM_DEFARGS                            \
   STG_CLASS_DEFARGS

#define COMM_PASSARGS                           \
   STG_CLASS_PASSARGS

Comm* Comm_New();

Comm* _Comm_New( COMM_DEFARGS );

void Comm_Init( Comm* self );

void _Comm_Init( void* self );

void Comm_Destruct( Comm* self );

void _Comm_Delete( void* self );

void Comm_Copy( void* self, const void* op );

void Comm_SetMPIComm( void* _self, MPI_Comm mpiComm );

void Comm_SetNeighbours( void* _self, int nNbrs, const int* nbrs );

void Comm_AddNeighbours( void* _self, int nNbrs, const int* nbrs );

void Comm_RemoveNeighbours( void* _self, int nNbrs, const int* nbrLocalIndices, IMap* map );

MPI_Comm Comm_GetMPIComm( const void* self );

int Comm_GetNumNeighbours( const void* self );

void Comm_GetNeighbours( const void* self, int* nNbrs, const int** nbrs );

int Comm_RankLocalToGlobal( const void* self, int local );

Bool Comm_RankGlobalToLocal( const void* self, int global, int* local );

void Comm_AllgatherInit( const void* _self, int srcSize, 
			 int* dstSizes, int itmSize );

void Comm_AllgatherBegin( const void* _self, const void* srcArray, 
			  void** dstArrays );

void Comm_AllgatherEnd( const void* _self );

void Comm_AlltoallInit( const void* _self, const int* srcSizes, 
			int* dstSizes, int itmSize );

void Comm_AlltoallBegin( const void* _self, const void** srcArrays, 
			 void** dstArrays );

void Comm_AlltoallEnd( const void* _self );

#endif /* __StGermain_Base_Automation_Comm_h__ */
