#!/usr/bin/bash

set -e


# Example Usage:
#   - Run from underworld2 repository head


ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH"
echo "************************************************************\n"

## The mpi and lavavu images should be automatically made via github actions
#docker build . --pull -f ./docs/development/docker/mpi/Dockerfile.openmpi -t underworldcode/openmpi:4.1.4-$ARCH
#docker build . --pull -f ./docs/development/docker/lavavu/Dockerfile -t underworldcode/lavavu:$ARCH

docker build . --pull \
  -f ./docs/velopment/docker/petsc/Dockerfile \
  --build-arg MPI_IMAGE "underworldcode/openmpi:4.1.4" \
  -t underworldcode/petsc:3.18.1-$ARCH

docker build . --pull \ 
  --build-arg PETSC_IMAGE "underworldcode/petsc:3.18.1" \
  -f ./docs/development/docker/underworld2/Dockerfile \
  -t underworldcode/underworld2:2.14.0b-$ARCH
