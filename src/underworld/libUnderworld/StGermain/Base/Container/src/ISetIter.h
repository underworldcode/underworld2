/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_ISetIter_h__
#define __StGermain_Base_Container_ISetIter_h__

extern const Type ISetIter_Type;
        
#define __ISetIter                              \
    __Iter                                      \
    ISet* iset;                                 \
    int tblInd;                                 \
    ISetItem* cur;

struct ISetIter { __ISetIter };

#ifndef ZERO
#define ZERO 0
#endif

#define ISETITER_DEFARGS                        \
    ITER_DEFARGS

#define ISETITER_PASSARGS                       \
    ITER_PASSARGS

ISetIter* ISetIter_New();

ISetIter* _ISetIter_New( ISETITER_DEFARGS );

void ISetIter_Init( ISetIter* self );

void _ISetIter_Init( void* self );

void ISetIter_Destruct( ISetIter* self );

void _ISetIter_Delete( void* self );

void ISetIter_Copy( void* self, const void* op );

void ISetIter_Next( void* self );

int ISetIter_GetKey( const void* self );

#endif /* __StGermain_Base_Container_ISetIter_h__ */
