#!/usr/bin/env bash

### Used for running the underworld3 container.
# This requires 'podman', not docker. Install podman from 
#   https://podman.io/docs/installation
#
# Once podman is installed and available on the command line, run with 
#   ./launch-container.sh
#
# this will launch a container with a jupyter server running on
#   http://localhost:20000
# Also, the machine (host) directory 
#   $HOME/uw_space
# will be mapped into the container under /home/mambauser/host.
# for transfering data to/from the container.
#
##

set -x 

# check to see if $HOME/uw_space exists, if not create it
mkdir -p $HOME/uw_space

# Set 'mambauser' uid/gid from base image, 'mambaorg/micromamba'
uid=57439
gid=57439

subuidSize=$(( $(podman info --format "{{ range .Host.IDMappings.UIDMap }}+{{.Size }}{{end }}" ) - 1 ))
subgidSize=$(( $(podman info --format "{{ range .Host.IDMappings.GIDMap }}+{{.Size }}{{end }}" ) - 1 ))

# This will set an jupyter server on HOST port 20000
# the uidmap / gidmap make the container UID compatible with the host system.
# DO NOT run the following command with root, this is explain further below.
podman run -it --rm \
  -p 20000:8888     \
  --uidmap $uid:0:1 \
  --uidmap 0:1:$uid \
  --uidmap $(($uid+1)):$(($uid+1)):$(($subuidSize-$uid)) \
  --gidmap $gid:0:1 \
  --gidmap 0:1:$gid \
  --gidmap $(($gid+1)):$(($gid+1)):$((subgidSize-$gid)) \
  -v "${HOME}/uw_space":/home/mambauser/host \
  localhost/uw2.17_new 

#  docker.io/underworldcode/underworld2:2.17

#  docker.io/underworldcode/underworld3:development 

## Description of rootless podman and uidmap/gidmap.
# Rootless podman allows a non-root user to run a container without elevated permissions.
# DO NOT use `sudo` to execute the above `podman run` command
# This is unlike rootful podman, or docker, thus it's more secure but has some complexity in the for of permission namespaces
# the host and container permissions are elegantly 'namespaced' to two id mappings.
#  1. host ids -> intermediate (/etc/subuid)
#  2. intermediate -> container ids (uidmaps arguments)
# 
# In the above `podman run` we set uidmaps to allow for straight forward container / host directory bindmounts
# container UID 57439 -> host UID 0 (if podman rootless this is host UID)
# container UID 0-57438 -> host UID 1-57439
# container UID 57440-delta -> host UID 57440-delta
# as the container would have root accesson the host.
# The $uid -> host UID 0 assumes podman "rootless" access, i.e. the host UID executing the container.
# All other UIDs in the container are mapped to something different.
#
# This **MUST** be run without sudo, otherwise the host UID would be root.
