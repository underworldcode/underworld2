/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_IMapIter_h__
#define __StGermain_Base_Container_IMapIter_h__

extern const Type IMapIter_Type;
        
#define __IMapIter                              \
    __Iter                                      \
    IMap* imap;                                 \
    int tblInd;                                 \
    IMapItem* cur;

struct IMapIter { __IMapIter };

#ifndef ZERO
#define ZERO 0
#endif

#define IMAPITER_DEFARGS                        \
    ITER_DEFARGS

#define IMAPITER_PASSARGS                       \
    ITER_PASSARGS

IMapIter* IMapIter_New();

IMapIter* _IMapIter_New( IMAPITER_DEFARGS );

void IMapIter_Init( IMapIter* self );

void _IMapIter_Init( void* self );

void IMapIter_Destruct( IMapIter* self );

void _IMapIter_Delete( void* self );

void IMapIter_Copy( void* self, const void* op );

void IMapIter_Next( void* self );

int IMapIter_GetKey( const void* self );

int IMapIter_GetValue( const void* self );

#endif /* __StGermain_Base_Container_IMapIter_h__ */
