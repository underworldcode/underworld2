/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StgDomain_Mesh_Decomp_h__
#define __StgDomain_Mesh_Decomp_h__

extern const Type Decomp_Type;
        
#define __Decomp                                \
    __Stg_Class                                 \
    MPI_Comm mpiComm;                          \
    int nGlobals;                              \
    IArray* locals;                            \
    IArray localsObj;                          \
    IMap* inv;                                 \
    IMap invObj;                               \
    int rngBegin;                              \
    int rngEnd;                                \
    IMap* owners;                              \
    IMap ownersObj;

struct Decomp { __Decomp };

#ifndef ZERO
#define ZERO 0
#endif

#define DECOMP_DEFARGS                          \
    STG_CLASS_DEFARGS

#define DECOMP_PASSARGS                         \
    STG_CLASS_PASSARGS

Decomp* Decomp_New();

Decomp* _Decomp_New( DECOMP_DEFARGS );

void Decomp_Init( Decomp* self );

void _Decomp_Init( void* self );

void Decomp_Destruct( Decomp* self );

void _Decomp_Delete( void* self );

void Decomp_Copy( void* self, const void* op );

void Decomp_SetMPIComm( void* _self, MPI_Comm mpiComm );

void Decomp_SetLocals( void* _self, int nLocals, const int* locals );

void Decomp_AddLocals( void* _self, int nLocals, const int* locals );

void Decomp_RemoveLocals( void* _self, int nLocals, const int* locals, IMap* map );

void Decomp_Clear( void* _self );

void Decomp_ClearLocals( void* _self );

MPI_Comm Decomp_GetComm( const void* self );

MPI_Comm Decomp_GetMPIComm( const void* self );

int Decomp_GetNumGlobals( const void* self );

int Decomp_GetNumLocals( const void* self );

void Decomp_GetLocals( const void* self, int* nLocals, const int** locals );

int Decomp_LocalToGlobal( const void* self, int local );

int Decomp_GlobalToLocal( const void* self, int global );

Bool Decomp_TryGlobalToLocal( const void* self, int global, int* local );

void Decomp_FindOwners( const void* _self, int nGlobals, const int* globals, 
                        int* ranks );

void Decomp_Update( Decomp* self );

void Decomp_UpdateOwnerMap( Decomp* self );

#endif /* __StgDomain_Mesh_Decomp_h__ */
