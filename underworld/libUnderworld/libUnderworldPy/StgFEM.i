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
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
}
%}

%include "numpy.i"
%init %{
import_array();
%}

%include "exception.i"

%import "StGermain.i"
%import "StgDomain.i"


%include "Discretisation/FeVariable.h"
%include "Discretisation/C0Generator.h"
%include "Discretisation/C2Generator.h"
%include "Discretisation/FeEquationNumber.h"
%include "Discretisation/FeMesh.h"
%include "Discretisation/Inner2DGenerator.h"
%include "Discretisation/dQ1Generator.h"
%include "Discretisation/IrregularMeshGaussLayout.h"
%include "Discretisation/types.h"
%include "SLE/SystemSetup/ForceTerm.h"
%include "SLE/SystemSetup/MGOpGenerator.h"
%include "SLE/SystemSetup/PETScMGSolver.h"
%include "SLE/SystemSetup/SLE_Solver.h"
%include "SLE/SystemSetup/SolutionVector.h"
%include "SLE/SystemSetup/SROpGenerator.h"
%include "SLE/SystemSetup/StiffnessMatrix.h"
%include "SLE/SystemSetup/StiffnessMatrixTerm.h"
%include "SLE/SystemSetup/SystemLinearEquations.h"
%include "SLE/SystemSetup/SolutionVector.h"
%include "SLE/SystemSetup/ForceVector.h"
%include "SLE/SystemSetup/types.h"
/*
%include "SLE/ProvidedSystems/AdvectionDiffusion/AdvectionDiffusionSLE.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/LumpedMassMatrixForceTerm.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Multicorrector.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Residual.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/Timestep.h"
%include "SLE/ProvidedSystems/AdvectionDiffusion/types.h"
*/
%include "SLE/ProvidedSystems/Energy/Energy_SLE_Solver.h"
%include "SLE/ProvidedSystems/Energy/types.h"
%include "SLE/ProvidedSystems/StokesFlow/Stokes_SLE.h"
%include "SLE/ProvidedSystems/StokesFlow/Stokes_SLE_UzawaSolver.h"
%include "SLE/ProvidedSystems/StokesFlow/types.h"
%include "SLE/ProvidedSystems/StokesFlow/UzawaPreconditionerTerm.h"
%include "SLE/types.h"
%include "Assembly/DivergenceMatrixTerm.h"
%include "Assembly/GradientStiffnessMatrixTerm.h"
%include "Assembly/types.h"
%include "Utils/IrregularMeshParticleLayout.h"
%include "Utils/SemiLagrangianIntegrator.h"
%include "Utils/MeshParticleLayout.h"
%include "Utils/types.h"
