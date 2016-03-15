/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_VectorSurfaceAssemblyTerm_NA__Fn__ni_h__
#define __Underworld_Utils_VectorSurfaceAssemblyTerm_NA__Fn__ni_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_SetFn( void* _self, Fn::Function* fn );

}

extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "types.h"

   /** Textual name of this class */
   extern const Type VectorSurfaceAssemblyTerm_NA__Fn__ni_Type;

   /** VectorSurfaceAssemblyTerm_NA__Fn__ni class contents */
   #define __VectorSurfaceAssemblyTerm_NA__Fn__ni \
    /* General info */ \
    __ForceTerm \
    \
    /* Virtual info */ \
    void* cppdata;     \
    VariableCondition*  bNodes; \
    IArray*    inc;


   struct VectorSurfaceAssemblyTerm_NA__Fn__ni { __VectorSurfaceAssemblyTerm_NA__Fn__ni };


   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FORCEASSEMBLYTERM_NA__FN_DEFARGS \
                FORCETERM_DEFARGS

   #define FORCEASSEMBLYTERM_NA__FN_PASSARGS \
                FORCETERM_PASSARGS

   VectorSurfaceAssemblyTerm_NA__Fn__ni* _VectorSurfaceAssemblyTerm_NA__Fn__ni_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Delete( void* residual );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Print( void* residual, Stream* stream );

   void* _VectorSurfaceAssemblyTerm_NA__Fn__ni_DefaultNew( Name name );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Build( void* residual, void* data );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Initialise( void* residual, void* data );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Execute( void* residual, void* data );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Destroy( void* residual, void* data );

   void _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec );


    void _VectorSurfaceAssemblyTerm_SetBNodes( void* _self, void* _bNodes );


#ifdef __cplusplus
}
#endif

#endif

