/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __petscccompat_h__
#define __petscccompat_h__

#include "petsc.h"

#include "../../../../StgDomain/Utils/src/PETScCompatibility.h"

/* Some defines that existed in petsc 3.1 but no longer in 3.2 */
#if !defined(EXTERN)
#define EXTERN extern
#endif
#if !defined(PETSC_STDCALL)
#define PETSC_STDCALL
#endif
#if !defined(PETSC_TEMPLATE)
#define PETSC_TEMPLATE
#endif
#if !defined(PETSC_HAVE_DLL_EXPORT)
#define PETSC_DLL_EXPORT
#define PETSC_DLL_IMPORT
#endif
#if !defined(PETSC_DLLEXPORT)
#define PETSC_DLLEXPORT
#endif
#if !defined(PETSCVEC_DLLEXPORT)
#define PETSCVEC_DLLEXPORT
#endif
#if !defined(PETSCMAT_DLLEXPORT)
#define PETSCMAT_DLLEXPORT
#endif
#if !defined(PETSCDM_DLLEXPORT)
#define PETSCDM_DLLEXPORT
#endif
#if !defined(PETSCKSP_DLLEXPORT)
#define PETSCKSP_DLLEXPORT
#endif
#if !defined(PETSCSNES_DLLEXPORT)
#define PETSCSNES_DLLEXPORT
#endif
#if !defined(PETSCTS_DLLEXPORT)
#define PETSCTS_DLLEXPORT
#endif
#if !defined(PETSCFORTRAN_DLLEXPORT)
#define PETSCFORTRAN_DLLEXPORT
#endif

#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >= 2 ) )

   #define KSP_COOKIE KSP_CLASSID
   #define MAT_COOKIE MAT_CLASSID
   #define SNES_COOKIE SNES_CLASSID
   #define MAT_NULLSPACE_COOKIE MAT_NULLSPACE_CLASSID
   #define loadintovector load
   #define loadintovectornative load
   #define __VecLoadIntoVector_Block(arg1, arg2) __VecLoadIntoVector_Block(arg2, arg1)


#endif

#endif

