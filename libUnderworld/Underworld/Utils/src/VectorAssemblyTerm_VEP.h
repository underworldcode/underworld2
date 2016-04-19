/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_VectorAssemblyTerm_VEP_h__
#define __Underworld_Utils_VectorAssemblyTerm_VEP_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>

struct VectorAssemblyTerm_VEP_cppdata
{
    Fn::Function* fn;
    Fn::Function::func func;
    std::shared_ptr<FEMCoordinate> input;
};

void _VectorAssemblyTerm_VEP_SetFneForce( void* _self, Fn::Function* fn );

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
   extern const Type VectorAssemblyTerm_VEP_Type;

   /** VectorAssemblyTerm_VEP class contents */
   #define __VectorAssemblyTerm_VEP \
      /* General info */ \
      __ForceTerm \
      \
      /* Virtual info */ \
      void*       funeForce;   \
      double**    GNx; \
      int         maxNodesPerEl;

   struct VectorAssemblyTerm_VEP { __VectorAssemblyTerm_VEP };


   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FORCEASSEMBLYTERM_NA__FN_DEFARGS \
                FORCETERM_DEFARGS

   #define FORCEASSEMBLYTERM_NA__FN_PASSARGS \
                FORCETERM_PASSARGS

   VectorAssemblyTerm_VEP* _VectorAssemblyTerm_VEP_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  );

   void _VectorAssemblyTerm_VEP_Delete( void* residual );

   void _VectorAssemblyTerm_VEP_Print( void* residual, Stream* stream );

   void* _VectorAssemblyTerm_VEP_DefaultNew( Name name );

   void _VectorAssemblyTerm_VEP_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

   void _VectorAssemblyTerm_VEP_Build( void* residual, void* data );

   void _VectorAssemblyTerm_VEP_Initialise( void* residual, void* data );

   void _VectorAssemblyTerm_VEP_Execute( void* residual, void* data );

   void _VectorAssemblyTerm_VEP_Destroy( void* residual, void* data );

   void _VectorAssemblyTerm_VEP_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec );


#ifdef __cplusplus
}
#endif

#endif

