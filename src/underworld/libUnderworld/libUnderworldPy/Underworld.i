/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

#if (SWIG_VERSION <= 0x040000)
%module (package="underworld.libUnderworld.libUnderworldPy") Underworld
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") Underworld
#endif

%include "exception.i"
%import "StGermain.i"
%import "StgDomain.i"
%import "StgFEM.i"
%import "PICellerator.i"
%import "Function.i"

%{
/* Includes the header in the wrapper code */
#include <vector>
#include <functional>
#include <Underworld/Function/src/FunctionIO.hpp>
#include <Underworld/Function/src/Function.hpp>
#include <Underworld/Function/src/SafeMaths.hpp>
#include <Underworld/Function/src/CustomException.hpp>
#include <Underworld/Function/src/MinMax.hpp>
#include <Underworld/Function/src/Count.hpp>
#include <Underworld/Function/src/Constant.hpp>
#include <Underworld/Function/src/SwarmVariableFn.hpp>
#include <Underworld/Function/src/ParticleFound.hpp>
#include <Underworld/Function/src/FeVariableFn.hpp>
#include <Underworld/Function/src/GradFeVariableFn.hpp>
#include <Underworld/Function/src/Map.hpp>
#include <Underworld/Function/src/Unary.hpp>
#include <Underworld/Function/src/Binary.hpp>
#include <Underworld/Function/src/Analytic.hpp>
#include <Underworld/Function/src/Tensor.hpp>
#include <Underworld/Function/src/IOIterators.hpp>
#include <Underworld/Function/src/Query.hpp>
#include <Underworld/Function/src/Shape.hpp>
#include <Underworld/Function/src/Relational.hpp>
#include <Underworld/Function/src/Conditional.hpp>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
}
%}

%include "Underworld/Rheology/src/ConstitutiveMatrix.h"
%include "Underworld/Rheology/src/ConstitutiveMatrixCartesian.h"
%include "Underworld/Rheology/src/types.h"
%include "Underworld/Utils/src/Exceptions.h"
%include "Underworld/Utils/src/types.h"

%include "Underworld/Utils/AdvectionDiffusion/src/AdvectionDiffusionSLE.h"
%include "Underworld/Utils/AdvectionDiffusion/src/LumpedMassMatrixForceTerm.h"
%include "Underworld/Utils/AdvectionDiffusion/src/Multicorrector.h"
%include "Underworld/Utils/AdvectionDiffusion/src/Residual.h"
%include "Underworld/Utils/AdvectionDiffusion/src/Timestep.h"
%include "Underworld/Utils/AdvectionDiffusion/src/types.h"

%include "Underworld/Utils/src/Fn_Integrate.h"
%include "Underworld/Utils/src/VectorAssemblyTerm_NA__Fn.h"
%include "Underworld/Utils/src/VectorAssemblyTerm_NA_i__Fn_i.h"
%include "Underworld/Utils/src/VectorSurfaceAssemblyTerm_NA__Fn__ni.h"
%include "Underworld/Utils/src/VectorAssemblyTerm_NA_j__Fn_ij.h"
%include "Underworld/Utils/src/MatrixAssemblyTerm_NA_i__NB_i__Fn.h"
%include "Underworld/Utils/src/MatrixAssemblyTerm_NA__NB__Fn.h"
%include "Underworld/Utils/src/MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni.h"
