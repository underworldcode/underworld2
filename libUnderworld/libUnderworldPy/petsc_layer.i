/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module petsc_layer
%include "exception.i"
%import "Function.i"

%{
/* Includes the header in the wrapper code */
#include<stdio.h>
#include<mpi.h>
#include<petsc.h>
#include<petscsys.h>
#include<petscviewerhdf5.h>

/* Includes the header in the wrapper code */
#include <vector>
#include <functional>
#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/SafeMaths.hpp>
#include <Underworld/Function/CustomException.hpp>
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

extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>


}


#include "petsc_layer.hpp"
%}

%include "petsc_layer.hpp"
