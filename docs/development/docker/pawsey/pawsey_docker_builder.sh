#!/usr/bin/env bash

set -e

# Example Usage:
#   - Run from underworld2 repository head
#   - mpi and lavavu dockers are automatically generated via github actions
#   - petsc and underworld2 must be created by runn the following script.


MPICH_VERSION=3.4.3 #3.4a2 #3.4.3 #4.0.1
MPI4PY_VERSION=3.1.6 #3.1.6 #3.1.6 #4.0.1
PYTHON_VERSION=3.11
PETSC_VERSION=3.22.2 #3.22.2

ARCH=$(uname -m)
# ARCH=x86_64 ### pawsey
echo "Will build docker image locally for architecture type: $ARCH"
echo "************************************************************"
echo ""

if [ "$ARCH" = "x86_64" ]
then
  PLATFORM="linux/amd64"
else
  PLATFORM="linux/arm64"
fi




echo " .. Now building mpich"
# The mpi and lavavu images should be automatically made via github actions
podman build . \
--rm --squash-all \
-f ./docs/development/docker/pawsey/mpich.dockerfile \
--platform $PLATFORM \
--build-arg PYTHON_VERSION=$PYTHON_VERSION \
--build-arg MPICH_VERSION=$MPICH_VERSION \
--build-arg MPI4PY_VERSION=$MPI4PY_VERSION \
-t underworldcode/mpich:$MPICH_VERSION-$ARCH

echo " .. Finished building mpich"

#podman save -o mpich_$MPICH_VERSION-$ARCH.tar underworldcode/mpich:$MPICH_VERSION-$ARCH

# echo " .. Now building lavavu"
# podman build . \
#   --rm --squash-all \
#   -f ./docs/development/docker/lavavu/Dockerfile \
#   --platform $PLATFORM \
#   --build-arg UBUNTU_VERSION=$UBUNTU_VERSION \
#   --build-arg PYTHON_VERSION=$PYTHON_VERSION \
#   -t underworldcode/lavavu:$ARCH

echo " .. Now building petsc"
podman build . \
 --rm --squash-all \
 -f ./docs/development/docker/pawsey/petsc.dockerfile \
 --platform $PLATFORM \
 --build-arg MPI4PY_VERSION=$MPI4PY_VERSION \
 --build-arg MPI_IMAGE="underworldcode/mpich:$MPICH_VERSION-$ARCH" \
 --build-arg PYTHON_VERSION=$PYTHON_VERSION \
 --build-arg PETSC_VERSION=$PETSC_VERSION \
 -t underworldcode/petsc:$PETSC_VERSION-$ARCH

echo " .. Finished building petsc"
# podman save -o petsc_$PETSC_VERSION-$ARCH.tar underworldcode/petsc:$PETSC_VERSION-$ARCH

echo " .. Now building UW2"
### don't use pull here as we want the petsc image above
podman build . \
  --rm --squash-all \
  -f ./docs/development/docker/pawsey/underworld.dockerfile \
  --platform $PLATFORM \
  --build-arg MPI4PY_VERSION=$MPI4PY_VERSION \
  --build-arg PYTHON_VERSION=$PYTHON_VERSION \
  --build-arg PETSC_IMAGE="underworldcode/petsc:$PETSC_VERSION-$ARCH" \
  --build-arg LAVAVU_IMAGE="underworldcode/lavavu:$ARCH" \
  -t underworldcode/underworld2:2.16.0b-$ARCH

### save to compressed file (tar), which can be converted by singularity on the HPC
podman save -o underworld2_2.16.0b-$ARCH.tar underworldcode/underworld2:2.16.0b-$ARCH

### transfer to HPC using rsync/scp
# rsync -P -a underworld2_2.16.0b-$ARCH.tar username@setonix.pawsey.org.au:/container/directory

### how to build from tar on HPC
# singularity build underworld2_2.16.0b-$ARCH.sif docker-archive://underworld2_2.16.0b-$ARCH.tar
