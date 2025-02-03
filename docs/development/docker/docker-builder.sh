#!/usr/bin/env bash

set -e

# Example Usage:
#   - Run from underworld2 repository head
#   - mpi and lavavu dockers are automatically generated via github actions
#   - petsc and underworld2 must be created by runn the following script.

PYTHON_VERSION=3.11
OMPI_VERSION=4.1.4
MPICH_VERSION=3.4.3
PETSC_VERSION=3.22.2

MPI_IMPLEMENTATION=opmi

BASE_IMAGE="python:$PYTHON_VERSION-slim-bookworm"

#### x86 images can be built on mac arm architecture using rosetta
ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH, with mpi implementation: $MPI_IMPLEMENTATION"
echo "************************************************************\n"


# Get the base image
podman pull $BASE_IMAGE

## The mpi and lavavu images should be automatically made via github actions

if [ "$MPI_IMPLEMENTATION" = "MPICH" ]
then
  ## Default is openmpi, but can be switched to mpich
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
  -f ./docs/development/docker/lavavu/Dockerfile \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  -t underworldcode/lavavu:$ARCH

podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/petsc/Dockerfile \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg MPI_IMAGE=$MPI_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_VERSION=$PETSC_VERSION \
  -t underworldcode/petsc-$mpi_lowercase:$PETSC_VERSION-$ARCH

### don't use pull here as we want the petsc image above
podman build . \
  --rm --squash-all \
  --build-arg BASE_IMAGE=$BASE_IMAGE \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_IMAGE="underworldcode/petsc-$mpi_lowercase:$PETSC_VERSION-$ARCH" \
  --build-arg LAVAVU_IMAGE="underworldcode/lavavu:$ARCH" \
  -f ./docs/development/docker/underworld2/Dockerfile \
  -t underworldcode/underworld2-$mpi_lowercase:2.16.0b-$ARCH


#docker push underworldcode/petsc-$mpi_lowercase:$PETSC_VERSION-$ARCH
#docker push underworldcode/underworld2-$mpi_lowercase:2.16.0b-$ARCH

#### if updates for both arm64 and x86_64 build manifest, ie
# docker manifest create underworldcode/petsc:3.18.1 \
#  -a underworldcode/petsc:3.18.1-x86_64 \
#  -a underworldcode/petsc:3.18.1-arm64
#
# docker manifest push underworldcode/petsc:3.18.1
#
# in future this should be automated



## How to use image on HPC with singularity/apptainer

### save the docker image
# podman save -o underworld2-$mpi_lowercase-2.16.0b-$ARCH.tar underworldcode/underworld2-$mpi_lowercase:2.16.0b-$ARCH
### upload to hpc
# scp underworld2-$mpi_lowercase-2.16.0b-$ARCH.tar user@setonix.pawsey.org.au:/path/to/store/container
### extract using singularity/apptainer on HPC
# module load singularity...
# singularity build underworld2-mpich-2.16.0b-x86_64.sif docker-archive://underworld2-mpich-2.16.0b-x86_64.tar


### Some good resources for using containers on HPC:
# (1) checking mpi is using system install not container
# https://pawseysc.github.io/containers-astro-python-workshop/3.hpc/index.html