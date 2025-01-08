#!/usr/bin/env bash

set -e

# Example Usage:
#   - Run from underworld2 repository head
#   - mpi and lavavu dockers are automatically generated via github actions
#   - petsc and underworld2 must be created by runn the following script.

UBUNTU_VERSION=24.04
PYTHON_VERSION=3.12
OMPI_VERSION=4.1.4
PETSC_VERSION=3.22.2

ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH"
echo "************************************************************\n"

# Get the ubuntu image
podman pull ubuntu:$UBUNTU_VERSION

## The mpi and lavavu images should be automatically made via github actions
podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/mpi/Dockerfile.openmpi \
  --build-arg UBUNTU_VERSION=$UBUNTU_VERSION \
  --build-arg OMPI_VERSION=$OMPI_VERSION \
  -t underworldcode/openmpi:$OMPI_VERSION-$ARCH

podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/lavavu/Dockerfile \
  --build-arg UBUNTU_VERSION=$UBUNTU_VERSION \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  -t underworldcode/lavavu:$ARCH

podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/petsc/Dockerfile \
  --build-arg MPI_IMAGE="underworldcode/openmpi:$OMPI_VERSION-$ARCH" \
  --build-arg UBUNTU_VERSION=$UBUNTU_VERSION \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_VERSION=$PETSC_VERSION \
  -t underworldcode/petsc:$PETSC_VERSION-$ARCH

### don't use pull here as we want the petsc image above
podman build . \
  --rm --squash-all \
  --build-arg UBUNTU_VERSION=$UBUNTU_VERSION \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_IMAGE="underworldcode/petsc:$PETSC_VERSION-$ARCH" \
  --build-arg LAVAVU_IMAGE="underworldcode/lavavu:$ARCH" \
  -f ./docs/development/docker/underworld2/Dockerfile \
  -t underworldcode/underworld2:2.16.0b-$ARCH

#docker push underworldcode/petsc:3.19.4-$ARCH 
#docker push underworldcode/underworld2:2.15.0b-$ARCH

#### if updates for both arm64 and x86_64 build manifest, ie
# docker manifest create underworldcode/petsc:3.18.1 \
#  -a underworldcode/petsc:3.18.1-x86_64 \
#  -a underworldcode/petsc:3.18.1-arm64
#
# docker manifest push underworldcode/petsc:3.18.1
#
# in future this should be automated
