#!/usr/bin/bash

set -e


# Example Usage:
#   - Run from underworld2 repository head
#   - mpi and lavavu dockers are automatically generated via github actions
#   - petsc and underworld2 must be created by runn the following script.


ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH"
echo "************************************************************\n"

# Get the ubuntu image
docker pull ubuntu:22.04

## The mpi and lavavu images should be automatically made via github actions
docker build . -f ./docs/development/docker/mpi/Dockerfile.openmpi -t underworldcode/openmpi:4.1.4-$ARCH
docker build . -f ./docs/development/docker/lavavu/Dockerfile -t underworldcode/lavavu:$ARCH

docker build . \
  -f ./docs/development/docker/petsc/Dockerfile \
  --build-arg MPI_IMAGE="underworldcode/openmpi:4.1.4-$ARCH" \
  -t underworldcode/petsc:3.19.4-$ARCH

## don't use pull here as we want the petsc image above
docker build . \
  --build-arg PETSC_IMAGE="underworldcode/petsc:3.19.4-$ARCH" \
  -f ./docs/development/docker/underworld2/Dockerfile \
  -t underworldcode/underworld2:2.15.1b-$ARCH


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
