/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_types_h__
#define __Underworld_Utils_types_h__

   typedef struct UnderworldContext		UnderworldContext;

   typedef struct VectorAssemblyTerm_NA__Fn VectorAssemblyTerm_NA__Fn;
   typedef struct VectorSurfaceAssemblyTerm_NA__Fn__ni VectorSurfaceAssemblyTerm_NA__Fn__ni;
   typedef struct VectorAssemblyTerm_VEP VectorAssemblyTerm_VEP;
   typedef struct MatrixAssemblyTerm_NA_i__NB_i__Fn MatrixAssemblyTerm_NA_i__NB_i__Fn;
   typedef struct MatrixAssemblyTerm_NA__NB__Fn MatrixAssemblyTerm_NA__NB__Fn;

   typedef struct RBFManager            RBFManager;
   typedef struct RBFFieldVariable      RBFFieldVariable;
   typedef struct Fn_Integrate          Fn_Integrate;
   typedef struct MatrixAssemblyTerm_RotationDof                MatrixAssemblyTerm_RotationDof;

#endif
