#!/usr/bin/bash

set -e


# Example Usage:
#   - Run from underworld2 repository head


ARCH=$(uname -m)
echo "Will build docker image locally for architecture type: $ARCH"
echo "************************************************************\n"

docker build . --pull -f ./docs/development/docker/mpi/Dockerfile.openmpi -t julesg/openmpi:4.1.4-$ARCH

docker build . --pull -f ./docs/development/docker/petsc/Dockerfile -t julesg/petsc:3.18.1-$ARCH

docker build . --pull -f ./docs/development/docker/lavavu/Dockerfile -t julesg/lavavu:$ARCH

docker build .  -f ./docs/development/docker/underworld2/Dockerfile -t julesg/underworld2:2.14.0b-$ARCH
