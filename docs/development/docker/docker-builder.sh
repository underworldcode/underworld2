#!/usr/bin/env bash

set -e -x

# Example Usage:
#   - Run from underworld2 repository head.
#   - ensure the podman VM (if using one) has sufficient RAM, eg 8Gb, 
#     otherwise unexpected errors will occur.
#   - (disabled) mpi and lavavu dockers are automatically generated via github actions.
#   - (disabled) petsc and underworld2 must be created by runn the following script.

PYTHON_VERSION=3.11
OMPI_VERSION=4.1.4
MPICH_VERSION=3.4.3
PETSC_VERSION=3.21.5

#MPI_IMPLEMENTATION=ompi
MPI_IMPLEMENTATION=MPICH

BASE_IMAGE="python:$PYTHON_VERSION-slim-bookworm"

#### x86 images can be built on mac arm architecture using rosetta
ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH, with mpi implementation: $MPI_IMPLEMENTATION"
echo "************************************************************\n"


# Get the base image
podman pull $BASE_IMAGE

# Build lavavu docker
podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/lavavu/Dockerfile \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  -t underworldcode/lavavu:$ARCH

if [ "$MPI_IMPLEMENTATION" = "MPICH" ]
then
  # Default is openmpi, but can be switched to mpich
  podman build . \
    --rm --squash-all \
    -f ./docs/development/docker/mpi/Dockerfile.mpich \
    --build-arg BASE_IMAGE=$BASE_IMAGE \
    --build-arg PYTHON_VERSION=$PYTHON_VERSION \
    --build-arg MPICH_VERSION=$MPICH_VERSION \
    -t underworldcode/mpich:$MPICH_VERSION-$ARCH
    
  MPI_IMAGE=underworldcode/mpich:$MPICH_VERSION-$ARCH
  mpi_lowercase="mpich"
else
  podman build . \
    --rm --squash-all \
    -f ./docs/development/docker/mpi/Dockerfile.openmpi \
    --build-arg BASE_IMAGE=$BASE_IMAGE \
    --build-arg PYTHON_VERSION=$PYTHON_VERSION \
    --build-arg OMPI_VERSION=$OMPI_VERSION \
    -t underworldcode/openmpi:$OMPI_VERSION-$ARCH
   
  MPI_IMAGE=underworldcode/openmpi:$OMPI_VERSION-$ARCH
  mpi_lowercase="ompi"
fi

podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/petsc/Dockerfile \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg MPI_IMAGE=$MPI_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_VERSION=$PETSC_VERSION \
  -t underworldcode/petsc:$PETSC_VERSION-$ARCH-$mpi_lowercase

### don't use pull here as we want the petsc image above
podman build . \
  --rm --squash-all \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_IMAGE="underworldcode/petsc:$PETSC_VERSION-$ARCH-$mpi_lowercase" \
  --build-arg LAVAVU_IMAGE="underworldcode/lavavu:$ARCH" \
  -f ./docs/development/docker/underworld2/Dockerfile \
  -t underworldcode/underworld2:2.16.0-$ARCH-$mpi_lowercase


# examples push to docker.io 
# 1st, find image id with podman list
# podman push 2074948abfd1 docker.io/underworldcode/underworld2:2.16.0-arm64-mpich

#### if updates for both arm64 and x86_64 build manifest, ie
# podman manifest create underworldcode/petsc:3.18.1 \
#  -a underworldcode/petsc:3.18.1-x86_64-mpich \
#  -a underworldcode/petsc:3.18.1-arm64-mpich
#
# podman manifest push underworldcode/petsc:3.18.1-mpich
#
# in future this should be automated

## How to use image on HPC with singularity/apptainer

### save the docker image
# podman save -o underworld2-$mpi_lowercase-2.16.0b-$ARCH.tar underworldcode/underworld2:2.16.0b-$ARCH-$mpi_lowercase
### upload to hpc
# scp underworld2-$mpi_lowercase-2.16.0b-$ARCH.tar user@setonix.pawsey.org.au:/path/to/store/container
### extract using singularity/apptainer on HPC
# module load singularity...
# singularity build underworld2-mpich-2.16.0b-x86_64.sif docker-archive://underworld2-mpich-2.16.0b-x86_64.tar


### Some good resources for using containers on HPC:
# (1) checking mpi is using system install not container
# https://pawseysc.github.io/containers-astro-python-workshop/3.hpc/index.html
