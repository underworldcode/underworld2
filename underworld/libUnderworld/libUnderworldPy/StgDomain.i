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
%module (package="underworld.libUnderworld.libUnderworldPy") StgDomain
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") StgDomain
#endif
%include "exception.i"
%import "StGermain.i"

%{
/* Includes the header in the wrapper code */
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
}


%}

#ifndef READ_HDF5
#define READ_HDF5
#endif

#ifndef WRITE_HDF5
#define WRITE_HDF5
#endif


%include "StgDomain_Typemaps.i"
%include "StgDomain/Geometry/src/types.h"
%include "StgDomain/Geometry/src/units.h"
%include "StgDomain/Mesh/src/MeshClass.h"
%include "StgDomain/Mesh/src/MeshGenerator.h"
%include "StgDomain/Mesh/src/CartesianGenerator.h"
%include "StgDomain/Mesh/src/MeshVariable.h"
%include "StgDomain/Mesh/src/Mesh_Algorithms.h"
%include "StgDomain/Mesh/src/Mesh_RegularAlgorithms.h"
%include "StgDomain/Mesh/src/types.h"
%include "StgDomain/Swarm/src/ParticleLayout.h"
%include "StgDomain/Swarm/src/GlobalParticleLayout.h"
%include "StgDomain/Swarm/src/SwarmVariable.h"
%include "StgDomain/Swarm/src/CellLayout.h"
%include "StgDomain/Swarm/src/ElementCellLayout.h"
%include "StgDomain/Swarm/src/PerCellParticleLayout.h"
%include "StgDomain/Swarm/src/ParticleCommHandler.h"
%include "StgDomain/Swarm/src/ParticleMovementHandler.h"
%include "StgDomain/Swarm/src/ParticleShadowSync.h"
%include "StgDomain/Swarm/src/ParticleShadowSync.h"
%include "StgDomain/Swarm/src/SingleCellLayout.h"
%include "StgDomain/Swarm/src/SpaceFillerParticleLayout.h"
%include "StgDomain/Swarm/src/SwarmClass.h"
%include "StgDomain/Swarm/src/types.h"
%include "StgDomain/Swarm/src/GaussParticleLayout.h"
%include "StgDomain/Swarm/src/GaussBorderParticleLayout.h"
%include "StgDomain/Utils/src/FieldVariable.h"
%include "StgDomain/Utils/src/DofLayout.h"
%include "StgDomain/Utils/src/RegularMeshUtils.h"
%include "StgDomain/Utils/src/SobolGenerator.h"
%include "StgDomain/Utils/src/TimeIntegrand.h"
%include "StgDomain/Utils/src/TimeIntegrator.h"
%include "StgDomain/Utils/src/types.h"
%include "StgDomain/Utils/src/Utils.h"
