/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module StgDomain

%include "exception.i"
%import "StGermain.i"

%{
/* Includes the header in the wrapper code */
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
}


%}

#ifndef READ_HDF5
#define READ_HDF5
#endif

#ifndef WRITE_HDF5
#define WRITE_HDF5
#endif


%include "StgDomain_Typemaps.i"
%include "Geometry/types.h"
%include "Geometry/units.h"
%include "Mesh/MeshClass.h"
%include "Mesh/MeshGenerator.h"
%include "Mesh/CartesianGenerator.h"
%include "Mesh/MeshVariable.h"
%include "Mesh/types.h"
%include "Swarm/ParticleLayout.h"
%include "Swarm/GlobalParticleLayout.h"
%include "Swarm/SwarmVariable.h"
%include "Swarm/CellLayout.h"
%include "Swarm/ElementCellLayout.h"
%include "Swarm/PerCellParticleLayout.h"
%include "Swarm/ParticleCommHandler.h"
%include "Swarm/ParticleMovementHandler.h"
%include "Swarm/ParticleShadowSync.h"
%include "Swarm/SingleCellLayout.h"
%include "Swarm/SpaceFillerParticleLayout.h"
%include "Swarm/SwarmClass.h"
%include "Swarm/types.h"
%include "Swarm/GaussParticleLayout.h"
%include "Swarm/GaussBorderParticleLayout.h"
%include "Utils/FieldVariable.h"
%include "Utils/DofLayout.h"
%include "Utils/RegularMeshUtils.h"
%include "Utils/SobolGenerator.h"
%include "Utils/TimeIntegrand.h"
%include "Utils/TimeIntegrator.h"
%include "Utils/types.h"
%include "Utils/Utils.h"
