/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_VectorAssemblyTerm_NA__Fn_h__
#define __Underworld_Utils_VectorAssemblyTerm_NA__Fn_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/src/Function.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>

struct VectorAssemblyTerm_NA__Fn_cppdata
{
    Fn::Function* fn;
    Fn::Function::func func;
    std::shared_ptr<FEMCoordinate> input;
};

void _VectorAssemblyTerm_NA__Fn_SetFn( void* _self, Fn::Function* fn );

}

extern "C" {
#endif

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
#include "types.h"

   /** Textual name of this class */
   extern const Type VectorAssemblyTerm_NA__Fn_Type;

   /** VectorAssemblyTerm_NA__Fn class contents */
   #define __VectorAssemblyTerm_NA__Fn \
      /* General info */ \
      __ForceTerm \
      \
      /* Virtual info */ \
      void*                   cppdata;   \
      FeMesh*                 geometryMesh;

   struct VectorAssemblyTerm_NA__Fn { __VectorAssemblyTerm_NA__Fn };


   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FORCEASSEMBLYTERM_NA__FN_DEFARGS \
                FORCETERM_DEFARGS

   #define FORCEASSEMBLYTERM_NA__FN_PASSARGS \
                FORCETERM_PASSARGS

   VectorAssemblyTerm_NA__Fn* _VectorAssemblyTerm_NA__Fn_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  );

   void _VectorAssemblyTerm_NA__Fn_Delete( void* residual );

   void _VectorAssemblyTerm_NA__Fn_Print( void* residual, Stream* stream );

   void* _VectorAssemblyTerm_NA__Fn_DefaultNew( Name name );

   void _VectorAssemblyTerm_NA__Fn_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

   void _VectorAssemblyTerm_NA__Fn_Build( void* residual, void* data );

   void _VectorAssemblyTerm_NA__Fn_Initialise( void* residual, void* data );

   void _VectorAssemblyTerm_NA__Fn_Execute( void* residual, void* data );

   void _VectorAssemblyTerm_NA__Fn_Destroy( void* residual, void* data );

   void _VectorAssemblyTerm_NA__Fn_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec );


#ifdef __cplusplus
}
#endif

#endif

