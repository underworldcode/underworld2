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
%module (package="underworld.libUnderworld.libUnderworldPy") StgFEM
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") StgFEM
#endif


%{
/* Includes the header in the wrapper code */
#define SWIG_FILE_WITH_INIT
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
}
%}

%include "numpy.i"
%init %{
import_array();
%}

%include "exception.i"

%import "StGermain.i"
%import "StgDomain.i"


%include "StgFEM/Discretisation/src/FeVariable.h"
%include "StgFEM/Discretisation/src/C0Generator.h"
%include "StgFEM/Discretisation/src/C2Generator.h"
%include "StgFEM/Discretisation/src/FeEquationNumber.h"
%include "StgFEM/Discretisation/src/FeMesh.h"
%include "StgFEM/Discretisation/src/Inner2DGenerator.h"
%include "StgFEM/Discretisation/src/dQ1Generator.h"
%include "StgFEM/Discretisation/src/IrregularMeshGaussLayout.h"
%include "StgFEM/Discretisation/src/types.h"
%include "StgFEM/SLE/SystemSetup/src/ForceTerm.h"
%include "StgFEM/SLE/SystemSetup/src/MGOpGenerator.h"
%include "StgFEM/SLE/SystemSetup/src/PETScMGSolver.h"
%include "StgFEM/SLE/SystemSetup/src/SLE_Solver.h"
%include "StgFEM/SLE/SystemSetup/src/SolutionVector.h"
%include "StgFEM/SLE/SystemSetup/src/SROpGenerator.h"
%include "StgFEM/SLE/SystemSetup/src/StiffnessMatrix.h"
%include "StgFEM/SLE/SystemSetup/src/StiffnessMatrixTerm.h"
%include "StgFEM/SLE/SystemSetup/src/SystemLinearEquations.h"
%include "StgFEM/SLE/SystemSetup/src/SolutionVector.h"
%include "StgFEM/SLE/SystemSetup/src/ForceVector.h"
%include "StgFEM/SLE/SystemSetup/src/types.h"
/*
%include "SLE/ProvidedSystems/AdvectionDiffusion/AdvectionDiffusionSLE.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/LumpedMassMatrixForceTerm.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Multicorrector.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Residual.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Timestep.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/types.h"
*/
%include "StgFEM/SLE/ProvidedSystems/Energy/src/Energy_SLE_Solver.h"
%include "StgFEM/SLE/ProvidedSystems/Energy/src/types.h"
%include "StgFEM/SLE/ProvidedSystems/StokesFlow/src/Stokes_SLE.h"
%include "StgFEM/SLE/ProvidedSystems/StokesFlow/src/Stokes_SLE_UzawaSolver.h"
%include "StgFEM/SLE/ProvidedSystems/StokesFlow/src/types.h"
%include "StgFEM/SLE/ProvidedSystems/StokesFlow/src/UzawaPreconditionerTerm.h"
%include "StgFEM/SLE/src/types.h"
%include "StgFEM/Assembly/src/DivergenceMatrixTerm.h"
%include "StgFEM/Assembly/src/GradientStiffnessMatrixTerm.h"
%include "StgFEM/Assembly/src/types.h"
%include "StgFEM/Utils/src/IrregularMeshParticleLayout.h"
%include "StgFEM/Utils/src/SemiLagrangianIntegrator.h"
%include "StgFEM/Utils/src/MeshParticleLayout.h"
%include "StgFEM/Utils/src/types.h"
