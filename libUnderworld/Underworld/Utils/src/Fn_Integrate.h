/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_Fn_Integrate_h__
#define __Underworld_Utils_Fn_Integrate_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>

struct Fn_Integrate_cppdata
{
    Fn::Function* fn;
    IO_double* sumLocal;
    IO_double* sumGlobal;
    Fn::Function::func func;
    std::shared_ptr<FEMCoordinate> input;
};

void _Fn_Integrate_SetFn( void* _self, Fn::Function* fn );

IO_double* Fn_Integrate_Integrate( void* fn_integrate );

}
extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

   /** Textual name of this class */
   extern const Type Fn_Integrate_Type;

   /** Fn_Integrate class contents */
   #define __Fn_Integrate \
      /* General info */ \
      __Stg_Component \
      \
      UnderworldContext*  context; \
      \
      IntegrationPointsSwarm* integrationSwarm; \
      Stream*                 errorStream; \
      int                     functionLabel; \
      double                  gIntegral; \
      void*                   cppdata;   \
      FeMesh*                 mesh;      \
      unsigned                dim;   

   struct Fn_Integrate { __Fn_Integrate };


   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FN_INTEGRATE_DEFARGS \
           STG_COMPONENT_DEFARGS

   #define FN_INTEGRATE_PASSARGS \
           STG_COMPONENT_PASSARGS

   Fn_Integrate* _Fn_Integrate_New(  FN_INTEGRATE_DEFARGS  );

   void* _Fn_Integrate_DefaultNew( Name name );

   void _Fn_Integrate_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

   void _Fn_Integrate_Build( void* residual, void* data );

   void _Fn_Integrate_Initialise( void* residual, void* data );

   void _Fn_Integrate_Execute( void* residual, void* data );

   void _Fn_Integrate_Destroy( void* residual, void* data );


#ifdef __cplusplus
}
#endif

#endif
