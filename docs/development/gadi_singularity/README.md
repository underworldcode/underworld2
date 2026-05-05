# Building Underworld2 for Gadi (NCI)

This directory contains two Containerfiles to build the Underworld2 (UW2) Singularity image for Gadi (nci.org.au).

## Build Order

### 1. Build PETSc layer first
```bash
podman build . \
    --format docker \
    -f ./docs/development/gadi_singularity/petsc.rhel \
    -t petsc-gadi:latest
```

### 2. Build Underworld2
```bash
podman build . \
    --format docker \
    --build-arg PETSC_IMAGE=petsc-gadi:latest
    -f ./docs/development/gadi_singularity/underworld.rhel \
    -t underworld2-gadi:latest
```

## What Each File Does

- **petsc.rhel** - Builds PETSc (main dependency for UW2)
- **underworld.rhel** - Builds Underworld2 and Badlands

Both use Rocky Linux (RHEL 8.10) to match Gadi's OS for compatibility.

## Running on Gadi

See example: [Running Underworld2 on Gadi](https://gist.github.com/julesghub/566f6593edf53f6bf9a919859ced58ae)
