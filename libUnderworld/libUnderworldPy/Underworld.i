/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module Underworld

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
#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/MinMax.hpp>
#include <Underworld/Function/Constant.hpp>
#include <Underworld/Function/SwarmVariableFn.hpp>
#include <Underworld/Function/ParticleFound.hpp>
#include <Underworld/Function/FeVariableFn.hpp>
#include <Underworld/Function/GradFeVariableFn.hpp>
#include <Underworld/Function/Map.hpp>
#include <Underworld/Function/Unary.hpp>
#include <Underworld/Function/Binary.hpp>
#include <Underworld/Function/Analytic.hpp>
#include <Underworld/Function/Tensor.hpp>
#include <Underworld/Function/IOIterators.hpp>
#include <Underworld/Function/Query.hpp>
#include <Underworld/Function/Shape.hpp>
#include <Underworld/Function/Relational.hpp>
#include <Underworld/Function/Conditional.hpp>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
}
%}

%include "Rheology/ConstitutiveMatrix.h"
%include "Rheology/ConstitutiveMatrixCartesian.h"
%include "Rheology/types.h"
%include "Utils/Exceptions.h"
%include "Utils/types.h"

%include "Utils/AdvectionDiffusion/AdvectionDiffusionSLE.h"
%include "Utils/AdvectionDiffusion/LumpedMassMatrixForceTerm.h"
%include "Utils/AdvectionDiffusion/Multicorrector.h"
%include "Utils/AdvectionDiffusion/Residual.h"
%include "Utils/AdvectionDiffusion/Timestep.h"
%include "Utils/AdvectionDiffusion/types.h"

%include "Utils/Fn_Integrate.h"
%include "Utils/VectorAssemblyTerm_NA__Fn.h"
%include "Utils/VectorAssemblyTerm_NA_i__Fn_i.h"
%include "Utils/VectorSurfaceAssemblyTerm_NA__Fn__ni.h"
%include "Utils/VectorAssemblyTerm_VEP.h"
%include "Utils/MatrixAssemblyTerm_NA_i__NB_i__Fn.h"
%include "Utils/MatrixAssemblyTerm_NA__NB__Fn.h"
